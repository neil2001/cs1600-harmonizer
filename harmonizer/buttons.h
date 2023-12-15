#ifndef BUTTONS_H
#define BUTTONS_H

/** Pins for each button of the harmonizer */
const int BTN_NORMAL = 0;
const int BTN_THIRD = 1;
const int BTN_FIFTH = 6;
const int BTN_DARTH = 7;


/** Global boolean variables to track which buttons
 *  were pressed, set to true by their respective ISRs */
bool normalBtnOn = false;
bool thirdBtnOn = false;
bool fifthBtnOn = false;
bool darthBtnOn = false;

#endif
