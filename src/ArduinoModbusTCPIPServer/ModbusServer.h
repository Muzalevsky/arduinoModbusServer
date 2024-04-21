#pragma once

#include "Memory.h"

class ModbusServer
{
public:
    ModbusServer(Memory *memory);

    unsigned int processPDU(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
private:
    //function 0x01
    unsigned int readDiscreteOutputs(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x02
    unsigned int readDiscreteInputs(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x03
    unsigned int readHoldingRegisters(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x04
    unsigned int readInputRegisters(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x05
    unsigned int writeSingleOutput(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x06
    unsigned int writeSingleRegister(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x0F
    unsigned int writeMultipleOutputs(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //function 0x10
    unsigned int writeMultipleRegisters(const unsigned char* mb_pdu, unsigned int pdu_len, unsigned char* mb_rsp);
    //process the exception responses
    unsigned int mb_exception(unsigned char* mb_rsp, unsigned char code);

private:
    Memory *m_memory{nullptr};
};
