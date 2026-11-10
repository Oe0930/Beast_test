#include "ISteeringController.hpp"

void ISteeringController::drive(float steering, float speed)
{
    steering = (steering < -1.0 ? -1.0 : (steering > 1.0 ? 1.0 : steering));
    // 負の速度は後退指令。前進と同じ大きさで左右輪へ渡す。
    speed = (speed < -1.0 ? -1.0 : (speed > 1.0 ? 1.0 : speed));

    float speedL = speed * (steering >= 0 ? 1.0 : 1.0 + steering*2);
    float speedR = speed * (steering <= 0 ? 1.0 : 1.0 - steering*2);

    if (isUsedA)
    {
        setSpeed(speedL, true);
    }
    if (isUsedB)
    {
        setSpeed(speedR, false);
    }
}

void ISteeringController::stop()
{
    if (isUsedA)
    {
        setSpeed(0, true);
    }
    if (isUsedB)
    {
        setSpeed(0, false);
    }
}
