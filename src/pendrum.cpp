#include "settings.hpp"
#include "PIDlib.cpp"
#include "BNO055Controller.hpp"
#include "TC78H653FTGController/TC78H653FTGController.hpp"

PendrumSettings pendrumSettings;

PID pid(1.0, 0.1, 0.01, true);
BNO055Controller bno(pendrumSettings.sdaPin, pendrumSettings.sclPin, pendrumSettings.bnoAddress);

TC78H653FTGPins TCpins =
{
    pendrumSettings.in1Pin,
    pendrumSettings.in2Pin,
    0xFF, // IN3: 未指定
    0xFF, // IN4: 未指定
};
TC78H653FTGController drive(TCpins, false, false, true, false);


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
    drive.setSpeed(0.5f, true);
    return;

    bno.Update(true, true);

    drive.drive(0, pid.calculate(target - bno.getEuler().x));
}
