#include "definitions.h"
#include "wifi_handler.h"
#include "web_server.h"
#include "sensors_handler.h"
#include "BotHandler.h"
#include "LcdProxy.h"

#define VERSION "0.9"
#define SETUP_DELAY 1000
#define BTN_LCD_DELAY 20
#define BOT_HANDLER_DELAY 1000
#define SERIAL_SPEED 115200
#define LCD_BTN_PIN 5

// global context (defined in definitions.h)
Context* context;

LcdProxy* lcd;
BotHandler* bot;

long lcdBtnOffLastMillis = 0;

void setup()
{
  delay(SETUP_DELAY);
  Serial.begin(SERIAL_SPEED);
  Serial.println("[main] reboot");

  pinMode(LCD_BTN_PIN, INPUT);
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
  bot = new BotHandler(context);
  bot->init();
  
  xTaskCreatePinnedToCore(sensorsHandlerProcess, "sensorsTask", 8172, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(webServerProcess, "webServerTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(processBotHandler, "processBotHandler", 8172, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(processLcdButton, "processLcdButton", 2048, NULL, 4, NULL, 1);
}

void processBotHandler(void* params)
{
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
    if (digitalRead(LCD_BTN_PIN) == HIGH)
    {
      long lcdBtnOnMillis = millis() - lcdBtnOffLastMillis;
      if (lcdBtnOnMillis > 10000) // if hold for 10 seconds - reboot
        ESP.restart();
      else if (lcdBtnOnMillis > 5000) // if hold for 5 seconds - reinit lcd
        lcd->init(false);
      
      lcd->enable();
    }
    else
    {
      lcd->checkDisable();

      lcdBtnOffLastMillis = millis();
    }
  
    lcd->printSensors(context->sensors);
    
    vTaskDelay(BTN_LCD_DELAY / portTICK_RATE_MS);
  }
}

void loop()
{
  vTaskDelay(10000 / portTICK_RATE_MS);
}
