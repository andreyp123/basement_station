#include "LcdProxy.h"

LcdProxy::LcdProxy()
{
  _lcd = new LiquidCrystal_I2C(LCD_ADDR, LCD_COLS, LCD_ROWS);
  _init = false;
  _enabled = false;
  _prevLine1 = "";
  _prevLine2 = "";
}

LcdProxy::~LcdProxy()
{
  delete _lcd;
}

void LcdProxy::init(bool enabled)
{
  _lcd->init();
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

void LcdProxy::enable()
{
  if (!_init)
    return;

  if (!_enabled)
  {
    _lcd->display();
    _lcd->backlight();
    _enabled = true;
  }
  _enableMillis = millis();
}

void LcdProxy::checkDisable()
{
  if (!_init)
    return;
  
  if (_enabled && millis() - _enableMillis > LCD_ENABLE_TIMEOUT)
  {
    _lcd->noDisplay();
    _lcd->noBacklight();
    _enabled = false;
  }
}

void LcdProxy::print(String line1, String line2)
{
  if (!_init)
    return;
  
  if (line1 != _prevLine1 || line2 != _prevLine2)
  {
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print(line1);
    _lcd->setCursor(0, 1);
    _lcd->print(line2);

    _prevLine1 = line1;
    _prevLine2 = line2;
  }
}

void LcdProxy::printSensors(SensorsInfo* sens)
{
  if (!_init)
    return;
  
  String line1 = "T: " + String(sens->tempC, 1) + "  H: " + String(sens->humProc, 1);
  String line2 = "P1: ---  P2: " + String(sens->wPresBar, 1);
  LcdProxy::print(line1, line2);  
}
