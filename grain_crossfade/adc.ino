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
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 |
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