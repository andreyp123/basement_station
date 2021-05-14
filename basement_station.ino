#include "definitions.h"
#include "SensorsHandler.h"
#include "WebServer.h"
#include "wifi_handler.h"
#include "BotHandler.h"
#include "LcdWithButton.h"

#define VERSION "1.1"
#define SERIAL_SPEED 115200
#define SETUP_DELAY 1000
#define LCD_BTN_DELAY 20
#define SENS_HANDLER_DELAY 300
#define BOT_HANDLER_DELAY 1000
#define WEB_SERVER_PROCESS_DELAY 500


void setup()
{
  delay(SETUP_DELAY);
  Serial.begin(SERIAL_SPEED);
  Serial.println("[main] reboot");
  
  QueueHandle_t queue = xQueueCreate(100, sizeof(EventMessage));
  if (queue == NULL)
  {
    Serial.println("[main] error creating queue");
  }
  SystemInfo* sysInfo = new SystemInfo(VERSION);
  SensorsInfo* sensors = new SensorsInfo();
  Context* context = new Context(sysInfo, sensors, queue);
  
  LcdWithButton* lcdWithBtn = new LcdWithButton(context);
  lcdWithBtn->init(true);
  lcdWithBtn->print("Starting...", "");
  
  SensorsHandler* sensHandler = new SensorsHandler(context);
  sensHandler->init();

  WebServer* webServer = new WebServer(context);
  initWiFi(webServer, context);
  
  BotHandler* bot = new BotHandler(context);
  bot->init();

  xTaskCreatePinnedToCore(processLcd, "processLcd", 2048, lcdWithBtn, 4, NULL, 1);
  xTaskCreatePinnedToCore(processSensors, "processSensors", 8172, sensHandler, 2, NULL, 1);
  xTaskCreatePinnedToCore(processWebServer, "processWebServer", 2048, webServer, 1, NULL, 1);
  xTaskCreatePinnedToCore(processBot, "processBot", 8172, bot, 3, NULL, 1);
}

void processSensors(void* params)
{
  SensorsHandler* sensHandler = (SensorsHandler*)params;
  while (true)
  {
    sensHandler->process();
    vTaskDelay(SENS_HANDLER_DELAY / portTICK_RATE_MS);
  }
}

void processWebServer(void* params)
{
  WebServer* webServer = (WebServer*)params;
  while (true)
  {
    if (!webServer->process())
    {
      vTaskDelay(WEB_SERVER_PROCESS_DELAY / portTICK_RATE_MS);
    }
  }
}

void processBot(void* params)
{
  BotHandler* bot = (BotHandler*)params;
  while (true)
  {
    bot->handleEvents();
    bot->handleMessages();
    vTaskDelay(BOT_HANDLER_DELAY / portTICK_RATE_MS);
  }
}

void processLcd(void* params)
{
  LcdWithButton* lcdWithBtn = (LcdWithButton*)params;
  while (true)
  {
    lcdWithBtn->process();
    vTaskDelay(LCD_BTN_DELAY / portTICK_RATE_MS);
  }
}

void loop()
{
  vTaskDelay(10000 / portTICK_RATE_MS);
}
