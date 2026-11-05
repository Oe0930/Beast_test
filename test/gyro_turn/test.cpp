// Hardware dependencies are mocked; these are the actual production functions.
#include "../../lib/BNO055Controller/BNO055Controller.cpp"
#include "../../src/gyroTurn.cpp"
#include <cassert>
#include <cstdio>
#include <limits>

void resetTest()
{
    Wire = TestWire();
    Test::now = 0;
    Test::motorMoving = false;
    Test::driveCalls = 0;
    Test::stopCalls = 0;
    Test::lastDriveAt = 0;
    Test::pidOutput = 1;
    Test::messages.clear();
    turnFault = false;
}

int main()
{
    // A valid zero heading must not be mistaken for a communication failure.
    resetTest();
    assert(bno.TryUpdateEuler());
    assert(bno.getEuler().x == 0);

    resetTest();
    Wire.heading = 1440;
    Wire.roll = -200;
    Wire.pitch = 320;
    assert(bno.TryUpdateEuler());
    assert(bno.getEuler().x == 90);
    assert(bno.getEuler().y == -12.5);
    assert(bno.getEuler().z == 20);
    Wire.failEulerAt = 2;
    assert(!bno.TryUpdateEuler());
    assert(bno.getEuler().x == 90); // failed data must not overwrite the last reading

    resetTest();
    Wire.nack = true;
    assert(!gyroTurn(90));
    assert(!Test::motorMoving);
    assert(Test::driveCalls == 0);

    resetTest();
    Wire.failEulerAt = 2;
    assert(!gyroTurn(90));
    assert(Test::driveCalls == 1); // fail while already moving
    assert(!Test::motorMoving);
    assert(Test::stopCalls >= 2);

    resetTest();
    Wire.badState = true;
    assert(!gyroTurn(90));
    assert(!Test::motorMoving);
    assert(Test::driveCalls == 0);

    resetTest();
    Wire.sensorError = 3;
    assert(!bno.TryUpdateEuler());
    Wire.sensorError = 0;
    Wire.radians = true;
    assert(!bno.TryUpdateEuler());

    resetTest();
    Wire.readDelay = 50;
    assert(!gyroTurn(90)); // readings stay valid, but the target is never reached
    assert(Test::now >= turnTimeoutMs);
    assert(Test::now <= turnTimeoutMs + 200);
    assert(Test::lastDriveAt < turnTimeoutMs);
    assert(!Test::motorMoving);

    resetTest();
    Wire.failEulerAt = 1;
    loop();
    assert(turnFault);
    const int readsAtFailure = Wire.eulerReads;
    loop();
    assert(Wire.eulerReads == readsAtFailure); // no automatic retry
    assert(!Test::motorMoving);

    resetTest();
    Wire.heading = 1440;
    assert(gyroTurn(90));
    assert(!Test::motorMoving);
    assert(Test::driveCalls == 0);

    resetTest();
    Test::pidOutput = std::numeric_limits<float>::quiet_NaN();
    assert(!gyroTurn(90));
    assert(!Test::motorMoving);
    assert(Test::driveCalls == 0);

    std::puts("PASS: valid zero, decoding, short read, NACK, state error, units, timeout, fault latch, success, invalid PID");
}
