#include <WiFi.h>
#include "LcdWithButton.h"

void LcdWithButton::init(bool lcdEnabled)
{
  _lcd->init(lcdEnabled);
  _init = true;
}

void LcdWithButton::print(String line1, String line2)
{
  if (!_init)
    return;
  _lcd->print(line1, line2);
}

void LcdWithButton::process()
{
  if (!_init)
    return;
  
  if (digitalRead(LCD_BTN_PIN) == HIGH) // if btn is pressed
  {
    // handle button hold
    long lcdBtnOnMillis = millis() - _lcdBtnOffLastMillis;
    if (lcdBtnOnMillis > REBOOT_TIMEOUT) // if hold button during REBOOT_TIMEOUT - reboot
    {
      ESP.restart();
    }
    else if (lcdBtnOnMillis > REINIT_LCD_TIMEOUT) // if hold button during REINIT_LCD_TIMEOUT - reinit lcd
    {
      _lcd->init(false);
      _printSensors = true;
    }

    // handle button press several times
    if (_lcd->enable()) // if lcd is just enabled, print sensors
      _printSensors = true;
    else if (_lcdBtnMode == LCD_BTN_OFF) // if lcd is already enabled, and btn is pressed (ON after OFF), switch printing content
      _printSensors = !_printSensors;

    // set current button mode
    _lcdBtnMode = LCD_BTN_ON;
  }
  else // if btn is not pressed
  {
    _lcd->checkDisable();
    _lcdBtnMode = LCD_BTN_OFF;
    _lcdBtnOffLastMillis = millis();
  }

  // print content (sensors or system info)
  if (_printSensors)
    _lcd->printSensors(_context->sensors);
  else
    _lcd->printSystemInfo(_context->systemInfo);
}
