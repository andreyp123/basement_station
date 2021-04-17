#pragma ones

#define WEB_SERVER_PORT 80 // web-server port (http default)
#define WEB_SERVER_PROCESS_DELAY 500 // timeout between processing new client connections


String webServerInit();

void webServerProcess(void* params);
