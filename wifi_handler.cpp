#include <WiFi.h>
#include "secrets.h"
#include "wifi_handler.h"
#include "web_server.h"

bool _initWebServer = false;

void wiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] connected to access point");
  Serial.println("[wifi] signal strength: " + String(WiFi.RSSI()) + " dBm");
}

void wiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] ip-address: " + WiFi.localIP().toString());
  
  // init web servers after connecting to AP and receiving IP address
  if (_initWebServer)
  {
    webServerInit();
  }
}

void wiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("[wifi] disconnected from access point, reason: " + String(info.disconnected.reason));
  Serial.println("[wifi] trying to reconnect...");
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
}

void initWiFi(bool initWebServer)
{
  _initWebServer = initWebServer;
  
  WiFi.disconnect(true);
  
  WiFi.onEvent(wiFiConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(wiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(wiFiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  
  Serial.println("[wifi] connecting to access point...");
  WiFi.begin(SEC_WIFI_SSID, SEC_WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_CONNECTION_DELAY);
  }
}
