#include "buttons.h"

/**
 * @brief ISR for the normal button sets the global 
 *        normalBtnOn bool to true 
 */
void normalISR(){
  if (DEBUG){
    Serial.println("NORMAL");
  }
  normalBtnOn = true;
}

/**
 * @brief ISR for the third button sets the global 
 *        thirdBtnOn bool to true 
 */
void thirdISR(){
  if (DEBUG){
    Serial.println("THIRD");
  }
  thirdBtnOn = true;
}

/**
 * @brief ISR for the fifth button sets the global 
 *        fifthBtnOn bool to true 
 */
void fifthISR(){
  if (DEBUG){
    Serial.println("FIFTH");
  }
  fifthBtnOn = true;
}

/**
 * @brief ISR for the darth button sets the global 
 *        darthBtnOn bool to true 
 */
void darthISR(){
  if (DEBUG){
    Serial.println("DARTH");
  }
  darthBtnOn = true;
}

/**
 * @brief Resets all global button bool values to false 
 */
void resetButtons(){
  normalBtnOn = thirdBtnOn = fifthBtnOn = darthBtnOn = false;
}