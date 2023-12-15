
/**
 * @brief Set the up watchdog to warn early every 2 seconds
 *        and to bark every 4 seconds 
 */
void setup_watchdog() {

  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  // Configure and enable WDT GCLK:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);
  // set GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg |= GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC(3);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID(3);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Configure and enable WDT:
  WDT->CONFIG.reg |= 1 << 12;
  while(WDT->STATUS.bit.SYNCBUSY);
  WDT->EWCTRL.reg |= 1 << 11;
  while(WDT->STATUS.bit.SYNCBUSY);
  WDT->CTRL.reg |= WDT_CTRL_ENABLE;
  while(WDT->STATUS.bit.SYNCBUSY);

  // Enable early warning interrupts on WDT:
  // reference WDT registers with WDT->registername.reg
  WDT->INTENSET.reg |= WDT_INTENSET_EW;
  while(WDT->STATUS.bit.SYNCBUSY);
}

/**
 * @brief Pets the watchdog by setting register and syncing 
 */
void pet_watchdog(){
  if (DEBUG){
    // Serial.println("petting watchdog");
  }
  WDT->CLEAR.reg = 0xA5;
  while(WDT->STATUS.bit.SYNCBUSY);
}

/**
 * @brief Handler for watchdog warning
 */
void WDT_Handler() {
  // Clear interrupt register flag
  WDT->INTFLAG.reg = 1;
  
  // Warn user that a watchdog reset may happen
  if (DEBUG){
    Serial.println("WATCHDOG ABOUT TO BARK\n");
  }
}

