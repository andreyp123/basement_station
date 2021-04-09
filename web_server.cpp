#include <WiFi.h>
#include "definitions.h"
#include "web_server.h"


WiFiServer server(WEB_SERVER_PORT);


void webServerInit()
{
  Serial.println("[web] initializing server...");
  server.begin();
}

void webServerProcess(void* params)
{
  Context* ctx = (Context*)params;
  
  while (true)
  {
    // waiting for incoming clients
    WiFiClient client = server.available();
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
            client.println(getHtml(ctx));
            break;
          }
        }
      }
      // give some time for client to receive data and close connection
      vTaskDelay(10 / portTICK_RATE_MS);
      client.stop();
      Serial.println("[web] client disconnected");  
    }
    else
    {
      vTaskDelay(WEB_SERVER_PROCESS_DELAY / portTICK_RATE_MS);
    }
    //
    ctx->sensors->wifiRssi = WiFi.RSSI();
  }
}

String getHtml(Context* ctx)
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
  html += "  <p>Temperature: " + String(ctx->sensors->tempC, 1) + " C</p>";
  html += "  <p>Humidity: " + String(ctx->sensors->humProc, 1) + "%</p>";
  html += "  <p>Light: " + String(ctx->sensors->ldrRawVal) + "</p>";
  html += "  <p>Water pressure: " + String(ctx->sensors->wPresOutRawVal) + "</p><br/>";
  html += "  <p>Start time: " + ctx->startTimeStr + "</p>";
  html += "  <p>Wi-fi signal: " + String(ctx->sensors->wifiRssi) + " dBm</p>";
  html += "</body>";
  html += "</html>";
  return html;
}