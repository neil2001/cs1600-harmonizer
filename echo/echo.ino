const int adcPin = A1;  // A0

const int MAX_RESULTS = 256;

volatile int results [MAX_RESULTS];
volatile int resultNumber;

volatile int count;
volatile int adcCount;

void setup ()
{
  count = 0;
  adcCount = 0;

  Serial.begin(9600);
  while(!Serial);
  Serial.print("starting\n");

  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(4); // do not divide gclk 4
  while(GCLK->STATUS.bit.SYNCBUSY);
  
  // use GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC_OSC8M;
  while(GCLK->STATUS.bit.SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(4) | GCLK_CLKCTRL_ID(0x1b); 
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN_GCLK5 |  // Select GLCK5
                        GCLK_CLKCTRL_ID_ADC |     // Connect to ADC
                        GCLK_CLKCTRL_CLKEN; 
  while (GCLK->STATUS.bit.SYNCBUSY);

  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!

  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

  // Set up NVIC:
  // NVIC_SetPriority(TC3_IRQn, 0);
  // NVIC_EnableIRQ(TC3_IRQn);

  NVIC_SetPriority(TC3_IRQn, 0); // Set the priority of TC3 interrupt
  NVIC_EnableIRQ(TC3_IRQn); // Enable TC3 interrupt

  Serial.println("Initialized!");

  // Reset Timer 1
  TcCount16* TC = (TcCount16*) TC3; // Typecast TC3 to 16-bit counter
  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NFRQ | TC_CTRLA_PRESCALER_DIV8 | TC_CTRLA_ENABLE; // Enable TC3
  while(TC->STATUS.bit.SYNCBUSY);

  TC->CC[0].reg = 400; // Set the compare value for a 50 kHz frequency

  TC->INTENSET.reg = TC_INTENSET_MC(1); // Enable compare channel 0 interrupt
  while(TC->STATUS.bit.SYNCBUSY);

  Serial.println ("timer TC3 interrupts enabled");
  
  // Set up ADC
  Serial.println ("setting up ADC");
  // attachInterrupt(adcPin, ADC_Handler, CHANGE);

  // ADC->CTRLB.reg = ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_PRESCALER_DIV16 | ADC_CTRLB_FREERUN; // 12-bit resolution, prescaler of 16
  // while(ADC->STATUS.bit.SYNCBUSY);
  // ADC->CTRLA.reg = ADC_CTRLA_ENABLE | ADC_CTRLA_PRESCALER_DIV512; // Enable ADC, free run mode, prescaler of 512
  // while(ADC->STATUS.bit.SYNCBUSY);
  
  // // Set reference voltage to AVcc (5V)
  // ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
  // while(ADC->STATUS.bit.SYNCBUSY);

  // // Set up ADC event system
  // NVIC_SetPriority(ADC_IRQn, 0);
  // NVIC_EnableIRQ(ADC_IRQn);

  // ADC->CTRLA.bit.ENABLE = 0;  // Disable ADC
  // ADC->CTRLB.bit.PRESCALER = 0x04; // Set prescaler to 16
  // ADC->INTENSET.bit.RESRDY = 1; // Enable result ready interrupt
  // ADC->INPUTCTRL.bit.MUXPOS = 0x01; // Set analog input to AIN0 (A0)
  // ADC->CTRLA.bit.ENABLE = 1;  // Enable ADC
  // ADC->CTRLA.bit.RUNSTDBY = 1; // Enable run in standby
  // ADC->CTRLB.bit.FREERUN = 1; // Enable free-running mode
  // while(ADC->STATUS.bit.SYNCBUSY);

  // NVIC_SetPriority(ADC_IRQn, 0);
  // NVIC_EnableIRQ(ADC_IRQn);

  Serial.println ("ADC set up");

  // wait for buffer to fill
  while (resultNumber < MAX_RESULTS) {
    // Serial.println(analogRead(adcPin));
    // Serial.print("adc count: ");
    // Serial.println(adcCount);
    // Serial.print("count: ");
    // Serial.println(count);
  }

  Serial.println("results full");

  for (int i = 0; i < MAX_RESULTS; i++) {
    Serial.println (results [i]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

void ADC_Handler() {
  Serial.println("adc handler called");
  adcCount = (adcCount + 1) % 100000;
  if (resultNumber >= MAX_RESULTS) {
    ADC->CTRLA.bit.ENABLE = 0;  // Turn off ADC
  } else {
    results [resultNumber++] = analogRead(adcPin);
  }
}

void TC3_Handler() {
  // if (resultNumber >= MAX_RESULTS) {
  //   ADC->CTRLA.bit.ENABLE = 0;  // Turn off ADC
  // } else {
  //   results [resultNumber++] = analogRead(adcPin);
  // }

  // TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0;
  
  // intcount += 1; // DO NOT DELETE THIS LINE
  
  // TODO (Step 8a) Toggle output pin
  // use PORT->Group[PORTB].registername.reg = (1 << PB_PIN)
  // PORT->Group[PORTB].OUTTGL.reg = (1 << PB_PIN);

  // count = (count + 1) % 100000;

  TcCount16* TC = (TcCount16*) TC3; // Typecast TC3 to 16-bit counter
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1; // Clear interrupt flag
    ADC->SWTRIG.bit.START = 1; // Start ADC conversion
  }  
}