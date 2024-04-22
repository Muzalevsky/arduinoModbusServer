#include "HardwareObserver.h"
#include "Memory.h"
#include "Modbus.h"
#include "ModbusTCPIPServer.h"

#include <SPI.h>
#include <Ethernet.h>



/*
 * SSI encoder with 4096 ticks per revolution
 */
const int ticksPerRev = 4096; // 4096 ticks per revolution
const float ticks2degCoef = 0.0879; //360 / ticksPerRev;
const float overflowDeltaDeg = 180; // If delta angle is more than this value, we crossed zero

const int DATA_PIN = 7;  //  к пину 10 подключаем DO data
const int CLOCK_PIN = 8; // к пину 11 подключаем CLK
const int CS_PIN = 6;  // к пину 13 подключаем CS сигнал окончания посылки

float angle = 0;
/*
 *
 */








uint8_t mac[] = { 0xC0, 0xFF, 0xEE, 0xD0, 0x0D, 0x00 };
IPAddress ip(192, 168, 0, 90);
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
    pinMode(DATA_PIN, INPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(CS_PIN, OUTPUT);

    //give some default values
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CS_PIN, HIGH);
    Serial.begin(115200);




    // Debug LED on 0x01 coil
    pinMode(LED_BUILTIN, OUTPUT);


    // Write static variable
    memory.writeRegister(1, 123);


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
//         Serial.println("\n-------New client connected\n------");

        while (client.connected())
        {
            if (client.available())
            {
//                 Serial.println("\n-------New frame\n------");
                int requestLength = readFrame(&client, requestFrame);
//                 Serial.print("request:");
//                 printFrameHex(requestFrame, requestLength);

                int answerLength = modbusServer.processModbusRequest(requestFrame, requestLength, answerFrame);

//                 Serial.print("response:");
//                 printFrameHex(answerFrame, answerLength);
                client.write(answerFrame, answerLength);


                // TODO: move encoder processing to interrupt

                uint32_t rawData = shiftIn(DATA_PIN, CLOCK_PIN, 18);
                uint32_t angleData = rawData;
                uint32_t statusData = rawData & 0x1F;

                angleData >>= 5;

                // Write angle
                uint16_t shortAngleData[2];
                shortAngleData[0] = angleData >> 16;
                shortAngleData[1] = angleData & 0xFFFF;

                memory.writeRegisters(2, 2, shortAngleData);

                angle = (float)(angleData) * ticks2degCoef;

            //    Serial.print(" status: ");
            //    Serial.print(statusData, DEC);
            //
            //    Serial.print(" raw: ");
            //    Serial.print(rawData, DEC);
            //
            //    Serial.print(" angle: ");
            //    Serial.println(angle, DEC);
            }
        }
        client.stop();
//         Serial.println("client disconnected");
    }
}


//read in a byte of data from the digital input of the board.
unsigned long shiftIn(const int DATA_PIN, const int CLOCK_PIN, const int bit_count)
{
    digitalWrite(CS_PIN, LOW);
    // delay for Tclkfe

    unsigned long data = 0;
    for (int i=0; i<bit_count - 1; i++)
    {
        data <<= 1;
        digitalWrite(CLOCK_PIN, LOW);
//        PORTB &= ~(1 << 7);
//        delayMicroseconds(3);
        digitalWrite(CLOCK_PIN, HIGH);
//        PORTB |= 1 << 7;
//        delayMicroseconds(3);

        data |= digitalRead(DATA_PIN);
    }
    digitalWrite(CS_PIN, HIGH); //  CS это сигнал окончания передачи на энкодер
//    PORTC |= 1 << 7;

    delayMicroseconds(25);
    digitalWrite(CS_PIN, LOW);
//    PORTC &= ~(1 << 7);
//    delayMicroseconds(20);
    return data;
}
