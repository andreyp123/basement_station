#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "definitions.h"
#include "secrets.h"
#include "web_server.h"
#include "sensors_handler.h"
#include "bot_handler.h"

#define VERSION "0.3"
#define SERIAL_SPEED 115200
#define WIFI_CONNECTION_DELAY 500
#define NTP_ADDR "pool.ntp.org"
#define TIME_CHECKING_DELAY 100
#define SETUP_DELAY 2000


void initWiFi()
{
  Serial.print("[main] connecting to wi-fi '" + String(SEC_WIFI_SSID) + "'...");  
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_CONNECTION_DELAY);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.println("[main] ip-address: " + WiFi.localIP().toString());
  Serial.println("[main] wi-fi signal strength: " + String(WiFi.RSSI()) + " dBm");
}

time_t initTime()
{
  Serial.print("[main] retrieving time...");
  configTime(0, 0, NTP_ADDR); // get UTC time via NTP
  time_t utcNow = time(nullptr);
  while (utcNow < 24 * 3600)
  {
    Serial.print(".");
    delay(TIME_CHECKING_DELAY);
    utcNow = time(nullptr);
  }
  Serial.println(utcNow);
  Serial.println("[main] start time: " + String(ctime(&utcNow)));
  return utcNow;
}

void setup()
{
  delay(SETUP_DELAY);
  Serial.begin(SERIAL_SPEED);
  Serial.println("[main] reboot");

  initWiFi();
  time_t utcNow = initTime();
  
  sensorsHandlerInit();
  String url = webServerInit();
  botHandlerInit();

  QueueHandle_t queue = xQueueCreate(100, sizeof(EventMessage));
  if (queue == NULL)
  {
    Serial.println("[main] error creating queue");
  }
  SystemInfo* sysInfo = new SystemInfo(VERSION, utcNow, url);
  SensorsInfo* sensors = new SensorsInfo();
  Context* context = new Context(sysInfo, sensors, queue);
  
  xTaskCreate(sensorsHandlerProcess, "sensorsTask", 8172, context, 2, NULL);
  xTaskCreate(webServerProcess, "webServerTask", 2048, context, 1, NULL);
  xTaskCreate(botHandlerProcess, "botTask", 8172, context, 3, NULL);
  Serial.println("[main] created tasks");
}

void loop()
{
  vTaskDelay(10000 / portTICK_RATE_MS);
}
