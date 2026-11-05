#pragma once

#include <Arduino.h>
#include "../ISteeringController.hpp"

// ラジコン
class TB6612FNGController : public ISteeringController
{
    private:
        // L
        uint8_t pinIn1A;
        uint8_t pinIn2A;
        uint8_t pinPwmA;
        // R
        uint8_t pinIn1B;
        uint8_t pinIn2B;
        uint8_t pinPwmB;

        //uint8_t pinStandby;

        bool isReversedA;
        bool isReversedB;



    public:
        TB6612FNGController(int pinIn1A, int pinIn2A, int pinPwmA, int pinIn1B, int pinIn2B, int pinPwmB, bool isReversedA, bool isReversedB) : 
            pinIn1A(pinIn1A), pinIn2A(pinIn2A), pinPwmA(pinPwmA), pinIn1B(pinIn1B), pinIn2B(pinIn2B), pinPwmB(pinPwmB), isReversedA(isReversedA), isReversedB(isReversedB)
        {
        }
        
        void setUp() override;
        void setSpeed(float speed, bool isA) override; // speed = [-1,1], A : L, B : R
        void stop() override;
};

