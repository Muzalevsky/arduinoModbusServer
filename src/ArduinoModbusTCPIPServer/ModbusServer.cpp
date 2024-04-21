#include "ModbusServer.h"

ModbusServer::ModbusServer(Memory *memory) : m_memory{memory}
{
}

unsigned int ModbusServer::processPDU(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    // copy the function number
    mb_resp[0] = mb_pdu[0];

    switch (mb_pdu[0])
    {
        case MODBUS_FC_READ_COILS:                  //0x01
            return readDiscreteOutputs(mb_pdu, length, mb_resp);
        case MODBUS_FC_READ_DISCRETE_INPUTS:                   //0x02
            return readDiscreteInputs(mb_pdu, length, mb_resp);
        case MODBUS_FC_READ_HOLDING_REGISTERS:                   //0x03
            return readHoldingRegisters(mb_pdu, length, mb_resp);
        case MODBUS_FC_READ_INPUT_REGISTERS:                     //0x04
            return readInputRegisters(mb_pdu, length, mb_resp);
        case MODBUS_FC_WRITE_SINGLE_COIL:                  //0x05
            return writeSingleOutput(mb_pdu, length, mb_resp);
        case MODBUS_FC_WRITE_SINGLE_REGISTER:                     //0x06
            return writeSingleRegister(mb_pdu, length, mb_resp);
        case MODBUS_FC_WRITE_MULTIPLE_COILS:                   //0x0F
            return writeMultipleOutputs(mb_pdu, length, mb_resp);
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:                    //0x10
            return writeMultipleRegisters(mb_pdu, length, mb_resp);
        default:
            return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
    }
}

unsigned int ModbusServer::mb_exception(unsigned char* mb_resp, unsigned char code)
{
    mb_resp[0] |= 0x80;               //set the most significant bit to 1
    mb_resp[1] = code;                //set the corresponding error code
    return 2;                        //return the size of the response
}

unsigned int ModbusServer::readDiscreteOutputs(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short nOutputs = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (nOutputs < 0x0001 || nOutputs > 0x07D0)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->coilAddressValid(address) ||
        !m_memory->coilAddressValid(address + nOutputs))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    bool data[nOutputs];
    m_memory->readCoils(address, nOutputs, data);

    unsigned char byte = 0;
    unsigned int bit = 0;
    for (int i = 0; i < nOutputs; i++)
    {
        if (bit == 0)
        {
            //we are starting a new byte, it is needed to be initialized to 0
            mb_resp[2 + byte] = 0x00;
            byte++;
        }

        mb_resp[1 + byte] |= ((unsigned char)data[i] << bit);
        bit++;
        if (bit == 8)
        {
            bit = 0;
        }
    }

    mb_resp[1] = byte;
    
    return (byte + 2);  //1 byte function code, 1 byte Byte count, N bytes
                        //  Coil Status
                        //*N = Quantity of Coils / 8 (+1 if Qty%8 != 0)
}

unsigned int ModbusServer::readDiscreteInputs(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short nInputs = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (nInputs == 0 || nInputs > 0x07D0)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->coilAddressValid(address)
        || !m_memory->coilAddressValid(address + nInputs))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    bool data[nInputs];
    m_memory->readCoils(address, nInputs, data);

    unsigned char byte = 0;
    unsigned int bit = 0;
    for (int i = 0; i < nInputs; i++)
    {
        if (bit == 0)
        {
            mb_resp[2 + byte] = 0x00;
            byte++;
        }

        mb_resp[1 + byte] |= ((unsigned char)data[i] << bit);
        bit++;
        if (bit == 8)
        {
            bit = 0;
        }
    }
    mb_resp[1] = byte;
    return (byte + 2);  //1 byte function code, 1 byte Byte count,
                        //  N bytes Inputs Status
                        //*N = Quantity of Inputs / 8 (+1 if Qty%8 != 0)
}

unsigned int ModbusServer::readHoldingRegisters(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short nRegisters = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (nRegisters == 0 || nRegisters > 0x007D)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->registerAddressValid(address)
        || !m_memory->registerAddressValid(address + nRegisters))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    unsigned short data[nRegisters];
    m_memory->readRegisters(address, nRegisters, data);

    for (int i = 0; i < nRegisters; i++)
    {
        mb_resp[2 + i * 2] = (unsigned char)((data[i] & 0xFF00) >> 8);
        mb_resp[2 + i * 2 + 1] = (unsigned char)(data[i] & 0x00FF);
    }
    mb_resp[1] = nRegisters * 2;

    return (nRegisters * 2 + 2);   //1 byte function code,
                                    //1 byte Byte count, N bytes Register Values
                                    // N = Quantity of Registers *2
}

