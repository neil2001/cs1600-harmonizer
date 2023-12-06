const int speakerPin = A0; // The pin connected to the speaker
const int bufferSize = 8; // 1024; // Size of the buffer
float sineBuffer[8] = {0,64,128,192,255,192,128,64}; // Buffer to store the sine wave values

void setup() {
  pinMode(speakerPin, OUTPUT);
  pinMode(5, INPUT);
  
  // Generate the sine wave values and store them in the buffer
  // generateSineWave();
  analogWriteResolution(8);
}

int count = 0;
long lastMillis = millis();

void loop() {
  // Continuously output the sine wave from the buffer to the speaker
  for (int i = 0; i < bufferSize; i++) {
    analogRead(5);
    analogWrite(speakerPin, sineBuffer[i]);
    // Serial.println(sineBuffer[i]);
    // count += 1;
    // if (millis() - lastMillis > 1000) {
    //   Serial.println(count);
    //   count = 0;
    //   lastMillis = millis();
    // }
    // delayMicroseconds(500); // Adjust delay based on your speaker and desired frequency
  }
}

// const float t_rate = bufferSize * 133000;


void generateSineWave() {
  float frequency = 100.0; // Frequency of the sine wave in Hz
  float amplitude = 100; // Amplitude of the sine wave (half of 255, as analogWrite range is 0-255)

  for (int i = 0; i < bufferSize; i++) {
    float time = i * 0.0005; // (1.0 / bufferSize); // Time in seconds
    float sineValue = amplitude * sin(2.0 * 3.14159 * i / frequency) + amplitude; // Sine wave formula
    sineBuffer[i] = sineValue;
  }
}