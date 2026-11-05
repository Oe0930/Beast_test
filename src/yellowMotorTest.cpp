#include <Arduino.h>
#include "TB6612FNGController/TB6612FNGController.hpp"

// A
uint8_t kIn1L = 17;
uint8_t kIn2L = 16;
uint8_t kPwmL = 3;
// B
uint8_t kIn1R = 18;
uint8_t kIn2R = 19;
uint8_t kPwmR = 13;

uint8_t kStandby = 21;

TB6612FNGController TTController(kIn1L, kIn2L, kPwmL, kIn1R, kIn2R, kPwmR, kStandby, false, false);

void setup() 
{
    TTController.setUp();
}

void loop() 
{
    int s = millis() / 500;

    TTController.setSpeed(s % 3 == 0 ? 1 : (s % 3 == 1 ? 0 : -1), true);
    TTController.setSpeed((s % 6)/2 == 0 ? 1 : ((s % 6)/2 == 1 ? 0 : -1), false);

    delay(250);
}

