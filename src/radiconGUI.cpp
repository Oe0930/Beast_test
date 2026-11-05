#include "settings.hpp"
#include <WiFi.h>
#include <WebServer.h>
#include <driver/gpio.h>
#include "BNO055Controller.hpp"
#include "TB6612FNGController/TB6612FNGController.hpp"

const IPAddress ipacp(192, 168, 100, 1);
const IPAddress subnetacp(255, 255, 255, 0);
WebServer Wserver(80);
bool ready = false;
float driveSteering = 0;
float drivePower = 0;
unsigned long lastCommandAt = 0;
uint8_t previousClients = 0;

// IMUの更新を行っていない現在のGUIでは初期化も省く。
// センサー未接続でも操作画面を起動できる。
constexpr bool enableImu = false;
RadiconSettings settings;
BNO055Controller bnoController(settings.sdaPin, settings.sclPin, settings.bnoAddress);
TB6612FNGController drive(settings.AIN1, settings.AIN2, settings.PWMA,
                        settings.BIN1, settings.BIN2, settings.PWMB,
                        settings.isReversedA, settings.isReversedB);

extern const char page[] asm("_binary_data_index_radicon_html_start");

void stopRobot()
{
    driveSteering = 0;
    drivePower = 0;
    drive.stop();
}

void applyDrive()
{
    if (drivePower == 0)
    {
        drive.stop();
    }
    else
    {
        drive.drive(driveSteering, drivePower);
    }
}

bool parseControlValue(const String& text, float& value)
{
    if (text.length() == 0 || text.length() > 16)
    {
        return false;
    }
    bool hasDigit = false;
    bool hasPoint = false;
    for (unsigned int i = 0; i < text.length(); ++i)
    {
        const char c = text[i];
        if (c >= '0' && c <= '9')
        {
            hasDigit = true;
        }
        else if (c == '.' && !hasPoint)
        {
            hasPoint = true;
        }
        else if ((c == '-' || c == '+') && i == 0)
        {
            // 符号は先頭のみ。
        }
        else
        {
            return false;
        }
    }
    if (!hasDigit)
    {
        return false;
    }
    char* end = nullptr;
    value = strtof(text.c_str(), &end);
    return *end == '\0' && isfinite(value) && value >= -1 && value <= 1;
}

void handleControl()
{
    Wserver.sendHeader("Cache-Control", "no-store");
    Wserver.send_P(200, "text/html; charset=utf-8", page);
}

void handleSettings()
{
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

void handleSteering()
{
    float steering;
    if (!Wserver.hasArg("steering") || !parseControlValue(Wserver.arg("steering"), steering))
    {
        Wserver.send(400, "text/plain", "Invalid steering (-1 to 1)");
        return;
    }
    driveSteering = steering;
    // 操舵だけの通信で古い走行指令の有効期限を延長しない。
    applyDrive();
    Wserver.sendHeader("Cache-Control", "no-store");
    Wserver.send(200, "text/plain", "Steering set to " + String(driveSteering, 3));
}

void handlePower()
{
    float power;
    if (!Wserver.hasArg("power") || !parseControlValue(Wserver.arg("power"), power))
    {
        Wserver.send(400, "text/plain", "Invalid power (-1 to 1)");
        return;
    }
    drivePower = power;
    lastCommandAt = millis();
    applyDrive();
    Wserver.sendHeader("Cache-Control", "no-store");
    Wserver.send(200, "text/plain", "Power set to " + String(drivePower, 3));
}

void checkStopConditions()
{
    if (drivePower != 0 && millis() - lastCommandAt >= commandTimeoutMs)
    {
        stopRobot();
        Serial.println("Command timeout: STOP");
    }
    const uint8_t clients = WiFi.softAPgetStationNum();
    if (clients != previousClients)
    {
        previousClients = clients;
        Serial.printf("AP connected clients: %u\n", clients);
    }
    if (clients == 0)
    {
        stopRobot();
    }
}

bool trySetupRobot()
{
    const int pins[] =
    {
        settings.AIN1, settings.AIN2, settings.PWMA,
        settings.BIN1, settings.BIN2, settings.PWMB
    };
    for (const int pin : pins)
    {
        if (!GPIO_IS_VALID_OUTPUT_GPIO(pin))
        {
            Serial.printf("Invalid motor output GPIO: %d\n", pin);
            return false;
        }
    }
    drive.setUp();
    stopRobot();
    if (enableImu)
    {
        bnoController.Setup();
    }
    return true;
}

bool trySetupServer()
{
    WiFi.disconnect();
    delay(100);
    if (!WiFi.mode(WIFI_AP) || !WiFi.softAP(ssid.c_str(), pass.c_str(), 1, 0, 1))
    {
        Serial.println("ERROR: softAP failed");
        return false;
    }
    if (!WiFi.softAPConfig(ipacp, ipacp, subnetacp))
    {
        WiFi.softAPdisconnect(true);
        Serial.println("ERROR: softAPConfig failed");
        return false;
    }
    Wserver.on("/", HTTP_GET, handleControl);
    Wserver.on("/setting", HTTP_GET, handleSettings);
    Wserver.on("/steering", HTTP_GET, handleSteering);
    Wserver.on("/power", HTTP_GET, handlePower);
    Wserver.onNotFound([]()
    {
        Wserver.send(404, "text/plain", "Not found");
    });
    Wserver.begin();
    Serial.print("AP SSID: "); Serial.println(WiFi.softAPSSID());
    Serial.print("AP IP: http://"); Serial.println(WiFi.softAPIP());
    Serial.println("Radicon GUI ready; IMU telemetry disabled.");
    return true;
}

void setup()
{
    Serial.begin(115200);
    ready = trySetupRobot() && trySetupServer();
    if (!ready)
    {
        Serial.println("Failed to initialize system");
    }
}

void loop()
{
    if (!ready)
    {
        delay(10);
        return;
    }
    checkStopConditions();
    Wserver.handleClient();
    checkStopConditions();
    delay(1);
}
