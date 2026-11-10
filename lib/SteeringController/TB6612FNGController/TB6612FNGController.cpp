#include "TB6612FNGController.hpp"

void TB6612FNGController::setUp()
{
    //pinMode(pinStandby, OUTPUT);  
    if (isUsedA)
    {
        pinMode(pinIn1A, OUTPUT);
        pinMode(pinIn2A, OUTPUT);
        pinMode(pinPwmA, OUTPUT);
    }
    if (isUsedB)
    {
        pinMode(pinIn1B, OUTPUT);
        pinMode(pinIn2B, OUTPUT);
        pinMode(pinPwmB, OUTPUT);
    }

    //digitalWrite(pinStandby, LOW);
    
    stop();
}

void TB6612FNGController::setSpeed(float speed, bool isA)
{
    if ((isA && !isUsedA) || (!isA && !isUsedB))
    {
        return;
    }
    if ((isA && isReversedA) || (!isA && isReversedB))
    {
        speed = -speed;
    }
    speed = constrain(speed, -1.0, 1.0);

    digitalWrite(isA ? pinIn1A : pinIn1B, speed > 0 ? HIGH : LOW);
    digitalWrite(isA ? pinIn2A : pinIn2B, speed > 0 ? LOW : HIGH);
    analogWrite(isA ? pinPwmA : pinPwmB, abs(speed) * 255);
    //digitalWrite(pinStandby, HIGH);
}

void TB6612FNGController::stop()
{
    if (isUsedA)
    {
        digitalWrite(pinIn1A, LOW);
        digitalWrite(pinIn2A, LOW);
        analogWrite(pinPwmA, 0);
    }
    if (isUsedB)
    {
        digitalWrite(pinIn1B, LOW);
        digitalWrite(pinIn2B, LOW);
        analogWrite(pinPwmB, 0);
    }
    //digitalWrite(pinStandby, LOW);
}
