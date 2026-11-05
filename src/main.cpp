#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <settings.hpp>
#include <Sg90Controller.hpp>

// Wi-Fi接続情報、サーボピン、通信タイムアウトはsettings.hppで設定する。
const IPAddress ipacp(192, 168, 100, 1);
const IPAddress subnetacp(255, 255, 255, 0);
WebServer Wserver(80);
Sg90Controller servo;
bool ready = false;
MotionEnum motion = STOP;
unsigned long lastCommandAt = 0;
uint8_t previousClients = 0;

void setMotion(MotionEnum next);
void handleControl();
void handleSettings();
void handleCommand();
void handleServo();
void handleNotFound();
bool tryBegin();
bool trySetupRobot();
bool trySetupServer();
void update();
void stopRobot();

void setup()
{
    Serial.begin(115200);
    delay(300);
    tryBegin();
}

void loop()
{
    update();
}

// PlatformIOがdata/index.htmlをファームウェアへ埋め込む際に作るシンボル。
// pageは埋め込まれたHTMLの先頭を指す。HTMLをここへ複製する必要はない。
extern const char page[] asm("_binary_data_index_html_start");

// 指令が変わったときだけログを出し、定期受信による同じログの連続を防ぐ。
// モータードライバーの種類・配線が未設定のため、GPIOは操作しない。
void setMotion(MotionEnum next)
{
    if (motion == next) return;
    motion = next;
    Serial.println("Motion: " + String(motion));
}

void handleControl()
{
    // 古い操作画面をキャッシュから読み込まないようにする。
    Wserver.sendHeader("Cache-Control", "no-store");
    // send_P()でフラッシュ内のHTMLを送る。UTF-8を指定して日本語を表示する。
    Wserver.send_P(200, "text/html; charset=utf-8", page);
}

void handleSettings()
{
    // 起動中のWi-Fi情報を使うため、このページは要求のたびに生成する。
    String html = "<!doctype html><html lang='ja'><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<title>接続情報</title><h1>ESP32 接続情報</h1><p>SSID: ";
    html += WiFi.softAPSSID();
    html += "</p><p>AP IP: ";
    html += WiFi.softAPIP().toString();
    html += "</p><p>ルーター接続不要・AP専用</p><a href='/'>操作画面へ</a></html>";
    Wserver.sendHeader("Cache-Control", "no-store");
    Wserver.send(200, "text/html; charset=utf-8", html);
}

void handleCommand() 
{
    // 例：/command?value=LEFT の「LEFT」を取り出す。
    // 引数なし・未知の文字列の場合も下の検証で拒否する。
    const String value = Wserver.arg("value");
    if (value != "FORWARD" && value != "BACKWARD" &&
        value != "LEFT" && value != "RIGHT" && value != "STOP")
    {
        // 不正な要求では、現在の指令と最終受信時刻を変更しない。
        Wserver.send(400, "text/plain", "Invalid command");
        return;
    }
    // 同じ指令の繰り返しでも時刻を更新し、通信が続いていることを記録する。
    lastCommandAt = millis();

    MotionEnum next;
    if (value == "FORWARD") next = FORWARD;
    else if (value == "BACKWARD") next = BACKWARD;
    else if (value == "LEFT") next = LEFT;
    else if (value == "RIGHT") next = RIGHT;
    else next = STOP;

    setMotion(next);
    Wserver.sendHeader("Cache-Control", "no-store");
    // ブラウザーはこの応答を通信ログに表示する。
    Wserver.send(200, "text/plain", String(motion));
}

void handleNotFound() 
{
    Wserver.send(404, "text/plain", "Not found");
}

void handleServo() 
{
    const String value = Wserver.arg("angle");
    // toInt()は不正文字列も0にするため、数字だけかを先に確認する。
    bool valid = value.length() > 0 && value.length() <= 3;
    for (unsigned int i = 0; i < value.length(); ++i)
    {
        if (value[i] < '0' || value[i] > '9') valid = false;
    }
    const int angle = value.toInt();
    if (!valid || angle > 180)
    {
        Wserver.send(400, "text/plain", "Invalid angle (0-180)");
        return;
    }

    servo.set(angle);

    Serial.printf("Servo angle: %d\n", angle);
    Wserver.sendHeader("Cache-Control", "no-store");
    Wserver.send(200, "text/plain", "SG90 angle=" + String(angle));
}


bool tryBegin() 
{
    if (!trySetupServer()) return false;
    if (!trySetupRobot()) return false;
    return true;
}

bool trySetupRobot() 
{
    servo.setUp(servoPin, 90, 5, 175);
    return true;
}

bool trySetupServer() 
{
    // 二重に呼ばれても、サーバーの登録やAPの起動を繰り返さない。
    if (ready) return true;
    Serial.println("\nRobot AP / article-based WebServer v4");
    // 既存のルーター接続を切ってからAPを起動する。
    WiFi.disconnect();
    delay(100);

    // softAPの引数：SSID、パスワード、チャンネル1、SSID公開、最大接続数1。
    if (!WiFi.softAP(ssid.c_str(), pass.c_str(), 1, 0, 1))
    {
        Serial.println("ERROR: softAP failed");
        return false;
    }
    // AP自身のIP、ゲートウェイ、サブネットを設定する。
    // この構成ではESP32自身のIPをゲートウェイとして指定する。
    if (!WiFi.softAPConfig(ipacp, ipacp, subnetacp))
    {
        Serial.println("ERROR: softAPConfig failed");
        // IP設定に失敗した場合は、途中まで起動したAPも停止する。
        WiFi.softAPdisconnect(true);
        return false;
    }

    // URLごとにmain.cpp内の関数を登録する。
    Wserver.on("/", HTTP_GET, handleControl);
    Wserver.on("/setting", HTTP_GET, handleSettings);
    Wserver.on("/command", HTTP_GET, handleCommand);
    Wserver.on("/servo", HTTP_GET, handleServo);
    Wserver.onNotFound(handleNotFound);

    // APとURLの準備が整ったら、HTTPサーバーを起動する。
    Wserver.begin();
    ready = true;

    // 接続先や無線設定をシリアルモニターで確認できるようにする。
    Serial.print("AP SSID: "); Serial.println(WiFi.softAPSSID());
    Serial.print("AP IP: http://"); Serial.println(WiFi.softAPIP());
    Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
    Serial.printf("WiFi mode: %d, channel: %d\n", WiFi.getMode(), WiFi.channel());
    Serial.println("HTTP server ready; SG90 enabled, drive motors disabled.");
    return true;
}

void update() 
{
    // 初期化に失敗した場合は、未起動のサーバーを処理しない。
    if (!ready)
    {
        delay(10);
        return;
    }

    // ブラウザーが閉じたり通信が途絶えたりしても、指令を保持し続けない。
    // unsigned long同士の差分で比較し、millis()が一周しても経過時間を判定する。
    if (motion != STOP && millis() - lastCommandAt >= commandTimeoutMs)
    {
        setMotion(STOP);
        Serial.println("Command timeout");
    }

    // 到着したHTTP要求を処理する。登録したhandleCommand()等はここから呼ばれる。
    Wserver.handleClient();

    // 台数が変わったときだけ記録し、最後の端末が切断したら即座にSTOPにする。
    const uint8_t clients = WiFi.softAPgetStationNum();
    if (clients != previousClients)
    {
        previousClients = clients;
        Serial.printf("AP connected clients: %u\n", clients);
        if (clients == 0) stopRobot();
    }
    
    delay(1);
}

void stopRobot() 
{
    setMotion(STOP);
    servo.move(0);
}
