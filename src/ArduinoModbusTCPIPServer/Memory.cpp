#include "Memory.h"

#include "Arduino.h"

Memory::Memory(HardwareObserver *hwObserver) :
m_hwObserver{hwObserver}
{
    for (unsigned char i = 0; i < m_coilNumber; i++)
    {
        m_coils[i] = false;
    }

    for (unsigned char i = 0; i < m_registerNumber; i++)
    {
        m_registers[i] = 0;
    }
}

bool Memory::coilAddressValid(unsigned char i) const
{
    return (i >= 0 && i < m_coilNumber);
}

bool Memory::registerAddressValid(unsigned char i) const
{
    return (i >= 0 && i < m_registerNumber);
}

void Memory::readCoils(unsigned char address, unsigned char count, bool * data) const
{
    for (auto i = 0; i < count; i++)
    {
        data[i] = m_coils[i + address];
    } 
}

void Memory::readRegisters(unsigned char address, unsigned char count, unsigned short * data) const
{
    for (int i = 0; i < count; i++)
    {
        data[i] = m_registers[i + address];
    }
}

void Memory::writeCoil(unsigned char address, bool value)
{
    m_coils[address] = value;

    if (address == 1)
    {
        m_hwObserver->setDebugLed(value);
    }
}

void Memory::writeRegister(unsigned char address, unsigned short value)
{
    m_registers[address] = value;
}

void Memory::writeCoils(unsigned char address, unsigned char count, bool * data)
{
    for (int i = 0; i < count; i++)
    {
        m_coils[address + i] = data[i];
    }
}

void Memory::writeRegisters(unsigned char address, unsigned char count, unsigned short* data)
{
    for (int i = 0; i < count; i++)
    {
        m_registers[address + i] = data[i];
    }
}
