const int PIR_SENSOR_PIN = 2;
const int TRIG_PIN = 3;
const int ECHO_PIN = 4;
const int LED_PIN = 6;

volatile bool motionDetected = false;

void motionISR() {
    motionDetected = true;
}

float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) {
        return 999;
    }

    float distance = duration * 0.034 / 2;
    return distance;
}

void setup() {
    Serial.begin(9600);

    pinMode(PIR_SENSOR_PIN, INPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_PIN), motionISR, RISING);

    Serial.println("System Initialized: PIR & Ultrasonic Sensor Active...");
}

void loop() {
    float distance = getDistance();
    Serial.print("Object Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (motionDetected || distance < 50) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("⚠️ ALERT: Movement or Close Object Detected! LED ON.");
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("✔️ System Clear: No Motion & Object is Far. LED OFF.");
    }

    motionDetected = false;
    delay(500);
}
