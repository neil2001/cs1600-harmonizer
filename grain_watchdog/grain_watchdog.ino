// Tests whether granular synthesis actually works using a timer counter at 
// different frequencies to play a given triangle wave
// The speaker will read from buffer at SPEAKER_FREQUENCY
// NOTE: Important to see how the microseconds TC5 is delayed by having analogWrite!!! Should be 50 microseconds

#define DEBUG true
#define BUFSIZE 1024

const float THIRD = 5.f / 4.f;
const float FIFTH = 3.f / 2.f;
const float NORMAL = 1.f;
const float DARTH = 3.f / 4.f;
const float SCALE_FACTOR = 0.9936721414f;//0.8343110211f; //0.9342492889f;
const float SCALE_OFFSET = -0.4689031131f;//1.16839408f;
const long SAMPLE_FREQUENCY = 23810;// 27778 rounded from 1/3.6e-5 = 27777.78
const long SCALED_SAMPLE_FREQUENCY = (SAMPLE_FREQUENCY * SCALE_FACTOR) - SCALE_OFFSET;

int MIC_PIN = A1;
int SPEAKER_PIN = A0;

const int BUF_LEN = BUFSIZE;
int buffer[BUFSIZE] = {0};

volatile int grain_duration = 10;
volatile int inIdx, outIdx;

extern const int BTN_NORMAL, BTN_THIRD, BTN_FIFTH, BTN_DARTH;
extern bool normalBtnOn, thirdBtnOn, fifthBtnOn, darthBtnOn;

float scaleFrequency(float frequency){
  return (frequency * SCALE_FACTOR) - SCALE_OFFSET;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(BTN_NORMAL, INPUT);
  pinMode(BTN_THIRD, INPUT);
  pinMode(BTN_FIFTH, INPUT);
  pinMode(BTN_DARTH, INPUT);

  attachInterrupt(digitalPinToInterrupt(BTN_NORMAL), normalISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_THIRD), thirdISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_FIFTH), fifthISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_DARTH), darthISR, RISING);

  pinMode(SPEAKER_PIN, OUTPUT);
  analogWriteResolution(12);

  // Initial speaker frequency set to normal 1:1
  tc5Configure(SCALED_SAMPLE_FREQUENCY);
  Serial.println("Initialized TC5, starting TC5");
  tcStartCounter();

  setup_ADC();
  Serial.println("Initialized ADC");

  setup_watchdog();

  outIdx = 0;
  inIdx = outIdx + grain_duration;
}

long lastMillis = 0;
long curMillis = 0;

void updatePitch(float pitchFactor){
  long pitchedSpeakerFrequency = SCALED_SAMPLE_FREQUENCY * pitchFactor;
  tc5Configure(pitchedSpeakerFrequency);
  tcStartCounter();
}

typedef enum {
  sNORMAL = 1,
  sTHIRD = 2,
  sFIFTH = 3,
  sDARTH = 4,
} state;

state updateFSM(state curState, bool normalBtn=false, bool thirdBtn=false, bool fifthBtn=false, bool darthBtn=false){
  state nextState;
  switch(curState){
    // State 1
    case sNORMAL:
      // Trans 1-2
      if (!normalBtn and thirdBtn and !fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("1-2");
        updatePitch(THIRD);
        nextState = sTHIRD;
      }
      // Trans 1-3
      else if (!normalBtn and !thirdBtn and fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("1-3");
        updatePitch(FIFTH);
        nextState = sFIFTH;
      }
      // Trans 1-4
      else if (!normalBtn and !thirdBtn and !fifthBtn and darthBtn){
        if (DEBUG) Serial.println("1-4");
        updatePitch(DARTH);
        nextState = sDARTH;
      }
      // Trans 1-1
      else nextState = sNORMAL;
      break;
    // State 2
    case sTHIRD:
      // Trans 2-1
      if (normalBtn and !thirdBtn and !fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("2-1");
        updatePitch(NORMAL);
        nextState = sNORMAL;
      }
      // Trans 2-3
      else if (!normalBtn and !thirdBtn and fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("2-3");
        updatePitch(FIFTH);
        nextState = sFIFTH;
      }
      // Trans 2-4
      else if (!normalBtn and !thirdBtn and !fifthBtn and darthBtn){
        if (DEBUG) Serial.println("2-4");
        updatePitch(DARTH);
        nextState = sDARTH;
      }
      // Trans 2-2
      else nextState = sTHIRD;
      break;
    // State 3
    case sFIFTH:
      // Trans 3-1
      if (normalBtn and !thirdBtn and !fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("3-1");
        updatePitch(NORMAL);
        nextState = sNORMAL;
      }
      // Trans 3-2
      else if (!normalBtn and thirdBtn and !fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("3-2");
        updatePitch(THIRD);
        nextState = sTHIRD;
      }
      // Trans 3-4
      else if (!normalBtn and !thirdBtn and !fifthBtn and darthBtn){
        if (DEBUG) Serial.println("3-4");
        updatePitch(DARTH);
        nextState = sDARTH;
      }
      // Trans 3-3
      else nextState = sFIFTH;
      break;

    // State 4
    case sDARTH:
      // Trans 4-1
      if (normalBtn and !thirdBtn and !fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("4-1");
        updatePitch(NORMAL);
        nextState = sNORMAL;
      }
      // Trans 4-2
      else if (!normalBtn and thirdBtn and !fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("4-2");
        updatePitch(THIRD);
        nextState = sTHIRD;
      }
      // Trans 4-3
      else if (!normalBtn and !thirdBtn and fifthBtn and !darthBtn){
        if (DEBUG) Serial.println("4-3");
        updatePitch(FIFTH);
        nextState = sFIFTH;
      }
      else nextState = sDARTH;
      break;
  }
  return nextState;
}

void loop() {
  static state CURRENT_STATE = sNORMAL;

  // Buffer is being written to 27777.7 times per second
  // meaning it completely fills buffer 277.7 (27,777.77 * 10ms / 1000ms)
  // per second. And therefore fills the buffer every 0.0036ms
  // This is extremely small and barely audible
  // Add read-in value to buffer and increment inIdx with wrap
  // NOTE: this function will delay the loop to x Hz/kHz
  buffer[inIdx++] = readADCSync();  
  inIdx %= BUFSIZE;
  if (inIdx == 0) {
    // Serial.println("petting watchdog");
    WDT->CLEAR.reg = 0xA5;
    while(WDT->STATUS.bit.SYNCBUSY);
  }

  CURRENT_STATE = updateFSM(CURRENT_STATE, normalBtnOn, thirdBtnOn, fifthBtnOn, darthBtnOn);

  // Reset the buttons every loop
  resetButtons();

}
