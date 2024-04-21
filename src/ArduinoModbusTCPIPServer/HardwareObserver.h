#pragma once

class HardwareObserver
{
public:
    void setDebugLed(bool enabled);
    void setPositionValue(int value);

    int positionValue();
};
