#pragma once
#include <Arduino.h>

// ================
//  ピン等
// ================

const int servoPin = 13;

// ================
//  Motion
// ================

enum MotionEnum 
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    STOP
};

// ================
//  インターネット
// ================

// アクセスポイントの接続情報。複数回includeされても定義が重複しないようにする。
const String ssid = "Oe-GUItest";
const String pass = "12345678";

// 有効な指令を最後に受信してから、STOPへ戻すまでの時間（ミリ秒）。
static constexpr unsigned long commandTimeoutMs = 600;

// ================
//  ラジコン設定
// ================

struct RadiconSettings
{
    // IMU
    const int sdaPin = 21;
    const int sclPin = 22;
    const uint8_t bnoAddress = 0x28;

    // モーターA
    const int PWMA = 16;
    const int AIN1 = 5;
    const int AIN2 = 17;

    // モーターB
    const int PWMB = 23;
    const int BIN1 = 18;
    const int BIN2 = 19;

    const bool isReversedA = false;
    const bool isReversedB = false;
};

// 倒立振子設定

struct PendrumSettings
{
    const int sdaPin = 32;
    const int sclPin = 27;
    const uint8_t bnoAddress = 0x28;

    const uint8_t in1Pin = 16;
    const uint8_t in2Pin = 17;
};
