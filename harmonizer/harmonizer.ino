// Tests whether granular synthesis actually works using a timer counter at 
// different frequencies to play a given triangle wave
// The speaker will read from buffer at SPEAKER_FREQUENCY
// NOTE: Important to see how the microseconds TC5 is delayed by having analogWrite!!! Should be 50 microseconds
//Serial.begin(baudRate)
#include "harmonizer.h"
#include "buttons.h"
#include "tests.h"

#define DEBUG true
#define FSM_TESTING false
#define BUFFER_TESTING false

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
}

/**
 * @brief Recalculates the frequency with which to write buffer values out to speaker 
 *        and then reconfigures and starts TC5 to that frequency
 * @param pitchFactor, a float representing the multiplier with which to pitch the
 *                     input frequency 
 */
void updatePitch(float pitchFactor){
  long pitchedSpeakerFrequency = SCALED_SAMPLE_FREQUENCY * pitchFactor;
  tc5Configure(pitchedSpeakerFrequency);
  tc5StartCounter();
}

/**
 * @brief Updates the state of the harmonizer to sNORMAL, sTHIRD, sFIFTH, or sDARTH
 *        based on inputs below... 
 * 
 * @param curState, the current state of the FSM 
 * @param normalBtn, a bool representing if the normal button was pressed 
 * @param thirdBtn, a bool representing if the third button was pressed
 * @param fifthBtn, a bool representing if the fifth button was pressed
 * @param darthBtn, a bool representing if the darth button was pressed,  
 * @return state, the state to transition to next 
 */
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

/**
 * @brief Every 36 microseconds, inserts values read-in from the ADC
 *        into the global circular buffer array and pets the watchdog.
 *        Also resets the buttons every loop 
 * 
 */
void loop() {
  static state CURRENT_STATE = sNORMAL;

  // Fills the buffer extremely fast (ideally every 0.0036ms)
  // which is barely audible
  // NOTE: this function will delay the loop to x Hz/kHz
  insertToBuffer(readADCSync());

  // Pet the watchdog everytime the buffer finishes filling
  if (inIdx == 0) pet_watchdog();

  // Update the harmonizer state based on the button inputs set by the ISRs
  CURRENT_STATE = updateFSM(CURRENT_STATE, normalBtnOn, thirdBtnOn, fifthBtnOn, darthBtnOn);

  // Reset the buttons every loop
  resetButtons();

  // For testing purposes we count how many times we read in a given
  // elapsed time
  readcount++;
  readcount %= COUNTMAX;
}
