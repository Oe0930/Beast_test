#include "LSM6DSV16XController.hpp"

bool LSM6DSV16XController::setUp()
{
    if (!Wire.begin(sdaPin, sclPin, 100000))
    {
        return false;
    }
    Wire.setTimeOut(100);
    
    // begin()は接続とデバイスIDを確認する。
    if (imu.begin(Wire, LSM6DSV16X_ADDRESS_LOW))
    {
        imuAddress = LSM6DSV16X_ADDRESS_LOW;
    }
    else if (imu.begin(Wire, LSM6DSV16X_ADDRESS_HIGH))
    {
        imuAddress = LSM6DSV16X_ADDRESS_HIGH;
    }
    else
    {
        return false;
    }

    if (!imu.deviceReset())
    {
        return false;
    }
    const uint32_t resetStartMs = millis();
    // このライブラリのgetDeviceReset()はリセット完了時にtrueを返す。
    while (!imu.getDeviceReset())
    {
        if (millis() - resetStartMs > 1000)
        {
            return false;
        }
        delay(2);
    }

    return imu.enableBlockDataUpdate()
        && imu.setAccelMode(LSM6DSV16X_XL_HIGH_PERFORMANCE_MD)
        && imu.setGyroMode(LSM6DSV16X_GY_HIGH_PERFORMANCE_MD)
        && imu.setAccelFullScale(LSM6DSV16X_2g)
        && imu.setGyroFullScale(LSM6DSV16X_250dps)
        && imu.setAccelDataRate(LSM6DSV16X_ODR_AT_120Hz)
        && imu.setGyroDataRate(LSM6DSV16X_ODR_AT_120Hz);
}

bool LSM6DSV16XController::update(bool isAccell, bool isGyro)
{
    if (!imu.checkStatus())
    {
        return false;
    }

    sfe_lsm_data_t _accel;
    sfe_lsm_data_t _gyro;
    if ((isAccell && !imu.getAccel(&_accel)) || (isGyro && !imu.getGyro(&_gyro)))
    {
        return false;
    }

    if (isAccell)
    {
        accel.x = _accel.xData;
        accel.y = _accel.yData;
        accel.z = _accel.zData;
    }
    if (isGyro)
    {
        gyro.x = _gyro.xData;
        gyro.y = _gyro.yData;
        gyro.z = _gyro.zData;
    }

    return true;
}

void LSM6DSV16XController::calibrate(bool isAccel, bool isGyro)
{
    if (!isGyro && !isAccel)
    {
        return;
    }

    uint32_t resetStartMs = millis();
    int sampleCount = 0;
    Vector3<float> accelSum = {0.0f, 0.0f, 0.0f};
    Vector3<float> gyroSum = {0.0f, 0.0f, 0.0f};

    while (millis() - resetStartMs < 1000)
    {
        if (update(isAccel, isGyro))
        {
            sampleCount++;

            if (isAccel)
            {
                accelSum.x += accel.x;
                accelSum.y += accel.y;
                accelSum.z += accel.z;
            }

            if (isGyro)
            {
                gyroSum.x += gyro.x;
                gyroSum.y += gyro.y;
                gyroSum.z += gyro.z;
            }
        }

        delay(1);
    }
    
    if (sampleCount > 0)
    {
        if (isAccel)
        {
            accelOffset.x = accelSum.x / sampleCount;
            accelOffset.y = accelSum.y / sampleCount;
            accelOffset.z = accelSum.z / sampleCount;
        }

        if (isGyro)
        {
            gyroOffset.x = gyroSum.x / sampleCount;
            gyroOffset.y = gyroSum.y / sampleCount;
            gyroOffset.z = gyroSum.z / sampleCount;
        }
    }
}