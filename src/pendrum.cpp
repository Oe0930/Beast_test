#include "settings.hpp"
#include "PIDlib.cpp"
#include "LSM6DSV16XController.hpp"
#include "TC78H653FTGController/TC78H653FTGController.hpp"

PID pid(1.0, 0.1, 0.01, true);

constexpr int sdaPin = 21;
constexpr int sclPin = 22;
LSM6DSV16XController imu(sdaPin, sclPin);

TC78H653FTGPins TCpins;
TC78H653FTGController drive(TCpins, false, false);


float gyroX = 0.0f;
uint32_t preUs = 0;
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

void setup()
{
    imu.setUp();
    imu.calibrate(false, true);

    drive.setUp();
}

void loop()
{
    if (!imu.update(true, true))
    {
        return;
    }

    drive.drive(0, pid.calculate(0.0 - calculateGyroX()));
}
