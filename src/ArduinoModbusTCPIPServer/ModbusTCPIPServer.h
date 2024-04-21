#pragma once

#include "ModbusServer.h"

class ModbusTCPIPServer : public ModbusServer
{
public:
    ModbusTCPIPServer(Memory *memory);
    
    unsigned int processModbusRequest(unsigned char* mb_req, unsigned int len, unsigned char* mb_rep = nullptr);
};


