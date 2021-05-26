#include <math.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "definitions.h"
#include "SensorsHandler.h"


const int SensorsHandler::LIGHT_ON_THRESHOLD = 15;
const float SensorsHandler::WPRES1_LOW_THRESHOLD = 0.1;
const float SensorsHandler::WPRES1_NORM_THRESHOLD = 0.6;
const float SensorsHandler::WPRES2_LOW_THRESHOLD = 2.2;
const float SensorsHandler::WPRES2_NORM_THRESHOLD = 2.3;

void SensorsHandler::init()
{
  Serial.println("[sens] initializing sensors...");
  pinMode(DHT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(WPRES1_PIN, INPUT);
  pinMode(WPRES2_PIN, INPUT);
  pinMode(WPRES_LED_PIN, OUTPUT);

  _dht.begin();
  sensor_t sens;
  _dht.temperature().getSensor(&sens);
  Serial.println("[sens] temp sensor=" + String(sens.name) + ", ver=" + String(sens.version) + ", id=" + String(sens.sensor_id) +
    ", maxVal=" + String(sens.max_value, 2) + ", minVal=" + String(sens.min_value, 2) + ", resolution=" + String(sens.resolution, 2));
  _dht.humidity().getSensor(&sens);
  Serial.println("[sens] hum sensor=" + String(sens.name) + ", ver=" + String(sens.version) + ", id=" + String(sens.sensor_id) +
    ", maxVal=" + String(sens.max_value, 2) + ", minVal=" + String(sens.min_value, 2) + ", resolution=" + String(sens.resolution, 2));
  Serial.println("[sens] min delay: " + String(sens.min_delay / 1000) + "ms");
}

void SensorsHandler::process()
{
  SensorsInfo* sens = _context->sensors;
  float avg = -1;
  
  // temperature
  sensors_event_t event;
  _dht.temperature().getEvent(&event);
  if (!isnan(event.temperature))
  {
    if ((avg = _tempBucket.addVal(event.temperature)) != -1)
      sens->tempC = avg;
  }
  else {}
    //Serial.println("[sens] error reading temp");
  
  // humidity
  _dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity))
  {
    if ((avg = _humBucket.addVal(event.relative_humidity)) != -1)
      sens->humProc = avg;
  }
  else {}
    //Serial.println("[sens] error reading hum");

  // lightness
  int rawVal = analogRead(LDR_PIN);
  if ((avg = _ldrValBucket.addVal(rawVal)) != -1)
  {
    sens->ldrRawVal = (int)avg;
    sens->lightOn = sens->ldrRawVal >= LIGHT_ON_THRESHOLD;
  }

  // 1st water pressure
  rawVal = analogRead(WPRES1_PIN);
  if ((avg = _wPres1Bucket.addVal(rawVal)) != -1)
  {
    sens->wPres1RawVal = (int)avg;
    sens->wPres1Bar = getWaterPressureBars(avg);
    _wPres1Valid = true; //validateWaterPressure(sens->wPres1Bar, _prevWPres1Bar, WPRES1_LOW_THRESHOLD, WPRES1_NORM_THRESHOLD);
    _prevWPres1Bar = sens->wPres1Bar;
  }

  // 2nd water pressure
  rawVal = analogRead(WPRES2_PIN);
  if ((avg = _wPres2Bucket.addVal(rawVal)) != -1)
  {
    sens->wPres2RawVal = (int)avg;
    sens->wPres2Bar = getWaterPressureBars(avg);
    _wPres2Valid = validateWaterPressure(sens->wPres2Bar, _prevWPres2Bar, WPRES2_LOW_THRESHOLD, WPRES2_NORM_THRESHOLD);
    _prevWPres2Bar = sens->wPres2Bar;
  }

  // led
  bool ledOn = !_wPres1Valid || !_wPres2Valid;
  digitalWrite(WPRES_LED_PIN, ledOn);

  // log sensors (after filling bucket)
  if (avg != -1)
  {
    Serial.println("[sens] " + sens->toString());
  }
}

float SensorsHandler::getWaterPressureBars(float rawVal)
{
  /*
  The theory is the following:
  float v1 = val * 3.3 / 4096; // voltage at the 3.3v input pin
  float v2 = v1 * 1.5; // voltage at the sensor output (mod 5v)
  float pKpa = (v2 - 0.5) / 4.0 * 1200.0; // according to spec [0.5..4.5]v to [0..1.2]Mpa
  float pBar = pKpa / 100.0; // 1Kpa = 100bar
  
  But for some reason, the theory does not fit the actual measurements.
  Magic numbers below are received empirically.
  */

  float retVal = 0.00409165 * rawVal - 0.47626841;
  return retVal > 0 ? retVal : 0;
}

bool SensorsHandler::validateWaterPressure(float wPresBar, float prevWPresBar, float lowThreshold, float normThreshold)
{
  bool retVal = wPresBar >= lowThreshold;

  // update actuality of the wpres event
  if (fabs(millis() - _prevWPresEventTime) > WPRES_STATE_ACTUAL_TIME)
  {
    _prevWPresEvent = WPRES_UNDEF;
    _prevWPresEventTime = millis();
  }

  // validate wpres and generate events:
  // low pressure
  if (wPresBar < lowThreshold && prevWPresBar >= lowThreshold && _prevWPresEvent != WPRES_LOW)
  {
    EventMessage eMsg(lowPressure);
    xQueueSend(_context->queue, &eMsg, 0);
    Serial.println("[sens] prepared lowPressure event");
    _prevWPresEvent = WPRES_LOW;
    _prevWPresEventTime = millis();
  }
  // normal pressure
  else if (wPresBar >= normThreshold && prevWPresBar < normThreshold && _prevWPresEvent != WPRES_NORM)
  {
    EventMessage eMsg(normPressure);
    xQueueSend(_context->queue, &eMsg, 0);
    Serial.println("[sens] prepared normPressure event");
    _prevWPresEvent = WPRES_NORM;
    _prevWPresEventTime = millis();
  }

  return retVal;
}
