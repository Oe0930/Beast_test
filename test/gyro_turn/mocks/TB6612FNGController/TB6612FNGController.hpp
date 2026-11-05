#pragma once
#include "Arduino.h"
class TB6612FNGController
{
public:
    TB6612FNGController(int, int, int, int, int, int, bool, bool)
    {
    }
    void setUp()
    {
    }
    void stop()
    {
        ++Test::stopCalls;
        Test::motorMoving = false;
    }
    void drive(float, float power)
    {
        ++Test::driveCalls;
        Test::lastDriveAt = millis();
        Test::motorMoving = power != 0;
    }
};
