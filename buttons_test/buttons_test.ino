#define DEBUG true

const int BTN_NORMAL = 0;
const int BTN_THIRD = 1;
const int BTN_FIFTH = 6;
const int BTN_DARTH = 7;

bool normalBtnOn = false;
bool thirdBtnOn = false;
bool fifthBtnOn = false;
bool darthBtnOn = false;

void normalISR(){
  if (DEBUG){
    Serial.println("NORMAL");
  }
  normalBtnOn = true;
}

void thirdISR(){
  if (DEBUG){
    Serial.println("THIRD");
  }
  thirdBtnOn = true;
}

void fifthISR(){
  if (DEBUG){
    Serial.println("FIFTH");
  }
  fifthBtnOn = true;
}

void darthISR(){
  if (DEBUG){
    Serial.println("DARTH");
  }
  darthBtnOn = true;
}

void resetButtons(){
  normalBtnOn = thirdBtnOn = fifthBtnOn = darthBtnOn = false;
}

void testButtonsISR(){
  Serial.print(normalBtnOn); Serial.print(", "); Serial.print(thirdBtnOn); Serial.print(", "); Serial.print(fifthBtnOn); Serial.print(", "); Serial.println(darthBtnOn);
}

void testButtons(){
  Serial.print(digitalRead(BTN_NORMAL)); Serial.print(", "); Serial.print(digitalRead(BTN_THIRD)); Serial.print(", "); Serial.print(digitalRead(BTN_FIFTH)); Serial.print(", "); Serial.println(digitalRead(BTN_DARTH));
}

void setup(){
  Serial.begin(9600);
  while (!Serial);

  pinMode(BTN_NORMAL, INPUT);
  pinMode(BTN_THIRD, INPUT);
  pinMode(BTN_FIFTH, INPUT);
  pinMode(BTN_DARTH, INPUT);

  attachInterrupt(digitalPinToInterrupt(BTN_NORMAL), normalISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_THIRD), thirdISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_FIFTH), fifthISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_DARTH), darthISR, RISING);
}

void loop(){
  testButtonsISR();
  resetButtons();
  delay(500);
}