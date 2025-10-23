#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

// ===========================
// CONFIGURACIÓN HARDWARE ESP32-S3
// ===========================
#define PIN_SOIL1 5          // GPIO 5 - Sensor humedad suelo 1
#define PIN_SOIL2 6          // GPIO 6 - Sensor humedad suelo 2
#define PIN_PUMP1 7          // GPIO 7 - Relé bomba 1
#define PIN_PUMP2 15         // GPIO 15 - Relé bomba 2
#define PIN_WATER_SUPPLY 12  // GPIO 12 - Sensor Agua (corregí el comentario)
#define PIN_BUZZER 16        // GPIO 16 - Buzzer
#define PIN_NEOPIXEL 48      // GPIO 48 - LED WS2812
#define PIN_WIFI_RESET 18    // GPIO 18 - Botón reset WiFi
#define NUMPIXELS 1          // Número de LEDs Neopixel
#define BME_SDA 8            // GPIO 8 - I2C SDA
#define BME_SCL 9            // GPIO 9 - I2C SCL
#define BME_I2C_ADDR 0x77    // Dirección I2C BME680
#define RTC_I2C_ADDR 0x57    // Dirección del RTC DS3231
#define BH1750_I2C_ADDR 0x23 // Dirección I2C BH1750FVI

#endif