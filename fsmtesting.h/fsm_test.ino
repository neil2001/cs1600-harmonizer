//#define TESTING 
/*
 * A struct to keep all three state inputs in one place
 */
typedef struct {
  bool normalBtn;
  bool thirdBtn; 
  bool fifthBtn;
  bool darthBtn; 
} state_inputs;
/*
 * A struct to keep all state variables in one place
 */
// typedef struct {
//   int BUFSIZE; 
//   int outIdx; 
//   int intIdx;  
// } state_vars;

bool testTransition(state startState,
                     state endState,
                     state_inputs testStateInputs, 
                     bool verbos);
/*        
 * Helper function for printing states
 */
char* s2str(state s) {
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
 * Given a start state, inputs, and starting values for state variables, tests that
 * updateFSM returns the correct end state and updates the state variables correctly
 * returns true if this is the case (test passed) and false otherwise (test failed)
 * 
 * Need to use "verbos" instead of "verbose" because verbose is apparently a keyword
 */
bool testTransition(state startState,
                     state endState,
                     state_inputs testStateInputs, 
                     bool verbos) {
  state resultState = updateFSM(startState, bool normalBtn, bool thirdBtn, bool fifthBtn, bool darthBtn);
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
    sprintf(sToPrint, "Inputs: mils %ld | numButtons %d | lastButton %s", testStateInputs.mils, testStateInputs.numButtons, o2str(testStateInputs.lastButton));
    Serial.println(sToPrint);
    sprintf(sToPrint, "          %2s | %2s | %5s | %3s | %3s | %5s | %9s | %11s | %9s", "x", "y", "o", "lxb", "uxb", "level", "timeStep", "savedClock", "countdown");
    Serial.println(sToPrint);
    sprintf(sToPrint, "expected: %2d | %2d | %5s | %3d | %3d | %5d | %9d | %11d | %9d", endStateVars.x, endStateVars.y, o2str(endStateVars.o), endStateVars.lxb, endStateVars.uxb, endStateVars.level, endStateVars.timeStep, endStateVars.savedClock, endStateVars.countdown);
    Serial.println(sToPrint);
    sprintf(sToPrint, "actual:   %2d | %2d | %5s | %3d | %3d | %5d | %9d | %11d | %9d", x, y, o2str(o), lxb, uxb, level, timeStep, savedClock, countdown);
    Serial.println(sToPrint);
    return false;
  }
}

const state testStatesIn[25] = {sNORMAL, sNORMAL, sNORMAL, sNORMAL, sTHIRD, sTHIRD, sTHIRD, sTHIRD, sFIFTH, sFIFTH, sFIFTH, sFIFTH, sDARTH, sDARTH, sDARTH, sDARTH};

const state testStatesOut[25] = {sNORMAL, sTHIRD, sFIFTH, sDARTH, sTHIRD, sNORMAL, sFIFTH, sDARTH, sFIFTH, sNORMAL, sTHIRD, sDARTH, sDARTH, sNORMAL, sTHIRD, sFIFTH};
                                                                                                                                                                                                                                                                                                             
const state_inputs testInputs[25] = {{!normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, !thirdBtn, fifthBtn, !darthBtn}, {!normalBtn, !thirdBtn, !fifthBtn, darthBtn}, {!normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, !thirdBtn, fifthBtn, !darthBtn}, {!normalBtn, !thirdBtn, !fifthBtn, darthBtn}, {!normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, !thirdBtn, !fifthBtn, darthBtn}, {!normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {normalBtn, !thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, thirdBtn, !fifthBtn, !darthBtn}, {!normalBtn, !thirdBtn, fifthBtn, !darthBtn}};
 
const int numTests = 25;
/*
 * Runs through all the test cases defined above
 */
bool testAllTests() {
  for (int i = 0; i < numTests; i++) {
    Serial.print("Running test ");
    Serial.println(i);
    if (!testTransition(curState, resultState, {bool normalBtn, bool thirdBtn, bool fifthBtn, bool darthBtn}, true)) {
      return false;
    }
    Serial.println();
  }
  Serial.println("All tests passed!");
  return true;
}
