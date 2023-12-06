#define BUFSIZE 1024
#define CLOCKFREQ 4000000
#define SAMPLE_FREQUENCY 27778 // rounded from 27777.77
#define DEBUG true

int MIC_PIN = A1;
int SPEAKER_PIN = A0;

int PB_PIN = 10;
const int BUF_LEN = BUFSIZE;

int buffer[BUFSIZE] = {0};
volatile int grain_duration = 0; // in millis
volatile int inIdx, outIdx;


void setup_TC4() {
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(5); // do not divide gclk 4
  while(GCLK->STATUS.bit.SYNCBUSY);
  
  // use GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC_OSC8M;
  while(GCLK->STATUS.bit.SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_ID(0x1c);  

  TC4->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!

  TC4->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

  // Set up NVIC:
  NVIC_SetPriority(TC4_IRQn, 0);
  NVIC_EnableIRQ(TC4_IRQn);

  TC4->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;
  TC4->COUNT16.CTRLA.reg = TC_CTRLA_ENABLE | TC_CTRLA_WAVEGEN(1) | TC_CTRLA_MODE(0) | TC_CTRLA_PRESCALER(1) | TC_CTRLA_PRESCSYNC(1);
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!
  
  TC4->COUNT16.CC[0].reg = CLOCKFREQ/(2*SAMPLE_FREQUENCY);
  while(TC4->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!

  TC4->COUNT16.INTENSET.reg |= TC_INTENSET_MC0;

  Serial.println("Initialized TC4!");
}

void setup_TC3() {
  // PORT->Group[PORTB].DIR.reg |= 1 << PB_PIN;

  // Sets the division factor of Generic Clock Controller
  // Sets the division factor to 0 (no division) for the 
  // clock generator with ID 4
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(4); 
  while(GCLK->STATUS.bit.SYNCBUSY);
  
  // GENCTRL used for configuring generic clock generator
  // - This bit enables the generic clock generator. When set, the generator is enabled.
  // - This field sets the ID of the generic clock generator. In this case, it's set to ID 4.
  // - This bit enables the improved duty cycle mode. When set, the duty cycle correction is enabled.
  // - This field sets the source for the generic clock generator. In this case, it's set to use the internal 8MHz oscillator (OSC8M) as the source.
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC_OSC8M;
  while(GCLK->STATUS.bit.SYNCBUSY);

  // Used to set clock to use the now configured generic clock 
  // generator #4
  // - Enable the clock
  // - This field sets the source generator for the generic clock. In this case, it's set to generator 4.
  // - This field sets the ID of the generic clock source. In this case, it's set to 0x1b.
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(4) | GCLK_CLKCTRL_ID(0x1b);  

  // Disable TC temporarily
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY);

  // Disabling the interrupt for match/capture channel 0 of Timer/Counter 3 on the SAMD21 microcontroller. Disabling this interrupt means that the associated interrupt service routine (ISR) will not be called when a specific condition (such as a match) occurs on channel 0 of Timer/Counter 3
  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

  // Set up NVIC for TC3 clock interrupt handler
  NVIC_SetPriority(TC3_IRQn, 0);
  NVIC_EnableIRQ(TC3_IRQn);

  // Prescales the clock to 1 and sets to wave generator mode
  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;
  TC3->COUNT16.CTRLA.reg = TC_CTRLA_ENABLE | TC_CTRLA_WAVEGEN(1) | TC_CTRLA_MODE(0) | TC_CTRLA_PRESCALER(1) | TC_CTRLA_PRESCSYNC(1);
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); 
  
  // Sets the TC to interrupt on a PWM rising edge at a given SAMPLE_FREQUENCY
  TC3->COUNT16.CC[0].reg = CLOCKFREQ/(2*SAMPLE_FREQUENCY);
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); 

  // Turn interrupts to TC3 on MC0 back on when done configuring
  TC3->COUNT16.INTENSET.reg |= TC_INTENSET_MC0;

  Serial.println("Initialized TC3!");
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
      Serial.print("ADC Result: "); Serial.println(result);
    }

    return result;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(SPEAKER_PIN, OUTPUT);
  analogWriteResolution(12);

  setup_TC3();
  // setup_TC4();
  // setup_ADC();

  outIdx = 0;
  inIdx = outIdx + grain_duration;

  Serial.println("Initialized");
  // readAndPrint();
}

void loop() {

  // Buffer is being written to 27777.7 times per second
  // meaning it completely fills buffer 277.7 (27,777.77 * 10ms / 1000ms)
  // per second. And therefore fills the buffer every 0.0036ms
  // This is extremely small and barely audible
  // Add read-in value to buffer and increment inIdx with wrap
  // NOTE: this function will delay the loop to x Hz/kHz
  buffer[inIdx++] = readADCSync();  
  inIdx %= BUFSIZE;

  // out++;
  // if (out > BUF_LEN) {
  //   Serial.println("out back to top");
  //   out = 0;
  // }
}

// void TC4_Handler() {
//   TC4->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0;

//   int result = buffer[out];
//   analogWrite(SPEAKER_PIN, result);
  
//   out++;
//   if (out > BUF_LEN) {
//     Serial.println("out back to top");
//     out = 0;
//   }
// }

void TC3_Handler() {
  int amplitude = buffer[outIdx];
  outIdx = (outIdx + 1) % BUFSIZE;

  analogWrite(SPEAKER_PIN, amplitude);

  if (DEBUG){
     Serial.print("Out Amplitude: "); Serial.println(amplitude);
  }
}
