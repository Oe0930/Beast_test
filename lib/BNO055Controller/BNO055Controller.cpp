#include "BNO055Controller.hpp"

void BNO055Controller::Setup()
{
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(100000);
    Wire.setTimeOut(100);

    if (!bno.begin())
    {
        Serial.printf("BNO055 not found at 0x%02X. Check power, SDA, SCL and ADR.\n", bnoAddress);
        while (true)
        {
            delay(1000);
        }
    }
}

bool BNO055Controller::readRegisters(uint8_t address, uint8_t* data, uint8_t length)
{
    Wire.beginTransmission(bnoAddress);
    Wire.write(address);
    const uint8_t error = Wire.endTransmission(false);
    if (error != 0)
    {
        Serial.printf("BNO055 register 0x%02X: I2C error %u\n", address, error);
        return false;
    }
    const size_t received = Wire.requestFrom(bnoAddress, length);
    if (received != length)
    {
        Serial.printf("BNO055 register 0x%02X: short read %u/%u bytes\n",
                      address, static_cast<unsigned>(received), length);
        while (Wire.available())
        {
            Wire.read();
        }
        return false;
    }
    for (uint8_t i = 0; i < length; ++i)
    {
        data[i] = Wire.read();
    }
    return true;
}

bool BNO055Controller::TryUpdateEuler()
{
    // Page 0: SYS_STATUS(0x39), SYS_ERR, UNIT_SEL, reserved, OPR_MODE。
    uint8_t status[5];
    if (!readRegisters(0x39, status, sizeof(status)))
    {
        return false;
    }
    if (status[0] != 5 || status[1] != 0 ||
        status[4] != OPERATION_MODE_NDOF || (status[2] & 0x04) != 0)
    {
        Serial.printf("BNO055 not ready: status=%u error=%u units=0x%02X mode=0x%02X\n",
                      status[0], status[1], status[2], status[4]);
        return false;
    }

    uint8_t data[6];
    if (!readRegisters(0x1A, data, sizeof(data)))
    {
        return false;
    }
    // レジスタはリトルエンディアン、1度=16LSB。
    const auto decode = [](const uint8_t* bytes)
    {
        const uint16_t word = static_cast<uint16_t>(bytes[0]) |
                              (static_cast<uint16_t>(bytes[1]) << 8);
        return static_cast<int16_t>(word) / 16.0;
    };
    euler.x = decode(data);
    euler.y = decode(data + 2);
    euler.z = decode(data + 4);
    return true;
}

void BNO055Controller::Update(bool iseular, bool isGyro, bool isQuaternion,
                              bool isAccell, bool isCalibrationStatus)
{
    if (iseular)
    {
        imu::Vector<3> eulerAngles = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
        euler.x = eulerAngles.x();
        euler.y = eulerAngles.y();
        euler.z = eulerAngles.z();
    }

    if (isAccell)
    {
        imu::Vector<3> accelVector = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
        accel.x = accelVector.x();
        accel.y = accelVector.y();
        accel.z = accelVector.z();
    }

    if (isCalibrationStatus)
    {
        bno.getCalibration(&calibrationStatus.system, &calibrationStatus.gyro, &calibrationStatus.accel, &calibrationStatus.mag);
    }

    if (isGyro)
    {
        imu::Vector<3> gyroVector = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
        gyro.x = gyroVector.x();
        gyro.y = gyroVector.y();
        gyro.z = gyroVector.z();
    }

    if (isQuaternion)
    {
        imu::Quaternion quaternionValue = bno.getQuat();
        quaternion.w = quaternionValue.w();
        quaternion.x = quaternionValue.x();
        quaternion.y = quaternionValue.y();
        quaternion.z = quaternionValue.z();
    }
}
