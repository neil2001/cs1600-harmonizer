// Tests whether ADC and TC5 can run concurrently at the specified frequencies
// Also microphone data will be read in and added to buffer at SAMPLE_FREQUENCY
// while the speaker will read from buffer at SPEAKER_FREQUENCY
// All tests pass!

#define BUFSIZE 17
const long SAMPLE_FREQUENCY = 50;// 27778 // rounded from 27777.77
const long SPEAKER_FREQUENCY = 20000;// 27778 // rounded from 27777.77
#define DEBUG true

float sineBuffer[BUFSIZE] = {0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4096, 3584, 3072, 2560, 2048, 1536, 1024, 512, 0};

int MIC_PIN = A1;
int SPEAKER_PIN = A0;

int PB_PIN = 10;
const int BUF_LEN = BUFSIZE;

int buffer[BUFSIZE] = {0};
volatile int grain_duration = 10; // in millis
volatile int inIdx, outIdx;


//this function gets called by the interrupt at <sampleRate>Hertz
bool startedTC5 = false;
bool completedTC5 = false;
bool printedTC5Once = false;
long lastTC5Micros, curTC5Micros;
void TC5_Handler (void) {
  
  int amplitude = sineBuffer[outIdx];
  outIdx = (outIdx + 1) % BUFSIZE;


  if (DEBUG){
    // Test code to ensure we're cycling at the correct frequency
    if (!startedTC5){
      lastTC5Micros = micros();
      startedTC5 = true;
    }
    else if (!completedTC5){
      curTC5Micros = micros();
      completedTC5 = true;
    }
    else if (!printedTC5Once){
      Serial.print("TC5 Handler Freq: "); Serial.println(curTC5Micros - lastTC5Micros);
      Serial.print("Out Amplitude: "); Serial.println(amplitude);
      printedTC5Once = true;
    }
  }

  // analogWrite(SPEAKER_PIN, amplitude);


  // END OF YOUR CODE
  TC5->COUNT16.INTFLAG.bit.MC0 = 1; //Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}

/* 
 *  TIMER SPECIFIC FUNCTIONS FOLLOW
 *  you shouldn't change these unless you know what you're doing
 */

//Configures the TC to generate output events at the sample frequency.
//Configures the TC in Frequency Generation mode, with an event output once
//each time the audio sample frequency period expires.
 void tc5Configure(int rate)
{
  tcDisable();


  // select the generic clock generator used as source to the generic clock multiplexer
  GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
  while (GCLK->STATUS.bit.SYNCBUSY);

  tcReset(); //reset TC5

  // Set Timer counter 5 Mode to 16 bits, it will become a 16bit counter ('mode1' in the datasheet)
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  // Set TC5 waveform generation mode to 'match frequency'
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  //set prescaler
  //the clock normally counts at the GCLK_TC frequency, but we can set it to divide that frequency to slow it down
  //you can use different prescaler divisons here like TC_CTRLA_PRESCALER_DIV1 to get a different range
  uint32_t period, prescaler;
  // if (!(prescaler = calcPrescaler(sampleRate, period))){
  //   Serial.print("Error calculating prescaler, incapable of producing frequency "); Serial.println(sampleRate);
  //   exit(1);
  // }
  // Prescaler range of DIV2 gives us enough range for what we need...
  prescaler = TC_CTRLA_PRESCALER_DIV2;
  period = SystemCoreClock / (2 * rate);
  Serial.print("Prescaler set to "); Serial.println(prescaler >> TC_CTRLA_PRESCALER_Pos);
  Serial.print("Period set to "); Serial.println(period);

  TC5->COUNT16.CTRLA.reg |= prescaler | TC_CTRLA_ENABLE; //it will divide GCLK_TC frequency by 1024

  //set the compare-capture register. 
  //The counter will count up to this value (it's a 16bit counter so we use uint16_t)
  //this is how we fine-tune the frequency, make it count to a lower or higher value
  //system clock should be 1MHz (8MHz/8) at Reset by default
  TC5->COUNT16.CC[0].reg = (uint16_t) (period);
  while (tcIsSyncing());
  
  // Configure interrupt request
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);
} 

//Function that is used to check if TC5 is done syncing
//returns true when it is done syncing
bool tcIsSyncing()
{
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

//This function enables TC5 and waits for it to be ready
void tcStartCounter()
{
    // Enable the TC5 interrupt request
  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  while (tcIsSyncing()); //wait until TC5 is done syncing 

  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; //set the CTRLA register
  while (tcIsSyncing()); //wait until snyc'd
}

//Reset TC5 
void tcReset()
{
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (tcIsSyncing());
  while (TC5->COUNT16.CTRLA.bit.SWRST);
}

//disable TC5
void tcDisable()
{
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tcIsSyncing());
}

