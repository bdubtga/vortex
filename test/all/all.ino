#include <Servo.h>

Servo myservo; // Create a servo object

int mot1_INA = 7;
int mot1_INB = 8;
int mot1_PWM = 5;
int mot2_INA = A3;
int mot2_INB = A4;
int mot2_PWM = 6;
void setup()
{
    Serial.begin(9600); // Initialize serial communication
    myservo.attach(3);  // Attach the servo to pin 3 (PWM pin)
    delay(1);
    pinMode(mot1_INA, OUTPUT);
    pinMode(mot1_INB, OUTPUT);
    pinMode(mot1_PWM, OUTPUT);
    pinMode(mot2_INA, OUTPUT);
    pinMode(mot2_INB, OUTPUT);
    pinMode(mot2_PWM, OUTPUT);

    digitalWrite(mot1_INA, LOW);
    digitalWrite(mot1_INB, HIGH);
    analogWrite(mot1_PWM, 255);
    delay(1000);
    digitalWrite(mot1_INA, LOW);
    digitalWrite(mot1_INB, LOW);
    analogWrite(mot1_PWM, 0);
    delay(1000);

    myservo.write(30); // Initial low throttle signal to arm the ESC
    delay(5000);       // Wait for 5 seconds to allow the ESC to initialize
}

void loop()
{

    myservo.write(80); // Send mapped value to ESC

    delay(15); // Small delay for stability
}