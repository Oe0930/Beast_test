#pragma once
#include "Arduino.h"
#include <array>

struct TestWire
{
    bool nack = false;
    bool badState = false;
    bool radians = false;
    uint8_t sensorError = 0;
    int failEulerAt = -1;
    int eulerReads = 0;
    uint32_t readDelay = 0;
    int16_t heading = 0;
    int16_t roll = 0;
    int16_t pitch = 0;
    uint8_t reg = 0;
    std::vector<uint8_t> received;
    size_t cursor = 0;

    void begin(int, int)
    {
    }
    void setClock(int)
    {
    }
    void setTimeOut(int)
    {
    }
    void beginTransmission(uint8_t)
    {
    }
    void write(uint8_t value)
    {
        reg = value;
    }
    uint8_t endTransmission(bool)
    {
        return nack ? 2 : 0;
    }
    size_t requestFrom(uint8_t, uint8_t)
    {
        Test::now += readDelay;
        cursor = 0;
        received.clear();
        if (reg == 0x39)
        {
            received =
            {
                static_cast<uint8_t>(badState ? 0 : 5), sensorError,
                static_cast<uint8_t>(radians ? 4 : 0), 0, 12
            };
        }
        else if (reg == 0x1A)
        {
            ++eulerReads;
            if (eulerReads == failEulerAt)
            {
                received =
                {
                    0, 0
                };
            }
            else
            {
                const int16_t values[] =
                {
                    heading, roll, pitch
                };
                for (const int16_t value : values)
                {
                    received.push_back(static_cast<uint8_t>(value));
                    received.push_back(static_cast<uint8_t>(static_cast<uint16_t>(value) >> 8));
                }
            }
        }
        return received.size();
    }
    int available()
    {
        return static_cast<int>(received.size() - cursor);
    }
    int read()
    {
        return received.at(cursor++);
    }
};
inline TestWire Wire;
