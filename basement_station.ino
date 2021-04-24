#include "definitions.h"
#include "wifi_handler.h"
#include "web_server.h"
#include "sensors_handler.h"
#include "bot_handler.h"
#include "LcdProxy.h"

#define VERSION "0.7"
#define SETUP_DELAY 1000
#define SERIAL_SPEED 115200
#define BTN_LCD_PIN 5

// global context (defined in definitions.h)
Context* context;

LcdProxy* lcd;

void setup()
{
  delay(SETUP_DELAY);
  Serial.begin(SERIAL_SPEED);
  Serial.println("[main] reboot");

  pinMode(BTN_LCD_PIN, INPUT);
  lcd = new LcdProxy();
  lcd->init(true);
  lcd->print("Starting...", "");
  
  QueueHandle_t queue = xQueueCreate(100, sizeof(EventMessage));
  if (queue == NULL)
  {
    Serial.println("[main] error creating queue");
  }
  SystemInfo* sysInfo = new SystemInfo(VERSION);
  SensorsInfo* sensors = new SensorsInfo();
  context = new Context(sysInfo, sensors, queue);
  
  initWiFi(&webServerInit);
  sensorsHandlerInit();
  botHandlerInit();
  
  xTaskCreate(sensorsHandlerProcess, "sensorsTask", 8172, NULL, 2, NULL);
  xTaskCreate(webServerProcess, "webServerTask", 2048, NULL, 1, NULL);
  xTaskCreate(botHandlerProcess, "botTask", 8172, NULL, 3, NULL);
}

void loop()
{
  if (digitalRead(BTN_LCD_PIN) == HIGH)
  {
    lcd->enable();
  }
  else
  {
    lcd->checkDisable();
  }

  lcd->printSensors(context->sensors);
  
  vTaskDelay(20 / portTICK_RATE_MS);
}
