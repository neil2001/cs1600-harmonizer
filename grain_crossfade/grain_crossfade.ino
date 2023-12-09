// Tests whether granular synthesis actually works using a timer counter at 
// different frequencies to play a given triangle wave
// The speaker will read from buffer at SPEAKER_FREQUENCY
// NOTE: Important to see how the microseconds TC5 is delayed by having analogWrite!!! Should be 50 microseconds
#define DEBUG true

#define BUFSIZE 2048
// #define BUFSIZE_THIRD 1024
const float SCALE_FACTOR = 0.9342492889f;// 440.f / 470.f;
const float THIRD = 5.f / 4.f;
const float FIFTH = 3.f / 2.f;
const float NORMAL = 1.f;
float PITCH_FACTOR = NORMAL;
const long SAMPLE_FREQUENCY = 27778;// 13889 rounded from 1/7.2e-5 = 13888.88
long SPEAKER_FREQUENCY = SAMPLE_FREQUENCY * SCALE_FACTOR * PITCH_FACTOR;

int MIC_PIN = A1;
int SPEAKER_PIN = A0;

const int BUF_LEN = BUFSIZE;
// int buffer[BUFSIZE] = {0};

int buffer_xf[BUFSIZE] = {0};

// cross fade makes use of an extra buffer:
// [......num samples ......|>>>>XFADE>>>] buffer of size BUFSIZE

#define XFADE 128                      // Number of samples for cross-fade
volatile int xf = 0;
int nSamples = 1024;
volatile int in = 0;
volatile int out = 0;

volatile int grain_duration = 10; // in millis
// volatile int inIdx, outIdx;

extern const int BTN_NORMAL;
extern const int BTN_THIRD;
extern const int BTN_FIFTH;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(BTN_NORMAL, INPUT);
  pinMode(BTN_THIRD, INPUT);
  pinMode(BTN_FIFTH, INPUT);

  attachInterrupt(digitalPinToInterrupt(BTN_NORMAL), normalISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_THIRD), thirdISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_FIFTH), fifthISR, RISING);

  pinMode(SPEAKER_PIN, OUTPUT);
  analogWriteResolution(12);

  tc5Configure(SPEAKER_FREQUENCY);
  Serial.println("Initialized TC5, starting TC5");
  tcStartCounter();

  setup_ADC();
  Serial.println("Initialized ADC");

  setup_watchdog();

  // outIdx = 0;
  // inIdx = outIdx + grain_duration;
  out = 0;
  in = out + grain_duration;
}

long lastMillis = 0;
long curMillis = 0;

void loop() {

  // Buffer is being written to 27777.7 times per second
  // meaning it completely fills buffer 277.7 (27,777.77 * 10ms / 1000ms)
  // per second. And therefore fills the buffer every 0.0036ms
  // This is extremely small and barely audible
  // Add read-in value to buffer and increment inIdx with wrap
  // NOTE: this function will delay the loop to x Hz/kHz

  buffer_xf[nSamples + xf] = buffer_xf[in];
  if(++xf >= XFADE) xf = 0;

  buffer_xf[in] = readADCSync();
  if (++in >= nSamples) {
    in = 0;

    // petting watch dog
    WDT->CLEAR.reg = 0xA5;
    while(WDT->STATUS.bit.SYNCBUSY);
  }
}
