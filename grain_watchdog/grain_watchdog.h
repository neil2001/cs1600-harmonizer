#ifndef grain_watchdog_h 
#define grain_watchdog_h 

typedef enum {
  sNORMAL = 1,
  sTHIRD = 2,
  sFIFTH = 3,
  sDARTH = 4,
} state;

/*
 * A struct to keep all three state inputs in one place
 */
typedef struct {
  bool normalBtn;
  bool thirdBtn; 
  bool fifthBtn;
  bool darthBtn; 
} state_inputs;
#endif 