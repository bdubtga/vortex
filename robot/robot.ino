//   ________  _______   ________         ___  ___          ___       __   ________  ___       ___       ___  ________
//  |\   __  \|\  ___ \ |\   ___  \      |\  \|\  \        |\  \     |\  \|\   __  \|\  \     |\  \     |\  \|\   ____\     
//  \ \  \|\ /\ \   __/|\ \  \\ \  \     \ \  \ \  \       \ \  \    \ \  \ \  \|\  \ \  \    \ \  \    \ \  \ \  \___|_
//   \ \   __  \ \  \_|/_\ \  \\ \  \  __ \ \  \ \  \       \ \  \  __\ \  \ \   __  \ \  \    \ \  \    \ \  \ \_____  \   
//    \ \  \|\  \ \  \_|\ \ \  \\ \  \|\  \\_\  \ \  \       \ \  \|\__\_\  \ \  \ \  \ \  \____\ \  \____\ \  \|____|\  \  
//     \ \_______\ \_______\ \__\\ \__\ \________\ \__\       \ \____________\ \__\ \__\ \_______\ \_______\ \__\____\_\  \ 
//      \|_______|\|_______|\|__| \|__|\|________|\|__|        \|____________|\|__|\|__|\|_______|\|_______|\|__|\_________\
//                                                                                                              \|_________|
#include <SPI.h>
#include <Wire.h>
#include "RF24.h"
#include "printf.h"
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define NEO_PIN 2       // On Trinket or Gemma, suggest changing this to 1
#define NEO_NUMPIXELS 2 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NEO_NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
uint32_t colors[] = {pixels.Color(255, 0, 0), pixels.Color(0, 255, 0), pixels.Color(0, 0, 255)};

const int datanum = 10; // Number of data points in array.
int data[datanum];      // Array to store data/

int brightness = 0;

RF24 ReceiveRadio(9, 10);
int retries = 0;

int mot1_INA = 7;
int mot1_INB = 8;
int mot1_PWM = 5;
int mot2_INA = A3;
int mot2_INB = A4;
int mot2_PWM = 6;

int leftstick_buffer = 100;
int leftstick_center = 1500;
int rightstick_buffer = 100;
int rightstick_center = 1500;

int armed = 0;

Servo myservo;

void setup()
{
    pixels.begin();       // INITIALIZE NeoPixel strip object (REQUIRED)
    Serial.begin(115200); // Start Serial monitor (115200 for RF Nano)
    // Configure and start RF radio
    ReceiveRadio.begin();
    ReceiveRadio.setAddressWidth(5);
    ReceiveRadio.openReadingPipe(1, 0xF0F0F0F066LL);
    ReceiveRadio.setChannel(115);           // 115 band above WIFI signals
    ReceiveRadio.setPALevel(RF24_PA_MAX);   // Max to provide maximum range
    ReceiveRadio.setDataRate(RF24_250KBPS); // experiment with this to try get more range
    ReceiveRadio.startListening();          // Changes RF module to recieve mode

    // Set the motor pins as outputs
    pinMode(mot1_INA, OUTPUT);
    pinMode(mot1_INB, OUTPUT);
    pinMode(mot1_PWM, OUTPUT);
    pinMode(mot2_INA, OUTPUT);
    pinMode(mot2_INB, OUTPUT);
    pinMode(mot2_PWM, OUTPUT);

    myservo.attach(3);
    delay(1000);
    myservo.write(30);
    delay(5000);

    // Clear the pixels to start
    pixels.clear();

    // Loop through each color
    for (int i = 0; i < 3; i++)
    {
        // Set both pixels to the current color
        pixels.setPixelColor(0, colors[i]);
        pixels.setPixelColor(1, colors[i]);
        // Show the updated colors
        pixels.show();
        // Wait for 200 milliseconds
        delay(200);
    }
    delay(200);
    pixels.clear();
    pixels.show();
}

int arm()
{
    armed = 1;
}
int disarm()
{
    armed = 0;
    digitalWrite(mot1_INA, LOW);
    digitalWrite(mot1_INB, LOW);
    analogWrite(mot1_PWM, 0);
    digitalWrite(mot2_INA, LOW);
    digitalWrite(mot2_INB, LOW);
    analogWrite(mot2_PWM, 0);

    myservo.write(30);
}

