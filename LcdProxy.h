#pragma ones

#include <LiquidCrystal_I2C.h>
#include "definitions.h"


class LcdProxy
{
private:
  static const uint8_t LCD_ADDR = 0x27; // I2C address of the LCD
  static const uint8_t LCD_ROWS = 2;
  static const uint8_t LCD_COLS = 16;
  static const int LCD_ENABLE_TIMEOUT = 20000; // lcd is to enabled during 20 sec

  LiquidCrystal_I2C _lcd;
  bool _init;
  bool _enabled;
  long _enableMillis;
  String _prevLine1;
  String _prevLine2;

public:
  LcdProxy():
    _lcd(LCD_ADDR, LCD_COLS, LCD_ROWS),
    _init(false),
    _enabled(false),
    _enableMillis(0),
    _prevLine1(""),
    _prevLine2("")
  {
  }
  void init(bool enabled);
  void enable();
  void checkDisable();
  void print(String line1, String line2);
  void printSensors(SensorsInfo* sens);
};
