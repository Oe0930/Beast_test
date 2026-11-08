#include "settings.hpp"
#include "PIDlib.cpp"
#include "LSM6DSV16XController.hpp"
#include "TC78H653FTGController/TC78H653FTGController.hpp"

PendrumSettings pendrumSettings;

PID pid(1.0, 0.1, 0.01, true);
LSM6DSV16XController imu(pendrumSettings.sdaPin, pendrumSettings.sclPin);

TC78H653FTGPins TCpins =
{
    pendrumSettings.in1Pin,
    pendrumSettings.in2Pin,
    0xFF, // IN3: 未指定
    0xFF, // IN4: 未指定
    0xFF  // STBY: 未指定
};
TC78H653FTGController drive(TCpins, false, false);


float gyroX = 0.0f;
uint32_t preUs = 0;

float target = 0;

float calculateGyroX()
{
    float dt = (micros() - preUs) / 1000000.0f;
    preUs = micros();

    float accelAngle = atan2f(imu.getAccel().y, imu.getAccel().z) * 180.0f / PI;
    float gyroAngle  = gyroX + (imu.getGyro().x / 1000.0f) * dt;

    constexpr float tau = 0.5f; // gyroをどれだけ参考にするかの時定数
    const float alpha = tau / (tau + dt);

    gyroX = alpha * gyroAngle + (1.0f - alpha) * accelAngle;
    return gyroX;
}

void setupTargetAngle()
{
    target = 0;
}

void setup()
{
    imu.setUp();
    imu.calibrate(false, true);

    drive.setUp();

    setupTargetAngle();
}

void loop()
{
    if (!imu.update(true, true))
    {
        return;
    }

    drive.drive(0, pid.calculate(target - calculateGyroX()));
}
