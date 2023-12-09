// int pushCount;
// bool externalOn;
// bool builtinOn;

// void setup() {
//   Serial.begin(9600);
//   while (!Serial);
//   Serial.println("====");

//   pushCount = 0;
//   externalOn = false;
//   builtinOn = false;

//   // **Set Pin 0 direction to input**
//   PORT->Group[PORTA].DIRCLR.reg = (1 << 22);
//   PORT->Group[PORTA].DIRCLR.reg = (1 << 23);
		 
//   // **Configure Pin 0 with the EIC**
		
//   // Enable input buffer, pull resistor, and multiplexing
//   // (Prelab 4.2 and 4.3)
//   PORT->Group[PORTA].PINCFG[22].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN | PORT_PINCFG_PMUXEN;

//   // PORT->Group[PORTA].PINCFG[23].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN | PORT_PINCFG_PMUXEN;

		 
//   // Select the multiplexer function
//   // (Prelab 4.4)
//   PORT->Group[PORTA].PMUX[11].reg |= PORT_PMUX_PMUXE_A; // how to select?
//   // PORT->Group[PORTA].PMUX[11].reg |= PORT_PMUX_PMUXO_A; // how to select?

//   // ** CHECK APBA and NMI**
//   // (Prelab 5.1)
//   Serial.println("APBA enabled? Bit 6 should be 1");
//   Serial.println(PM->APBAMASK.reg, BIN); // prints binary value
//   Serial.println("NMI? Bit 3 should be 0");
//   Serial.println(EIC->NMICTRL.reg, BIN);
		
//   // ** Set up GCLK for EIC **
//   // (Prelab 5.2-5.7)
//   GCLK->GENDIV.reg = 0x00000000 | GCLK_GENDIV_ID(4);
//   while(GCLK->STATUS.bit.SYNCBUSY); // write-synchronized (15.6.6)
//   GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(4) | GCLK_CLKCTRL_ID(5);
//   GCLK->GENCTRL.reg = GCLK_GENCTRL_OE | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC(5) | GCLK_GENCTRL_ID(4);
//   while(GCLK->STATUS.bit.SYNCBUSY); // write-synchronized
		
//   // ** Activate EXTINT 6 (pin 0) in EIC **
//   // (Prelab 6.1-6.3)
//   EIC->CONFIG[0].reg = EIC_CONFIG_FILTEN6 | EIC_CONFIG_SENSE6_RISE | EIC_CONFIG_FILTEN7 | EIC_CONFIG_SENSE7_RISE;
//   // EIC->CONFIG[1].reg = ;
//   // Two other registers (Prelab 6.2 and 6.3):
//   EIC->EVCTRL.reg |= EIC_EVCTRL_EXTINTEO6 | EIC_EVCTRL_EXTINTEO7;
//   EIC->WAKEUP.reg |= EIC_WAKEUP_WAKEUPEN6 | EIC_WAKEUP_WAKEUPEN7;

//   Serial.println("setting control reg");
		
//   // ** Enable EIC** 
//   // (Prelab 6.4)
//   EIC->CTRL.reg = EIC_CTRL_ENABLE;
//   while(EIC->STATUS.bit.SYNCBUSY); // write-synchronized (15.6.6)

//   // ** Clear interrupt flags **
//   // (Prelab 6.5)
//   Serial.println("clearing interrupt flag");

//   // EIC->INTFLAG.reg |= EIC_INTFLAG_EXTINT6;
//   EIC->INTENSET.reg |= EIC_INTENSET_EXTINT6 | EIC_INTENSET_EXTINT7;
        
//   Serial.println("Interrupts ready!");

//   // Set up priority and handler
//   // NVIC = Nested Vector Interrupt Controller
//   NVIC_SetPriority(EIC_IRQn, 3); // do not need the highest priority
//   NVIC_EnableIRQ(EIC_IRQn); // enable EIC_handler
// }

// void loop(void) {

// }

// void EIC_Handler(void) {
//   pushCount++;
//   Serial.print("Button push count: ");
//   Serial.println(pushCount);

//   if (EIC->INTFLAG.reg & EIC_INTENSET_EXTINT6) {
//     if (builtinOn) {
//       int mask = ~(1<<20);
//       PORT->Group[PORTA].OUT.reg &= mask;
//     } else {
//       PORT->Group[PORTA].OUT.reg |= 1 << 20;
//     }
//     builtinOn = !builtinOn;
//     EIC->INTFLAG.reg |= EIC_INTFLAG_EXTINT6;
//   } else if (EIC->INTFLAG.reg & EIC_INTENSET_EXTINT7) {
//     if (externalOn) {
//       int mask = ~(1<<21);
//       PORT->Group[PORTA].OUT.reg &= mask;
//     } else {
//       PORT->Group[PORTA].OUT.reg |= 1 << 21;
//     }
//     externalOn = !externalOn;
//     EIC->INTFLAG.reg |= EIC_INTFLAG_EXTINT7;
//   }
// }


void test_buttons_setup(){
  pinMode(0, INPUT);
}
void test_buttons_loop(){
   Serial.print(digitalRead(0)); Serial.print(", "); Serial.print(digitalRead(1)); Serial.print(", "); Serial.println(digitalRead(2));
}


const int BTN_NORMAL = 0;
const int BTN_THIRD = 1;
const int BTN_FIFTH = 6;
int mode = 0;


void normalISR(){
  Serial.println("NORMAL");
  mode = 0;
}

void thirdISR(){
  Serial.println("THIRD");
  mode = 3;
}

void fifthISR(){
  Serial.println("FIFTH");
  mode = 5;
}

void setup(){
  pinMode(BTN_NORMAL, INPUT);
  pinMode(BTN_THIRD, INPUT);
  pinMode(BTN_FIFTH, INPUT);

  attachInterrupt(digitalPinToInterrupt(BTN_NORMAL), normalISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_THIRD), thirdISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_FIFTH), fifthISR, RISING);
}

void loop(){
  // Serial.println(mode);
  // test_buttons_loop();
}
