#pragma ones

#include <DHT.h>
#include <DHT_U.h>
#include "definitions.h"

class SensorsHandler
{
private:
  static const uint8_t DHT_PIN = 19;  // digital pin for temperature/humidity sensor DHT11
  // only pins 32-39 can be used for ADC in case of wi-fi:
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html
  static const uint8_t LDR_PIN = 32; // analog pin for light dependency resistor
  static const uint8_t WPRES_PIN = 34; // analog pin for output water pressure sensor HK1100C
  static const int BUCKET_SIZE = 10;
  static const int LIGHT_ON_THRESHOLD;
  static const float WPRES_LOW_THRESHOLD;
  static const float WPRES_NORM_THRESHOLD;

  Context* _context;
  DHT_Unified _dht;
  AvgBucket _tempBucket;
  AvgBucket _humBucket;
  AvgBucket _ldrValBucket;
  AvgBucket _presBucket;

  float _prevWPresBarVal; // previous water pressure value (for validation logic)

  float getWaterPressureBars(int rawVal);
  void validateWaterPressure();

public:
  SensorsHandler(Context* context) :
    _context(context),
    _dht(DHT_PIN, DHT11),
    _tempBucket(BUCKET_SIZE),
    _humBucket(BUCKET_SIZE),
    _ldrValBucket(BUCKET_SIZE),
    _presBucket(BUCKET_SIZE),
    _prevWPresBarVal(-1)
  {
  }
  void init();
  void process();
};
