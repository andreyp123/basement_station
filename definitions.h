#pragma once

#include <time.h>

struct SystemInfo
{
  String version;
  time_t startTime;
  String startTimeStr;
  String serverUrl;
  long wifiRssi;
  bool internet;

  SystemInfo(String v):
    version(v),
    startTime(0),
    startTimeStr(""),
    serverUrl(""),
    wifiRssi(0),
    internet(false)
  {
  }

  String toString() const
  {
    return "system: ver=" + version + ", time=" + startTimeStr + "/" + String(startTime) +
      ", url=" + serverUrl + ", wifi=" + String(wifiRssi) + ", inet=" + String(internet);
  }

  void setStartTime(time_t t)
  {
    startTime = t;
    
    char strBuf[24];
    strftime(strBuf, sizeof(strBuf), "%Y-%m-%d %H:%M:%S", localtime(&startTime));
    startTimeStr = String(strBuf);
  }

  String getWifiStr() { return String(wifiRssi) + " dBm"; }
  String getInternet() { return internet ? "on" : "off"; }
};

struct SensorsInfo
{
  float tempC;
  float humProc;
  int ldrRawVal;
  bool lightOn;
  int wPres1RawVal;
  float wPres1Bar;
  int wPres2RawVal;
  float wPres2Bar;

  String toString() const
  {
    return "sensors: temp=" + String(tempC, 2) + ", hum=" + String(humProc, 2) +
      ", light=" + String(lightOn) + "/" + String(ldrRawVal) +
      ", wpres1=" + String(wPres1Bar, 2) + "/" + String(wPres1RawVal) +
      ", wpres2=" + String(wPres2Bar, 2) + "/" + String(wPres2RawVal);
  }

  String getTempStr()  { return String(tempC, 1) + " C"; }
  String getHumStr()   { return String(humProc, 1) + " %"; }
  String getLightStr() { return lightOn ? "on" : "off"; }
  String getPres1Str() { return String(wPres1Bar, 1) + " bar"; }
  String getPres2Str()  { return String(wPres2Bar, 1) + " bar"; }
};

struct Context
{
  SystemInfo* systemInfo;
  SensorsInfo* sensors;
  QueueHandle_t queue;

  Context(SystemInfo* i, SensorsInfo* s, QueueHandle_t q)
  {
    systemInfo = i;
    sensors = s;
    queue = q;
  }
};

enum EventType
{
  undefined = 0,
  lowPressure = 1,
  normPressure = 2
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

struct AvgBucket
{
  const int size;
  float sum = 0;
  int index = 0;

  AvgBucket(int s) : size(s) { }
  
  float addVal(float val)
  {
    float avg = -1; // not ready
    sum += val;
    if (index == size - 1)
    {
      avg = sum / size;
      sum = 0;
      index = 0;
    }
    else
    {
      index++;
    }
    return avg;
  }
};
