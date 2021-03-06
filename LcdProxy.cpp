#include <WiFi.h>
#include "LcdProxy.h"


void LcdProxy::init(bool enabled)
{
  Serial.println("[lcd] initializing lcd...");
  
  _lcd.init();
  _init = true;
  if (enabled)
  {
    LcdProxy::enable();
  }
  else
  {
    _enableMillis = -LCD_ENABLE_TIMEOUT;
  }
}

bool LcdProxy::enable()
{
  if (!_init)
    return false;

  bool retVal = false;
  if (!_enabled)
  {
    _lcd.display();
    _lcd.backlight();
    _enabled = true;
    Serial.println("[lcd] enabled");
    retVal = true;
  }
  _enableMillis = millis();
  return retVal;
}

bool LcdProxy::disable()
{
  if (!_init)
    return false;
  
  bool retVal = false;
  if (_enabled)
  {
    _lcd.noDisplay();
    _lcd.noBacklight();
    _enabled = false;
    Serial.println("[lcd] disabled");
    retVal = true;
  }
  return retVal;
}

bool LcdProxy::checkDisable()
{
  if (!_init)
    false;

  bool retVal = false;
  if (_enabled && millis() - _enableMillis > LCD_ENABLE_TIMEOUT)
  {
    retVal = disable();
  }
  return retVal;
}

void LcdProxy::print(String line1, String line2)
{
  if (!_init)
    return;
  
  if (line1 != _prevLine1 || line2 != _prevLine2)
  {
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(line1);
    _lcd.setCursor(0, 1);
    _lcd.print(line2);
    Serial.println("[lcd] print line1: " + line1 + "; line2: " + line2);

    _prevLine1 = line1;
    _prevLine2 = line2;
  }
}

void LcdProxy::printSensors(SensorsInfo* sens)
{
  if (!_init)
    return;
  
  String line1 = "T: " + String(sens->tempC, 1) + "  H: " + String(sens->humProc, 1);
  String line2 = "P1: " + String(sens->wPres1Bar, 1) + "  P2: " + String(sens->wPres2Bar, 1);
  LcdProxy::print(line1, line2);
}

void LcdProxy::printSystemInfo(SystemInfo* sysInfo)
{
  if (!_init)
    return;
  
  String line1 = "Internet: " + sysInfo->getInternet();
  String line2 = "WiFi: " + sysInfo->getWifiStr();
  
  LcdProxy::print(line1, line2);
}
