#pragma once

#include <Arduino.h>
#include "Structs.hpp"

#include <Wire.h>
#include <Adafruit_BNO055.h>

/*
    lib_deps =
        adafruit/Adafruit BNO055
        adafruit/Adafruit Unified Sensor
*/

class BNO055Controller
{
    private :

    Adafruit_BNO055 bno;
    const int sdaPin;
    const int sclPin;
    const uint8_t bnoAddress;
    bool readRegisters(uint8_t address, uint8_t* data, uint8_t length);

    Vector3<double> euler;
    Vector3<double> gyro; // XYZ軸まわりの角速度（度/秒）。
    Quaternion<double> quaternion; // 姿勢を表すw, x, y, z（単位なし）。
    Vector3<double> accell;

    // 0    ：未校正
    // 1,2  ：校正の途中
    // 3    ：校正完了
    struct CalibrationStatus
    {
        uint8_t system = 0;
        uint8_t gyro = 0;
        uint8_t accel = 0;
        uint8_t mag = 0;
    } calibrationStatus;

    public:

    BNO055Controller(int sdaPin, int sclPin, uint8_t bnoAddress)
        : bno(55, bnoAddress, &Wire), sdaPin(sdaPin), sclPin(sclPin), bnoAddress(bnoAddress) {}

    void Setup();
    // 追加項目は末尾で指定する。既存の3引数の呼び出しも使用できる。
    void Update(bool iseular = true, bool isGyro = true, bool isQuaternion = true, bool isAccell = true, bool isCalibrationStatus = true);

    // 旋回制御用。通信・NDOF動作状態を確認し、成功時だけ角度を更新する。
    // falseの場合、以前の角度を使って走行を続けてはいけない。
    bool TryUpdateEuler();

    Vector3<double> getEuler() { return euler; }
    Vector3<double> getGyro() { return gyro; }
    Quaternion<double> getQuaternion() { return quaternion; }
    Vector3<double> getAccell() { return accell; }
    CalibrationStatus getCalibrationStatus() { return calibrationStatus; }

    bool reset = false;
};
