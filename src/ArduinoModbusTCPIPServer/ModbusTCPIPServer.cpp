#include "ModbusTCPIPServer.h"

ModbusTCPIPServer::ModbusTCPIPServer(Memory *memory) : ModbusServer(memory)
{
}

unsigned int ModbusTCPIPServer::processModbusRequest(unsigned char* mb_req, unsigned int requestLength, unsigned char* mb_resp)
{
    if (requestLength < 8 || mb_req == nullptr)
    {
        return 0;
    }

    // mb_req[0]   
    // mb_req[1] - Transaction Identifier

    // Protocol Identifier
    // The Protocol field is zero to indicate Modbus protocol.
    if (mb_req[2] != 0 || mb_req[3] != 0)
    {
        return 0;
    }

    // The Length field is the number of following bytes in request.
    unsigned short length = (mb_req[4] << 8) + (mb_req[5]);
    if (length != (requestLength - 6))
    {
        return 0;
    }

    if (mb_resp == nullptr)
    {
        mb_resp = mb_req;
    }
    else
    {
        mb_resp[0] = mb_req[0];
        mb_resp[1] = mb_req[1];
        mb_resp[2] = 0;
        mb_resp[3] = 0;

        // TODO: slave ID
        // Unit Address
        mb_resp[6] = mb_req[6];
    }

    // +7 means start of the message part
    // -1 because of unit address
    int responseLength = processPDU(mb_req + 7, length - 1, mb_resp + 7);

    // +1 because of the unit address
    mb_resp[4] = (((responseLength + 1) & 0xFF00) >> 8);
    mb_resp[5] = ((responseLength + 1) & 0x00FF);

    return responseLength + 7;
}
