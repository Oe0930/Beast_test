#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_LSM6DSV16X.h>
#include "Structs.hpp"

class LSM6DSV16XController
{
    private:
        SparkFun_LSM6DSV16X imu;
        int sdaPin;
        int sclPin;
        
        uint8_t imuAddress = 0;

        Vector3<float> accel;
        Vector3<float> accelOffset = {0.0f, 0.0f, 0.0f};
        Vector3<float> gyro;
        Vector3<float> gyroOffset = {0.0f, 0.0f, 0.0f};

    public:
        LSM6DSV16XController(int sdaPin, int sclPin) : sdaPin(sdaPin), sclPin(sclPin) {};

        bool setUp();
        bool update(bool isAccel = true, bool isGyro = true);

        Vector3<float> getAccel() { return accel - accelOffset; };
        Vector3<float> getGyro() { return gyro - gyroOffset; };

        void calibrate(bool isAccel = true, bool isGyro = true);
};