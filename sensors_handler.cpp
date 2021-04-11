#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "definitions.h"
#include "sensors_handler.h"


DHT_Unified dht(DHT_PIN, DHT_TYPE);
int prevLdrRawVal = -1; // previous ldr value (for validation logic)
float prevWPresBarVal = -1; // previous water pressure value (for validation logic)


void sensorsHandlerInit()
{
  Serial.println("[sens] initializing sensors...");
  pinMode(DHT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  dht.begin();

  sensor_t sens;
  dht.temperature().getSensor(&sens);
  Serial.println("[sens] temp sensor=" + String(sens.name) + ", ver=" + String(sens.version) + ", id=" + String(sens.sensor_id) +
    ", maxVal=" + String(sens.max_value, 2) + ", minVal=" + String(sens.min_value, 2) + ", resolution=" + String(sens.resolution, 2));
  dht.humidity().getSensor(&sens);
  Serial.println("[sens] hum sensor=" + String(sens.name) + ", ver=" + String(sens.version) + ", id=" + String(sens.sensor_id) +
    ", maxVal=" + String(sens.max_value, 2) + ", minVal=" + String(sens.min_value, 2) + ", resolution=" + String(sens.resolution, 2));
  Serial.println("[sens] min delay: " + String(sens.min_delay / 1000) + "ms");
}

float getWaterPressureBars(int rawVal)
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

void sensorsHandlerProcess(void* params)
{
  Context* ctx = (Context*)params;
  Sensors* sens = ctx->sensors;
  
  while (true)
  {
    Serial.println("[sens] reading sensors...");
    
    // temperature
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (!isnan(event.temperature))
      sens->tempC = event.temperature;
    else
      Serial.println("[sens] error reading temp");
    
    // humidity
    dht.humidity().getEvent(&event);
    if (!isnan(event.relative_humidity))
      sens->humProc = event.relative_humidity;
    else
      Serial.println("[sens] error reading hum");

    // ldr
    sens->ldrRawVal = analogRead(LDR_PIN);

    // water pressure
    int rawVal = analogRead(WPRES_PIN);
    sens->wPresRawVal = rawVal;
    sens->wPresBar = getWaterPressureBars(rawVal);

    Serial.println("[sens] " + sens->toString());

    // perform validations
    //validateLight(ctx);
    validateWaterPressure(ctx);
    
    vTaskDelay(SENS_PROCESS_DELAY / portTICK_RATE_MS);
  }
}

void validateLight(Context* ctx)
{
  int ldrRawVal = ctx->sensors->ldrRawVal;
  
  if (ldrRawVal < LDR_THRESHOLD)
    {
      if (prevLdrRawVal >= LDR_THRESHOLD)
      {
        EventMessage eMsg(lowLight);
        xQueueSend(ctx->queue, &eMsg, 0);
        Serial.println("[sens] prepared lowLight event");
      }
    }
    else
    {
      if (prevLdrRawVal < LDR_THRESHOLD && prevLdrRawVal != -1)
      {
        EventMessage eMsg(normLight);
        xQueueSend(ctx->queue, &eMsg, 0);
        Serial.println("[sens] prepared normLight event");
      }
    }
    
    prevLdrRawVal = ldrRawVal;
}

void validateWaterPressure(Context* ctx)
{
  float wPresBar = ctx->sensors->wPresBar;
  
  if (wPresBar < WPRES_BAR_THRESHOLD)
    {
      if (prevWPresBarVal >= WPRES_BAR_THRESHOLD)
      {
        EventMessage eMsg(lowPressure);
        xQueueSend(ctx->queue, &eMsg, 0);
        Serial.println("[sens] prepared lowPressure event");
      }
    }
    else
    {
      if (prevWPresBarVal < WPRES_BAR_THRESHOLD && prevWPresBarVal != -1)
      {
        EventMessage eMsg(normPressure);
        xQueueSend(ctx->queue, &eMsg, 0);
        Serial.println("[sens] prepared normPressure event");
      }
    }
    
    prevWPresBarVal = wPresBar;
}
