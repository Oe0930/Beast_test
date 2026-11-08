#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_LSM6DSV16X.h>
#include "LSM6DSV16XController.hpp"
#include "settings.hpp"

// 秋月 AE-LSM6DSV16X / ESP32 DevKit (3.3 V)
// 3V3 -> 3V3, GND -> GND, SDA -> GPIO21, SCL -> GPIO22
// CS -> 3V3（J4は開放）、SDO/SA0 -> GNDで0x6A。
constexpr uint32_t printIntervalMs = 100;

uint32_t lastPrintMs = 0;
uint32_t lastSampleMs = 0;

PendrumSettings pendrumSettings;
LSM6DSV16XController imu(pendrumSettings.sdaPin, pendrumSettings.sclPin);
float gyroX = 0.0f;


uint32_t preUs = 0;
void calculateGyroX()
{
    float dt = (micros() - preUs) / 1000000.0f;
    preUs = micros();

    float accelAngle = atan2f(imu.getAccel().y, imu.getAccel().z) * 180.0f / PI;
    float gyroAngle  = gyroX + (imu.getGyro().x / 1000.0f) * dt;

    constexpr float tau = 0.5f; // gyroをどれだけ参考にするかの時定数
    const float alpha = tau / (tau + dt);

    gyroX = alpha * gyroAngle + (1.0f - alpha) * accelAngle;
}



void setup()
{
    Serial.begin(115200);
    delay(1000);
    if (!imu.setUp())
    {
        Serial.println("LSM6DSV16X not found or initialization failed.");
        while (true)
        {
            delay(1000);
        }
    }

    imu.calibrate(false, true);
}

void loop()
{
    if (!imu.update(true, true))
    {
        Serial.println("Failed to update IMU data.");
        return;
    }

    calculateGyroX();

    if (millis() - lastPrintMs >= printIntervalMs)
    {
        lastPrintMs = millis();
        // ライブラリの戻り値はmgとmdps。gとdeg/sに換算して表示する。
        Serial.printf("%.4f,%.4f,%.4f,%.2f,%.2f,%.2f\n",
                      imu.getAccel().x / 1000.0f, imu.getAccel().y / 1000.0f, imu.getAccel().z / 1000.0f,
                      imu.getGyro().x / 1000.0f, imu.getGyro().y / 1000.0f, imu.getGyro().z / 1000.0f);
        Serial.printf("GyroX : %.2f \n", gyroX);
    }
}