unsigned int ModbusServer::readInputRegisters(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short nRegisters = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (nRegisters == 0x0000 || nRegisters > 0x007D)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->registerAddressValid(address)
        || !m_memory->registerAddressValid(address + nRegisters))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    unsigned short data[nRegisters];
    m_memory->readRegisters(address, nRegisters, data);

    for (int i = 0; i < nRegisters; i++)
    {
        mb_resp[2 + i * 2] = (unsigned char)((data[i] & 0xFF00) >> 8);
        mb_resp[2 + i * 2 + 1] = (unsigned char)(data[i] & 0x00FF);
    }
    mb_resp[1] = nRegisters * 2;

    return (nRegisters * 2 + 2);   //1 byte function code, 1 byte Byte
                                        //      count, N bytes Register Values
                                        // N = Quantity of Registers *2
}

unsigned int ModbusServer::writeSingleOutput(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short outputValue = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (outputValue != 0x0000 && outputValue != 0xFF00)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->coilAddressValid(address))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    m_memory->writeCoil(address, (outputValue == 0xFF00));

    if (mb_resp != mb_pdu)
    {
        for (int i = 1; i < 5; i++)
        {
            mb_resp[i] = mb_pdu[i];
        }
    }
    return 5;
}

unsigned int ModbusServer::writeSingleRegister(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    // TODO: Strange check
    unsigned short registerValue = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (registerValue < 0x0000 || registerValue > 0xFFFF)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->registerAddressValid(address))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    m_memory->writeRegister(address, registerValue);

    if (mb_resp != mb_pdu)
    {
        for (int i = 1; i < 5; i++)
        {
            mb_resp[i] = mb_pdu[i];
        }
    }
    return 5;
}

unsigned int ModbusServer::writeMultipleOutputs(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 5)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short nOutputs = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (nOutputs == 0x0000 || nOutputs > 0x07B0)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned char n = nOutputs / 8;
    if (nOutputs % 8 != 0)
    {
        n++;
    }

    if ((n != mb_pdu[5]) || (length < 6 + mb_pdu[5]))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->coilAddressValid(address)
        || !m_memory->coilAddressValid(address + nOutputs))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    bool data[nOutputs];
    unsigned int bit = 0;
    unsigned char byte = 0;

    for (int i = 0; i < nOutputs; i++)
    {
        data[i] = ((mb_pdu[6 + byte] >> bit) & 0x01);
        bit++;
        if (bit == 8)
        {
            bit = 0;
            byte++;
        }
    }

    m_memory->writeCoils(address, nOutputs, data);

    for (int i = 1; i < 5; i++)
    {
        mb_resp[i] = mb_pdu[i];
    }
    return 5;
}

unsigned int ModbusServer::writeMultipleRegisters(const unsigned char* mb_pdu, unsigned int length, unsigned char* mb_resp)
{
    if (length < 7)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short nRegisters = ((unsigned short)(mb_pdu[3]) << 8) + mb_pdu[4];
    if (nRegisters == 0x0000 || nRegisters > 0x007B)
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned char n = nRegisters * 2;
    if (n != mb_pdu[5] || length < (6 + mb_pdu[5]))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);     //0x03
    }

    unsigned short address = ((unsigned short)(mb_pdu[1]) << 8) + mb_pdu[2];
    if (!m_memory->registerAddressValid(address)
        || !m_memory->registerAddressValid(address + nRegisters))
    {
        return mb_exception(mb_resp, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);   //0x02
    }

    unsigned short data[nRegisters];
    for (int i = 0; i < nRegisters; i++)
    {
        data[i] = (((unsigned short) mb_pdu[6 + i * 2] ) << 8) + mb_pdu[6 + i * 2 + 1];
    }

    m_memory->writeRegisters(address, nRegisters, data);

    for (int i = 1; i < 5; i++)
    {
        mb_resp[i] = mb_pdu[i];
    }

    return 5;
}
