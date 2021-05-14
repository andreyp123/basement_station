# Basement station

## Description

The station is placed in my house basement to measure:
- temperature
- humidity
- lightness
- water pressure before the pump (aka input pressure)
- water pressure after the pump (aka output pressure)
- water pump mode (on/off) -- TODO

The station shows current measurements on the LCD screen.

The station hosts web-server to retrieve simple web page containing current measurements.

The station hosts telegram bot to
- retrieve current measurements
- notify about various events, including low water pressure

The station hosts MQTT client to send current measurements into smart home system. -- TODO

## Technicat details

The project is based on ESP32, 30 pin version.
Pin map: https://raw.githubusercontent.com/AchimPieters/esp32-homekit-camera/master/Images/ESP32-30PIN-DEVBOARD.png

Sensors and modules:
- Temperature/humidity DHT11
- Water pressure HK1100C
- LDR GL5516
- LCD 1602A + I2C adapter
- LED

Platforms and libraries:
- Arduino IDE
- Esp32 board by Espressif (https://github.com/espressif/arduino-esp32)
  - additional boards manager urls: https://dl.espressif.com/dl/package_esp32_index.json
  - board: DOIT ESP32 DEVKIT V1
- Adafruit Unified Sensor (https://github.com/adafruit/Adafruit_Sensor)
- DHT sensor library (https://github.com/adafruit/DHT-sensor-library)
- Universal Telegram Bot (https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
- Liquid Crystal I2C (https://github.com/johnrickman/LiquidCrystal_I2C). Libraries Manager in Arduino IDE has older version, so the library is installed manually from github.
- ESP32 Ping (https://github.com/marian-craciunescu/ESP32Ping)

## TODO

- Wifi: calculate avg signal strength to avoid too quick value changes
- Smart home system integration: send measurements to mqtt
- Telegram bot: skip too old messages
- Voltage detector in the pump circuit
