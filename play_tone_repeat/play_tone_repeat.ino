const int speakerPin = 11; // Connect the speaker to pin 9
int toneDuration = 1000; // Tone duration in milliseconds
int silenceDuration = 1000;
int value;
unsigned long previousMillis = 0; // Store the last time the tone was started


const int micpin = A1;

int mic;
const int baseline = 337;
int amplitude;
bool toneOn = false;

void setup() {
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
}

void loop() {


  mic = analogRead(micpin);
  amplitude = abs(mic-baseline);

  value = amplitude > 200 ? HIGH : LOW;
  
  
  // unsigned long currentMillis = millis(); // Get the current time


  // if (toneOn && (currentMillis - previousMillis >= toneDuration)) {
  //   // noTone(speakerPin); // Stop the tone
  //   // analogWrite(speakerPin, 0);
  //   value = 0;
  //   toneOn = false; // Set toneOn to false
  //   previousMillis = currentMillis; // Update the last time the tone was stopped
  // } else if (!toneOn && (currentMillis - previousMillis >= silenceDuration)) {
  //   // tone(speakerPin, 440); // Start the tone
  //   value = 255;
  //   toneOn = true; // Set toneOn to true
  //   previousMillis = currentMillis; // Update the last time the tone was started
  // }

  analogWrite(speakerPin, micpin);


  Serial.println(amplitude);
}

void startHarmony() {
  
}
