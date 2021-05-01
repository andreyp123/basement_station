#pragma ones

#include "LcdProxy.h"

/*
 * Defines display (LCD) along with control button.
 */
class LcdWithButton
{
private:
  static const bool LCD_BTN_ON = true;
  static const bool LCD_BTN_OFF = false;
  static const uint8_t LCD_BTN_PIN = 5;
  static const int REINIT_LCD_TIMEOUT = 5000;
  static const int REBOOT_TIMEOUT = 10000; // REBOOT_TIMEOUT should be greater than REINIT_LCD_TIMEOUT

  Context* _context;
  LcdProxy* _lcd;
  bool _init;
  bool _lcdBtnMode;
  long _lcdBtnOffLastMillis;
  bool _printSensors;

public:
  LcdWithButton(Context* context):
    _context(context),
    _init(false),
    _lcdBtnMode(LCD_BTN_OFF),
    _lcdBtnOffLastMillis(0),
    _printSensors(true)
  {
    pinMode(LCD_BTN_PIN, INPUT);
    _lcd = new LcdProxy();
  }
  
  ~LcdWithButton()
  {
    delete _lcd;
  }
  
  void init(bool lcdEnabled);
  void print(String line1, String line2);
  void process();
};
