#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "definitions.h"
#include "secrets.h"
#include "bot_handler.h"


const String eventChatId = SEC_GROUP_CHAT_ID;
const String trustedChatIds[] = {
  SEC_GROUP_CHAT_ID,
  SEC_USER1_CHAT_ID,
  SEC_USER2_CHAT_ID };
const String botCommands = F(
  "["
    "{\"command\":\"help\",  \"description\":\"Get bot usage help\"},"
    "{\"command\":\"start\", \"description\":\"Message sent when you open a chat with a bot\"},"
    "{\"command\":\"system\",\"description\":\"Get system information\"},"
    "{\"command\":\"temp\",\"description\":\"Get temperature\"},"
    "{\"command\":\"hum\",\"description\":\"Get humidity\"},"
    "{\"command\":\"light\",\"description\":\"Get lightness\"},"
    "{\"command\":\"wpres\",\"description\":\"Get water pressure\"}"
  "]");

WiFiClientSecure securedClient;
UniversalTelegramBot bot(SEC_BOT_TOKEN, securedClient);
unsigned long botLastTime; // last time messages' scan has been done


void botHandlerInit()
{
  Serial.println("[bot] initializing bot...");
  
  securedClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  bot.setMyCommands(botCommands);
  botLastTime = millis();
}

bool isTrustedChatId(String chatId)
{
  for (int i = 0; i < sizeof(trustedChatIds); i++)
    if (trustedChatIds[i] == chatId)
      return true;
  return false;
}

void handleNewMessages(int numNewMessages)
{
  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = bot.messages[i];

    // handle only messages from trusted chats
    if (!isTrustedChatId(msg.chat_id))
    {
      Serial.println("[bot] skipped message from untrusted party " + msg.from_name + " (" + String(msg.chat_id) + "): " + msg.text);
      continue;
    }
    Serial.println("[bot] received message from " + msg.from_name + " (" + String(msg.chat_id) + "): " + msg.text);
    
    // todo: skip too old messages
    
    if (msg.text == "/help")
      answer = "Available commands: /start, /system, /temp, /hum, /light, /wpres";
    else if (msg.text == "/start")
      answer = "Ok, " + msg.from_name + ", let's start! Use /help to see all available commands.";
    else if (msg.text == "/system")
      answer = "Start time: " + context->systemInfo->startTimeStr + "\nWi-fi signal: " + context->systemInfo->getWifiStr() +
        "\nUrl: " + context->systemInfo->serverUrl + "\nVersion: " + context->systemInfo->version;
    else if (msg.text == "/temp")
      answer = "Temperature: " + context->sensors->getTempStr();
    else if (msg.text == "/hum")
      answer = "Humidity: " + context->sensors->getHumStr();
    else if (msg.text == "/light")
      answer = "Light: " + context->sensors->getLightStr();
    else if (msg.text == "/wpres")
      answer = "Water pressure: " + context->sensors->getPresStr();
    else
      answer = "";

    if (answer != "")
    {
      bot.sendMessage(msg.chat_id, answer, "Markdown");
      Serial.println("[bot] sent answer: " + answer);
    }
  }
}

void checkEventMessages()
{
  EventMessage event;
  while (xQueueReceive(context->queue, &event, 0) == pdPASS) // != errQUEUE_EMPTY
  {
    Serial.println("[bot] " + event.toString());
        
    // skip too old events
    int ttl = event.ttl > 0 ? event.ttl : DEFAULT_EVENT_TTL;
    if (millis() - event.timespan > ttl)
    {
      Serial.println("[bot] event skipped");
      continue;
    }
        
    String botMsg = "";
    if (event.eType == lowPressure)
      botMsg = "Low pressure " + context->sensors->getPresStr();
    else if (event.eType == normPressure)
      botMsg = "Normal pressure " + context->sensors->getPresStr();

    if (botMsg != "")
    {
      bot.sendMessage(eventChatId, botMsg, "Markdown");
      Serial.println("[bot] sent notification to " + String(eventChatId) + ": " + botMsg);
    }
  }
}

void checkBotMessages()
{
  if (millis() - botLastTime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("[bot] new messages: " + String(numNewMessages));
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    botLastTime = millis();
  }
}

void botHandlerProcess(void* params)
{
  while (true)
  {
    checkEventMessages();
    checkBotMessages();
    
    vTaskDelay(BOT_PROCESS_DELAY / portTICK_RATE_MS);
  }
}
