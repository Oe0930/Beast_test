#include <Arduino.h>

class PID
{
    private:
    double dt = 1; // default time step
    double preMicros = micros();

    double kp;
    double ki;
    double kd;

    double I;
    double preError;

    bool isUseDt;

    public : 

    PID(double kp, double ki, double kd, bool isUseDt)
    {
        this->kp = kp;
        this->ki = ki;
        this->kd = kd;
        I = 0;
        preError = 0;
        this->isUseDt = isUseDt;
    };

    // errorには呼び出し側で計算した偏差（目標値 - 現在値）を渡す。
    double calculate(double error)
    {
        if (isUseDt)
        {
            dt = (micros() - preMicros) / 1000000.0;
            preMicros = micros();
        }

        dt = max(dt, 1e-6);
        
        I += error * dt;
        double D = (error - preError) / dt;
        preError = error;

        return kp * error + ki * I + kd * D;
    }

    void reset()
    {
        I = 0;
        preError = 0;
        preMicros = micros();
    }

    void setKp(double kp)
    {
        this->kp = kp;
    }

    void setKi(double ki)
    {
        this->ki = ki;
    }

    void setKd(double kd)
    {
        this->kd = kd;
    }
}; 
