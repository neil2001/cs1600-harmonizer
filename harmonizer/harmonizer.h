#ifndef HARMONIZER_H 
#define HARMONIZER_H 

/**
 * States for state machine
 */
typedef enum {
  sNORMAL = 1,
  sTHIRD = 2,
  sFIFTH = 3,
  sDARTH = 4,
} state;

/**
 * These are the constants for grain_watchdog
 */
const int BUFSIZE = 1024; 
const float THIRD = 5.f / 4.f;
const float FIFTH = 3.f / 2.f;
const float NORMAL = 1.f;
const float DARTH = 3.f / 4.f;
const float SCALE_FACTOR = 0.9936721414f;//0.8343110211f; //0.9342492889f;
const float SCALE_OFFSET = -0.4689031131f;//1.16839408f;
const long SAMPLE_FREQUENCY = 23810;// 27778 rounded from 1/3.6e-5 = 27777.78
const long SCALED_SAMPLE_FREQUENCY = (SAMPLE_FREQUENCY * SCALE_FACTOR) - SCALE_OFFSET;

/**
 * These are the pins the microphone and speaker are connected to
 */
int MIC_PIN = A1;
int SPEAKER_PIN = A0;

/**
 * Global variables for the circular buffer and the input and output index
 * Initially buffer is an array of zeros, inIdx is 0, and outIdx is 10
 */
int buffer[BUFSIZE] = {0};
int inIdx = 0;
int outIdx = 10;

/**
 * Function declarations used in testing files and grain_watchdog
 */
void insertToBuffer(int amplitude);
void insertToBuffer(int amplitude, int &index, int *amplitudes, int bufsize);
int readFromBuffer();
int readFromBuffer(int &index, int *amplitudes, int bufsize);

#endif 