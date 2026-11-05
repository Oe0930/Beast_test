#include "Sg90Controller.hpp"
#include <driver/gpio.h>
#include <math.h>

namespace
{
    constexpr uint32_t pwmHz = 50;
    constexpr uint8_t resolutionBits = 16;
    constexpr uint32_t periodUs = 1000000 / pwmHz;
    constexpr uint32_t periodTicks = 1UL << resolutionBits;

    uint32_t pulseDuty(int pulseUs)
    {
        // 16bit PWMは65536カウントで1周期。四捨五入してデューティへ変換。
        return (static_cast<uint32_t>(pulseUs) * periodTicks + periodUs / 2) / periodUs;
    }

    bool attachPwm(int pin, uint8_t channel)
    {
        if (!GPIO_IS_VALID_OUTPUT_GPIO(pin) || channel >= 16)
        {
            Serial.println("Servo: invalid output GPIO or LEDC channel");
            return false;
        }
        if (ledcSetup(channel, pwmHz, resolutionBits) == 0)
        {
            Serial.println("Servo: LEDC setup failed");
            return false;
        }
        ledcWrite(channel, 0);
        ledcAttachPin(pin, channel);
        return true;
    }

    void detachPwm(int pin, uint8_t channel)
    {
        ledcWrite(channel, 0);
        ledcDetachPin(pin);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}

Sg90Controller::Sg90Controller(uint8_t pwmChannel) : channel(pwmChannel)
{
}

void Sg90Controller::setUp(int _pin, int defaultAngle, int _minAngle, int _maxAngle)
{
    detach();
    pin = _pin;
    minAngle = constrain(_minAngle, 0, 180);
    maxAngle = constrain(_maxAngle, minAngle, 180);
    lastAngle = -1;
    set(defaultAngle);
}

void Sg90Controller::attach()
{
    if (isAttached || !attachPwm(pin, channel))
    {
        return;
    }
    isAttached = true;
    const int pulseUs = lroundf(minUs + currentAngle * (maxUs - minUs) / 180.0f);
    ledcWrite(channel, pulseDuty(pulseUs));
}

void Sg90Controller::detach()
{
    if (isAttached)
    {
        detachPwm(pin, channel);
        isAttached = false;
    }
}

void Sg90Controller::setAutoDetach(bool enabled)
{
    autoDetach = enabled;
}

void Sg90Controller::set(float angle)
{
    if (!isfinite(angle))
    {
        return;
    }
    currentAngle = constrain(angle, static_cast<float>(minAngle), static_cast<float>(maxAngle));
    if (currentAngle == lastAngle)
    {
        // 従来と同じく、同じ角度でset()を呼び続けた場合に自動解除する。
        if (autoDetach && millis() - lastUsedTime > 3000)
        {
            detach();
        }
        return;
    }
    attach();
    if (!isAttached)
    {
        return;
    }
    const int pulseUs = lroundf(minUs + currentAngle * (maxUs - minUs) / 180.0f);
    ledcWrite(channel, pulseDuty(pulseUs));
    lastAngle = currentAngle;
    lastUsedTime = millis();
}

void Sg90Controller::move(float speed)
{
    set(currentAngle + speed);
}

void Sg90Controller::stop()
{
    if (autoDetach)
    {
        detach();
    }
}

RotationServoController::RotationServoController(uint8_t pwmChannel) : channel(pwmChannel)
{
}

void RotationServoController::attach()
{
    if (isAttached || !attachPwm(pin, channel))
    {
        return;
    }
    isAttached = true;
    ledcWrite(channel, pulseDuty(lastUs < 0 ? baseUs : lastUs));
}

void RotationServoController::detach()
{
    if (isAttached)
    {
        detachPwm(pin, channel);
        isAttached = false;
    }
}

void RotationServoController::setUp(int _pin, int _baseUs, int _usRange)
{
    detach();
    pin = _pin;
    baseUs = constrain(_baseUs, 500, 2500);
    usRange = constrain(_usRange, 0, min(baseUs - 500, 2500 - baseUs));
    minUs = baseUs - usRange;
    maxUs = baseUs + usRange;
    lastUs = baseUs;
    // 初期化中の回転を防ぎ、move()が呼ばれるまで出力しない。
}

void RotationServoController::move(float speed)
{
    if (!isfinite(speed))
    {
        return;
    }
    speed = constrain(speed, -1.0f, 1.0f);
    const int targetUs = constrain(baseUs + static_cast<int>(speed * usRange), minUs, maxUs);
    // stop()やdetach()後も同じ指令で出力を再開できる。
    attach();
    if (isAttached)
    {
        ledcWrite(channel, pulseDuty(targetUs));
        lastUs = targetUs;
    }
}

void RotationServoController::stop()
{
    lastUs = baseUs;
    detach();
}
