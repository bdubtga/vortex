//   ________  _______   ________         ___  ___          ___       __   ________  ___       ___       ___  ________
//  |\   __  \|\  ___ \ |\   ___  \      |\  \|\  \        |\  \     |\  \|\   __  \|\  \     |\  \     |\  \|\   ____\     
//  \ \  \|\ /\ \   __/|\ \  \\ \  \     \ \  \ \  \       \ \  \    \ \  \ \  \|\  \ \  \    \ \  \    \ \  \ \  \___|_
//   \ \   __  \ \  \_|/_\ \  \\ \  \  __ \ \  \ \  \       \ \  \  __\ \  \ \   __  \ \  \    \ \  \    \ \  \ \_____  \   
//    \ \  \|\  \ \  \_|\ \ \  \\ \  \|\  \\_\  \ \  \       \ \  \|\__\_\  \ \  \ \  \ \  \____\ \  \____\ \  \|____|\  \  
//     \ \_______\ \_______\ \__\\ \__\ \________\ \__\       \ \____________\ \__\ \__\ \_______\ \_______\ \__\____\_\  \ 
//      \|_______|\|_______|\|__| \|__|\|________|\|__|        \|____________|\|__|\|__|\|_______|\|_______|\|__|\_________\
//                                                                                                              \|_________|
#include <RF24.h> //librarys for rf nano
#include <RF24_config.h>
#include <nRF24L01.h>
#include <PPMReader.h>

const int datanum = 10;
int data[datanum];

RF24 SendRadio(9, 10);

byte ppmPin = 3;
byte ppmChannels = 10;
PPMReader ppm(ppmPin, ppmChannels);

void setup()
{
    Serial.begin(115200); // Start serial communication. (115200 baud for RF-NANO)

    SendRadio.begin();                         // Start the nRF24L01+ module.
    SendRadio.setAddressWidth(5);              // Set the address width to 5 bytes.
    SendRadio.openWritingPipe(0xF0F0F0F066LL); // Sends data on this 40-bit address
    SendRadio.setChannel(115);                 // 115 band above WIFI signals
    SendRadio.setPALevel(RF24_PA_MAX);         // Lower power level to reduce interference issues
    SendRadio.setDataRate(RF24_250KBPS);       // Lower data rate to increase range and stability
    SendRadio.stopListening();                 // Stop Receiving and start transmitting
}

void loop()
{
    for (byte channel = 1; channel <= ppmChannels; ++channel)
    {
        unsigned value = ppm.latestValidChannelValue(channel, 0);
        data[channel - 1] = value; // Assuming 'data' is an array of appropriate size and type
    }
    SendRadio.openWritingPipe(0xF0F0F0F066); // Sends data on this 40-bit address
    SendRadio.write(data, datanum * sizeof(int));
    ; // Send the data array.
    for (int i = 0; i < ppmChannels; ++i)
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
}