#pragma once

#include <Arduino.h>

// Arduino-ESP32 2.x LEDCによるサーボ制御。外部Servoライブラリは不要。
// 使用するチャンネルと、そのチャンネルが共有するタイマーは50Hz専用にする。
class Sg90Controller
{
private:
    int pin = -1;
    // 従来のESP32Servoのattach(pin)と同じ角度→パルス幅範囲。
    int minUs = 500;
    int maxUs = 2500;
    float currentAngle = 0;
    unsigned long lastUsedTime = 0;
    float lastAngle = -1;
    bool autoDetach = true;
    uint8_t channel;

public:
    int minAngle = 0;
    int maxAngle = 180;
    bool isAttached = false;

    explicit Sg90Controller(uint8_t pwmChannel = 14);
    void setUp(int pin, int defaultAngle, int _minAngle, int _maxAngle);
    void attach();
    void detach();
    void setAutoDetach(bool enabled);
    void set(float angle);
    // speedは1回の呼び出しで加算する角度。
    void move(float speed);
    void stop();
};

class RotationServoController
{
private:
    int pin = -1;
    int minUs = 700;
    int maxUs = 2300;
    int baseUs = 1500;
    int usRange = 800;
    int lastUs = -1;
    uint8_t channel;

public:
    bool isAttached = false;

    explicit RotationServoController(uint8_t pwmChannel = 15);
    void setUp(int pin, int _baseUs, int _usRange);
    void attach();
    void detach();
    // speedは-1〜1。0が中立。
    void move(float speed);
    void stop();
};
