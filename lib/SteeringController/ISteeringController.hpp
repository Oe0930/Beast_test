#pragma once

class ISteeringController
{
    protected:
        bool isUsedA = true;
        bool isUsedB = true;
        bool isReversedA = false;
        bool isReversedB = false;

    public:
        ISteeringController(bool reversedA = false, bool reversedB = false,
                            bool usedA = true, bool usedB = true)
            : isUsedA(usedA), isUsedB(usedB),
              isReversedA(reversedA), isReversedB(reversedB)
        {
        }

        virtual ~ISteeringController() = default;
        virtual void setUp() = 0;
        virtual void setSpeed(float speed, bool isL) = 0; // [-1,1], true: L, false: R
        virtual void stop() = 0;
        void drive(float steering, float speed); // steering = [-1,1], speed = [-1,1]（負数で後退）
};
