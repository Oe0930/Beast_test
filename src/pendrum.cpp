#include "settings.hpp"
#include "PIDlib.cpp"
#include "BNO055Controller.hpp"
#include "DRV8830Controller.hpp"

PendrumSettings pendrumSettings;

PID pid(1.0, 0.1, 0.01, true);
BNO055Controller bno(pendrumSettings.sdaPin, pendrumSettings.sclPin, pendrumSettings.bnoAddress);

DRV8830Controller drive(pendrumSettings.sdaPin, pendrumSettings.sclPin, false, pendrumSettings.drvAddress);


float gyroX = 0.0f;
uint32_t preUs = 0;

float target = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    //bno.setUp();
    //bno.calibrate(false, true);

    drive.setUp();

    //setupTargetAngle();
}

void setupTargetAngle()
{
    delay(1000);
    target = 0;
}

void loop()
{
    drive.setSpeed(0.5f);
    return;

    bno.Update(true, true);

    drive.setSpeed(pid.calculate(target - bno.getEuler().x));
}
