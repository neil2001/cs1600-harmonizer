const int ADC_PIN = A1;
const int DAC_PIN = A0;
const int SPEAKER_PIN = 8;
const int BASELINE = 337;
const int DELAY = 2000;


// Measured in Hz
const int FEMALE_HIGH_FREQ = 255;
const int FEMALE_LOW_FREQ = 165;
const int MALE_HIGH_FREQ = 155;
const int MALE_LOW_FREQ = 85;

const int SILENCE_THRESH = 185;

void playTone(int freq, long duration){
  tone(SPEAKER_PIN, freq, duration);
  delay(duration);
  noTone(SPEAKER_PIN);
}

void setup ()
{
  Serial.begin(9600);
  while(!Serial);
  Serial.print("Starting\n");

  // pinMode(SPEAKER_PIN, OUTPUT);
  // analogWriteResolution(10);

  for (int i=0; i < 10; i++){
    playTone(i*50, 1000);
  }
}

unsigned long lastTime = millis();
int TRIGGER = 0;

void loop() {

  if (TRIGGER){
  }
  else {
  }

  if (millis() - lastTime > DELAY){
    TRIGGER = 1;
    lastTime = millis();
  }
  else {
    TRIGGER = 0;
  }

  int input = analogRead(ADC_PIN);
  int amplitude = input - BASELINE;

  if (amplitude > SILENCE_THRESH){
    Serial.println(amplitude);
  }
  
}