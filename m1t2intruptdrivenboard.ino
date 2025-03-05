const int motionSensorPin = 2;  // Motion sensor connected to pin 2 (Interrupt pin)
const int ledPin = 13;          // Built-in LED pin
volatile bool motionDetected = false; // Flag to track motion

// Interrupt Service Routine (ISR) to handle motion detection
void motionISR() {
    motionDetected = true;  // Set flag when motion is detected
}

void setup() {
    pinMode(motionSensorPin, INPUT);  // Set sensor pin as input
    pinMode(ledPin, OUTPUT);          // Set LED pin as output
    Serial.begin(9600);               // Start serial communication

    // Attach interrupt to pin 2, trigger on RISING edge (motion detected)
    attachInterrupt(digitalPinToInterrupt(motionSensorPin), motionISR, RISING);
}

void loop() {
    if (motionDetected) {  // If motion is detected
        digitalWrite(ledPin, HIGH);  // Turn LED ON
        Serial.println("Motion Detected! LED ON");  
        delay(2000);  // Keep LED on for visibility
        digitalWrite(ledPin, LOW);  // Turn LED OFF
        Serial.println("LED OFF");  
        motionDetected = false;  // Reset flag
    }
}
