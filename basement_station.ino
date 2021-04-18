#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "definitions.h"
#include "secrets.h"
#include "wifi_handler.h"
#include "web_server.h"
#include "sensors_handler.h"
#include "bot_handler.h"

#define VERSION "0.5"
#define SETUP_DELAY 2000
#define SERIAL_SPEED 115200
#define NTP_ADDR "pool.ntp.org"
#define TIME_CHECKING_DELAY 100


time_t initTime()
{
  Serial.println("[main] retrieving time...");
  configTime(0, 0, NTP_ADDR); // get UTC time via NTP
  time_t utcNow = time(nullptr);
  while (utcNow < 24 * 3600)
  {
    delay(TIME_CHECKING_DELAY);
    utcNow = time(nullptr);
  }
  Serial.println("[main] time: " + String(utcNow) + " -- " + String(ctime(&utcNow)));
  return utcNow;
}

void setup()
{
  delay(SETUP_DELAY);
  Serial.begin(SERIAL_SPEED);
  Serial.println("[main] reboot");
  
  initWiFi(true);
  time_t utcNow = initTime();
  sensorsHandlerInit();
  botHandlerInit();

  QueueHandle_t queue = xQueueCreate(100, sizeof(EventMessage));
  if (queue == NULL)
  {
    Serial.println("[main] error creating queue");
  }
  SystemInfo* sysInfo = new SystemInfo(VERSION, utcNow);
  SensorsInfo* sensors = new SensorsInfo();
  Context* context = new Context(sysInfo, sensors, queue);
  
  xTaskCreate(sensorsHandlerProcess, "sensorsTask", 8172, context, 2, NULL);
  xTaskCreate(webServerProcess, "webServerTask", 2048, context, 1, NULL);
  xTaskCreate(botHandlerProcess, "botTask", 8172, context, 3, NULL);
}

void loop()
{
  vTaskDelay(10000 / portTICK_RATE_MS);
}