void setup_ADC(){
  /* Enable GCLK1 for the ADC */
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
                      GCLK_CLKCTRL_GEN_GCLK1 |
                      GCLK_CLKCTRL_ID_ADC;

  /* Wait for bus synchronization. */
  while (GCLK->STATUS.bit.SYNCBUSY) {}; 

  // use GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC_OSC8M;

  /* Wait for bus synchronization. */
  while (GCLK->STATUS.bit.SYNCBUSY) {}; 

  // Load calibration values
  uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;
  uint32_t linearity = (*((uint32_t *) ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;
  linearity |= ((*((uint32_t *) ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;

  /* Wait for bus synchronization. */
  while (ADC->STATUS.bit.SYNCBUSY) {};

  /* Write the calibration data. */
  ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);

  /* Wait for bus synchronization. */
  while (ADC->STATUS.bit.SYNCBUSY) {};

  /* Use the internal VCC reference. This is 1/2 of what's on VCCA.
    since VCCA is typically 3.3v, this is 1.65v.
  */
  ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;

  /* Only capture one sample. The ADC can actually capture and average multiple
    samples for better accuracy, but there's no need to do that for this
    example.
  */
  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1;

  // Set the sample time length to accommodate for input impedance.
  ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(1);

  /* Set the clock prescaler to 512, which will run the ADC at
    8 Mhz / 64 = 31.25 kHz.
    Set the resolution to 12bit.
  */
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 |
                  ADC_CTRLB_RESSEL_12BIT;

  /* Configure the input parameters.

    - GAIN_DIV2 means that the input voltage is halved. This is important
      because the voltage reference is 1/2 of VCCA. So if you want to
      measure 0-3.3v, you need to halve the input as well.

    - MUXNEG_GND means that the ADC should compare the input value to GND.

    - MUXPOST_PIN3 means that the ADC should read from AIN10, or PB02.
      Therefore ADC_INPUTCTRL_MUXPOSE_PIN10 corresponds to the AIN value
  */
  ADC->INPUTCTRL.reg = ADC_INPUTCTRL_GAIN_DIV2 |
                      ADC_INPUTCTRL_MUXNEG_GND |
                      ADC_INPUTCTRL_MUXPOS_PIN10;



  /* Set PB02 as an input pin. */
  PORT->Group[PORTB].DIRCLR.reg = PORT_PB02;

  /* Enable the peripheral multiplexer for PB02. */
  PORT->Group[PORTB].PINCFG[2].reg |= PORT_PINCFG_PMUXEN;

  /* Set PB09 to function B which is analog input. */
  PORT->Group[PORTB].PMUX[1].reg = PORT_PMUX_PMUXE_B;

  /* Wait for bus synchronization. */
  while (ADC->STATUS.bit.SYNCBUSY) {};

  /* Enable the ADC. */
  ADC->CTRLA.bit.ENABLE = true;
}

bool startedADC = false;
bool completedADC = false;
bool printedADCOnce = false;
long curADCMicros;
long lastADCMicros;
int readADCSync(){
  /* Wait for bus synchronization. */
    while (ADC->STATUS.bit.SYNCBUSY) {};

    /* Start the ADC using a software trigger. */
    ADC->SWTRIG.bit.START = true;

    /* Wait for the result ready flag to be set. */
    while (ADC->INTFLAG.bit.RESRDY == 0);

    /* Clear the flag. */
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    /* Read the value. */
    uint32_t result = ADC->RESULT.reg;


    // Debugging information
    if (DEBUG){

      // Test code to ensure we're cycling at the correct frequency
      if (!startedADC){
        lastADCMicros = micros();
        startedADC = true;
      }
      else if (!completedADC){
        curADCMicros = micros();
        completedADC = true;
      }
      else if (!printedADCOnce){
        Serial.print("ADC Result: "); Serial.println(result);
        Serial.print("ADC Freq: "); Serial.println(curADCMicros - lastADCMicros);
        printedADCOnce = true;
      }
    }

    return result;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(SPEAKER_PIN, OUTPUT);
  analogWriteResolution(12);

  tc5Configure(SPEAKER_FREQUENCY);
  Serial.println("Initialized TC5, starting TC5");
  tcStartCounter();

  setup_ADC();
  Serial.println("Initialized ADC");

  outIdx = 0;
  inIdx = outIdx + grain_duration;
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
  buffer[inIdx++] = readADCSync();  
  inIdx %= BUFSIZE;


  // curMillis = millis();
  // Serial.println(curMillis - lastMillis);
  // lastMillis = curMillis;
}
