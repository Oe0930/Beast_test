#include "settings.hpp"
#include <Adafruit_BNO055.h>
#include "BNO055Controller.hpp"
#include "TB6612FNGController/TB6612FNGController.hpp"
#include "PIDlib.cpp"
#include <math.h>
 
// ESP32 DevKitのI2C配線。使用するピンに合わせて変更する。

RadiconSettings settings;
BNO055Controller bno(settings.sdaPin, settings.sclPin, settings.bnoAddress);

TB6612FNGController drive(settings.AIN1, settings.AIN2, settings.PWMA, settings.BIN1, settings.BIN2, settings.PWMB, false, false);
PID gyroPID(1.0, 0.0, 0.0, true); // 仮のPIDパラメータ、必要に応じて調整
constexpr uint32_t turnTimeoutMs = 10000;
bool turnFault = false;

void print(String message)
{
    Serial.println(message);
}

bool gyroTurn(int target)
{
    drive.stop();
    print("Starting gyro turn");
    float dassyutujoukennnohanni = 1.0f;

    gyroPID.reset();

    print("Finish reset");
    
    uint32_t preMillis = millis();
    while(true)
    {
        if (!bno.TryUpdateEuler())
        {
            drive.stop();
            print("Gyro turn aborted: IMU read/state error");
            continue;
        }
        
        const float currentAngle = bno.getEuler().x;
        const float error = target - currentAngle;

        if(abs(error) < dassyutujoukennnohanni)
        {
            drive.stop();
            break;
        }

        const float output = gyroPID.calculate(error);
        if (!isfinite(output))
        {
            drive.stop();
            print("Gyro turn aborted: invalid PID output");
            return false;
        }
        drive.drive(1, output);

        if (millis() - preMillis > 500)
        {
            print("X : " + String(bno.getEuler().x) + " Y : " + String(bno.getEuler().y) + " Z : " + String(bno.getEuler().z));
            preMillis = millis();
        }
        delay(10);
    }

    print("Finished gyro turn");
    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nBNO055 test");

    drive.setUp();
    drive.stop();
    bno.Setup();
}

void loop()
{
    // 異常後は再起動するまで次の旋回を開始しない。
    if (turnFault)
    {
        delay(100);
        return;
    }
    if (!gyroTurn(90))
    {
        turnFault = true;
        return;
    }
    delay(10);
    if (!gyroTurn(270))
    {
        turnFault = true;
        return;
    }
    delay(10);
}
