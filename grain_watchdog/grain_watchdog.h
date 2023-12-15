#ifndef grain_watchdog_h 
#define grain_watchdog_h 

typedef enum {
  sNORMAL = 1,
  sTHIRD = 2,
  sFIFTH = 3,
  sDARTH = 4,
} state;

typedef bool (*TestFunctionPtr)();

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

void insertToBuffer(int amplitude);
void insertToBuffer(int amplitude, int &index, int *amplitudes, int bufsize);
int readFromBuffer();
int readFromBuffer(int &index, int *amplitudes, int bufsize);