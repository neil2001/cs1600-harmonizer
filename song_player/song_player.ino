// #include "rtttl_parser.h"

/*
 * LAB STEP 3
 */
const int CLOCKFREQ = 4000000; // FIXME;

/*
 * LAB STEP 8a
 */
const int PB_PIN = 10; // FIXME;

const String song = "spooky:d=4,o=6,b=127:8c,f,8a,f,8c,b5,2g,8f,e,8g,e,8e5,a5,2f,8c,f,8a,f,8c,b5,2g,8f,e,8c,d,8e,1f,8c,8d,8e,8f,1p,8d,8e,8f_,8g,1p,8d,8e,8f_,8g,p,8d,8e,8f_,8g,p,c,8e,1f";
int noteFrequencies[100];
int noteDurations[100];
int songLen;

int currNote;
 
volatile int intcount;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  /*
   * LAB STEP 8a
   */
  // TODO: Set piezo speaker as output:
  PORT->Group[PORTB].DIR.reg |= 1 << PB_PIN;

  /*
   * LAB STEP 4
   */
  // TODO: Configure and enable GCLK4 for TC:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(4); // do not divide gclk 4
  while(GCLK->STATUS.bit.SYNCBUSY);
  
  // use GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC_OSC8M;
  while(GCLK->STATUS.bit.SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(4) | GCLK_CLKCTRL_ID(0x1b);  


  /*
   * LAB STEP 5
   */
  // TODO: Check if APB is enabled:
  Serial.println(PM->APBCMASK.reg & (1 << 11), BIN); // FIXME: use PM->APBX.reg (change X to the correct letter and mask the relevant bit)

  /*
   * LAB STEP 6
   */
  // TODO: Disable TC (for now)
  // use TC3->COUNT16.CTRLA.reg and TC3->COUNT16.INTENCLR.reg to set up TC register
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!

  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

  // Set up NVIC:
  NVIC_SetPriority(TC3_IRQn, 0);
  NVIC_EnableIRQ(TC3_IRQn);

  Serial.println("Initialized!");
  intcount = 0;
  currNote = 0;

  // testAllNotes(); // Remove for step 8b of the lab

  /*
   * LAB STEP 8b
   */
  // Parse a song

  // songLen = rtttlToBuffers(song, noteFrequencies, noteDurations);
  // if (songLen == -1) {
  //   Serial.println("ERROR PARSING SONG!");
  //   while(true);
  // }

  /*
   * LAB STEP 9
   */
  // Clear and enable WDT
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  // TODO: Configure and enable WDT GCLK:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);
  // set GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg |= GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC(3);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID(3);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // TODO: Configure and enable WDT:
  // use WDT->CONFIG.reg, WDT->EWCTRL.reg, WDT->CTRL.reg
  WDT->CONFIG.reg |= 1 << 12;
  WDT->EWCTRL.reg |= 1 << 11;
  WDT->CTRL.reg |= WDT_CTRL_ENABLE;
  while(WDT->STATUS.bit.SYNCBUSY);


  // TODO: Enable early warning interrupts on WDT:
  // reference WDT registers with WDT->registername.reg
  WDT->INTENSET.reg |= WDT_INTENSET_EW;
} /* end of setup function */

/*
 * Sets correct TC timer value and enables TC interrupt so that the interrupt (NOT this function)
 * toggles the output pin at the desired frequency
 * Non-blocking: note can play while program executes
 */
void playNote(int freq) {
  // Reference TC with TC3->COUNT16.registername.reg
  
  // TODO: Turn off interrupts to TC3 on MC0 when configuring
  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

  // TODO: Configure TC3 to interrupt at the correct frequency
  // Use TC3->COUNT16.CC[0].reg for CC0
  TC3->COUNT16.CTRLA.reg = TC_CTRLA_ENABLE | TC_CTRLA_WAVEGEN(1) | TC_CTRLA_MODE(0) | TC_CTRLA_PRESCALER(1) | TC_CTRLA_PRESCSYNC(1);
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!

  
  TC3->COUNT16.CC[0].reg = CLOCKFREQ/(2*freq);
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); // NOTE: ASK ABOUT THIS !!

  // TODO: Turn interrupts to TC3 on MC0 back on when done configuring
  TC3->COUNT16.INTENSET.reg |= TC_INTENSET_MC0;

}

/*
 * Disables TC timer and turns off output pin
 */
void stopPlay() {
  // TODO: Reference TC with TC3->COUNT16.registername.reg
  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  
  // TODO (step 8a): Reference pin with PORT->Group[PORTB].registername.reg
  PORT->Group[PORTB].OUTTGL.reg |= (1 << PB_PIN);
}

/*
 * Plays a given note for a given duration (a frequency of 0 is a rest, i.e. no sound should play for the duration)
 * Blocking: everything except interrupts will have to wait for this function
 */
void playNoteDuration(int freq, int durMillis) {
  // TODO: use playNote and stopPlay to write this function
  if (freq != 0) {
    playNote(freq);
  }
  delay(durMillis);
  stopPlay();
}

void loop() {
  // TODO: play a song
  playNoteDuration(880, 1000);
  // Serial.print("played note: ");
  // Serial.println(currNote);
  // if (currNote == (songLen-1)) {
  //   // Serial.println("Finished playing song");
  //   delay(2000);
  //   currNote = 0;
  // } else {
  //   currNote++;
  // }

  // TODO (Step 9): pet the watchdog
  WDT->CLEAR.reg = 0xA5;

  while(WDT->STATUS.bit.SYNCBUSY);
}

void TC3_Handler() {
  // TODO: Clear interrupt register flag
  // (use register TC3->COUNT16.registername.reg)
  TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0;
  
  intcount += 1; // DO NOT DELETE THIS LINE
  
  // TODO (Step 8a) Toggle output pin
  // use PORT->Group[PORTB].registername.reg = (1 << PB_PIN)
  PORT->Group[PORTB].OUTTGL.reg = (1 << PB_PIN);
}

void WDT_Handler() {
  // TODO: Clear interrupt register flag
  // (reference register with WDT->registername.reg)
  WDT->INTFLAG.reg = 1;
  
  // TODO: Warn user that a watchdog reset may happen
  Serial.println("HEYY WATCHDOG ABOUT TO BARK\n");
}
