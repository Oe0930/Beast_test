#pragma once

class ISteeringController
{
    public:
        virtual ~ISteeringController() = default;
        virtual void setUp() = 0;
        virtual void setSpeed(float speed, bool isL) = 0; // [-1,1], true: L, false: R
        virtual void stop();
        void drive(float steering, float speed); // steering = [-1,1], speed = [-1,1]（負数で後退）
};