void loop()
{

    if (ReceiveRadio.available())
    {
        retries = 0;
        ReceiveRadio.read(data, datanum * sizeof(int));
        for (int i = 0; i < 10; ++i)
        {
            if (i > 0)
                Serial.print("  ");
            Serial.print(data[i]);
            for (int j = 0; j < (5 - String(data[i]).length()); ++j)
            {
                Serial.print(" ");
            }
        }
        Serial.println(); // Move to the next line after printing all data

        if (data[4] > 1500)
        {
            // arm
            armed = 1;
        }
        else
        {
            armed = 0;
        }

        if (armed == 1)
        {
            // armed
            pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green color
            pixels.setPixelColor(1, pixels.Color(0, 255, 0)); // Green color
            pixels.show();
            if (data[7] > 1500)
            {
                // weapon on
                pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Red color
                pixels.setPixelColor(1, pixels.Color(255, 255, 0)); // Red color
                pixels.show();

                myservo.write(100);
            }
            else if (data[7] < 1500)
            {
                // weapon off
                myservo.write(30);
            }
            // Left stick handling
            if (data[2] > leftstick_center + leftstick_buffer)
            {
                // Move forward
                digitalWrite(mot1_INA, HIGH);
                digitalWrite(mot1_INB, LOW);

                int pwmValue = map(data[2], leftstick_center + leftstick_buffer, 2000, 0, 255);
                Serial.print("  mapL Forward: ");
                Serial.println(pwmValue);
                analogWrite(mot1_PWM, pwmValue);
            }
            else if (data[2] < leftstick_center - leftstick_buffer)
            {
                // Move backward
                digitalWrite(mot1_INA, LOW);
                digitalWrite(mot1_INB, HIGH);
                if (data[2] < 500)
                {
                    int pwmValue = 255;
                    Serial.print("  mapL Backward: ");
                    Serial.println(pwmValue);
                    analogWrite(mot1_PWM, pwmValue);
                }
                else if (data[2] > 500)
                {
                    int pwmValue = map(data[2], leftstick_center - leftstick_buffer, 1000, 0, 255);
                    Serial.print("  mapL Backward: ");
                    Serial.println(pwmValue);
                    analogWrite(mot1_PWM, pwmValue);
                }
            }
            else
            {
                // Stop motors
                digitalWrite(mot1_INA, LOW);
                digitalWrite(mot1_INB, LOW);
                analogWrite(mot1_PWM, 0);
            }

            // Right stick handling
            if (data[1] > rightstick_center + rightstick_buffer)
            {
                // Turn right
                digitalWrite(mot2_INA, LOW);
                digitalWrite(mot2_INB, HIGH);

                int pwmValue = map(data[1], rightstick_center + rightstick_buffer, 2000, 0, 255);
                Serial.print("  mapR Right: ");
                Serial.println(pwmValue);
                analogWrite(mot2_PWM, pwmValue);
            }
            else if (data[1] < rightstick_center - rightstick_buffer)
            {
                // Turn left
                digitalWrite(mot2_INA, HIGH);
                digitalWrite(mot2_INB, LOW);

                if (data[1] < 500)
                {
                    int pwmValue = 255;
                    Serial.print("  mapR Left: ");
                    Serial.println(pwmValue);
                    analogWrite(mot2_PWM, pwmValue);
                }
                else if (data[1] > 500)
                {
                    int pwmValue = map(data[1], rightstick_center - rightstick_buffer, 1000, 0, 255);
                    Serial.print("  mapR Left: ");
                    Serial.println(pwmValue);
                    analogWrite(mot2_PWM, pwmValue);
                }
            }
            else
            {
                // Stop turning motors
                digitalWrite(mot2_INA, LOW);
                digitalWrite(mot2_INB, LOW);
                analogWrite(mot2_PWM, 0);
            }
        }
        else
        {
            // disarmed
            pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Red color
            pixels.setPixelColor(1, pixels.Color(0, 0, 0)); // Red color
            pixels.show();

            digitalWrite(mot1_INA, LOW);
            digitalWrite(mot1_INB, LOW);
            analogWrite(mot1_PWM, 0);
            digitalWrite(mot2_INA, LOW);
            digitalWrite(mot2_INB, LOW);
            analogWrite(mot2_PWM, 0);

            myservo.write(30);
        }
    }
    else
    {
        retries++;
        Serial.print("Faile to recieve data. Attempt: ");
        Serial.println(retries);
        if (retries > 5)
        {
            for (int i = 0; i < 1; i++)
            {
                // Turn on the lights
                pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red color
                pixels.setPixelColor(1, pixels.Color(255, 0, 0)); // Red color
                pixels.show();

                // disarm the bot
                disarm();
            }
        }
        if (retries > 15)
        {
            ReceiveRadio.stopListening();
            ReceiveRadio.startListening();
        }
    }
}