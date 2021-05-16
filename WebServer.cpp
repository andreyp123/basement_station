#include <WiFi.h>
#include "definitions.h"
#include "WebServer.h"


void WebServer::init()
{
  Serial.println("[web] initializing server...");
  
  String url = getUrl();
  _context->systemInfo->serverUrl = url;
  Serial.println("[web] url: " + url);
  
  _server.begin();

  _init = true;
}

bool WebServer::process()
{
  if (!_init)
    return false;
  
  bool retVal = false;
  
  // waiting for incoming clients
  WiFiClient client = _server.available();
  if (client)
  {
    Serial.println("[web] new client " + client.remoteIP().toString());
    char requestBuf[80];
    memset(requestBuf, 0, sizeof(requestBuf));
    int charCount = 0;
    while (client.connected())
    {
      if (client.available())
      {
        // read request
        char c = client.read();
        requestBuf[charCount] = c;
        if (charCount < sizeof(requestBuf) - 1)
        {
          charCount++;
        }
        
        // read until the end of string and send the response
        if (c == '\n')
        {
          Serial.print("[web] request: " + String(requestBuf));
          
          // response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println(getHtml());
          break;
        }
      }
    }
    // give some time for client to receive data and close connection
    vTaskDelay(10 / portTICK_RATE_MS);
    client.stop();
    Serial.println("[web] client disconnected");
    retVal = true;
  }
  
  if (_wifiCounter.Inc())
    _context->systemInfo->wifiRssi = WiFi.RSSI();
  
  return retVal;
}

String WebServer::getHtml()
{
  String html = "<!DOCTYPE HTML>";
  html += "<html>";
  html += "<head>";
  html += "  <meta charset=\"utf-8\">";
  html += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "  <title>Basement station</title>";
  html += "</head>";
  html += "<body>";
  html += "  <h1>Basement station</h1>";
  html += "  <h2>Sensors</h2>";
  html += "  <p>Temperature: " + _context->sensors->getTempStr() + "</p>";
  html += "  <p>Humidity: " + _context->sensors->getHumStr() + "</p>";
  html += "  <p>Water pressure: " + _context->sensors->getPres1Str() + " / " + _context->sensors->getPres2Str() + "</p>";
  html += "  <p>Light: " + _context->sensors->getLightStr() + "</p><br/>";
  html += "  <h2>System</h2>";
  html += "  <p>Start time: " + _context->systemInfo->startTimeStr + "</p>";
  html += "  <p>Wi-fi signal: " + _context->systemInfo->getWifiStr() + "</p>";
  html += "  <p>Internet: " + _context->systemInfo->getInternet() + "</p>";
  html += "  <p>Version: " + _context->systemInfo->version + "</p>";
  html += "</body>";
  html += "</html>";
  return html;
}

String WebServer::getUrl()
{
  return "http://" + WiFi.localIP().toString() + ":" + WEB_SERVER_PORT;
}
