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

void setup()
{
    pixels.begin();       // INITIALIZE NeoPixel strip object (REQUIRED)
    Serial.begin(115200); // Start serial monitor (115200 for RF Nano)
    // Configure and start RF radio
    ReceiveRadio.begin();
    ReceiveRadio.setAddressWidth(5);
    ReceiveRadio.openReadingPipe(1, 0xF0F0F0F066LL);
    ReceiveRadio.setChannel(115);           // 115 band above WIFI signals
    ReceiveRadio.setPALevel(RF24_PA_MAX);   // Max to provide maximum range
    ReceiveRadio.setDataRate(RF24_250KBPS); // experiment with this to try get more range
    ReceiveRadio.startListening();          // Changes RF module to recieve mode

    // Define an array of colors (R, G, B)

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
            }
            else if (data[7] < 1500)
            {
                // weapon off
            }
        }
        else
        {
            // disarmed
            pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Red color
            pixels.setPixelColor(1, pixels.Color(0, 0, 0)); // Red color
            pixels.show();
        }
    }
    else
    {
        retries++;
        Serial.print("Faile to recieve data. Attempt: ");
        Serial.println(retries);
        if (retries > 2)
        {
            for (int i = 0; i < 1; i++)
            {
                // Turn on the lights
                pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red color
                pixels.setPixelColor(1, pixels.Color(255, 0, 0)); // Red color
                pixels.show();
                delay(500); // Wait for delayTime milliseconds

                // Turn off the lights
                pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
                pixels.setPixelColor(1, pixels.Color(0, 0, 0)); // Off
                pixels.show();
                delay(500); // Wait for delayTime milliseconds again
            }
        }
        if (retries > 15)
        {
            ReceiveRadio.stopListening();
            ReceiveRadio.startListening();
        }
    }
}