#ifndef TESTS_H
#define TESTS_H

#define COUNTMAX 10000

/*
 * A struct to keep all three state inputs in one place
 */
typedef struct {
  bool normalBtn;
  bool thirdBtn; 
  bool fifthBtn;
  bool darthBtn; 
} state_inputs;

/**
 * Tests passed into runTest() must take form of function pointer
 * that returns boolean value and takes no parameters 
 */
typedef bool (*TestFunctionPtr)();

/**
 * Testing variables 
 */
volatile int writecount = 0;
volatile int readcount = 0;

#endif
