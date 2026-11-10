#include "TC78H653FTGController.hpp"

void TC78H653FTGController::setUp()
{
    if (isUsedA)
    {
        pinMode(pins.enableA, OUTPUT);
        pinMode(pins.phaseA, OUTPUT);
        digitalWrite(pins.enableA, LOW);
        digitalWrite(pins.phaseA, LOW);
    }
    if (isUsedB)
    {
        pinMode(pins.enableB, OUTPUT);
        pinMode(pins.phaseB, OUTPUT);
        digitalWrite(pins.enableB, LOW);
        digitalWrite(pins.phaseB, LOW);
    }
    stop();
}

void TC78H653FTGController::setSpeed(float speed, bool isA)
{
    if ((isA && !isUsedA) || (!isA && !isUsedB))
    {
        return;
    }
    if ((isA && isReversedA) || (!isA && isReversedB))
    {
        speed = -speed;
    }
    speed = constrain(speed, -1.0f, 1.0f);

    const uint8_t enable = isA ? pins.enableA : pins.enableB;
    const uint8_t phase = isA ? pins.phaseA : pins.phaseB;
    const uint8_t direction = speed > 0.0f ? HIGH : LOW;

    // Disable only this bridge before changing direction.
    if (digitalRead(phase) != direction)
    {
        analogWrite(enable, 0);
        digitalWrite(phase, direction);
    }

    analogWrite(enable, static_cast<int>(abs(speed) * 255.0f));
}

void TC78H653FTGController::stop()
{
    if (isUsedA)
    {
        analogWrite(pins.enableA, 0);
    }
    if (isUsedB)
    {
        analogWrite(pins.enableB, 0);
    }
}
