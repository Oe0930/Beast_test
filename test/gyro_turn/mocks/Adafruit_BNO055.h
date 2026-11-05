#pragma once
#include "Arduino.h"
#include "Wire.h"
constexpr uint8_t OPERATION_MODE_NDOF = 12;
namespace imu
{
    template <int N>
    struct Vector
    {
        double x()
        {
            return 0;
        }
        double y()
        {
            return 0;
        }
        double z()
        {
            return 0;
        }
    };
    struct Quaternion
    {
        double w()
        {
            return 1;
        }
        double x()
        {
            return 0;
        }
        double y()
        {
            return 0;
        }
        double z()
        {
            return 0;
        }
    };
}
class Adafruit_BNO055
{
public:
    enum VectorType
    {
        VECTOR_EULER,
        VECTOR_ACCELEROMETER,
        VECTOR_GYROSCOPE
    };
    Adafruit_BNO055(int, uint8_t, TestWire*)
    {
    }
    bool begin()
    {
        return true;
    }
    imu::Vector<3> getVector(VectorType)
    {
        return imu::Vector<3>();
    }
    imu::Quaternion getQuat()
    {
        return imu::Quaternion();
    }
    void getCalibration(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
    {
        *a = *b = *c = *d = 0;
    }
};
