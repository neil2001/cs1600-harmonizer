// extern int out;
extern int nSamples;

int crossfade();

//this function gets called by the interrupt at <sampleRate>Hertz
bool startedTC5 = false;
bool completedTC5 = false;
bool printedTC5Once = false;
long lastTC5Micros, curTC5Micros;

void TC5_Handler (void) {
  
  int amplitude = crossfade();
  if(++out >= nSamples) out = 0;

  // if (outIdx == 0) {
  //   WDT->CLEAR.reg = 0xA5;
  //   while(WDT->STATUS.bit.SYNCBUSY);
  // }


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

  analogWrite(SPEAKER_PIN, amplitude);


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