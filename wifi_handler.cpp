#include <WiFi.h>
#include <ESP32Ping.h>
#include <time.h>
#include "definitions.h"
#include "secrets.h"
#include "WebServer.h"
#include "wifi_handler.h"


IPAddress inetIP(8, 8, 8, 8);
WebServer* _webServer;
Context* _context;
bool _connected;

time_t getTime()
{
  Serial.println("[wifi] retrieving time...");
  configTime(10800, 0, NTP_ADDR); // get UTC+3 time via NTP
  time_t timeNow = time(nullptr);
  int elapsedMsec = 0;
  while (timeNow < 24 * 3600 || elapsedMsec < TIME_CHECKING_TIMEOUT)
  {
    delay(TIME_CHECKING_DELAY);
    elapsedMsec += TIME_CHECKING_DELAY;
    timeNow = time(nullptr);
  }
  if (timeNow < 24 * 3600)
    timeNow = 0;
  Serial.println("[wifi] time: " + String(timeNow));
  return timeNow;
}

void checkInternet()
{
  // check internet connection
  _context->systemInfo->internet = Ping.ping(inetIP);
  // set time (if not set before)
  if (!_context->systemInfo->startTime)
  {
    _context->systemInfo->setStartTime(getTime());
  }
}

void wiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] connected to access point");
  _context->systemInfo->wifiRssi = WiFi.RSSI();
  Serial.println("[wifi] signal strength: " + _context->systemInfo->getWifiStr());
}

void wiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] ip-address: " + WiFi.localIP().toString());
  _connected = true;
  // after connecting to AP and receiving IP address
  // - (re)init web server
  _webServer->init();
  // - check internet (including time initialization)
  checkInternet();
}

void wiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] disconnected from access point, reason: " + String(info.disconnected.reason));
  _connected = false;
  _context->systemInfo->wifiRssi = 0;
  _context->systemInfo->internet = false;

  Serial.println("[wifi] trying to reconnect...");
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
}

void runCheckingInternet(void* params)
{
  while (true)
  {
    if (_connected)
    {
      checkInternet();
    }
    Serial.println("[wifi] " + _context->systemInfo->toString());
    
    vTaskDelay(INET_CHECKING_TIMEOUT / portTICK_RATE_MS);
  }
}

void initWiFi(WebServer* webServer, Context* context)
{
  _webServer = webServer;
  _context = context;
  _connected = false;
  
  WiFi.disconnect(true);
  
  WiFi.onEvent(wiFiConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(wiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(wiFiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

  xTaskCreatePinnedToCore(runCheckingInternet, "runCheckingInternet", 2048, NULL, 1, NULL, 1);
  
  Serial.println("[wifi] connecting to access point...");
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
}
