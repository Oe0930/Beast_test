#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

using std::size_t;
using std::abs;
namespace Test
{
    inline uint32_t now = 0;
    inline bool motorMoving = false;
    inline int driveCalls = 0;
    inline int stopCalls = 0;
    inline uint32_t lastDriveAt = 0;
    inline float pidOutput = 1;
    inline std::vector<std::string> messages;
}
class String
{
public:
    std::string text;
    String(const char* value) : text(value)
    {
    }
    String(double value) : text(std::to_string(value))
    {
    }
    String(const std::string& value) : text(value)
    {
    }
    friend String operator+(const String& left, const String& right)
    {
        return String(left.text + right.text);
    }
};
struct TestSerial
{
    void begin(int)
    {
    }
    void println(const String& value)
    {
        Test::messages.push_back(value.text);
    }
    template <typename... Args>
    void printf(const char*, Args...)
    {
    }
};
inline TestSerial Serial;
inline uint32_t millis()
{
    return Test::now;
}
inline void delay(uint32_t ms)
{
    Test::now += ms;
}
