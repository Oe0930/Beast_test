#pragma once

#include <Arduino.h>
#include <Wire.h>

class DRV8830Controller
{
private:
    int sdapin;
    int sclpin;

    bool isReversed;

    int address;

    float maxVol = 1.45f;
    float minVol = 0.0f;

public:
    DRV8830Controller(
        int sdapin,
        int sclpin,
        bool isReversed,
        int address)
        : sdapin(sdapin), sclpin(sclpin), isReversed(isReversed), address(address)
    {
    }

    void setUp();
    void setSpeed(float speed); // [-1,1], true: L, false: R
};
