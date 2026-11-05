#pragma once

#include <Arduino.h>
#include "../ISteeringController.hpp"

// 倒立振子
struct TC78H653FTGPins
{
    uint8_t enableA; // IN1/ENB_A: PWM
    uint8_t phaseA;  // IN2/PHA_A: direction
    uint8_t enableB; // IN3/ENB_B: PWM
    uint8_t phaseB;  // IN4/PHA_B: direction
    uint8_t standby;
};

class TC78H653FTGController : public ISteeringController
{
private:
    TC78H653FTGPins pins;
    bool isReversedA;
    bool isReversedB;

public:
    TC78H653FTGController(
        const TC78H653FTGPins& pins,
        bool isReversedA,
        bool isReversedB)
        : pins(pins), isReversedA(isReversedA), isReversedB(isReversedB)
    {
    }

    void setUp() override;
    void setSpeed(float speed, bool isA) override; // [-1,1], true: L, false: R
    void stop() override; // Outputs OFF; coast to a stop.
};
