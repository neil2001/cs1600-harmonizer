const int BTN_NORMAL = 0;
const int BTN_THIRD = 1;
const int BTN_FIFTH = 6;
int mode = 0;

void normalISR(){
  Serial.println("NORMAL");
  mode = 0;
  PITCH_FACTOR = NORMAL;
  SPEAKER_FREQUENCY = SAMPLE_FREQUENCY * SCALE_FACTOR * PITCH_FACTOR;
  tc5Configure(SPEAKER_FREQUENCY);
  tcStartCounter();
}

void thirdISR(){
  Serial.println("THIRD");
  mode = 3;
  PITCH_FACTOR = THIRD;
  SPEAKER_FREQUENCY = SAMPLE_FREQUENCY * SCALE_FACTOR * PITCH_FACTOR;
  tc5Configure(SPEAKER_FREQUENCY);
  tcStartCounter();
}

void fifthISR(){
  Serial.println("FIFTH");
  mode = 5;
  PITCH_FACTOR = FIFTH;
  SPEAKER_FREQUENCY = SAMPLE_FREQUENCY * SCALE_FACTOR * PITCH_FACTOR;
  tc5Configure(SPEAKER_FREQUENCY);
  tcStartCounter();
}