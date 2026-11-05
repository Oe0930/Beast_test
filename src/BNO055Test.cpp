#include <Arduino.h>
#include <Adafruit_BNO055.h>
#include "BNO055Controller.hpp"

// ESP32 DevKitのI2C配線。使用するピンに合わせて変更する。
constexpr int sdaPin = 21;
constexpr int sclPin = 22;
constexpr uint8_t bnoAddress = 0x28; // ADRがHIGHの場合は0x29。
constexpr unsigned long sampleIntervalMs = 200;

BNO055Controller bno(sdaPin, sclPin, bnoAddress);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nBNO055 test");

    bno.Setup();
}

void loop()
{
    // オイラー角、加速度、校正状態、角速度、クォータニオンを更新する。
    bno.Update(true, true, true, true, true);

    Serial.printf("%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%u,%u,%u,%u\n",
                  bno.getEuler().x, bno.getEuler().y, bno.getEuler().z,
                  bno.getAccell().x, bno.getAccell().y, bno.getAccell().z,
                  bno.getCalibrationStatus().system, bno.getCalibrationStatus().gyro,
                  bno.getCalibrationStatus().accel, bno.getCalibrationStatus().mag);
    const Vector3<double> gyro = bno.getGyro();
    const Quaternion<double> quaternion = bno.getQuaternion();
    Serial.printf("Gyro[dps]: %.2f,%.2f,%.2f Quaternion[w,x,y,z]: %.4f,%.4f,%.4f,%.4f\n",
                  gyro.x, gyro.y, gyro.z,
                  quaternion.w, quaternion.x, quaternion.y, quaternion.z);
    delay(sampleIntervalMs);
}
