#pragma ones

#include <DHT.h>
#include <DHT_U.h>
#include "definitions.h"

/*
 * Implements functionality for reading and processing various sensors such as DHT11 (temperature/humidity),
 * HK1100C (water pressure), LDR and LED.
 */
class SensorsHandler
{
private:
  static const uint8_t DHT_PIN = 19;  // digital pin for temperature/humidity sensor DHT11
  static const uint8_t WPRES_LED_PIN = 4; // digital output pin for led, indicating low water pressure
  // only pins 32-39 can be used for ADC in case of wi-fi:
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html
  static const uint8_t LDR_PIN = 33; // analog pin for light dependency resistor
  static const uint8_t WPRES1_PIN = 34; // analog pin for 1st water pressure sensor HK1100C
  static const uint8_t WPRES2_PIN = 35; // analog pin for 2nd water pressure sensor HK1100C
  
  static const int BUCKET_SIZE = 10;
  static const int LIGHT_ON_THRESHOLD;
  static const float WPRES1_LOW_THRESHOLD;
  static const float WPRES1_NORM_THRESHOLD;
  static const float WPRES2_LOW_THRESHOLD;
  static const float WPRES2_NORM_THRESHOLD;

  Context* _context;
  DHT_Unified _dht;
  
  // buckets to collect sensors avg values
  AvgBucket _tempBucket;
  AvgBucket _humBucket;
  AvgBucket _ldrValBucket;
  AvgBucket _wPres1Bucket;
  AvgBucket _wPres2Bucket;

  // previous water pressure values
  float _prevWPres1Bar;
  float _prevWPres2Bar;
  // defines if water pressure values are valid
  bool _wPres1Valid;
  bool _wPres2Valid;

  float getWaterPressureBars(float rawVal);
  bool validateWaterPressure(float wPresBar, float prevWPresBar, float lowThreshold, float normThreshold);

public:
  SensorsHandler(Context* context) :
    _context(context),
    _dht(DHT_PIN, DHT11),
    _tempBucket(BUCKET_SIZE),
    _humBucket(BUCKET_SIZE),
    _ldrValBucket(BUCKET_SIZE),
    _wPres1Bucket(BUCKET_SIZE),
    _wPres2Bucket(BUCKET_SIZE),
    _prevWPres1Bar(-1),
    _prevWPres2Bar(-1),
    _wPres1Valid(true),
    _wPres2Valid(true)
  {
  }
  void init();
  void process();
};
