const int PIR_SENSOR_PIN = 2;  
const int LED_PIN = 6;         

void setup() {
    Serial.begin(9600);        
    pinMode(PIR_SENSOR_PIN, INPUT);  
    pinMode(LED_PIN, OUTPUT);        
    Serial.println("PIR Motion Sensor Ready...");
}

void loop() {
    int motionDetected = digitalRead(PIR_SENSOR_PIN);  

    if (motionDetected == HIGH) {  
        digitalWrite(LED_PIN, HIGH);  
        Serial.println("Motion Detected! LED ON");
    } else {
        digitalWrite(LED_PIN, LOW);  
        Serial.println("No Motion. LED OFF");
    }

    delay(500);  
}
