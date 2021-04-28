#pragma ones

#define LCD_BTN_ON true
#define LCD_BTN_OFF false


class LcdButton
{
private:
  static const uint8_t LCD_BTN_PIN = 5;
  static const int REINIT_LCD_TIMEOUT = 5000;
  static const int REBOOT_TIMEOUT = 10000; // REBOOT_TIMEOUT should be greater than REINIT_LCD_TIMEOUT
  
  LcdProxy* _lcd;
  bool _lcdBtnMode;
  long _lcdBtnOffLastMillis;

public:
  LcdButton(LcdProxy* lcd) :
    _lcd(lcd),
    _lcdBtnMode(LCD_BTN_OFF),
    _lcdBtnOffLastMillis(0)
  {
    pinMode(LCD_BTN_PIN, INPUT);
  }

  bool process()
  {
    if (digitalRead(LCD_BTN_PIN) == HIGH)
    {
      long lcdBtnOnMillis = millis() - _lcdBtnOffLastMillis;
      if (lcdBtnOnMillis > REBOOT_TIMEOUT) // if hold button during REBOOT_TIMEOUT - reboot
      {
        ESP.restart();
      }
      else if (lcdBtnOnMillis > REINIT_LCD_TIMEOUT) // if hold button during REINIT_LCD_TIMEOUT - reinit lcd
      {
        _lcd->init(false);
      }
      
      _lcd->enable();
      _lcdBtnMode = LCD_BTN_ON;
    }
    else
    {
      _lcd->checkDisable();
      _lcdBtnMode = LCD_BTN_OFF;
      _lcdBtnOffLastMillis = millis();
    }

    return _lcdBtnMode;
  }
};
