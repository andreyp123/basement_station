#pragma ones

#include <LiquidCrystal_I2C.h>
#include "definitions.h"

#define LCD_ADDR 0x27 // I2C address of the LCD
#define LCD_ROWS 2
#define LCD_COLS 16
#define LCD_ENABLE_TIMEOUT 20000 // lcd is to enabled during 20 sec

class LcdProxy
{
private:
  LiquidCrystal_I2C* _lcd;
  bool _init;
  bool _enabled;
  long _enableMillis;
  String _prevLine1;
  String _prevLine2;

public:
  LcdProxy();
  ~LcdProxy();
  void init(bool enabled);
  void enable();
  void checkDisable();
  void print(String line1, String line2);
  void printSensors(SensorsInfo* sens);
};
