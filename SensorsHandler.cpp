#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "definitions.h"
#include "SensorsHandler.h"


const int SensorsHandler::LIGHT_ON_THRESHOLD = 15;
const float SensorsHandler::WPRES_LOW_THRESHOLD = 2.0;
const float SensorsHandler::WPRES_NORM_THRESHOLD = 2.1;

void SensorsHandler::init()
{
  Serial.println("[sens] initializing sensors...");
  pinMode(DHT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(WPRES_PIN, INPUT);

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

  // water pressure
  rawVal = analogRead(WPRES_PIN);
  if ((avg = _presBucket.addVal(rawVal)) != -1)
  {
    sens->wPresRawVal = (int)avg;
    sens->wPresBar = getWaterPressureBars(rawVal);
    validateWaterPressure();
  }

  // log sensors (after filling bucket)
  if (avg != -1)
  {
    Serial.println("[sens] " + sens->toString());
  }
}

float SensorsHandler::getWaterPressureBars(int rawVal)
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
  
  return 0.00409165 * rawVal - 0.47626841;
}

void SensorsHandler::validateWaterPressure()
{
  float wPresBar = _context->sensors->wPresBar;
  
  if (wPresBar < WPRES_LOW_THRESHOLD && _prevWPresBarVal >= WPRES_LOW_THRESHOLD)
  {
    EventMessage eMsg(lowPressure);
    xQueueSend(_context->queue, &eMsg, 0);
    Serial.println("[sens] prepared lowPressure event");
  }
  else if (wPresBar >= WPRES_NORM_THRESHOLD && _prevWPresBarVal < WPRES_NORM_THRESHOLD && _prevWPresBarVal != -1)
  {
    EventMessage eMsg(normPressure);
    xQueueSend(_context->queue, &eMsg, 0);
    Serial.println("[sens] prepared normPressure event");
  }
  
  _prevWPresBarVal = wPresBar;
}
