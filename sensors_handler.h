#pragma ones

#define DHT_PIN 19 // digital pin for temperature/humidity sensor DHT11
#define DHT_TYPE DHT11 // dht sensor type

// only pins 32-39 can be used for ADC in case of wi-fi (https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)
#define LDR_PIN 32 // analog pin for light dependency resistor
#define WPRES_OUT_PIN 34 // analog pin for output water pressure sensor HK1100C

#define LDR_THRESHOLD 150
#define WPRES_OUT_BAR_THRESHOLD 2

#define SENS_PROCESS_DELAY 5000 // timeout between reading sensors data


void sensorsHandlerInit();

void sensorsHandlerProcess(void* params);

void validateLight(Context* ctx);

void validateWaterPressure(Context* ctx);
