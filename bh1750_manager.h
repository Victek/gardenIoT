// bh1750_manager.h
// Gestor del sensor de luz BH1750FVI
// Vicente Soriano - 2025
// Reemplazo de LDR por sensor I2C de alta precisión

#ifndef BH1750_MANAGER_H
#define BH1750_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

// Dirección I2C del BH1750 (ADDR pin a GND = 0x23, ADDR pin a VCC = 0x5C)
#define BH1750_I2C_ADDR 0x23

// Comandos BH1750
#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_CONTINUOUS_HIGH_RES_MODE 0x10   // 1 lx resolución, 120ms
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2 0x11 // 0.5 lx resolución, 120ms
#define BH1750_CONTINUOUS_LOW_RES_MODE 0x13    // 4 lx resolución, 16ms
#define BH1750_ONE_TIME_HIGH_RES_MODE 0x20
#define BH1750_ONE_TIME_HIGH_RES_MODE_2 0x21
#define BH1750_ONE_TIME_LOW_RES_MODE 0x23

class BH1750Manager {
private:
  uint8_t i2cAddress;
  bool initialized;
  float lastLuxValue;
  unsigned long lastReadTime;
  
  // Enviar comando al BH1750
  bool sendCommand(uint8_t command) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(command);
    return (Wire.endTransmission() == 0);
  }
  
  // Leer 2 bytes del sensor
  uint16_t readRawValue() {
    uint16_t level = 0;
    
    Wire.requestFrom(i2cAddress, (uint8_t)2);
    if (Wire.available() == 2) {
      level = Wire.read();
      level <<= 8;
      level |= Wire.read();
    }
    
    return level;
  }

public:
  BH1750Manager(uint8_t addr = BH1750_I2C_ADDR) : i2cAddress(addr), initialized(false), lastLuxValue(0), lastReadTime(0) {}
  
  // Inicializar el sensor
  bool begin() {
    Serial.println("[BH1750] Inicializando sensor de luz...");
    
    // Verificar si el sensor responde
    Wire.beginTransmission(i2cAddress);
    if (Wire.endTransmission() != 0) {
      Serial.println("[BH1750] ❌ Sensor no detectado en dirección 0x" + String(i2cAddress, HEX));
      initialized = false;
      return false;
    }
    
    // Encender el sensor
    if (!sendCommand(BH1750_POWER_ON)) {
      Serial.println("[BH1750] ❌ Error al encender el sensor");
      initialized = false;
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Configurar modo de lectura continua de alta resolución
    if (!sendCommand(BH1750_CONTINUOUS_HIGH_RES_MODE)) {
      Serial.println("[BH1750] ❌ Error al configurar modo de lectura");
      initialized = false;
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(120)); // Esperar primera medición (120ms en modo HIGH RES)
    
    initialized = true;
    Serial.println("[BH1750] ✅ Sensor inicializado correctamente");
    Serial.println("[BH1750]    Dirección I2C: 0x" + String(i2cAddress, HEX));
    Serial.println("[BH1750]    Modo: Continuo Alta Resolución (1 lx)");
    
    return true;
  }
  
  // Leer nivel de luz en lux (0-65535)
  float readLightLevel() {
    if (!initialized) {
      Serial.println("[BH1750] ⚠️ Sensor no inicializado");
      return lastLuxValue; // Devolver último valor conocido
    }
    
    uint16_t rawValue = readRawValue();
    
    // Convertir a lux (fórmula del datasheet)
    // En modo HIGH_RES: lux = rawValue / 1.2
    float lux = rawValue / 1.2;
    
    lastLuxValue = lux;
    lastReadTime = millis();
    
    return lux;
  }
  
  // Obtener estado del sensor
  bool isInitialized() {
    return initialized;
  }
  
  // Obtener último valor leído (sin nueva lectura)
  float getLastValue() {
    return lastLuxValue;
  }
  
  // Obtener tiempo desde última lectura
  unsigned long getTimeSinceLastRead() {
    return millis() - lastReadTime;
  }
  
  // Resetear el sensor
  bool reset() {
    Serial.println("[BH1750] Reseteando sensor...");
    
    if (!sendCommand(BH1750_RESET)) {
      Serial.println("[BH1750] ❌ Error al resetear");
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Reconfigurar después del reset
    if (!sendCommand(BH1750_POWER_ON)) {
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    if (!sendCommand(BH1750_CONTINUOUS_HIGH_RES_MODE)) {
      return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(120));
    
    Serial.println("[BH1750] ✅ Sensor reseteado");
    return true;
  }
  
  // Apagar el sensor (ahorro de energía)
  void powerDown() {
    if (initialized) {
      sendCommand(BH1750_POWER_DOWN);
      Serial.println("[BH1750] 💤 Sensor en modo suspensión");
    }
  }
  
  // Encender el sensor
  void powerOn() {
    if (initialized) {
      sendCommand(BH1750_POWER_ON);
      vTaskDelay(pdMS_TO_TICKS(10));
      sendCommand(BH1750_CONTINUOUS_HIGH_RES_MODE);
      vTaskDelay(pdMS_TO_TICKS(120));
      Serial.println("[BH1750] ⚡ Sensor activado");
    }
  }
  
  // Diagnóstico del sensor
  void printDiagnostics() {
    Serial.println("\n[BH1750] 🔍 DIAGNÓSTICO DEL SENSOR");
    Serial.println("=====================================");
    Serial.println("  Estado: " + String(initialized ? "✅ Inicializado" : "❌ No inicializado"));
    Serial.println("  Dirección I2C: 0x" + String(i2cAddress, HEX));
    Serial.println("  Último valor: " + String(lastLuxValue, 1) + " lux");
    Serial.println("  Tiempo desde última lectura: " + String(getTimeSinceLastRead()) + " ms");
    
    if (initialized) {
      float testReading = readLightLevel();
      Serial.println("  Lectura de prueba: " + String(testReading, 1) + " lux");
      
      // Interpretación del valor
      if (testReading < 1) {
        Serial.println("  Interpretación: 🌑 Oscuridad total");
      } else if (testReading < 100) {
        Serial.println("  Interpretación: 🌙 Muy poca luz / Noche");
      } else if (testReading < 100) {
        Serial.println("  Interpretación: 💡 Iluminación artificial / Interior");
      } else if (testReading < 1000) {
        Serial.println("  Interpretación: ☁️ Día nublado / Sombra");
      } else if (testReading < 2000) {
        Serial.println("  Interpretación: ⛅ Día parcialmente nublado");
      } else {
        Serial.println("  Interpretación: ☀️ Sol directo / Pleno día");
      }
    }
    Serial.println("=====================================\n");
  }
};

// Instancia global del sensor BH1750
BH1750Manager lightSensor;

// Función de conversión LUX → Porcentaje (0-100%)
// Mapeo: 0 lux = 0%, 10000 lux = 100%
inline float luxToPercent(float lux) {
  // Usar constrain para mantener compatibilidad con sistema actual
  return constrain(map((long)lux, 0, 10000, 0, 100), 0, 100);
}

// Función de conversión Porcentaje → LUX (para offsets)
inline float percentToLux(float percent) {
  return map((long)percent, 0, 100, 0, 10000);
}

#endif // BH1750_MANAGER_H