#pragma ones

#define BOT_MTBS 1000 // mean time (milliseconds) between scan messages
#define DEFAULT_EVENT_TTL 60000 // default event messages ttl (in milliseconds)
#define CHAT_MESSAGE_TTL 60000 // chat message ttl (in milliseconds). after this time bot will not answer
#define BOT_PROCESS_DELAY 1000 // timeout between processing bot messages and events


void botHandlerInit();

void botHandlerProcess(void* params);
