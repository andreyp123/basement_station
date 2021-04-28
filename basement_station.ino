#include "definitions.h"
#include "SensorsHandler.h"
#include "WebServer.h"
#include "wifi_handler.h"
#include "BotHandler.h"
#include "LcdProxy.h"
#include "LcdButton.h"

#define VERSION "0.10"
#define SERIAL_SPEED 115200
#define SETUP_DELAY 1000
#define LCD_BTN_DELAY 20
#define SENS_HANDLER_DELAY 300
#define BOT_HANDLER_DELAY 1000
#define WEB_SERVER_PROCESS_DELAY 500


Context* context;
LcdProxy* lcd;
LcdButton* lcdBtn;


void setup()
{
  delay(SETUP_DELAY);
  Serial.begin(SERIAL_SPEED);
  Serial.println("[main] reboot");
  
  lcd = new LcdProxy();
  lcd->init(true);
  lcd->print("Starting...", "");
  
  lcdBtn = new LcdButton(lcd);
  
  QueueHandle_t queue = xQueueCreate(100, sizeof(EventMessage));
  if (queue == NULL)
  {
    Serial.println("[main] error creating queue");
  }
  SystemInfo* sysInfo = new SystemInfo(VERSION);
  SensorsInfo* sensors = new SensorsInfo();
  context = new Context(sysInfo, sensors, queue);

  SensorsHandler* sensHandler = new SensorsHandler(context);
  sensHandler->init();

  WebServer* webServer = new WebServer(context);
  initWiFi(webServer, context);
  
  BotHandler* bot = new BotHandler(context);
  bot->init();
  
  xTaskCreatePinnedToCore(processSensorsHandler, "processSensorsHandler", 8172, sensHandler, 2, NULL, 0);
  xTaskCreatePinnedToCore(processWebServer, "processWebServer", 2048, webServer, 1, NULL, 1);
  xTaskCreatePinnedToCore(processBotHandler, "processBotHandler", 8172, bot, 3, NULL, 1);
  xTaskCreatePinnedToCore(processLcdButton, "processLcdButton", 2048, NULL, 4, NULL, 1);
}

void processSensorsHandler(void* params)
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

void processBotHandler(void* params)
{
  BotHandler* bot = (BotHandler*)params;
  while (true)
  {
    bot->handleEvents();
    bot->handleMessages();
    vTaskDelay(BOT_HANDLER_DELAY / portTICK_RATE_MS);
  }
}

void processLcdButton(void* params)
{
  while (true)
  {
    lcdBtn->process();
    lcd->printSensors(context->sensors);
    vTaskDelay(LCD_BTN_DELAY / portTICK_RATE_MS);
  }
}

void loop()
{
  vTaskDelay(10000 / portTICK_RATE_MS);
}
