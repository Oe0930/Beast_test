#include "DRV8830Controller.hpp"

void DRV8830Controller::setUp()
{
    Wire.begin(sdapin, sclpin);
    Wire.setClock(100000);
    delay(100);

    setSpeed(0);
}

void DRV8830Controller::setSpeed(float speed)
{
    Wire.beginTransmission(address);

    if (isReversed) speed = -speed;
    speed = constrain(speed, -1.0f, 1.0f);

    uint8_t direction = speed == 0 ? 3 : (speed > 0 ? 1 : 2);
    float absSpeed = abs(speed);

    float voltage = minVol + (maxVol - minVol) * absSpeed;
    uint8_t voltageByte = (uint8_t)(voltage * ((float)100 / 8));
    voltageByte = constrain(voltageByte, 6, 63);
    

    Wire.write(0x00);
    Wire.write((voltageByte << 2) | direction);
    Wire.endTransmission();
}