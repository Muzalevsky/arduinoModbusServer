#include "HardwareObserver.h"

#include "Arduino.h"

void HardwareObserver::setDebugLed(bool enabled)
{
    digitalWrite(LED_BUILTIN, enabled ? HIGH : LOW);
}

void HardwareObserver::setPositionValue(int value)
{

}

int HardwareObserver::positionValue()
{
    return 0;
}
