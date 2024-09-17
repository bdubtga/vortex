#include <SPI.h>
#include <Wire.h>
#include "RF24.h"
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define NEO_PIN 2         // On Trinket or Gemma, suggest changing this to 1
#define NEO_NUMPIXELS 2   // Popular NeoPixel ring size
#define NEO_BRIGHTNESS 50 // Adjust brightness to save power
#define RETRY_LIMIT 15
#define DATA_SIZE 12
#define MOTOR_STOP 0

Adafruit_NeoPixel pixels(NEO_NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
uint32_t colors[] = {pixels.Color(255, 0, 0), pixels.Color(0, 255, 0), pixels.Color(0, 0, 255)};

RF24 ReceiveRadio(9, 10);
Servo myservo;

int retries = 0;
int data[DATA_SIZE];

const int mot1_INA = 7;
const int mot1_INB = 8;
const int mot1_PWM = 5;
const int mot2_INA = A3;
const int mot2_INB = A4;
const int mot2_PWM = 6;

const int leftstick_buffer = 100;
const int leftstick_center = 1500;
const int rightstick_buffer = 100;
const int rightstick_center = 1500;

int armed = 0;

void setup()
{
    pixels.begin();
    pixels.setBrightness(NEO_BRIGHTNESS);
    Serial.begin(115200);

    setupRadio();
    setupMotors();
    setupServo();
    animatePixels();
}

void setupRadio()
{
    ReceiveRadio.begin();
    ReceiveRadio.setAddressWidth(5);
    ReceiveRadio.openReadingPipe(1, 0xF0F0F0F066LL);
    ReceiveRadio.setChannel(115);
    ReceiveRadio.setPALevel(RF24_PA_MAX);
    ReceiveRadio.setDataRate(RF24_250KBPS);
    ReceiveRadio.startListening();
}

void setupMotors()
{
    pinMode(mot1_INA, OUTPUT);
    pinMode(mot1_INB, OUTPUT);
    pinMode(mot1_PWM, OUTPUT);
    pinMode(mot2_INA, OUTPUT);
    pinMode(mot2_INB, OUTPUT);
    pinMode(mot2_PWM, OUTPUT);
}

void setupServo()
{
    myservo.attach(3);
    myservo.write(30);
    delay(1000);
}

void animatePixels()
{
    for (int i = 0; i < 3; i++)
    {
        pixels.setPixelColor(0, colors[i]);
        pixels.setPixelColor(1, colors[i]);
        pixels.show();
        delay(200);
    }
    pixels.clear();
    pixels.show();
}

void loop()
{
    if (ReceiveRadio.available())
    {
        retries = 0;
        ReceiveRadio.read(data, sizeof(data));
        processReceivedData();
    }
    else
    {
        handleRetries();
    }
}

void processReceivedData()
{
    printData();

    armed = (data[4] > 1500) ? 1 : 0;
    if (armed)
    {
        handleArmedState();
    }
    else
    {
        disarm();
    }
}

void printData()
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        Serial.print(data[i]);
        Serial.print(i < DATA_SIZE - 1 ? "  " : "\n");
    }
}

void handleArmedState()
{
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.setPixelColor(1, pixels.Color(0, 255, 0));
    pixels.show();

    if (data[7] > 1500)
    {
        pixels.setPixelColor(0, pixels.Color(255, 255, 0));
        pixels.setPixelColor(1, pixels.Color(255, 255, 0));
        pixels.show();
        myservo.write(map(data[11], 1500, 2000, 30, 160));
    }
    else
    {
        myservo.write(30);
    }

    handleMotors();
}

void handleMotors()
{
    handleLeftStick();
    handleRightStick();
}

void handleLeftStick()
{
    if (data[2] > leftstick_center + leftstick_buffer)
    {
        moveMotor(mot1_INA, mot1_INB, mot1_PWM, map(data[2], leftstick_center + leftstick_buffer, 2000, 0, 255), "forward");
    }
    else if (data[2] < leftstick_center - leftstick_buffer)
    {
        moveMotor(mot1_INA, mot1_INB, mot1_PWM, map(data[2], leftstick_center - leftstick_buffer, 1000, 0, 255), "backward");
    }
    else
    {
        stopMotor(mot1_INA, mot1_INB, mot1_PWM);
    }
}

void handleRightStick()
{
    if (data[1] > rightstick_center + rightstick_buffer)
    {
        moveMotor(mot2_INA, mot2_INB, mot2_PWM, map(data[1], rightstick_center + rightstick_buffer, 2000, 0, 255), "backward");
    }
    else if (data[1] < rightstick_center - rightstick_buffer)
    {
        moveMotor(mot2_INA, mot2_INB, mot2_PWM, map(data[1], rightstick_center - rightstick_buffer, 1000, 0, 255), "forward");
    }
    else
    {
        stopMotor(mot2_INA, mot2_INB, mot2_PWM);
    }
}

void moveMotor(int INA, int INB, int PWM, int pwmValue, const char *direction)
{
    if (strcmp(direction, "forward") == 0)
    {
        digitalWrite(INA, HIGH);
        digitalWrite(INB, LOW);
    }
    else if (strcmp(direction, "backward") == 0)
    {
        digitalWrite(INA, LOW);
        digitalWrite(INB, HIGH);
    }
    else
    {
        // Stop the motor if the direction is not recognized
        digitalWrite(INA, LOW);
        digitalWrite(INB, LOW);
    }

    analogWrite(PWM, pwmValue);
}

void stopMotor(int INA, int INB, int PWM)
{
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
    analogWrite(PWM, MOTOR_STOP);
}

void disarm()
{
    armed = 0;
    stopMotor(mot1_INA, mot1_INB, mot1_PWM);
    stopMotor(mot2_INA, mot2_INB, mot2_PWM);
    myservo.write(30);
    pixels.clear();
    pixels.show();
}

void handleRetries()
{
    retries++;
    Serial.print("Failed to receive data. Attempt: ");
    Serial.println(retries);

    if (retries > 5)
    {
        disarm();
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.setPixelColor(1, pixels.Color(255, 0, 0));
        pixels.show();
    }

    if (retries > RETRY_LIMIT)
    {
        ReceiveRadio.stopListening();
        ReceiveRadio.startListening();
    }
}
