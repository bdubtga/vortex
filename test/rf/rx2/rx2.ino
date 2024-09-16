#include <SPI.h>
#include "RF24.h"

#define DATA_SIZE 12

RF24 ReceiveRadio(9, 10);

int retries = 0;
int data[DATA_SIZE];

void setup()
{
    Serial.begin(115200);
    setupRadio();
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

void loop()
{
    if (ReceiveRadio.available())
    {
        retries = 0;
        ReceiveRadio.read(data, sizeof(data));
        processReceivedData();
    }
}

void processReceivedData()
{
    printData();
}

void printData()
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        Serial.print(data[i]);
        Serial.print(i < DATA_SIZE - 1 ? "  " : "\n");
    }
}