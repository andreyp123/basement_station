#pragma ones

#define DHT_PIN 19 // digital pin for temperature/humidity sensor DHT11
#define DHT_TYPE DHT11 // dht sensor type

// only pins 32-39 can be used for ADC in case of wi-fi (https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)
#define LDR_PIN 32 // analog pin for light dependency resistor
#define WPRES_PIN 34 // analog pin for output water pressure sensor HK1100C

#define LCD_ADDR 0x27 // I2C address of the LCD
#define LCD_ROWS 2
#define LCD_COLS 16

#define LIGHT_ON_THRESHOLD 15
#define WPRES_LOW_THRESHOLD 2.0
#define WPRES_NORM_THRESHOLD 2.1

#define SENS_PROCESS_DELAY 500 // timeout between reading sensors data

void sensorsHandlerInit();

void sensorsHandlerProcess(void* params);
