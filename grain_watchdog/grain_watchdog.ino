// Tests whether granular synthesis actually works using a timer counter at 
// different frequencies to play a given triangle wave
// The speaker will read from buffer at SPEAKER_FREQUENCY
// NOTE: Important to see how the microseconds TC5 is delayed by having analogWrite!!! Should be 50 microseconds
//Serial.begin(baudRate)
#include "grain_watchdog.h"

#define DEBUG true
#define BUFSIZE 1024
#define FSM_TESTING false
#define BUFFER_TESTING true
#define COUNTMAX 10000

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

int buffer[BUFSIZE] = {0};
volatile int writecount = 0;
volatile int readcount = 0;

volatile int inIdx, outIdx;

extern const int BTN_NORMAL, BTN_THIRD, BTN_FIFTH, BTN_DARTH;
extern bool normalBtnOn, thirdBtnOn, fifthBtnOn, darthBtnOn;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (FSM_TESTING){
    testAllTests();
    exit(0);
  }

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
  tc5StartCounter();

  setup_ADC();
  Serial.println("Initialized ADC");

  if (BUFFER_TESTING) {
    runBufferTests();
    exit(0);
  }

  setup_watchdog();

  outIdx = 0;
  inIdx = 10;
}

long lastMillis = 0;
long curMillis = 0;

void updatePitch(float pitchFactor){
  long pitchedSpeakerFrequency = SCALED_SAMPLE_FREQUENCY * pitchFactor;
  tc5Configure(pitchedSpeakerFrequency);
  tc5StartCounter();
}

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

/**
 * @brief Inserts one 12-bit amplitude into the global buffer array at inIdx
 *        Ensures inIdx increments such that it wraps around to start of buffer
 * 
 * @param amplitude, the 12-bit integer amplitude to insert 
 */
void insertToBuffer(int amplitude){
  buffer[inIdx++] = amplitude;  
  inIdx %= BUFSIZE; 
}

/**
 * @brief Mocked version of insertToBuffer for testing 
 * 
 * @param amplitude 
 * @param index 
 * @param amplitudes 
 * @param bufsize 
 */
void insertToBuffer(int amplitude, int &index, int *amplitudes, int bufsize){
  amplitudes[index++] = amplitude;  
  index %= bufsize; 
}

/**
 * @brief Reads one value from the global buffer array at index, outIdx, and returns it
 *        Ensures that outIdx is incremented such that it if it's at the end, it will
 *        wrap around to beginning
 * 
 * @return int, the 12-bit amplitude stored in buffer at outIdx
 */
int readFromBuffer(){
  int amplitude = buffer[outIdx];
  outIdx = (outIdx + 1) % BUFSIZE;
  return amplitude;
}

/**
 * @brief Mock version of readFromBuffer where you may specify the
 *        the index, amplitudes, and bufsize 
 * 
 * @param index 
 * @param amplitudes 
 * @param bufsize 
 * @return int, the amplitude at amplitudes[index] and the index
 *         should be incremented and wrap if equal to bufsize 
 */
int readFromBuffer(int &index, int *amplitudes, int bufsize){
  int amplitude = amplitudes[index];
  index = (index + 1) % bufsize;
  return amplitude;
}

void loop() {
  static state CURRENT_STATE = sNORMAL;

  // Buffer is being written to 27777.7 times per second
  // meaning it completely fills buffer 277.7 (27,777.77 * 10ms / 1000ms)
  // per second. And therefore fills the buffer every 0.0036ms
  // This is extremely small and barely audible
  // Add read-in value to buffer and increment inIdx with wrap
  // NOTE: this function will delay the loop to x Hz/kHz
  insertToBuffer(readADCSync());
  if (inIdx == 0) {
    // Serial.println("petting watchdog");
    WDT->CLEAR.reg = 0xA5;
    while(WDT->STATUS.bit.SYNCBUSY);
  }

  CURRENT_STATE = updateFSM(CURRENT_STATE, normalBtnOn, thirdBtnOn, fifthBtnOn, darthBtnOn);

  // Reset the buttons every loop
  resetButtons();
  readcount++;
  readcount %= COUNTMAX;
}
