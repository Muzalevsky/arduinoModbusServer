#include "HardwareObserver.h"
#include "Memory.h"
#include "Modbus.h"
#include "ModbusTCPIPServer.h"

#include <SPI.h>
#include <Ethernet.h>

uint8_t mac[] = { 0xC0, 0xFF, 0xEE, 0xD0, 0x0D, 0x00 };
IPAddress ip(192, 168, 13, 2);
EthernetServer ethServer(MODBUS_TCP_DEFAULT_PORT);

uint8_t requestFrame[MODBUS_TCP_MAX_ADU_LENGTH];
uint8_t answerFrame[MODBUS_TCP_MAX_ADU_LENGTH];

HardwareObserver hwObserver;
Memory memory{&hwObserver};
ModbusTCPIPServer modbusServer{&memory};

void printHex(const uint8_t* tab, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (tab[i] < 0x10)
        {
            Serial.print('0');
        }
        Serial.print(tab[i],HEX);
        Serial.print(" ");
    }
}

void printFrameHex(const uint8_t* tab, int len)
{
    printHex(tab, len);
    Serial.println();
}

void setup() 
{
    // Debug LED on 0x01 coil
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);

    Ethernet.begin(mac, ip);
    ethServer.begin();
}

int readFrame(EthernetClient* client, uint8_t* frame){
    int i = 0;
    do 
    {
        frame[i++] = client->read();
    }
    while(client->available());

    return i;
}

void loop() 
{
    EthernetClient client = ethServer.available();
    if (client) 
    {
        Serial.println("\n-------New client connected\n------");

        while (client.connected()) 
        {
            if (client.available()) 
            {
                Serial.println("\n-------New frame\n------");
                int requestLength = readFrame(&client, requestFrame);
                Serial.print("request:"); 
                printFrameHex(requestFrame, requestLength);

                int answerLength = modbusServer.processModbusRequest(requestFrame, requestLength, answerFrame);

                Serial.print("response:"); 
                printFrameHex(answerFrame, answerLength);
                client.write(answerFrame, answerLength);
            }
        }
        client.stop();
        Serial.println("client disconnected");
    }
}
