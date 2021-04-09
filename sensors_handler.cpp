#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "definitions.h"
#include "sensors_handler.h"


DHT_Unified dht(DHT_PIN, DHT_TYPE);
int prevLdrRawVal = -1;// previous ldr value (for validation logic)


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
    sens->wPresOutRawVal = analogRead(WPRES_OUT_PIN);

    Serial.println("[sens] " + sens->toString());

    // perform validations
    validateLight(ctx);
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
  // TODO
}
