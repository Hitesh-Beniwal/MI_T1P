// Define sensor pins
#define PIR_PIN 2        // Motion sensor (HC-SR505) - Interrupt Pin
#define TRIG_PIN 4       // Ultrasonic sensor Trigger
#define ECHO_PIN 5       // Ultrasonic sensor Echo
#define LED_PIN 6        // External LED

// Variables
volatile bool motionDetected = false;

void setup() {
    Serial.begin(9600);
    
    // Setup sensor pins
    pinMode(PIR_PIN, INPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // Attach interrupt for motion detection
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);
}

void loop() {
    int distance = getDistance();  // Get distance from ultrasonic sensor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Condition to turn on LED
    if (motionDetected || distance < 50) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("⚠ ALERT: Motion or Close Object Detected! LED ON.");
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("✅ System Clear: No Motion & Object is Far. LED OFF.");
    }

    // Reset motion flag
    motionDetected = false;

    delay(500);  // Delay to prevent excessive looping
}

// Interrupt Service Routine (ISR) for Motion Detection
void motionISR() {
    motionDetected = true;
}

// Function to get distance using HC-SR04 Ultrasonic Sensor
int getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    int distance = duration * 0.034 / 2; // Convert to cm
    return distance;
}
