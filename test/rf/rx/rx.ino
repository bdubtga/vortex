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
RF24 ReceiveRadio(9, 10);

byte value[32];

void ReadData()
{
    uint8_t bytes;
    if (ReceiveRadio.available())
    {
        while (ReceiveRadio.available())
        {
            bytes = ReceiveRadio.getPayloadSize();
            ReceiveRadio.read(value, bytes);
        }
        Serial.print("ReadData");
        Serial.print(".........");
        Serial.println(value[0]);
    }
    else {
      Serial.println("fail");
    }
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    printf_begin();
    Serial.println(F("LGT RF-NANO v2.0 Receive Test"));

    //
    // Setup and configure rf radio
    //
    ReceiveRadio.begin();
    ReceiveRadio.setAddressWidth(5);
    ReceiveRadio.openReadingPipe(1, 0xF0F0F0F066LL);
    ReceiveRadio.setChannel(115);         // 115 band above WIFI signals
    ReceiveRadio.setPALevel(RF24_PA_MAX); // MIN power low rage
    ReceiveRadio.setDataRate(RF24_1MBPS); // Minimum speed
    ReceiveRadio.startListening();
    Serial.println("Receive Setup Initialized");
    ReceiveRadio.printDetails();
    delay(500);
}

void loop()
{
    ReadData();
}