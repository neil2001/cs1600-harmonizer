#include <AudioFrequencyMeter.h>

AudioFrequencyMeter meter;
const int SPEAKER_PIN = 8;
const int ADC_PIN = A1;
const int DAC_PIN = A0;
const int BASELINE = 337;
const int DELAY = 2000;

// Measured in Hz
const int FEMALE_HIGH_FREQ = 255;
const int FEMALE_LOW_FREQ = 165;
const int MALE_HIGH_FREQ = 155;
const int MALE_LOW_FREQ = 85;

const int SILENCE_THRESH = 185;

void setup() {

  Serial.begin(9600);
  Serial.println("Started");
  // meter.setBandwidth(70.00, 1500);    // Ignore frequency out of this range
  meter.begin(A1, 45000);             // Initialize A0 at sample rate of 45kHz
}

unsigned long lastTime = 0;
unsigned long duration = 0;
int TRIGGER = 0;

void playTone(int freq, long duration){
  // if (TRIGGER){
  //   noTone(SPEAKER_PIN);
  // } 
  // else {
  //   tone(SPEAKER_PIN, freq, duration);
  //   lastTime = millis();
  // }
  tone(SPEAKER_PIN, freq, duration);

}

void loop() {

  // put your main code here, to run repeatedly:
  float frequency = meter.getFrequency();
  // int input = analogRead(ADC_PIN);
  // int amplitude = input - BASELINE;

  if (frequency > 0){
    Serial.print(frequency);
    Serial.println(" Hz");

    // playTone(frequency, 50);
  tone(SPEAKER_PIN, frequency, 50);

  }


  if (millis() - lastTime > duration){
    TRIGGER = 1;
    lastTime = millis();
  }
  else {
    TRIGGER = 0;
  }
}