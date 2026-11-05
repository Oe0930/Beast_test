#include "TC78H653FTGController.hpp"

void TC78H653FTGController::setUp()
{
    digitalWrite(pins.standby, LOW);
    pinMode(pins.standby, OUTPUT);
    digitalWrite(pins.enableA, LOW);
    digitalWrite(pins.enableB, LOW);
    digitalWrite(pins.phaseA, LOW);
    digitalWrite(pins.phaseB, LOW);
    pinMode(pins.enableA, OUTPUT);
    pinMode(pins.enableB, OUTPUT);
    pinMode(pins.phaseA, OUTPUT);
    pinMode(pins.phaseB, OUTPUT);
    stop();
}

void TC78H653FTGController::setSpeed(float speed, bool isA)
{
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
    digitalWrite(pins.standby, HIGH);
}

void TC78H653FTGController::stop()
{
    digitalWrite(pins.standby, LOW);
    analogWrite(pins.enableA, 0);
    analogWrite(pins.enableB, 0);
}
