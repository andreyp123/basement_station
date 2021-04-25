#include <WiFi.h>
#include <time.h>
#include "definitions.h"
#include "secrets.h"
#include "wifi_handler.h"


void (*initWebServerFunc)(void);

time_t initTime()
{
  Serial.println("[wifi] retrieving time...");
  configTime(10800, 0, NTP_ADDR); // get UTC+3 time via NTP
  time_t utcNow = time(nullptr);
  while (utcNow < 24 * 3600)
  {
    delay(TIME_CHECKING_DELAY);
    utcNow = time(nullptr);
  }
  Serial.println("[wifi] time: " + String(utcNow));
  return utcNow;
}

void wiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] connected to access point");
  Serial.println("[wifi] signal strength: " + String(WiFi.RSSI()) + " dBm");
}

void wiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] ip-address: " + WiFi.localIP().toString());
  
  // after connecting to AP and receiving IP address
  // - (re)init web server
  initWebServerFunc();
  // - set time
  context->systemInfo->setStartTime(initTime());
}

void wiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] disconnected from access point, reason: " + String(info.disconnected.reason));
  Serial.println("[wifi] trying to reconnect...");
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
}

void initWiFi(void (*initWebServerFuncPtr)())
{
  initWebServerFunc = initWebServerFuncPtr;
  
  WiFi.disconnect(true);
  
  WiFi.onEvent(wiFiConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(wiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(wiFiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  
  Serial.println("[wifi] connecting to access point...");
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
}
