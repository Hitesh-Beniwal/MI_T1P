const int motionSensorPin = 2;  // Motion sensor input pin
const int ledPin = 13;          // Built-in LED output pin

void setup() {
    pinMode(motionSensorPin, INPUT);  // Set sensor pin as input
    pinMode(ledPin, OUTPUT);          // Set LED pin as output
    Serial.begin(9600);               // Start serial communication
}

void loop() {
    int motionDetected = digitalRead(motionSensorPin);  // Read sensor value

    if (motionDetected == HIGH) {  
        digitalWrite(ledPin, HIGH); 
        Serial.println("Motion Detected! LED ON"); 
    } else {  
        digitalWrite(ledPin, LOW); // Turn LED OFF
        Serial.println("No Motion. LED OFF"); 
    }
    
    delay(500); // Small delay to prevent rapid flickering
}
