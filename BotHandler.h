#pragma ones

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "secrets.h"

/*
 * Implements functionality for handling telegram bot messages and events.
 */
class BotHandler
{
private:
  static const int DEFAULT_EVENT_TTL = 60000; // default event messages ttl (in milliseconds)
  static const int BOT_MTBS = 1000; // mean time (milliseconds) between scan messages
  static const String EVENT_CHAT_ID; // chat id where to send events
  static const String TRUSTED_CHAT_IDS[];
  static const String BOT_COMMANDS;
  
  WiFiClientSecure _securedClient;
  UniversalTelegramBot _bot;
  unsigned long _botLastTime; // last time messages' scan has been done
  bool _init;
  Context* _context;

  bool isTrustedChatId(String chatId);
  void handleNewMessages(int numNewMessages);

public:
  BotHandler(Context* ctx):
    _bot(SEC_BOT_TOKEN, _securedClient),
    _botLastTime(0),
    _init(false),
    _context(ctx)
  {
  }
  void init();
  void handleEvents();
  void handleMessages();
};
