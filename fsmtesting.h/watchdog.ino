void setup_watchdog() {
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
  while(WDT->STATUS.bit.SYNCBUSY);
  WDT->EWCTRL.reg |= 1 << 11;
  while(WDT->STATUS.bit.SYNCBUSY);
  WDT->CTRL.reg |= WDT_CTRL_ENABLE;
  while(WDT->STATUS.bit.SYNCBUSY);

  // TODO: Enable early warning interrupts on WDT:
  // reference WDT registers with WDT->registername.reg
  WDT->INTENSET.reg |= WDT_INTENSET_EW;
  while(WDT->STATUS.bit.SYNCBUSY);
}

void WDT_Handler() {
  // Clear interrupt register flag
  // (reference register with WDT->registername.reg)
  WDT->INTFLAG.reg = 1;
  
  // TODO: Warn user that a watchdog reset may happen
  if (DEBUG){
    Serial.println("WATCHDOG ABOUT TO BARK\n");
  }
}

