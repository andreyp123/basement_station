#include <WiFi.h>
#include "definitions.h"
#include "BotHandler.h"


const String BotHandler::EVENT_CHAT_ID =
  SEC_USER1_CHAT_ID;//SEC_GROUP_CHAT_ID;
const String BotHandler::TRUSTED_CHAT_IDS[] =
  {
    SEC_GROUP_CHAT_ID,
    SEC_USER1_CHAT_ID,
    SEC_USER2_CHAT_ID
  };
const String BotHandler::BOT_COMMANDS = F(
  "["
    "{\"command\":\"help\",  \"description\":\"Get bot usage help\"},"
    "{\"command\":\"start\", \"description\":\"Start messaging with the bot\"},"
    "{\"command\":\"system\",\"description\":\"Get system information\"},"
    "{\"command\":\"sensors\",\"description\":\"Get sensors information\"}"
  "]");


void BotHandler::init()
{
  Serial.println("[bot] initializing bot...");
  
  _securedClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  _bot.setMyCommands(BOT_COMMANDS);
  _botLastTime = millis();
  _init = true;
}

void BotHandler::handleEvents()
{
  if (!_init)
    return;
  
  EventMessage event;
  while (xQueueReceive(_context->queue, &event, 0) == pdPASS) // != errQUEUE_EMPTY
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
      botMsg = "Low pressure " + _context->sensors->getPres1Str() + " / " + _context->sensors->getPres2Str();
    else if (event.eType == normPressure)
      botMsg = "Normal pressure " + _context->sensors->getPres1Str() + " / " + _context->sensors->getPres2Str();

    if (botMsg != "")
    {
      _bot.sendMessage(EVENT_CHAT_ID, botMsg, "Markdown");
      Serial.println("[bot] sent notification to " + String(EVENT_CHAT_ID) + ": " + botMsg);
    }
  }
}

void BotHandler::handleMessages()
{
  if (!_init)
    return;
  
  if (millis() - _botLastTime > BOT_MTBS)
  {
    int numNewMessages = _bot.getUpdates(_bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("[bot] new messages: " + String(numNewMessages));
      handleNewMessages(numNewMessages);
      numNewMessages = _bot.getUpdates(_bot.last_message_received + 1);
    }

    _botLastTime = millis();
  }
}

void BotHandler::handleNewMessages(int numNewMessages)
{
  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = _bot.messages[i];

    // handle only messages from trusted chats
    if (!isTrustedChatId(msg.chat_id))
    {
      Serial.println("[bot] skipped message from untrusted party " + msg.from_name + " (" + String(msg.chat_id) + "): " + msg.text);
      continue;
    }
    Serial.println("[bot] received message from " + msg.from_name + " (" + String(msg.chat_id) + "): " + msg.text);
    
    // todo: skip too old messages
    
    if (msg.text == "/help")
      answer = "Available commands: /start, /system, /sensors";
    else if (msg.text == "/start")
      answer = "Ok, " + msg.from_name + ", let's start! Use /help to see all available commands.";
    else if (msg.text == "/system")
      answer = "Start time: " + _context->systemInfo->startTimeStr + "\nWi-fi signal: " + _context->systemInfo->getWifiStr() +
        "\nUrl: " + _context->systemInfo->serverUrl + "\nVersion: " + _context->systemInfo->version;
    else if (msg.text == "/sensors")
      answer = "Temperature: " + _context->sensors->getTempStr() + "\nHumidity: " + _context->sensors->getHumStr() +
        "\nLight: " + _context->sensors->getLightStr() + "\nWater pressure: " + _context->sensors->getPres1Str() + " / " + _context->sensors->getPres2Str();
    else
      answer = "";

    if (answer != "")
    {
      _bot.sendMessage(msg.chat_id, answer, "Markdown");
      Serial.println("[bot] sent answer: " + answer);
    }
  }
}

bool BotHandler::isTrustedChatId(String chatId)
{
  for (int i = 0; i < sizeof(TRUSTED_CHAT_IDS); i++)
    if (TRUSTED_CHAT_IDS[i] == chatId)
      return true;
  return false;
}
