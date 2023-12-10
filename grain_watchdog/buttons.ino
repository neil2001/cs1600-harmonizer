const int BTN_NORMAL = 0;
const int BTN_THIRD = 1;
const int BTN_FIFTH = 6;

bool normalBtnOn = 0;
bool thirdBtnOn = 0;
bool fifthBtnOn = 0;

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

void resetButtons(){
  normalBtnOn = thirdBtnOn = fifthBtnOn = false;
}