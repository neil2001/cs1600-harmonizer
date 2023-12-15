#include "grain_watchdog.h" 



bool testTransition(state startState,
                     state endState,
                     state_inputs testInputs, 
                     bool verbos);
/*        
 * Helper function for printing states
 */
char *s2str(state s) {
  switch(s) {
    // case sSPEAKER_EMIT:
    // return "(0) SPEAKER_EMIT";
    case sNORMAL:
      return "(1) NORMAL";
    case sTHIRD:
      return "(2) THIRD";
    case sFIFTH:
      return "(3) FIFTH";
    case sDARTH:
      return "(4) DARTH";
    default:
      return "???";
  }
}
/*
 * Given a start state and inputs, test that updateFSM returns the correct end state; 
 if this is the case (test passed) and false otherwise (test failed)
 * 
 * Need to use "verbos" instead of "verbose" because verbose is apparently a keyword
 */
bool testTransition(state startState,
                     state endState,
                     state_inputs testInputs, 
                     bool verbos) {
  state resultState = updateFSM(startState, testInputs.normalBtn, testInputs.thirdBtn, testInputs.fifthBtn, testInputs.darthBtn);
  bool passedTest = (endState == resultState);
  if (! verbos) {
    return passedTest;
  } else if (passedTest) {
    char sToPrint[200];
    sprintf(sToPrint, "Test from %s to %s PASSED", s2str(startState), s2str(endState));
    Serial.println(sToPrint);
    return true;
  } else {
    char sToPrint[200];
    Serial.println(s2str(startState));
    sprintf(sToPrint, "Test from %s to %s FAILED", s2str(startState), s2str(endState));
    Serial.println(sToPrint);
    sprintf(sToPrint, "End state expected: %s | actual: %s", s2str(endState), s2str(resultState));
    Serial.println(sToPrint);
    return false;
  }
}

const state testStatesIn[16] = {sNORMAL, sNORMAL, sNORMAL, sNORMAL, sTHIRD, sTHIRD, sTHIRD, sTHIRD, sFIFTH, sFIFTH, sFIFTH, sFIFTH, sDARTH, sDARTH, sDARTH, sDARTH};

const state testStatesOut[16] = {sNORMAL, sTHIRD, sFIFTH, sDARTH, sTHIRD, sNORMAL, sFIFTH, sDARTH, sFIFTH, sNORMAL, sTHIRD, sDARTH, sDARTH, sNORMAL, sTHIRD, sFIFTH};
                                                                                                                                                                                                                                                                                                             
state_inputs testInputs[16] = {{false, false, false, false}, {false, true, false, false}, {false, false, true, false}, {false, false, false, true}, {false, false, false, false}, {true, false, false, false}, {false, false, true, false}, {false, false, false,true}, {false, false, false, false}, {true, false, false, false}, {false, true, false, false}, {false, false, false, true}, {false, false, false, false}, {true, false, false, false}, {false, true, false, false}, {false, false, true, false}};
 
const int numTests = 16;
/*
 * Runs through all the test cases defined above
 */
bool testAllTests() {
  for (int i = 0; i < numTests; i++) {
    Serial.print("Running test ");
    Serial.println(i);
    if (!testTransition(testStatesIn[i], testStatesOut[i], testInputs[i], true)) {
      return false;
    }
    Serial.println();
  }
  Serial.println("All tests passed!");
  return true;
}
