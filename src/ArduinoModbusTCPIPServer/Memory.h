#pragma once

#include "HardwareObserver.h"
#include "Modbus.h"

class Memory
{
public:
    Memory(HardwareObserver *hwObserver);

    bool coilAddressValid(unsigned char i) const;
    bool registerAddressValid(unsigned char i) const;

    void readCoils(unsigned char address, unsigned char count, bool * data) const;
    void readRegisters(unsigned char address, unsigned char count, unsigned short * data) const;

    void writeCoil(unsigned char address, bool value);
    void writeRegister(unsigned char address, unsigned short value);
    void writeCoils(unsigned char address, unsigned char count, bool* data);
    void writeRegisters(unsigned char address, unsigned char count, unsigned short* data);

private:
    const static unsigned char m_coilNumber{50};
    const static unsigned char m_registerNumber{20};

    HardwareObserver *m_hwObserver{nullptr};

    bool m_coils[m_coilNumber];
    unsigned short m_registers[m_registerNumber];
};
