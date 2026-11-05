#pragma once
#include "Arduino.h"
class PID
{
public:
    PID(double, double, double, bool)
    {
    }
    void reset()
    {
    }
    float calculate(float)
    {
        return Test::pidOutput;
    }
};
