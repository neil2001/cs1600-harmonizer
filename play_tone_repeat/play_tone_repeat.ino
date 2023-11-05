const int speakerPin = 11;  // Connect the speaker to pin 9
const int micpin = A1;


int toneDuration = 1000; // Tone duration in milliseconds
int silenceDuration = 1000;
int value;
unsigned long previousMillis = 0;  // Store the last time the tone was started


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
  amplitude = abs(mic - baseline);

  // value = amplitude > 200 ? 255 : 0;

  unsigned long currentMillis = millis(); // Get the current time


  if (toneOn && (currentMillis - previousMillis >= toneDuration)) {
    noTone(speakerPin); // Stop the tone
    // analogWrite(speakerPin, 0);
    // value = 0;
    toneOn = false; // Set toneOn to false
    previousMillis = currentMillis; // Update the last time the tone was stopped
  } else if (!toneOn && (currentMillis - previousMillis >= silenceDuration)) {
    tone(speakerPin, 440); // Start the tone
    // value = 255;
    toneOn = true; // Set toneOn to true
    previousMillis = currentMillis; // Update the last time the tone was started
  }

  // analogWrite(speakerPin, micpin);


  Serial.println(amplitude);
}

void startHarmony() {
  ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_10BIT_Val;
  ADC->SAMPCTRL.bit.SAMPLEN = 0x1F; // Set maximum sampling time
  ADC->CTRLB.bit.FREERUN = 1; // enable freerun
  ADC->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_AIN0_Val; // Set input to ADC AIN0 (pin A0)
  
  NVIC_EnableIRQ(ADC_IRQn);

  ADC->CTRLA.bit.ENABLE = 1; // Enable ADC
  ADC->SWTRIG.bit.START = 1; // Start conversions
  ADC->INTENSET.bit.RESRDY = 1; // Enable result ready interrupt
}

void ADC_Handler() {
  // Process the ADC result here
  // this should put it in the buffer
  Serial.println("adc handler called");
  adcCount = (adcCount + 1) % 100000;
  if (resultNumber >= MAX_RESULTS) {
    ADC->CTRLA.bit.ENABLE = 0;  // Turn off ADC
  } else {
    results [resultNumber++] = ADC->RESULT.reg;
  }
}

void TC2_Handler() {
  // this should read from the buffer and play some bits of audio sample.
  // should perform cross fade between the two buffers as well
}
