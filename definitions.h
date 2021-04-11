#pragma once

struct Sensors
{
  // system
  long wifiRssi;

  // external sensors
  float tempC;
  float humProc;
  int ldrRawVal;
  int wPresRawVal;
  float wPresBar;

  String toString() const
  {
    return "sensors: wifi=" + String(wifiRssi) + ", temp=" + String(tempC, 1) + ", hum=" + String(humProc, 1) +
      ", ldr=" + String(ldrRawVal) + ", wpres_raw=" + String(wPresRawVal) + ", wpres_bar=" + String(wPresBar, 1);
  }
};

struct Context
{
  time_t startTime;
  String startTimeStr;
  Sensors* sensors;
  QueueHandle_t queue;

  Context(time_t t, Sensors* s, QueueHandle_t q)
  {
    startTime = t;
    sensors = s;
    queue = q;

    char strBuf[24];
    strftime(strBuf, sizeof(strBuf), "%Y-%m-%d %H:%M:%S utc", localtime(&startTime));
    startTimeStr = String(strBuf);
  }
};

enum EventType
{
  undefined = 0,
  lowLight = 1,
  normLight = 2,
  lowPressure = 3,
  normPressure = 4
};

struct EventMessage
{
  EventType eType = undefined;
  unsigned long timespan = 0;
  int ttl = -1;

  EventMessage() { }

  EventMessage(EventType t)
  {
    eType = t;
    timespan = millis();
  }

  EventMessage(EventType t, int ttl)
  {
    eType = t;
    timespan = millis();
    ttl = ttl;
  }

  String toString() const
  {
    return "event: type=" + String(eType) + ", timespan=" + String(timespan) + ", ttl=" + ttl;
  }
};
