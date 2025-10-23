// Sistema Riego
// Vicente Soriano@092025

// ESP32S3, RTOS, arduino IDE
// Funciones:
// Web, Pronóstico Meteo, Telegram
// 2 canales,
// humedad ambiente, humedad del suelo, temperatura ambiente, presión atm. y VOC
// Luz Solar local y Radiación solar externa por Servicio Meteo.100
// humedad suelo (2 canales),
// modo auto, Manual, adaptativo (autoaprendizaje)
// Inferencias para extraer el VPD y autoriego por decisión con sensores locales
// Compensación de sensores de humedad por tipo de suelo.
// Calibración guiada de Sensores de humedad de suelo.
// Tarjeta Meteo-Open (solo si se selecciona en Weather y hay conexión a internet disponible)
// Sistema de restricciones de riego por horario y por viento excesivo (externo)
// mDNS
// Backup/Restore completo
// Conexión de ia_dashboard a salida de inferencia
// Comandos para la API .. miriego.local/api/sensors , /api/status , /api/learning"
// Ayuda-manual online
// Sistema de reloj en backup con exactitud de +-2 ppm.
// BH1750FVI incluido, sustituye a LDR


// TODO para la siguiente versión.
// MQTT / WEBHook

// ===========================
// DECLARACIÓN ANTICIPADA
// ===========================
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "pin_definitions.h"    // ← PRIMERO (define pines y hardware)
#include "system_types.h"       // ← SEGUNDO (usa los pines)  
#include "system_setup.h"       // ← TERCERO (usa las estructuras)
#include "html_templates.h"     // ← CUARTO
#include "ia-dashboard.h"
#include "adaptative_learning.h" 
#include "vpd_functions.h"


// ===========================
// CREDENCIALES WIFI FALLBACK
// ===========================
const char* WIFI_SSID_FALLBACK = "";
const char* WIFI_PASS_FALLBACK = "";

// ===========================
// CONFIGURACIÓN mDNS
// ===========================
//const char* MDNS_HOSTNAME = "miriego";  // Acceso: http://miriego.local
const char* MDNS_HOSTNAME = "miriego";
// para detectar si el acceso via web está siendo usado.
unsigned long lastWebRequest = 0;

struct ZoneIrrigationDecision {
  bool shouldIrrigate = false;
  String reason = "";
  float vpdFactor = 1.0;
  float tempFactor = 1.0;
  float urgency = 0.0;
  String analysis = "";
};


  SensorHistory sensorHistory;
  bool internetWarningLogged = false;
  IrrigationCycle currentCycle1, currentCycle2;
  ZoneLearning learningZone1, learningZone2;
  AdaptiveLearningSystem adaptiveSystem;

  // NUEVOS CAMPOS PARA INFERENCIA INTELIGENTE
  float irrigationNeed1 = 0;  // Necesidad calculada por inferencia (0-1)
  float irrigationNeed2 = 0;  // Necesidad calculada por inferencia (0-1)
  unsigned long lastIrrigation1 = 0;
  unsigned long lastIrrigation2 = 0;

// ===========================
// VARIABLES GLOBALES
// ===========================
Adafruit_BME680 bme;
WebServer server(80);
DNSServer dnsServer;
WebServer configServer(80);
SemaphoreHandle_t mutexData = NULL;
SemaphoreHandle_t mutexConfig;
Config config;
SensorData data;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// =========================================================
// RTC DS3231 (definido en rtc_manager.h, solo como referencia
//==========================================================

// Variables WiFi
String ssid_stored = "";
String pass_stored = "";
bool wifiConnected = false;
bool wifiConfigMode = false;

// Variables para logs del monitor serie
String serialLogs[10];  // Buffer circular de 10 logs
int logIndex = 0;
unsigned long lastLogTime = 0;

// Variables para el botón de reset WiFi
unsigned long lastButtonPress = 0;
bool buttonPressed = false;
unsigned long buttonPressStart = 0;
const unsigned long BUTTON_HOLD_TIME = 3000;  // 3 segundos
const unsigned long DEBOUNCE_TIME = 50;       // 50ms debounce

// ===========================
// DECLARACIONES DE FUNCIONES
// ===========================
void handleConfigPortal();
void handleWifiScan();
void handleSaveConfig();
void handleRoot();
void handleSetConfig();
void handlePump(int pump, bool on);
void handleAuto(bool on);
void handleResetWifi();
void handleWaterSupplyOn();
void handleWaterSupplyOff();
void addSerialLog(String message);
void handleLogs();
String getRestrictionsStatus();
void handleWifiResetButton();
void configureNTP();
String getFormattedTime();
String getFormattedTimeFromEpoch(unsigned long epoch);
void taskReadSensors(void* pv);
void taskAutoIrrigation(void* pv);
void taskWeatherGuard(void* pv);
void deleteAllConfig();
void handleFactoryReset();
bool sendTelegramMessage(String message);
void handleTestTelegram();
void setupVPDHandlers();
void attemptWifiReconnection();
void initializeSystemRegardlessOfWifi();
void attemptQuickWifiReconnection();
void handleTelegramCommand(String chat_id, String text);
void taskTelegramCommands(void* pv);
void controlPump(int pump, bool on);
void setLedStatus(int red, int green, int blue);
void handleTestPumps();
void handleSetCalibration();
void updateSensorHistory(float temp, float hum, float light);



// ===========================
// CONFIGURACIÓN NTP
// ===========================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // Hora Madrid, París, Roma
const int daylightOffset_sec = 3600;  // Horario de verano +1 hora


// ===========================
// CONFIGURACIÓN RED
// ===========================
  bool configureNetwork() {
  Serial.println("[RED] ==========================================");
  Serial.println("[RED] Iniciando configuración de red");
  Serial.println("[RED] Modo: " + String(config.useDHCP ? "DHCP Automático" : "IP Estática"));

  // Si estamos usando DHCP
  if (config.useDHCP) {
    // Para DHCP, simplemente resetear cualquier config previa
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    Serial.println("[RED] ✅ Configurado para usar DHCP");
    Serial.println("[RED] El router asignará la IP automáticamente");
    addSerialLog("[RED] Modo DHCP activado");
    return true;
  }

  // Si estamos usando IP ESTÁTICA
  else {
    Serial.println("[RED] Configurando IP estática...");

    IPAddress ip, gateway, subnet, dns1, dns2;

    // Validar IP
    if (!ip.fromString(config.staticIP)) {
      Serial.println("[RED] ❌ Error: IP estática inválida: " + config.staticIP);
      addSerialLog("[RED] ❌ IP estática inválida");
      return false;
    }

    // Validar Gateway
    if (!gateway.fromString(config.staticGateway)) {
      Serial.println("[RED] ❌ Error: Gateway inválido: " + config.staticGateway);
      addSerialLog("[RED] ❌ Gateway inválido");
      return false;
    }

    // Validar Subnet
    if (!subnet.fromString(config.staticSubnet)) {
      Serial.println("[RED] ❌ Error: Subnet inválido: " + config.staticSubnet);
      addSerialLog("[RED] ❌ Subnet inválido");
      return false;
    }

    // DNS opcionales (usar gateway como DNS por defecto si no se especifica)
    if (config.staticDNS1.length() > 0) {
      if (!dns1.fromString(config.staticDNS1)) {
        Serial.println("[RED] ⚠️ DNS1 inválido, usando gateway como DNS");
        dns1 = gateway;
      }
    } else {
      dns1 = gateway;  // Usar gateway como DNS primario por defecto
      Serial.println("[RED] Usando gateway como DNS1 por defecto");
    }

    if (config.staticDNS2.length() > 0) {
      if (!dns2.fromString(config.staticDNS2)) {
        Serial.println("[RED] ⚠️ DNS2 inválido, usando DNS1");
        dns2 = dns1;
      }
    } else {
      dns2 = dns1;  // Usar DNS1 como DNS2 si no se especifica
    }

    // IMPORTANTE: La configuración debe hacerse ANTES de WiFi.begin()
    // Si ya estamos conectados, esto requiere reconexión
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[RED] ⚠️ AVISO: Cambiar configuración de red con WiFi activo");
      Serial.println("[RED] La nueva configuración se aplicará tras reiniciar");

      // Intentar aplicar config en caliente (puede no funcionar correctamente)
      WiFi.disconnect(false);  // Desconectar sin borrar credenciales
      delay(100);

      bool configResult = WiFi.config(ip, gateway, subnet, dns1, dns2);

      if (!configResult) {
        Serial.println("[RED] ❌ Error aplicando configuración en caliente");
        addSerialLog("[RED] ❌ Error config IP en caliente");
        return false;
      }

      // Reconectar con la nueva config
      WiFi.begin(ssid_stored.c_str(), pass_stored.c_str());

      // Esperar reconexión
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[RED] ✅ Reconectado con nueva configuración");
      } else {
        Serial.println("\n[RED] ❌ Fallo reconexión. Reinicie el sistema");
        addSerialLog("[RED] ❌ Requiere reinicio manual");
        return false;
      }
      } else {
      // No hay WiFi activo, solo aplicar la configuración
      bool configResult = WiFi.config(ip, gateway, subnet, dns1, dns2);

      if (!configResult) {
        Serial.println("[RED] ❌ Error aplicando configuración IP estática");
        addSerialLog("[RED] ❌ Fallo config IP estática");
        return false;
      }

      Serial.println("[RED] ✅ Configuración IP estática preparada");
      Serial.println("[RED] Se aplicará en la próxima conexión WiFi");
    }

    // Log de la configuración aplicada
    Serial.println("[RED] ==========================================");
    Serial.println("[RED] Configuración IP estática establecida:");
    Serial.println("[RED]   - IP: " + config.staticIP);
    Serial.println("[RED]   - Gateway: " + config.staticGateway);
    Serial.println("[RED]   - Subnet: " + config.staticSubnet);
    Serial.println("[RED]   - DNS1: " + (config.staticDNS1.length() > 0 ? config.staticDNS1 : "Gateway"));
    Serial.println("[RED]   - DNS2: " + (config.staticDNS2.length() > 0 ? config.staticDNS2 : "DNS1"));
    Serial.println("[RED] ==========================================");

    // Verificar la configuración actual real
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[RED] Configuración REAL aplicada:");
      Serial.println("[RED]   - IP real: " + WiFi.localIP().toString());
      Serial.println("[RED]   - Gateway real: " + WiFi.gatewayIP().toString());
      Serial.println("[RED]   - Subnet real: " + WiFi.subnetMask().toString());
      Serial.println("[RED]   - DNS real: " + WiFi.dnsIP().toString());

      // Verificar si coincide con lo solicitado
      if (WiFi.localIP().toString() != config.staticIP) {
        Serial.println("[RED] ⚠️ ADVERTENCIA: La IP real no coincide con la configurada");
        Serial.println("[RED] ⚠️ Puede requerir reinicio del sistema");
        addSerialLog("[RED] ⚠️ IP no coincide - reinicie");
      } else {
        addSerialLog("[RED] ✅ IP Fija: " + config.staticIP);
      }
    }

    return true;
  }
}

// ===========================
// FUNCION mDNS
// ===========================
bool setupmDNS() {
  if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
    Serial.println("[mDNS] No se puede iniciar - Sin conexión WiFi");
    return false;
  }
  
  Serial.println("[mDNS] Configurando servicio mDNS...");
  
  // Pequeño delay para asegurar que la conexión WiFi esté estable
  delay(100);

  if (!MDNS.begin(MDNS_HOSTNAME)) {
    Serial.println("[mDNS] ❌ Error iniciando mDNS - Primer intento");
    
    // Segundo intento con limpieza
    MDNS.end();
    delay(500);

  if (!MDNS.begin(MDNS_HOSTNAME)) {
      Serial.println("[mDNS] ❌ Error iniciando mDNS - Segundo intento fallido");
      addSerialLog("[mDNS] ❌ Error iniciando servicio");
      return false;
    }
  }

  Serial.println("[mDNS] ✅ Servicio mDNS iniciado correctamente");
  Serial.print("[mDNS]   - Hostname: ");
  Serial.print(MDNS_HOSTNAME);
  Serial.println(".local");
  Serial.print("[mDNS]   - URL de acceso: http://");
  Serial.print(MDNS_HOSTNAME);
  Serial.println(".local");
  
  // Añadir servicios HTTP
  MDNS.addService("http", "tcp", 80);
  
  // Agregar información adicional del servicio
  MDNS.addServiceTxt("http", "tcp", "board", "ESP32-S3");
  MDNS.addServiceTxt("http", "tcp", "version", "1.0");
  MDNS.addServiceTxt("http", "tcp", "device", "sistema-riego");
  
  addSerialLog("[mDNS] ✓ Activo: " + String(MDNS_HOSTNAME) + ".local");
  return true;
  }


//=========================================
// VALORES DEL VOC/VPD | FUNCIÓNES PARA LOS ICONOS
//=========================================

String getAirQualityText(float resistance) {
    // Sensor de gas BME680: mayor resistencia = mejor calidad
    if (resistance > 50) return "EXCELENTE";
    else if (resistance > 20) return "BUENA";
    else if (resistance > 10) return "MODERADA";
    else if (resistance > 5) return "POBRE";
    else return "MALA";
}

// Función para obtener color según calidad del aire
String getAirQualityColor(float resistance) {
    if (resistance > 50) return "#10b981";  // Verde
    else if (resistance > 20) return "#06b6d4";  // Cyan
    else if (resistance > 10) return "#f59e0b";  // Amarillo
    else if (resistance > 5) return "#ef4444";   // Rojo
    else return "#991b1b";  // Rojo oscuro
}

// ====================================================================
// FUNCIÓN NUEVA - Estado Ambiental General
// ====================================================================
AmbientState getAmbientState(float vpd, float hum, float temp, float soil1, float soil2) {
  AmbientState state;
  
  // CASO 1: RIESGO DE HONGOS (alta humedad + bajo VPD)
  if (hum > 85 && vpd < 0.4) {
    state.icon = "🍄";
    state.text = "RIESGO HONGOS";
    state.detail = "No regar";
    state.gradient = "background: linear-gradient(135deg, rgba(168, 85, 247, 0.8), rgba(147, 51, 234, 0.8));";
    return state;
  }
  
  // CASO 2: ESTRÉS SEVERO (alto VPD + temperatura alta + suelo seco)
  float soilAvg = (soil1 + soil2) / 2.0;
  if (vpd > 1.6 && temp > 32 && soilAvg < 30) {
    state.icon = "🥵";
    state.text = "ESTRÉS SEVERO";
    state.detail = "Regar urgente";
    state.gradient = "background: linear-gradient(135deg, rgba(239, 68, 68, 0.8), rgba(220, 38, 38, 0.8));";
    return state;
  }
  
  // CASO 3: VPD BAJO (humedad muy alta)
  if (vpd < 0.4) {
    state.icon = "💧";
    state.text = "VPD BAJO";
    state.detail = "Reducir riego";
    state.gradient = "background: linear-gradient(135deg, rgba(6, 182, 212, 0.8), rgba(8, 145, 178, 0.8));";
    return state;
  }
  
  // CASO 4: VPD ALTO (estrés por sequedad)
  if (vpd > 1.6) {
    state.icon = "🔥";
    state.text = "VPD ALTO";
    state.detail = "Aumentar riego";
    state.gradient = "background: linear-gradient(135deg, rgba(245, 158, 11, 0.8), rgba(217, 119, 6, 0.8));";
    return state;
  }
  
  // CASO 5: SUELO SECO (independiente de VPD)
  if (soilAvg < 25) {
    state.icon = "🏜️";
    state.text = "SUELO SECO";
    state.detail = "Necesita riego";
    state.gradient = "background: linear-gradient(135deg, rgba(139, 69, 19, 0.8), rgba(101, 67, 33, 0.8));";
    return state;
  }
  
  // CASO 6: SUELO SATURADO
  if (soilAvg > 80) {
    state.icon = "🌊";
    state.text = "SUELO SATURADO";
    state.detail = "No regar";
    state.gradient = "background: linear-gradient(135deg, rgba(6, 182, 212, 0.8), rgba(99, 102, 241, 0.8));";
    return state;
  }
  
  // CASO 7: ÓPTIMO (por defecto)
  state.icon = "✅";
  state.text = "ÓPTIMO";
  state.detail = "Mantener rutina";
  state.gradient = "background: linear-gradient(135deg, rgba(16, 185, 129, 0.8), rgba(5, 150, 105, 0.8));";
  return state;
}

// Función principal de análisis de zona
ZoneIrrigationDecision analyzeZoneIrrigation(int zone, float soilMoisture, float threshold) {
  ZoneIrrigationDecision decision;

  // Obtener datos actuales de forma segura
  float vpd, temperature, humidity;
  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(100)) == pdTRUE) {
    vpd = data.vpd;
    temperature = data.temp;
    humidity = data.hum;
    xSemaphoreGive(mutexData);
  } else {
    return decision;  // Si no puede acceder a los datos, devolver decisión vacía
  }

  // 1. ANÁLISIS BÁSICO DEL SUELO
  float moistureDeficit = threshold - soilMoisture;
  decision.analysis = "Déficit suelo: " + String(moistureDeficit, 1) + "% | ";

  // 2. FACTOR VPD CON UMBRALES CONFIGURABLES
  if (vpd > config.vpdCriticalHigh) {
    decision.vpdFactor = config.vpdFactorCriticalHigh;
    decision.reason += "VPD crítico alto (" + String(vpd, 2) + " kPa) - RIEGO URGENTE | ";
  } else if (vpd < config.vpdCriticalLow) {
    decision.vpdFactor = config.vpdFactorCriticalLow;
    decision.reason += "VPD crítico bajo (" + String(vpd, 2) + " kPa) - RIESGO HONGOS | ";
  } else if (vpd >= config.vpdOptimalLow && vpd <= config.vpdOptimalHigh) {
    decision.vpdFactor = config.vpdFactorOptimal;
    decision.reason += "VPD óptimo (" + String(vpd, 2) + " kPa) - CONDICIONES IDEALES | ";
  } else {
    decision.vpdFactor = config.vpdFactorNormal;
    decision.reason += "VPD normal (" + String(vpd, 2) + " kPa) | ";
  }

  // 3. FACTOR TEMPERATURA CON UMBRALES CONFIGURABLES
  if (temperature > config.tempThresholdHigh) {
    decision.tempFactor = config.tempFactorHigh;
    decision.reason += "Temp alta (" + String(temperature, 1) + "°C) aumenta demanda | ";
  } else if (temperature < config.tempThresholdLow) {
    decision.tempFactor = config.tempFactorLow;
    decision.reason += "Temp baja (" + String(temperature, 1) + "°C) reduce demanda | ";
  } else {
    decision.tempFactor = 1.0;
    decision.reason += "Temp normal (" + String(temperature, 1) + "°C) | ";
  }

  // 4. CALCULAR URGENCIA COMPUESTA
  decision.urgency = (moistureDeficit / threshold) * decision.vpdFactor * decision.tempFactor;
  decision.urgency = constrain(decision.urgency, 0.0, 1.0);

  decision.analysis += "Factor VPD: " + String(decision.vpdFactor, 1) + "x | ";
  decision.analysis += "Factor Temp: " + String(decision.tempFactor, 1) + "x | ";
  decision.analysis += "Urgencia: " + String(decision.urgency * 100, 0) + "%";

  // 5. DECISIÓN FINAL CON LÓGICA INTELIGENTE
  if (moistureDeficit > 5) {  // Suelo claramente seco
    if (vpd > config.vpdCriticalHigh) {
      decision.shouldIrrigate = true;
      decision.reason += "DECISIÓN: REGAR URGENTE";
    } else if (vpd < config.vpdCriticalLow) {
      decision.shouldIrrigate = false;
      decision.reason += "DECISIÓN: NO REGAR (riesgo hongos)";
    } else {
      decision.shouldIrrigate = true;
      decision.reason += "DECISIÓN: REGAR NORMAL";
    }
  } else if (moistureDeficit > 0) {  // Suelo ligeramente seco
    if (vpd > config.vpdCriticalHigh) {
      decision.shouldIrrigate = true;
      decision.reason += "DECISIÓN: REGAR (VPD crítico)";
    } else {
      decision.shouldIrrigate = false;
      decision.reason += "DECISIÓN: ESPERAR (suelo OK)";
    }
  } else {
    decision.shouldIrrigate = false;
    decision.reason += "DECISIÓN: NO REGAR (suelo húmedo)";
  }

  return decision;
}


// ===========================
// FUNCIONES RESTRICCIONES NORMATIVAS
// ===========================
bool isTimeRestricted() {
  if (!config.timeRestrictionEnabled) return false;
  
  // Obtener hora actual
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Si no hay conexión NTP, no aplicar restricción
    return false;
  }
  
  // Convertir hora actual a minutos desde medianoche
  int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  
  // Parsear horas de restricción
  int startHour, startMin, endHour, endMin;
  if (sscanf(config.restrictionStartTime.c_str(), "%d:%d", &startHour, &startMin) != 2) {
    return false; // Error parsing
  }
  if (sscanf(config.restrictionEndTime.c_str(), "%d:%d", &endHour, &endMin) != 2) {
    return false; // Error parsing
  }
  
  int startMinutes = startHour * 60 + startMin;
  int endMinutes = endHour * 60 + endMin;
  
  // Manejar rangos que cruzan medianoche (ej: 22:00 - 06:00)
  if (startMinutes > endMinutes) {
    // Rango cruza medianoche
    return (currentMinutes >= startMinutes || currentMinutes <= endMinutes);
  } else {
    // Rango normal en el mismo día
    return (currentMinutes >= startMinutes && currentMinutes <= endMinutes);
  }
}

bool isWindRestricted() {
  if (!config.windRestrictionEnabled) return false;
  
  // Verificar si tenemos datos de viento válidos
  if (!data.weather.data_valid) return false;
  
  // Comparar con velocidad máxima permitida
  return (data.weather.wind_speed > config.maxWindSpeed);
}

String getTimeRestrictionStatus() {
  if (!config.timeRestrictionEnabled) return "DESACTIVADA";
  
  if (config.timeRestrictionActive) {
    return "ACTIVA (" + config.restrictionStartTime + "-" + config.restrictionEndTime + ")";
  } else {
    return "INACTIVA (" + config.restrictionStartTime + "-" + config.restrictionEndTime + ")";
  }
}

String getWindRestrictionStatus() {
  if (!config.windRestrictionEnabled) return "DESACTIVADA";
  
  if (config.windRestrictionActive) {
    return "ACTIVA (Viento: " + String(data.weather.wind_speed, 1) + " km/h)";
  } else {
    return "INACTIVA (Viento: " + String(data.weather.wind_speed, 1) + " km/h)";
  }
}

// ===========================
// FUNCIONES CALIBRACIÓN SUSTRATO
// ===========================
String getSubstrateDisplayName(String profile) {
  if (profile == "universal") return "Universal";
  else if (profile == "clay") return "Arcilla";
  else if (profile == "sandy") return "Arenoso";
  else if (profile == "loam") return "Franco";
  else if (profile == "peat") return "Turba";
  else if (profile == "coco") return "Coco";
  else if (profile == "rockwool") return "Lana Roca";
  else if (profile == "perlite") return "Perlita";
  else if (profile == "vermiculite") return "Vermiculita";
  else return "Universal";
}

String getSubstrateColor(String profile) {
  if (profile == "clay") return "rgba(139, 69, 19, 0.7)";
  else if (profile == "sandy") return "rgba(245, 222, 179, 0.7)";
  else if (profile == "loam") return "rgba(101, 67, 33, 0.7)";
  else if (profile == "peat") return "rgba(165, 42, 42, 0.7)";
  else if (profile == "coco") return "rgba(150, 75, 0, 0.7)";
  else if (profile == "rockwool") return "rgba(128, 128, 128, 0.7)";
  else if (profile == "perlite") return "rgba(230, 230, 230, 0.7)";
  else if (profile == "vermiculite") return "rgba(255, 215, 0, 0.7)";
  else return "rgba(99, 102, 241, 0.7)";
}

String getSubstrateIcon(String profile) {
  if (profile == "clay") return "fas fa-mountain";
  else if (profile == "sandy") return "fas fa-umbrella-beach";
  else if (profile == "loam") return "fas fa-tractor";
  else if (profile == "peat") return "fas fa-leaf";
  else if (profile == "coco") return "fas fa-tree";
  else if (profile == "rockwool") return "fas fa-gem";
  else if (profile == "perlite") return "fas fa-snowflake";
  else if (profile == "vermiculite") return "fas fa-star";
  else return "fas fa-seedling";
}

float getAutoCalibrationFactor(String profile) {
  if (profile == "clay") return 1.35;
  if (profile == "sandy") return 0.75;
  if (profile == "loam") return 1.1;
  if (profile == "peat") return 1.25;
  if (profile == "coco") return 0.95;
  if (profile == "rockwool") return 0.85;
  if (profile == "perlite") return 0.7;
  if (profile == "vermiculite") return 1.4;
  return 1.0;
}

// ===========================
// SISTEMA DE CALIBRACIÓN SENSORES SUELO HUMEDAD
// ===========================

// ✅ Función 1: Lectura RAW del ADC (NO BLOQUEANTE)
int readSoilADC(int pin) {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(pin);
    vTaskDelay(pdMS_TO_TICKS(10));  // ✅ CAMBIADO
  }
  return sum / 5;
}

// ✅ Función 2: Conversión RAW sin re-lectura
float convertRawToPercent(int rawADC, bool isInverseSensor) {
  if (isInverseSensor) {
    // Sensor INVERSO: más seco = ADC más alto
    return constrain(map(rawADC, 0, 4095, 100, 0), 0, 100);
  } else {
    // Sensor NORMAL: más seco = ADC más bajo
    return constrain(map(rawADC, 0, 4095, 0, 100), 0, 100);
  }
}

// ✅ Función 3: Lectura calibrada CORRECTA
float readCalibratedSoil(int pin, SoilSensorCalibration& cal) {
  // ✅ LEER SOLO UNA VEZ
  int rawADC = readSoilADC(pin);
  cal.lastRawADC = rawADC;

  float result;

  // Si hay calibración manual válida
  if (cal.calibrationType == SoilSensorCalibration::MANUAL_TWO_POINT && 
      cal.isManuallyCalibrated && 
      cal.dryPointADC != cal.wetPointADC) {
    
    // ✅ RESPETAR EL TIPO DE SENSOR DETECTADO
    if (cal.isInverseSensor) {
      // Sensor INVERSO: dryPoint > wetPoint
      result = map(rawADC, cal.wetPointADC, cal.dryPointADC, 100, 0);
    } else {
      // Sensor NORMAL: wetPoint > dryPoint
      result = map(rawADC, cal.dryPointADC, cal.wetPointADC, 0, 100);
    }
    result = constrain(result, 0, 100);
    
  } else {
    // Sin calibración: usar conversión RAW
    result = convertRawToPercent(rawADC, cal.isInverseSensor) * cal.substrateFactor;
    result = constrain(result, 0, 100);
  }

  cal.lastPercentage = result;
  return result;
}

// ✅ Para uso en el web durante calibración
float readSoilRawPercent(int pin) {
  int rawADC = readSoilADC(pin);
  // Por defecto asumir inverso (es lo más común)
  return convertRawToPercent(rawADC, true);
}

// ===========================
// FUNCIONES CONFIGURACIÓN
// ===========================

// ===========================
// FUNCIONES BOMBAS
// ===========================
    void handlePump(int pump, bool on) {
    // VERIFICAR RESTRICCIONES SOLO AL ACTIVAR (no al desactivar)
    if (on) {
    bool canActivate = true;
    String restrictionReason = "";

    // Verificar suministro de agua
    if (config.waterSupplyControl && !data.waterSupply) {
      canActivate = false;
      restrictionReason = "Fallo en suministro de agua";
    }
    // Verificar pronóstico de lluvia
    else if (config.weatherGuard && config.rainExpected) {
      canActivate = false;
      restrictionReason = "Lluvia pronosticada";
    }
    // Verificar protección temperatura
    else if (config.tempProtection && (data.temp < config.minTempThreshold || data.temp > config.maxTempThreshold)) {
      canActivate = false;
      restrictionReason = "Temperatura fuera de rango seguro";
    }
    // Verificar protección humedad ambiental
    else if (config.humidityProtection && (data.hum > config.humidityThreshold)) {
      canActivate = false;
      restrictionReason = "Humedad ambiental demasiado alta";
    }
    // Verificar protección VPD
    else if (config.vpdProtection && isVPDBlocking(data.vpd)) {
      canActivate = false;
      restrictionReason = "Condiciones VPD no seguras para riego";
    }

    if (!canActivate) {
      // FEEDBACK VISUAL DE BLOQUEO - LED ROJO PARPADEANTE
      for (int i = 0; i < 3; i++) {
        setLedStatus(255, 0, 0);  // Rojo
        delay(200);
        setLedStatus(0, 0, 0);  // Apagado
        delay(200);
      }

      // Restaurar LED según estado
      if (wifiConnected) {
        setLedStatus(0, 255, 0);  // Verde
      } else {
        setLedStatus(255, 165, 0);  // Naranja
      }

      Serial.println("[MANUAL] Bomba " + String(pump) + " bloqueada: " + restrictionReason);
      addSerialLog("[MANUAL] ❌ Bomba " + String(pump) + " bloqueada - " + restrictionReason);

      server.sendHeader("Location", "/");
      server.send(303);
      return;
    }
  }
      // Si pasa todas las verificaciones, activar/desactivar
      controlPump(pump, on);
      server.sendHeader("Location", "/");
      server.send(303);
}

// ===========================
// FUNCIONES WiFi
// ===========================
void handleResetWifi() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='10;url=http://192.168.4.1'>";
  html += "</head><body>";
  html += "<div style='text-align:center;font-family:Arial;padding:50px;'>";
  html += "<h1>🌿 Sistema Riego</h1>";
  html += "<div style='background:#fff3cd;color:#856404;padding:20px;border-radius:10px;'>";
  html += "WiFi reseteado correctamente<br>";
  html += "Reiniciando en modo configuración...<br><br>";
  html += "En 10 segundos, conecta a:<br>";
  html += "<strong>Red: RiegoIoT-Access</strong><br>";
  html += "<strong>Contraseña: 12345678</strong><br>";
  html += "<strong>URL: http://192.168.4.1</strong>";
  html += "</div></div></body></html>";

  server.send(200, "text/html", html);

  delay(1000);
  server.stop();
  deleteWifiConfig();
  delay(500);

  Serial.println("[WIFI] Reiniciando para entrar en modo configuración...");
  ESP.restart();
}

// ===========================
// FUNCIONES Reset
// ===========================
void handleFactoryReset() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='10;url=/'>";
  html += "</head><body>";
  html += "<div style='text-align:center;font-family:Arial;padding:50px;'>";
  html += "<h1>🌿 Sistema Riego</h1>";
  html += "<div style='background:#fff3cd;color:#856404;padding:20px;border-radius:10px;'>";
  html += "⚠️ CONFIGURACIÓN BORRADA ⚠️<br><br>";
  html += "Todos los ajustes han sido restaurados a valores de fábrica<br><br>";
  html += "El sistema se reiniciará en 10 segundos...<br><br>";
  html += "Deberás configurar el WiFi nuevamente<br>";
  html += "<strong>Red: RiegoIoT-Access</strong><br>";
  html += "<strong>Contraseña: 12345678</strong><br>";
  html += "</div></div></body></html>";

  server.send(200, "text/html", html);

  // Borrar toda la configuración
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    deleteAllConfig();
    xSemaphoreGive(mutexConfig);
  }

  delay(2000);
  ESP.restart();
}

// ===========================
// FUNCIONES COMPROBACION SUMINISTRO AGUA
// ===========================
void handleWaterSupplyOn() {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    config.waterSupplyControl = true;
    saveConfig();
    xSemaphoreGive(mutexConfig);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleWaterSupplyOff() {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    config.waterSupplyControl = false;
    saveConfig();
    xSemaphoreGive(mutexConfig);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ===========================
// INICIALIZACIÓN PSRAM
// ===========================
void initializeTelegramPSRAM() {
    Serial.println("[PSRAM] Inicializando Telegram en PSRAM...");
    
    telegramPSRAM.psramAvailable = psramFound();
    
    if (telegramPSRAM.psramAvailable) {
        Serial.println("[PSRAM] ✅ Disponible - " + String(ESP.getPsramSize() / 1024) + " KB");
        
        // Asignar buffers en PSRAM
        telegramPSRAM.telegramToken = (char*) ps_calloc(100, sizeof(char));
        telegramPSRAM.chatID = (char*) ps_calloc(50, sizeof(char));
        
        if (telegramPSRAM.telegramToken && telegramPSRAM.chatID) {
            Serial.println("[PSRAM] ✅ Buffers asignados en PSRAM");
        } else {
            Serial.println("[PSRAM] ❌ Error asignando PSRAM, usando fallback");
            telegramPSRAM.psramAvailable = false;
            initializeTelegramFallback();
        }
    } else {
        Serial.println("[PSRAM] ⚠️ No disponible - Usando heap normal");
        initializeTelegramFallback();
    }
}

void initializeTelegramFallback() {
    telegramPSRAM.telegramToken = (char*) malloc(100 * sizeof(char));
    telegramPSRAM.chatID = (char*) malloc(50 * sizeof(char));
    
    if (telegramPSRAM.telegramToken && telegramPSRAM.chatID) {
        Serial.println("[PSRAM] ✅ Buffers asignados en heap normal");
    } else {
        Serial.println("[PSRAM] ❌ Error crítico: No se pudieron asignar buffers");
    }
}


// ===========================
// SETUP VPD HANDLERS
// ===========================
void setupVPDHandlers() {
  server.on("/vpdprotectionon", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.vpdProtection = true;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/vpdprotectionoff", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.vpdProtection = false;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/vpdon", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.vpdEnabled = true;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/vpdoff", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.vpdEnabled = false;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });
}

// ===========================
// SISTEMA DE INFERENCIA INTELIGENTE
// ===========================
void updateSensorHistory(float temp, float hum, float light) {
  unsigned long currentTime = millis();
  
  if (sensorHistory.lastUpdate > 0 && (currentTime - sensorHistory.lastUpdate) > 40000) { // Cada 40 seg
    unsigned long timeDiff = currentTime - sensorHistory.lastUpdate;
    float minutesDiff = timeDiff / 60000.0;
    
    // Calcular tasas de cambio (por minuto)
    sensorHistory.tempChangeRate = abs(temp - sensorHistory.lastTemp) / minutesDiff;
    sensorHistory.humChangeRate = abs(hum - sensorHistory.lastHum) / minutesDiff;
    sensorHistory.lightChangeRate = abs(light - sensorHistory.lastLight) / minutesDiff;
    
    // Loggear cambios significativos para calibración
    if (sensorHistory.tempChangeRate > 1.0 || sensorHistory.humChangeRate > 3.0) {
      Serial.printf("[INFERENCE] 📊 Cambios: ΔT=%.2f°C/min, ΔH=%.2f%%/min, ΔL=%.1f%%/min\n",
                   sensorHistory.tempChangeRate, sensorHistory.humChangeRate, sensorHistory.lightChangeRate);
    }
  }
  
  // Actualizar historial
  sensorHistory.lastTemp = temp;
  sensorHistory.lastHum = hum;
  sensorHistory.lastLight = light;
  sensorHistory.lastUpdate = currentTime;
}

bool inferWindBySensorChanges() {
  // Múltiples criterios para detectar viento
  bool rapidTempChanges = (sensorHistory.tempChangeRate > 1.8);    // +1.8°C por minuto
  bool rapidHumChanges = (sensorHistory.humChangeRate > 6.0);      // +6% humedad por minuto
  bool unstableConditions = (sensorHistory.tempChangeRate > 1.0 && sensorHistory.humChangeRate > 3.0);
  
  // Viento causa cambios rápidos y erráticos
  bool windDetected = (rapidTempChanges || rapidHumChanges || unstableConditions);
  
  if (windDetected && !data.smartModeActive) {
    Serial.println("[WIND-INFERENCE] 💨 Viento detectado por patrones de cambio");
    data.smartModeActive = true;
  }
  
  return windDetected;
}

bool inferRainByLightAndHumidity(float light, float hum, float temp) {
  // Detección sofisticada de lluvia/noche
  bool isVeryDark = (light < 8.0);    // Noche o tormenta
  bool isDark = (light < 20.0);       // Atardecer o nublado
  bool isHighHumidity = (hum > 80.0); // Aire cargado de humedad
  
  // Lluvia = oscuro + humedad alta
  bool rainLikely = isVeryDark && isHighHumidity;
  
  // Noche = oscuro pero humedad normal
  bool nightTime = isVeryDark && !isHighHumidity;
  
  // Cambio rápido a condiciones oscuras = frente tormentoso
  bool rapidDarkening = (sensorHistory.lightChangeRate < -10.0); // Luz disminuyendo rápido
  
  if (rainLikely) {
    Serial.println("[RAIN-INFERENCE] 🌧️ Lluvia inferida: Oscuro + Humedad alta");
  } else if (rapidDarkening) {
    Serial.println("[RAIN-INFERENCE] ⛈️ Tormenta aproximándose: Luz disminuyendo rápido");
  }
  
  return (rainLikely || rapidDarkening);
}

bool inferNightTime(float light, float hum) {
  // Detección inteligente de noche vs mal tiempo
  bool isVeryDark = (light < 10.0);
  bool isModerateDark = (light < 25.0);
  bool isStableHumidity = (sensorHistory.humChangeRate < 2.0); // Humedad estable = noche
  
  // Noche = oscuro estable con humedad constante
  bool likelyNight = isVeryDark && isStableHumidity;
  
  // Anochecer/amanecer = luz moderada estable
  bool twilight = isModerateDark && isStableHumidity;
  
  return (likelyNight || twilight);
}

//=================================
// Aprendizaje por zonas
//================================
void updateZoneLearning(int zone, float moistureGain, unsigned long irrigationTime, float efficiency) {
  ZoneLearning* learning = (zone == 1) ? &learningZone1 : &learningZone2;
  
  learning->totalCycles++;
  
  // Considerar exitoso si ganancia > 5% y eficiencia > 0.1
  if (moistureGain > 5.0 && efficiency > 0.1) {
    learning->successfulCycles++;
    
    // Actualizar promedios (media móvil)
    learning->avgIrrigationTime = (learning->avgIrrigationTime * 0.7) + (irrigationTime * 0.3);
    learning->avgMoistureRecovery = (learning->avgMoistureRecovery * 0.7) + (moistureGain * 0.3);
    learning->avgEfficiency = (learning->avgEfficiency * 0.7) + (efficiency * 0.3);
  }
  
  // Calcular score de eficiencia general
  learning->efficiencyScore = (learning->successfulCycles * 100.0) / learning->totalCycles;
  
  Serial.printf("[LEARNING-Z%d] Score: %.1f%% | Ciclos: %d/%d | Eff: %.2f\n",
                zone, learning->efficiencyScore, learning->successfulCycles, 
                learning->totalCycles, learning->avgEfficiency);
}

void analyzeIrrigationEfficiency(int zone, IrrigationCycle& cycle, float finalMoisture) {
  if (!cycle.completed && cycle.startTime > 0) {
    unsigned long irrigationTime = millis() - cycle.startTime;
    cycle.finalMoisture = finalMoisture;
    cycle.completed = true;
    
    // Calcular ganancia de humedad
    float moistureGain = cycle.finalMoisture - cycle.initialMoisture;
    
    // Calcular eficiencia (ganancia por minuto de riego)
    if (irrigationTime > 0 && moistureGain > 0) {
      cycle.efficiency = moistureGain / (irrigationTime / 60000.0); // % por minuto
      
      // Actualizar aprendizaje de la zona
      updateZoneLearning(zone, moistureGain, irrigationTime, cycle.efficiency);
      
      Serial.printf("[LEARNING-Z%d] Eficiencia: %.2f%%/min | Ganancia: %.1f%% | Tiempo: %lu ms\n", 
                    zone, cycle.efficiency, moistureGain, irrigationTime);
                    
      // Telegram opcional de eficiencia
      if (wifiConnected && config.telegramToken.length() > 0) {
        String telegramMsg = "📊 *Eficiencia Zona " + String(zone) + "*\n";
        telegramMsg += "• Ganancia: " + String(moistureGain, 1) + "%\n";
        telegramMsg += "• Tiempo: " + String(irrigationTime / 1000.0, 1) + "s\n";
        telegramMsg += "• Eficiencia: " + String(cycle.efficiency, 2) + "%/min\n";
        telegramMsg += "• Score: " + String((zone == 1) ? learningZone1.efficiencyScore : learningZone2.efficiencyScore, 1) + "%";
        sendTelegramAlert("📈 Análisis Riego", telegramMsg);
      }
    }
  }
}

void optimizeThresholds() {

      // Solo optimizar si estamos en modo IA
      if (config.irrigationMode != 2) return;
      unsigned long currentTime = millis();

      float threshold1 = config.aiThreshold1;  // Conversión implícita
      float threshold2 = config.aiThreshold2;

      // ===========================
      // ZONA 1 - OPTIMIZACIÓN BIDIRECCIONAL
      // ===========================
      bool zone1Optimized = adaptiveSystem.applyOptimization(
        1,                                    // Zona
        threshold1,                           // Threshold a ajustar (por referencia)
        learningZone1.lastOptimization,       // Última optimización
        learningZone1.efficiencyScore,        // Eficiencia
        learningZone1.avgMoistureRecovery,    // Recuperación promedio
        learningZone1.avgIrrigationTime,      // Tiempo promedio
        learningZone1.successfulCycles,       // Ciclos exitosos
        learningZone1.totalCycles             // Total ciclos
      );
      
      if (zone1Optimized) {
        saveConfig(); // Guardar cambios
        addSerialLog("[ADAPTIVE-Z1] Threshold optimizado a " + String(config.aiThreshold1, 1) + "%");
      }
      
      // ===========================
      // ZONA 2 - OPTIMIZACIÓN BIDIRECCIONAL
      // ===========================
      bool zone2Optimized = adaptiveSystem.applyOptimization(
        2,
        threshold2,
        learningZone2.lastOptimization,
        learningZone2.efficiencyScore,
        learningZone2.avgMoistureRecovery,
        learningZone2.avgIrrigationTime,
        learningZone2.successfulCycles,
        learningZone2.totalCycles
      );
      
      if (zone2Optimized) {
        saveConfig();
        addSerialLog("[ADAPTIVE-Z2] Threshold optimizado a " + String(config.aiThreshold2, 1) + "%");
      }
      
      // Optimizar cada 24 horas o después de 10 ciclos - Zona 1
      if (currentTime - learningZone1.lastOptimization > 86400000 || learningZone1.totalCycles >= 10) {
        if (learningZone1.efficiencyScore < 60.0 && learningZone1.totalCycles >= 10) {
          // Baja eficiencia - ajustar thresholds
          config.threshold1 += 2.0; // Modificar threshold
          Serial.printf("[OPTIMIZE-Z1] Threshold ajustado: %.1f%% (Eff: %.1f%%)\n", 
                        config.threshold1, learningZone1.efficiencyScore);
        }
        learningZone1.lastOptimization = currentTime;
      }
      // Lo mismo para Zona 2
      if (currentTime - learningZone2.lastOptimization > 86400000 || learningZone2.totalCycles >= 10) {
        if (learningZone2.efficiencyScore < 60.0 && learningZone2.totalCycles >= 10) {
          config.threshold2 += 2.0;
          Serial.printf("[OPTIMIZE-Z2] Threshold ajustado: %.1f%% (Eff: %.1f%%)\n", 
                        config.threshold2, learningZone2.efficiencyScore);
        }
        learningZone2.lastOptimization = currentTime;
        }
      }

  // ===========================
  // FUNCIONES COMPROBACION FUNCIONAMIENTO TELEGRAM
  // ===========================
  bool sendTelegramMessage(String message) {
    // Verificaciones previas
    if (!wifiConnected) {
        Serial.println("[TELEGRAM] ❌ Sin WiFi");
        return false;
    }

    // Verificar buffers PSRAM
    if (telegramPSRAM.telegramToken == nullptr || telegramPSRAM.chatID == nullptr || 
        strlen(telegramPSRAM.telegramToken) == 0 || strlen(telegramPSRAM.chatID) == 0) {
        Serial.println("[TELEGRAM] ❌ Credenciales PSRAM no configuradas");
        return false;
    }

    // Log de memoria
    size_t freeHeap = ESP.getFreeHeap();
    size_t freePSRAM = ESP.getFreePsram();
    Serial.printf("[TELEGRAM] Memoria - Heap libre: %d bytes, PSRAM libre: %d bytes\n", 
                  freeHeap, freePSRAM);

    if (freeHeap < 25000) {
        Serial.printf("[TELEGRAM] ⚠️ Memoria heap baja (%d bytes)\n", freeHeap);
    }

    Serial.println("[TELEGRAM] 📤 Enviando mensaje...");

    // Buffer temporal en PSRAM para mensajes largos
    char* messageBuffer = nullptr;
        
    if (message.length() > 200 && telegramPSRAM.psramAvailable) {
        messageBuffer = (char*) ps_malloc(message.length() + 1);
        if (messageBuffer) {
            message.toCharArray(messageBuffer, message.length() + 1);
            Serial.println("[TELEGRAM] ✅ Mensaje largo en buffer PSRAM");
        }
    }

    // Configurar cliente si es necesario
    static bool clientConfigured = false;
    if (!clientConfigured) {
        secured_client.setTimeout(5000);
        secured_client.setInsecure();
        secured_client.setHandshakeTimeout(10);
        clientConfigured = true;
    }

    // Crear bot si no existe
    if (bot == NULL) {
        Serial.println("[TELEGRAM] 📱 Creando bot...");
        bot = new UniversalTelegramBot(telegramPSRAM.telegramToken, secured_client);
        bot->longPoll = 0;
    }

    // Enviar mensaje
    unsigned long startTime = millis();
    bool result = false;
    
    try {
        result = bot->sendMessage(telegramPSRAM.chatID, message, "");
    } catch (...) {
        Serial.println("[TELEGRAM] ❌ Excepción al enviar");
        result = false;
    }
    
    unsigned long elapsed = millis() - startTime;
    
    // Liberar buffer PSRAM si se usó
    if (messageBuffer) {
    free(messageBuffer);
    }

    if (result) {
    Serial.printf("[TELEGRAM] ✅ Enviado en %lums (PSRAM: %s)\n", 
                 elapsed, telegramPSRAM.psramAvailable ? "SÍ" : "NO");
    } else {
    Serial.printf("[TELEGRAM] ❌ Falló después de %lums\n", elapsed);
    }   

    return result;
  }

// ===========================
// TAREA PARA COMANDOS TELEGRAM
// ===========================
void taskTelegramCommands(void* pv) {
  const unsigned long BOT_CHECK_INTERVAL = 3000;      // 3 segundos entre revisiones
  const unsigned long BOT_TIMEOUT = 5000;              // 5 segundos timeout
  const unsigned long BOT_RESTART_INTERVAL = 3600000;  // Recrear cada hora
  
  unsigned long lastBotCheck = 0;
  unsigned long botCreatedAt = 0;
  int consecutiveErrors = 0;
  const int MAX_ERRORS = 5;

  Serial.println("[TELEGRAM-TASK] ✅ Iniciada con sistema robusto anti-bloqueo");

  while (true) {
    // Verificar WiFi y configuración PSRAM
    if (!wifiConnected || telegramPSRAM.telegramToken == nullptr || 
    strlen(telegramPSRAM.telegramToken) == 0) {
      if (bot != NULL) {
        delete bot;
        bot = NULL;
        botCreatedAt = 0;
        Serial.println("[TELEGRAM] ⚠️ WiFi perdido, bot eliminado");
      }
      vTaskDelay(pdMS_TO_TICKS(5000));
      continue;
    }


    unsigned long currentTime = millis();

    // Recrear bot cada hora para evitar memory leaks
    if (bot != NULL && (currentTime - botCreatedAt > BOT_RESTART_INTERVAL)) {
      Serial.println("[TELEGRAM] 🔄 Mantenimiento: recreando bot");
      delete bot;
      bot = NULL;
      botCreatedAt = 0;
      consecutiveErrors = 0;
    }

    // Esperar intervalo entre revisiones
    if (currentTime - lastBotCheck < BOT_CHECK_INTERVAL) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    // Crear bot si no existe
    if (bot == NULL) {
      Serial.println("[TELEGRAM] 📱 Creando bot con PSRAM...");
      
      secured_client.setTimeout(BOT_TIMEOUT);
      secured_client.setInsecure();
      secured_client.setHandshakeTimeout(10);
      
      bot = new UniversalTelegramBot(telegramPSRAM.telegramToken, secured_client);
      bot->longPoll = 0;
      botCreatedAt = currentTime;
      
      Serial.println("[TELEGRAM] ✅ Bot creado");
    }

    // Obtener mensajes con timeout y error handling
    unsigned long operationStart = millis();
    bool operationSuccess = false;

    try {
      int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
      
      // Verificar timeout
      if (millis() - operationStart > BOT_TIMEOUT) {
        Serial.println("[TELEGRAM] ⚠️ Timeout en getUpdates");
        consecutiveErrors++;
      } else if (numNewMessages > 0) {
        Serial.printf("[TELEGRAM] 📩 %d mensaje(s) nuevo(s)\n", numNewMessages);
        consecutiveErrors = 0;
        
        // Procesar cada mensaje
        for (int i = 0; i < numNewMessages; i++) {
          String chatId = bot->messages[i].chat_id;
          String text = bot->messages[i].text;
          
          Serial.printf("[TELEGRAM] De %s: %s\n", chatId.c_str(), text.c_str());
          
          // Procesar comando con timeout
          unsigned long cmdStart = millis();
          handleTelegramCommand(chatId, text);
          
          unsigned long cmdDuration = millis() - cmdStart;
          if (cmdDuration > 2000) {
            Serial.printf("[TELEGRAM] ⚠️ Comando tardó %lums\n", cmdDuration);
          }
          
          // Pequeña pausa entre comandos
          vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        operationSuccess = true;
      } else {
        // No hay mensajes, todo OK
        operationSuccess = true;
      }
    } catch (...) {
      Serial.println("[TELEGRAM] ❌ Excepción capturada");
      consecutiveErrors++;
    }

    // Si hay muchos errores, recrear bot
    if (consecutiveErrors >= MAX_ERRORS) {
      Serial.println("[TELEGRAM] ⛔ Demasiados errores, recreando bot...");
      if (bot != NULL) {
        delete bot;
        bot = NULL;
      }
      consecutiveErrors = 0;
      botCreatedAt = 0;
      
      // Esperar antes de reintentar
      vTaskDelay(pdMS_TO_TICKS(10000));
    }

    lastBotCheck = currentTime;
    
    // Delay apropiado para no saturar CPU
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void handleTestTelegram() {
  if (!wifiConnected) {
    server.send(200, "text/html", "<html><body style='font-family:Arial;text-align:center;padding:50px;'><div style='background:#f8d7da;color:#721c24;padding:20px;border-radius:10px;'>❌ Error: Sin conexión WiFi</div></body></html>");
    return;
  }

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='3;url=/'>";
  html += "</head><body style='font-family:Arial;text-align:center;padding:50px;'>";

  String testMessage = "🌿 *Sistema de Riego IoT*\n\n";
  testMessage += "✅ Prueba de conexión\n";
  testMessage += "📊 Estado actual:\n";
  testMessage += "• Temp: " + String(data.temp, 1) + "°C\n";
  testMessage += "• Humedad: " + String(data.hum, 1) + "%\n";
  testMessage += "• Suelo 1: " + String(data.soil1, 1) + "%\n";
  testMessage += "• Suelo 2: " + String(data.soil2, 1) + "%\n";
  testMessage += "• Luz: " + String(data.light, 1) + "%\n";
  testMessage += "🕐 " + getFormattedTime();

  // ✅ USA LA NUEVA FUNCIÓN sendTelegramMessage
  if (sendTelegramMessage(testMessage)) {
    html += "<div style='background:#d4edda;color:#155724;padding:20px;border-radius:10px;'>";
    html += "✅ Mensaje de prueba enviado correctamente</div>";
  } else {
    html += "<div style='background:#f8d7da;color:#721c24;padding:20px;border-radius:10px;'>";
    html += "❌ Error enviando mensaje. Verifica la configuración</div>";
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}



// Función para enviar alertas automáticas
void sendTelegramAlert(String alertType, String details) {
  if (!wifiConnected) return;

  if (config.telegramToken.length() == 0 || config.chatID.length() == 0) {
    return;
  }

  String message = "🚨 *ALERTA - Sistema de Riego*\n\n";
  message += "Tipo: " + alertType + "\n";
  message += details + "\n";
  message += "🕐 " + getFormattedTime();

  // ✅ USA sendTelegramMessage (que ya tiene timeouts)
  sendTelegramMessage(message);
}

// =============================
// Tarea para gestión comandos Telegram
// =============================
void handleTelegramCommand(String chat_id, String text) {
  if (!wifiConnected) return;

  Serial.println("[TELEGRAM CMD] Recibido: " + text + " de: " + chat_id);

  // Verificar que el comando viene del chat autorizado
  if (chat_id != config.chatID) {
    bot->sendMessage(chat_id, "⛔ No autorizado", "");
    return;
  }

  // Convertir a minúsculas para facilitar comparación
  text.toLowerCase();

  // COMANDO: /start o /help
  if (text == "/start" || text == "/help") {
    String welcome = "🌿 *Sistema de Riego IoT*\n\n";
    welcome += "Comandos disponibles:\n\n";
    welcome += "📊 *Información:*\n";
    welcome += "/estado - Estado actual\n";
    welcome += "/sensores - Lectura sensores\n";
    welcome += "/vpd - Estado VPD actual\n";
    welcome += "/zonas - Análisis por zonas\n\n";
    welcome += "💧 *Control Bombas:*\n";
    welcome += "/pump1on - Activar bomba 1\n";
    welcome += "/pump1off - Desactivar bomba 1\n";
    welcome += "/pump2on - Activar bomba 2\n";
    welcome += "/pump2off - Desactivar bomba 2\n";
    welcome += "/alloff - Desactivar las bombas\n";
    welcome += "/testbombas - Probar Bombas 5 seg.\n\n";
    welcome += "🤖 *Modos:*\n";
    welcome += "/manual - Modo manual\n";
    welcome += "/auto - Modo automático\n";
    welcome += "/adaptativo - Sistema adaptativo\n";
    welcome += "/vpdswitch - Activar/Desactivar el sistema VPD\n\n";
    welcome += "⚙️ *Configuración:*\n";
    welcome += "/config - Ver configuración\n";
    welcome += "/setth1 XX - Umbral bomba 1 (%)\n";
    welcome += "/setth2 XX - Umbral bomba 2 (%)\n";
    welcome += "/restricciones - Ver restricciones";

    bot->sendMessage(chat_id, welcome, "Markdown");
  }

  // COMANDO: /estado
  else if (text == "/estado") {
  String status = "📊 *Estado del Sistema*\n\n";
  String modeText = "";
  if (config.irrigationMode == 0) modeText = "MANUAL 👤";
  else if (config.irrigationMode == 1) modeText = "AUTOMÁTICO 🤖";
  else if (config.irrigationMode == 2) modeText = "ADAPTATIVO 🧠";
  
  status += "Bomba 1: " + String(data.pump1 ? "ACTIVA ✅" : "INACTIVA ⭕") + "\n";
  status += "Bomba 2: " + String(data.pump2 ? "ACTIVA ✅" : "INACTIVA ⭕") + "\n";
  status += "Suministro agua: " + String(data.waterSupply ? "OK ✅" : "FALLO ❌") + "\n";
  status += "📊 Presión atmosférica: " + String(data.pressure, 1) + " mbar\n";
  status += "🌧️ Lluvia esperada: " + String(config.maxRainExpected, 1) + " mm\n";
  status += "📊 Memoria libre: " + String(ESP.getFreeHeap() / 1024.0) + " KB\n";
  AmbientState ambient = getAmbientState(data.vpd, data.hum, data.temp, data.soil1, data.soil2);
      status += "\n🌿 *Estado General:* " + ambient.icon + " " + ambient.text + "\n";
      status += "   💡 " + ambient.detail + "\n";
  
  // TIEMPO DE FUNCIONAMIENTO
  unsigned long uptime = millis() / 1000;
  unsigned long days = uptime / 86400;
  unsigned long hours = (uptime % 86400) / 3600;
  unsigned long minutes = (uptime % 3600) / 60;
  unsigned long seconds = uptime % 60;
  
  String uptimeStr = "";
  if (days > 0) uptimeStr += String(days) + "d ";
  if (hours > 0) uptimeStr += String(hours) + "h ";
  uptimeStr += String(minutes) + "m " + String(seconds) + "s";
  
  status += "⏰ Tiempo funcionando: " + uptimeStr + "\n";
  status += "\nÚltima actualización: " + getFormattedTime();

  bot->sendMessage(chat_id, status, "Markdown");
  }
  
  // COMANDO: /sensores
  else if (text == "/sensores") {
    String sensors = "🌡️ *Lectura de Sensores*\n\n";
    sensors += "Temper. ambiente: " + String(data.temp, 1) + "°C\n";
    sensors += "Humedad ambiente: " + String(data.hum, 1) + "%\n";
    sensors += "Pres. atmosférica: " + String(data.pressure, 1) + "mbar\n";
    sensors += "Calidad del aire: " + String(data.airQuality, 1) + "KΩ\n";
    sensors += "Humedad subst. 1: " + String(data.soil1, 1) + "% (umbral: " + String(config.threshold1) + "%)\n";
    sensors += "Humedad subst. 2: " + String(data.soil2, 1) + "% (umbral: " + String(config.threshold2) + "%)\n";
    sensors += "Luz solar: " + String(data.light, 1) + "lux\n";

    bot->sendMessage(chat_id, sensors, "Markdown");
  }

  // COMANDO: /pump1on
  else if (text == "/pump1on") {
  if (config.irrigationMode == 0) {  // Solo si está en MANUAL
    controlPump(1, true);
    bot->sendMessage(chat_id, "✅ Bomba 1 ACTIVADA", "");
    addSerialLog("[TELEGRAM] Bomba 1 activada remotamente");
  } else {
    String modeText = (config.irrigationMode == 1) ? "AUTOMÁTICO" : "ADAPTATIVO";
    bot->sendMessage(chat_id, "⚠️ Sistema en modo " + modeText + ". Cambia a /manual primero", "");
    }
  }

  // COMANDO: /pump1off
  else if (text == "/pump1off") {
  if (config.irrigationMode == 0) {
    controlPump(1, false);
    bot->sendMessage(chat_id, "⭕ Bomba 1 DESACTIVADA", "");
    addSerialLog("[TELEGRAM] Bomba 1 desactivada remotamente");
  } else {
    String modeText = (config.irrigationMode == 1) ? "AUTOMÁTICO" : "ADAPTATIVO";
    bot->sendMessage(chat_id, "⚠️ Sistema en modo " + modeText + ". Cambia a /manual primero", "");
  }
  }

  // COMANDO: /pump2on
  else if (text == "/pump2on") {
  if (config.irrigationMode == 0) {
    controlPump(2, true);
    bot->sendMessage(chat_id, "✅ Bomba 2 ACTIVADA", "");
    addSerialLog("[TELEGRAM] Bomba 2 activada remotamente");
  } else {
    String modeText = (config.irrigationMode == 1) ? "AUTOMÁTICO" : "ADAPTATIVO";
    bot->sendMessage(chat_id, "⚠️ Sistema en modo " + modeText + ". Cambia a /manual primero", "");
  }
  }

  // COMANDO: /pump2off
  else if (text == "/pump2off") {
  if (config.irrigationMode == 0) {
    controlPump(2, false);
    bot->sendMessage(chat_id, "⭕ Bomba 2 DESACTIVADA", "");
    addSerialLog("[TELEGRAM] Bomba 2 desactivada remotamente");
  } else {
    String modeText = (config.irrigationMode == 1) ? "AUTOMÁTICO" : "ADAPTATIVO";
    bot->sendMessage(chat_id, "⚠️ Sistema en modo " + modeText + ". Cambia a /manual primero", "");
    }
  }

  // COMANDO: /alloff
  else if (text == "/alloff") {
    controlPump(1, false);
    controlPump(2, false);
    bot->sendMessage(chat_id, "⭕ Todas las bombas DESACTIVADAS", "");
    addSerialLog("[TELEGRAM] Todas las bombas desactivadas remotamente");
  }

  // COMANDO: /testbombas o /testpumps - PRUEBA DE DIAGNÓSTICO SIN RESTRICCIONES
  else if (text == "/testbombas") {
    // Enviar mensaje de inicio
    bot->sendMessage(chat_id, "🔧 *MODO PRUEBA DE DIAGNÓSTICO*\n\n⚠️ Ignorando todas las restricciones\nActivando ambas bombas por 5 segundos...", "Markdown");

    // Log en el sistema
    Serial.println("[TELEGRAM TEST] ⚡ Prueba de bombas iniciada remotamente (sin restricciones)");
    addSerialLog("[TELEGRAM] 🔧 Prueba bombas (diagnóstico)");

    // LED amarillo para modo prueba
    setLedStatus(255, 255, 0);

    // ACTIVAR BOMBAS DIRECTAMENTE (sin usar controlPump para evitar restricciones)
    digitalWrite(PIN_PUMP1, LOW);  // Activar bomba 1 directamente
    digitalWrite(PIN_PUMP2, LOW);  // Activar bomba 2 directamente

    // Actualizar estado en data
    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(100)) == pdTRUE) {
      data.pump1 = true;
      data.pump2 = true;
      xSemaphoreGive(mutexData);
    }

    // Esperar 5 segundos con feedback visual
    unsigned long testStart = millis();
    unsigned long lastBlink = 0;
    bool ledState = true;

    while (millis() - testStart < 5000) {
      // Parpadeo naranja durante la prueba
      if (millis() - lastBlink > 500) {
        if (ledState) {
          setLedStatus(255, 165, 0);  // Naranja
        } else {
          setLedStatus(255, 100, 0);  // Naranja más oscuro
        }
        ledState = !ledState;
        lastBlink = millis();
      }
      delay(100);
    }

    // APAGAR BOMBAS DIRECTAMENTE
    digitalWrite(PIN_PUMP1, HIGH);  // Apagar bomba 1
    digitalWrite(PIN_PUMP2, HIGH);  // Apagar bomba 2

    // Actualizar estado en data
    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(100)) == pdTRUE) {
      data.pump1 = false;
      data.pump2 = false;
      xSemaphoreGive(mutexData);
    }

    // Restaurar LED según estado del sistema
    if (wifiConnected) {
      setLedStatus(0, 255, 0);  // Verde si hay WiFi
    } else {
      setLedStatus(255, 165, 0);  // Naranja si no hay WiFi
    }

    // Enviar confirmación con información del estado actual
    String result = "✅ *PRUEBA DE DIAGNÓSTICO COMPLETADA*\n\n";
    result += "Bomba 1: ✓ Probada\n";
    result += "Bomba 2: ✓ Probada\n";
    result += "Duración: 5 segundos\n";
    result += "Estado actual: Inactivas\n\n";
    result += "📊 *Estado del sistema:*\n";

    // Actualizar el modo con 3 estados
    String modeText = "";
    if (config.irrigationMode == 0) modeText = "MANUAL";
    else if (config.irrigationMode == 1) modeText = "AUTOMATICO";
    else if (config.irrigationMode == 2) modeText = "ADAPTATIVO";

    result += "• Modo de Riego: " + modeText + "\n";
    result += "• Suministro Agua: " + String(data.waterSupply ? "CORRECTO" : "SIN AGUA") + "\n";
    result += "• Tempe. Ambiente: " + String(data.temp, 1) + "°C\n";
    result += "• Humed. Ambiente: " + String(data.hum, 1) + "%\n";

    bot->sendMessage(chat_id, result, "Markdown");

    Serial.println("[TELEGRAM TEST] ✅ Prueba completada - Bombas inactivas");
    addSerialLog("[TELEGRAM] ✅ Prueba diagnóstico completada");
  }

  else if (text == "/manual") {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    config.irrigationMode = 0;
    saveConfig();
    xSemaphoreGive(mutexConfig);
  }
  bot->sendMessage(chat_id, "✅ Modo MANUAL activado", "");
  }
  else if (text == "/auto") {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    config.irrigationMode = 1;
    saveConfig();
    xSemaphoreGive(mutexConfig);
  }
  bot->sendMessage(chat_id, "🤖 Modo AUTOMÁTICO activado", "");
  }
  else if (text == "/adaptativo") {
  if (learningZone1.totalCycles >= 10 && learningZone2.totalCycles >= 10) {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.irrigationMode = 2;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    bot->sendMessage(chat_id, "🧠 Modo ADAPTATIVO activado", "");
  } else {
    String msg = "⚠️ Datos insuficientes\n\n";
    msg += "Zona 1: " + String(learningZone1.totalCycles) + "/10 ciclos\n";
    msg += "Zona 2: " + String(learningZone2.totalCycles) + "/10 ciclos";
    bot->sendMessage(chat_id, msg, "");
  }
  }
  // COMANDO: /vpdprotección
  else if (text == "/vpdswitch") {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.vpdProtection = !config.vpdProtection;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    String msg = config.vpdProtection ? "✅ Protección VPD ACTIVADA" : "⭕ Protección VPD DESACTIVADA";
    bot->sendMessage(chat_id, msg, "");
    addSerialLog("[TELEGRAM] VPD protection " + String(config.vpdProtection ? "ON" : "OFF"));
  }



  // COMANDO: /config
  else if (text == "/config") {
    String cfg = "⚙️ *Configuración Actual*\n\n";
    cfg += "Umbral bomba 1: " + String(config.threshold1) + "%\n";
    cfg += "Umbral bomba 2: " + String(config.threshold2) + "%\n";
    cfg += "Protección solar: " + String(config.lightProtection ? "SÍ" : "NO") + " (" + String(config.lightThreshold) + "%)\n";
    cfg += "Protección temp: " + String(config.tempProtection ? "SÍ" : "NO") + " (" + String(config.minTempThreshold, 1) + "-" + String(config.maxTempThreshold, 1) + "°C)\n";
    cfg += "Protección humedad: " + String(config.humidityProtection ? "SÍ" : "NO") + " (" + String(config.humidityThreshold, 0) + "%)\n";
    cfg += "Sistema salud VPD: " + String(config.vpdProtection ? "SÍ" : "NO") + "\n";
    cfg += "Control Agua: " + String(config.waterSupplyControl ? "SÍ" : "NO");

    bot->sendMessage(chat_id, cfg, "Markdown");
  }

  // COMANDO: /restricciones
  else if (text == "/restricciones") {
    String rest = "🛡️ *Estado de Restricciones*\n\n";

    bool lightBlocking = config.lightProtection && (data.light > config.lightThreshold);
    bool waterBlocking = config.waterSupplyControl && !data.waterSupply;
    bool tempBlocking = config.tempProtection && (data.temp < config.minTempThreshold || data.temp > config.maxTempThreshold);
    bool humidityBlocking = config.humidityProtection && (data.hum > config.humidityThreshold);
    bool weatherBlocking = config.weatherGuard && config.rainExpected;
    bool vpdBlocking = config.vpdProtectionActive;

    rest += "☀️ Solar: " + String(lightBlocking ? "BLOQUEANDO ❌" : "OK ✅") + "\n";
    rest += "💧 Suministro: " + String(waterBlocking ? "FALLO ❌" : "OK ✅") + "\n";
    rest += "🌡️ Temperatura: " + String(tempBlocking ? "BLOQUEANDO ❌" : "OK ✅") + "\n";
    rest += "💦 Humedad: " + String(humidityBlocking ? "BLOQUEANDO ❌" : "OK ✅") + "\n";
    rest += "🌧️ Lluvia: " + String(weatherBlocking ? "ESPERADA ❌" : "OK ✅") + "\n\n";
    rest += "💦 VPD: " + String(vpdBlocking ? "BLOQUEANDO ❌" : "OK ✅") + "\n";


    bool anyBlocking = lightBlocking || waterBlocking || tempBlocking || humidityBlocking || weatherBlocking;
    rest += "Estado general: " + String(anyBlocking ? "⛔ RIEGO BLOQUEADO" : "✅ RIEGO PERMITIDO");

    bot->sendMessage(chat_id, rest, "Markdown");
  }

  // COMANDO: /setth1 XX
  else if (text.startsWith("/setth1 ")) {
    int value = text.substring(8).toInt();
    if (value >= 0 && value <= 100) {
      if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.threshold1 = value;
        saveConfig();
        xSemaphoreGive(mutexConfig);
      }
      bot->sendMessage(chat_id, "✅ Umbral bomba 1 ajustado a " + String(value) + "%", "");
      addSerialLog("[TELEGRAM] Umbral 1 cambiado a " + String(value) + "%");
    } else {
      bot->sendMessage(chat_id, "❌ Valor inválido. Usa un número entre 0 y 100", "");
    }
  }

  // COMANDO: /setth2 XX
  else if (text.startsWith("/setth2 ")) {
    int value = text.substring(8).toInt();
    if (value >= 0 && value <= 100) {
      if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.threshold2 = value;
        saveConfig();
        xSemaphoreGive(mutexConfig);
      }
      bot->sendMessage(chat_id, "✅ Umbral bomba 2 ajustado a " + String(value) + "%", "");
      addSerialLog("[TELEGRAM] Umbral 2 cambiado a " + String(value) + "%");
    } else {
      bot->sendMessage(chat_id, "❌ Valor inválido. Usa un número entre 0 y 100", "");
    }
  }

  // COMANDO: /vpd
  else if (text == "/vpd") {
    String vpdInfo = "🌿 *Estado VPD (Déficit Presión Vapor)*\n\n";
    vpdInfo += "Valor actual: " + String(data.vpd, 2) + " kPa\n";
    vpdInfo += "Calidad: " + getVPDQuality(data.vpd) + "\n";
    vpdInfo += "Rango óptimo: " + String(config.vpdMinThreshold, 1) + "-" + String(config.vpdMaxThreshold, 1) + " kPa\n";
    vpdInfo += "Protección: " + String(config.vpdProtection ? "ACTIVA" : "INACTIVA") + "\n";
    vpdInfo += "Estado: " + String(config.vpdProtectionActive ? "BLOQUEANDO" : "OK");

    bot->sendMessage(chat_id, vpdInfo, "Markdown");
  }

  // COMANDO: /zonas
  else if (text == "/zonas") {
    ZoneIrrigationDecision zone1 = analyzeZoneIrrigation(1, data.soil1, config.threshold1);
    ZoneIrrigationDecision zone2 = analyzeZoneIrrigation(2, data.soil2, config.threshold2);

    String zonesInfo = "🌿 *Análisis Inteligente por Zonas*\n\n";
    zonesInfo += "📊 *Condiciones:*\n";
    zonesInfo += "• VPD: " + String(data.vpd, 2) + " kPa (" + getVPDQuality(data.vpd) + ")\n";
    zonesInfo += "• Temp: " + String(data.temp, 1) + "°C | Hum: " + String(data.hum, 1) + "%\n\n";

    zonesInfo += "🌱 *Zona 1:*\n";
    zonesInfo += "• Suelo: " + String(data.soil1, 1) + "% (umbral: " + String(config.threshold1) + "%)\n";
    zonesInfo += "• Urgencia: " + String(zone1.urgency * 100, 0) + "%\n";
    zonesInfo += "• Factor VPD: " + String(zone1.vpdFactor, 1) + "x\n";
    zonesInfo += "• Recomendación: " + String(zone1.shouldIrrigate ? "REGAR" : "ESPERAR") + "\n\n";

    zonesInfo += "🌱 *Zona 2:*\n";
    zonesInfo += "• Suelo: " + String(data.soil2, 1) + "% (umbral: " + String(config.threshold2) + "%)\n";
    zonesInfo += "• Urgencia: " + String(zone2.urgency * 100, 0) + "%\n";
    zonesInfo += "• Factor VPD: " + String(zone2.vpdFactor, 1) + "x\n";
    zonesInfo += "• Recomendación: " + String(zone2.shouldIrrigate ? "REGAR" : "ESPERAR") + "\n";

    bot->sendMessage(chat_id, zonesInfo, "Markdown");
  }

  // COMANDO NO RECONOCIDO
  else {
    bot->sendMessage(chat_id, "❓ Comando no reconocido. Usa /help para ver comandos disponibles", "");
  }
}


// ===========================
// FUNCIONES UTILITARIAS
// ===========================


//===========================================
// GUARDA CONFIGURACIÓN
//===========================================


void saveWifiConfig(String ssid, String password) {
  Serial.println("[WIFI] Guardando credenciales en LittleFS...");

  for (int intento = 1; intento <= 3; intento++) {
    if (intento > 1) {
      Serial.printf("[WIFI] Reintentando (%d/3)...\n", intento);
      delay(100 * intento);
    }

    if (!LittleFS.begin()) {
      Serial.printf("[WIFI] Intento %d: LittleFS no montado\n", intento);
      if (intento == 1) LittleFS.format();
      continue;
    }



    File f = LittleFS.open("/wifi.json", "w");
    if (!f) {
      Serial.printf("[WIFI] Intento %d: Error abriendo archivo\n", intento);
      LittleFS.end();
      continue;
    }

    StaticJsonDocument<256> doc;
    doc["ssid"] = ssid;
    doc["password"] = password;

    size_t bytesWritten = serializeJson(doc, f);
    f.flush();
    f.close();

    delay(50);

    Serial.print("[CONFIG] Configuración guardada: ");
    Serial.print(bytesWritten);
    Serial.println(" bytes");
    Serial.println("[CONFIG] ✓ Campos de red añadidos");


    if (LittleFS.exists("/wifi.json")) {
      Serial.println("[WIFI] ✓ Credenciales guardadas correctamente");
      LittleFS.end();
      return;
    }

    Serial.printf("[WIFI] Intento %d: Archivo no verificado\n", intento);
    LittleFS.end();
  }

  Serial.println("[WIFI] ✗ Error: No se pudieron guardar las credenciales");
}

bool loadWifiConfig() {
  if (!LittleFS.begin()) {
    Serial.println("[WIFI] LittleFS no disponible, formateando...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("[WIFI] Error crítico: No se puede montar LittleFS");
      return false;
    }
  }

  File f = LittleFS.open("/wifi.json", "r");
  if (f) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, f);
    f.close();

    if (error) {
      Serial.print("[WIFI] Error deserializando JSON: ");
      Serial.println(error.c_str());
      LittleFS.end();
      return false;
    }

    ssid_stored = doc["ssid"].as<String>();
    pass_stored = doc["password"].as<String>();

    LittleFS.end();

    if (ssid_stored.length() > 0) {
      Serial.print("[WIFI] Credenciales cargadas: ");
      Serial.println(ssid_stored);
      return true;
    }
  } else {
    Serial.println("[WIFI] No hay credenciales WiFi guardadas");
  }

  LittleFS.end();
  return false;
}

void deleteWifiConfig() {
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }

  if (LittleFS.exists("/wifi.json")) {
    LittleFS.remove("/wifi.json");
    Serial.println("[WIFI] Archivo wifi.json eliminado");
  }

  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  ssid_stored = "";
  pass_stored = "";

  Serial.println("[WIFI] Credenciales WiFi eliminadas completamente");
  LittleFS.end();
}

String urlEncode(String str) {
  String encoded = "";
  char c;
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == ' ') {
      encoded += '+';
    } else if (c == ',') {
      encoded += "%2C";
    } else {
      encoded += '%';
      char hex[3];
      sprintf(hex, "%02X", c);
      encoded += hex;
    }
  }
  return encoded;
}

// ===========================
// FUNCIÓN BOTÓN RESET WIFI
// ===========================
void handleWifiResetButton() {
  static unsigned long lastDebounceTime = 0;
  static bool lastButtonState = HIGH;

  bool currentButtonState = digitalRead(PIN_WIFI_RESET);

  // Debounce del botón
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    // Botón presionado (LOW porque usa pullup)
    if (currentButtonState == LOW && !buttonPressed) {
      buttonPressed = true;
      buttonPressStart = millis();
      Serial.println("[BUTTON] Botón WiFi reset presionado...");
      addSerialLog("[BUTTON] Presiona 3 seg para reset WiFi");

      // LED naranja para indicar que está presionado
      setLedStatus(255, 165, 0);
    }
    // Botón soltado
    else if (currentButtonState == HIGH && buttonPressed) {
      buttonPressed = false;
      unsigned long pressDuration = millis() - buttonPressStart;

      if (pressDuration >= BUTTON_HOLD_TIME) {
        Serial.println("[BUTTON] Reset WiFi activado!");
        addSerialLog("[BUTTON] ✓ WiFi reset - Reiniciando...");

        // Feedback visual y sonoro
        for (int i = 0; i < 5; i++) {
          setLedStatus(255, 0, 0);
          digitalWrite(PIN_BUZZER, HIGH);
          delay(100);
          setLedStatus(0, 0, 0);
          digitalWrite(PIN_BUZZER, LOW);
          delay(100);
        }

        // Eliminar credenciales y reiniciar
        deleteWifiConfig();
        delay(1000);
        ESP.restart();
      } else {
        Serial.println("[BUTTON] Presión corta, reset cancelado");
        addSerialLog("[BUTTON] Reset cancelado - presión muy corta");

        // Restaurar LED según estado del sistema
        if (wifiConnected) {
          setLedStatus(0, 255, 0);
        } else {
          setLedStatus(255, 165, 0);
        }
      }
    }

    // Feedback durante la presión larga
    if (buttonPressed) {
      unsigned long pressDuration = millis() - buttonPressStart;
      if (pressDuration >= BUTTON_HOLD_TIME) {
        // Parpadeo rápido cuando ya se puede hacer reset
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 100) {
          static bool blinkState = false;
          setLedStatus(blinkState ? 255 : 0, 0, 0);
          blinkState = !blinkState;
          lastBlink = millis();
        }
      } else {
        // Parpadeo naranja mientras se mantiene presionado
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 200) {
          static bool blinkState = false;
          setLedStatus(blinkState ? 255 : 100, blinkState ? 165 : 50, 0);
          blinkState = !blinkState;
          lastBlink = millis();
        }
      }
    }
  }

  lastButtonState = currentButtonState;
}

//====================================//
// Restaurar a los valores de fábrica //
//====================================//
void deleteAllConfig() {
  Serial.println("[RESET] Iniciando borrado total de configuración...");

  if (!LittleFS.begin()) {
    Serial.println("[RESET] Error montando LittleFS");
    return;
  }

  // Borrar archivo de configuración principal
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
    Serial.println("[RESET] config.json eliminado");
  }

  // Borrar archivo de WiFi también (opcional, puedes comentar estas líneas si quieres mantener el WiFi)
  if (LittleFS.exists("/wifi.json")) {
    LittleFS.remove("/wifi.json");
    Serial.println("[RESET] wifi.json eliminado");
  }

  LittleFS.end();

  // Restablecer valores por defecto en memoria
  config.irrigationMode = 1;
  config.threshold1 = 30;
  config.threshold2 = 30;
  config.offsetTemp = 0;
  config.offsetHum = 0;
  config.offsetPressure = 0;
  config.offsetAirQuality = 0;
  config.offsetSoil1 = 0;
  config.offsetSoil2 = 0;
  // CAMPOS DE LUZ SOLAR
  config.offsetLight = 0;
  config.lightProtection = true;
  config.lightThreshold = 70;
  // CAMPOS DE PROTECCION
  config.waterSupplyControl = true;
  config.tempProtection = true;
  config.minTempThreshold = 5.0;
  config.maxTempThreshold = 35.0;
  config.humidityProtection = true;
  config.humidityThreshold = 80.0;
  //CAMPOS PRONOSTICO DEL TIEMPO
  config.weatherGuard = true;
  config.weatherProvider = "openmeteo";
  config.weatherApiKey = "";
  config.weatherCity = "";
  config.weatherRainThresholdHours = 4;
  config.rainExpected = false;
  config.weatherLastStatus = -1;
  config.lastWeatherCheck = 0;
  config.maxRainExpected = 0.0;
  config.gmtOffset = 1;
  config.daylightSaving = true;
  config.timeFormat24h = true;
  // CAMPOS VPD:
  config.vpdProtection = true;
  config.vpdProtectionActive = false;
  config.vpdEnabled = true;
  config.vpdMinThreshold = 0.4;
  config.vpdMaxThreshold = 1.5;
  config.vpdCriticalHigh = 1.8;
  config.vpdCriticalLow = 0.3;
  config.vpdOptimalLow = 0.8;
  config.vpdOptimalHigh = 1.2;
  config.vpdFactorCriticalHigh = 1.5;
  config.vpdFactorCriticalLow = 0.3;
  config.vpdFactorOptimal = 1.2;
  config.vpdFactorNormal = 1.0;
  config.tempFactorHigh = 1.3;
  config.tempFactorLow = 0.7;
  config.tempThresholdHigh = 28.0;
  config.tempThresholdLow = 18.0;
  // CAMPOS DE CALIBRACION DE SUSTRATO
  config.soilProfile = "universal";
  config.advancedCalibration = false;
  config.universalCalibration = 1.0;
  config.clayCalibration = 1.35;
  config.sandyCalibration = 0.75;
  config.loamCalibration = 1.1;
  config.peatCalibration = 1.25;
  config.cocoCalibration = 0.95;
  config.rockwoolCalibration = 0.85;
  config.perliteCalibration = 0.7;
  config.vermiculiteCalibration = 1.4;
  //Config Telegram
  config.telegramToken = "";
  config.chatID = "";
  //Configuración por defecto de red
  config.useDHCP = true;
  config.staticIP = "192.168.1.100";
  config.staticGateway = "192.168.1.1";
  config.staticSubnet = "255.255.255.0";
  config.staticDNS1 = "8.8.8.8";
  config.staticDNS2 = "8.8.4.4";

  // Guardar los valores por defecto
  saveConfig();

  Serial.println("[RESET] Configuración restaurada a valores de fábrica");
  addSerialLog("[RESET] ✓ Config. restaurada a fábrica");
}

// ===========================
// LIMPIEZA PSRAM
// ===========================
void cleanupTelegramPSRAM() {
    Serial.println("[PSRAM] Limpiando buffers...");
    
    if (telegramPSRAM.telegramToken) {
        free(telegramPSRAM.telegramToken);
        telegramPSRAM.telegramToken = nullptr;
    }
    
    if (telegramPSRAM.chatID) {
        free(telegramPSRAM.chatID);
        telegramPSRAM.chatID = nullptr;
    }
    
    Serial.println("[PSRAM] ✅ Buffers liberados");
}


// ===========================
// FUNCIONES PARA LOGS SERIE
// ===========================
void addSerialLog(String message) {
  serialLogs[logIndex] = String(millis() / 1000) + "s: " + message;
  logIndex = (logIndex + 1) % 10;
  lastLogTime = millis();
}

void handleLogs() {
  String json = "[";
  for (int i = 0; i < 10; i++) {
    int idx = (logIndex + i) % 10;
    if (serialLogs[idx].length() > 0) {
      if (json.length() > 1) json += ",";
      json += "\"" + serialLogs[idx] + "\"";
    }
  }
  json += "]";
  server.send(200, "application/json", json);
}

String getRestrictionsStatus() {
  String status = "";

  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {

      bool lightBlocking = config.lightProtection && (data.light > config.lightThreshold);
      bool waterBlocking = config.waterSupplyControl && !data.waterSupply;
      bool tempBlocking = config.tempProtection && (data.temp < config.minTempThreshold || data.temp > config.maxTempThreshold);
      bool humidityBlocking = config.humidityProtection && (data.hum > config.humidityThreshold);

      status += "PROTECCIÓN SOLAR: " + String(lightBlocking ? "🔴 BLOQUEANDO" : "✅ OK");
      status += "|Luz: " + String(data.light, 1) + "%/" + String(config.lightThreshold) + "%";

      status += "|SUMINISTRO AGUA: " + String(waterBlocking ? "🔴 FALLO" : "✅ OK");
      status += "|Estado: " + String(data.waterSupply ? "PRESENTE" : "AUSENTE");

      status += "|TEMPERATURA: " + String(tempBlocking ? "🔴 BLOQUEANDO" : "✅ OK");
      status += "|Temp: " + String(data.temp, 1) + "°C (" + String(config.minTempThreshold, 1) + "-" + String(config.maxTempThreshold, 1) + "°C)";

      status += "|HUMEDAD AMBIENTAL: " + String(humidityBlocking ? "🔴 BLOQUEANDO" : "✅ OK");
      status += "|Humedad: " + String(data.hum, 1) + "%/" + String(config.humidityThreshold) + "%";

      // Estado de previsión de lluvia
      if (config.weatherLastStatus == -1) {
        status += "|PREVISIÓN LLUVIA: 🔶 DESACTIVADA";
      } else if (config.weatherLastStatus == 0) {
        status += "|PREVISIÓN LLUVIA: ⚠️ SIN DATOS";
      } else if (config.weatherLastStatus == 2) {
        status += "|PREVISIÓN LLUVIA: 🔴 BLOQUEANDO (próx " + String(config.weatherRainThresholdHours) + "h)";
      } else {
        status += "|PREVISIÓN LLUVIA: ✅ INACTIVA (próx " + String(config.weatherRainThresholdHours) + "h)";
      }

      String modeText = "";
if (config.irrigationMode == 0) modeText = "MANUAL";
else if (config.irrigationMode == 1) modeText = "AUTO";
else if (config.irrigationMode == 2) modeText = "ADAPT";

status += "|MODO: " + modeText;

      // Incluir bloqueo por lluvia solo si está activado y hay lluvia
      bool weatherBlocking = config.weatherGuard && config.weatherLastStatus == 2;
      status += "|BLOQUEO TOTAL: " + String((lightBlocking || waterBlocking || tempBlocking || humidityBlocking || weatherBlocking) ? "🔴 ACTIVO" : "✅ INACTIVO");

      xSemaphoreGive(mutexData);
      xSemaphoreGive(mutexConfig);
    }
  }

  return status;
}

void setLedStatus(int red, int green, int blue) {
  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();
}

void testNeopixel() {
  Serial.println("Testing WS2812 LED...");
  setLedStatus(255, 0, 0);
  delay(500);
  setLedStatus(0, 255, 0);
  delay(500);
  setLedStatus(0, 0, 255);
  delay(500);
  setLedStatus(0, 0, 0);
  Serial.println("LED test completed");
}

//=============================
// Control de bombas
//===============================
void controlPump(int pump, bool on) {
  int pin = (pump == 1) ? PIN_PUMP1 : PIN_PUMP2;
  digitalWrite(pin, on ? LOW : HIGH);

  // FEEDBACK VISUAL DEL ESTADO DE BOMBAS
  if (on) {
    // Bomba activada - LED azul
    setLedStatus(0, 0, 255);
  } else {
    // Bomba apagada - restaurar según estado sistema
    if (wifiConnected) {
      setLedStatus(0, 255, 0);  // Verde si hay WiFi
    } else {
      setLedStatus(255, 165, 0);  // Naranja si no hay WiFi
    }
  }

  String message = "[PUMP] Bomba " + String(pump) + " " + (on ? "ACTIVADA" : "DESACTIVADA");
  Serial.println(message);
  addSerialLog(message);

  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {
    if (pump == 1) data.pump1 = on;
    else data.pump2 = on;
    xSemaphoreGive(mutexData);
  }
}

//===========================
// Lectura de Sensor de Luz BH1750FVI
//===========================
float readLight() {
  // Leer valor RAW en lux del BH1750
  float lux = lightSensor.readLightLevel();
  
  // Convertir a porcentaje (0-100%) para compatibilidad
  // Mapeo: 0 lux = 0%, 20000 lux = 100%
  float percent = luxToPercent(lux);
  
  // Debug cada 50 segundos
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 50000) {
    Serial.printf("[LIGHT] RAW: %.0f lux → %.1f%%\n", lux, percent);
    lastDebug = millis();
  }
  
  return percent;
}


// ===========================
// FUNCIONES DE NTP y HORA
// ===========================
void configureNTP() {
  if (!wifiConnected) return;

  Serial.println("[NTP] Configurando servicio de hora...");

  // Usar los valores configurados por el usuario
  long gmtOffsetSec = config.gmtOffset * 3600;
  int daylightOffsetSec = config.daylightSaving ? 3600 : 0;

  // Configurar servidor NTP con offsets dinámicos
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);

  // Esperar a que se sincronice la hora
  Serial.print("[NTP] Sincronizando hora");
  int attempts = 0;
  struct tm timeinfo;

  while (!getLocalTime(&timeinfo) && attempts < 10) {
    Serial.print(".");
    attempts++;
    delay(1000);
  }

  if (attempts >= 10) {
    Serial.println("\n[NTP] Fallo al obtener la hora después de 10 intentos");
    addSerialLog("[NTP] ❌ Error sincronizando hora");
    
    // Intentar usar RTC como fallback
    if (rtcAvailable) {
      Serial.println("[NTP] Usando RTC DS3231 como respaldo...");
      syncRTCtoSystem();
      currentTimeSource = TIME_SOURCE_RTC_DS3231;
      addSerialLog("[TIME] ✅ Usando RTC DS3231");
    }
    
    return;
  }

  // Mostrar hora obtenida
  char timeString[64];
  strftime(timeString, sizeof(timeString), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println("\n[NTP] Hora sincronizada: " + String(timeString));
  Serial.println("[NTP] GMT Offset: " + String(config.gmtOffset) + ", Horario verano: " + String(config.daylightSaving ? "Sí" : "No"));
  addSerialLog("[NTP] ✅ Hora sincronizada: " + String(timeString));
  
  //Sincronizar NTP al RTC si está disponible
  if (rtcAvailable) {
    syncNTPtoRTC();
  }
  currentTimeSource = TIME_SOURCE_NTP;
}

String getFormattedTime() {
  // Actualizar fuente de tiempo
  updateTimeFromBestSource();
  
  if (currentTimeSource == TIME_SOURCE_NTP || currentTimeSource == TIME_SOURCE_RTC_DS3231) {
    // Usar NTP o RTC
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      // Intentar con RTC si falla
      if (rtcAvailable && currentTimeSource != TIME_SOURCE_RTC_DS3231) {
        syncRTCtoSystem();
        if (!getLocalTime(&timeinfo)) {
          return "Error de hora";
        }
      } else {
        return "Error de hora";
      }
    }

    char timeString[64];
    if (config.timeFormat24h) {
      strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M", &timeinfo);
    } else {
      strftime(timeString, sizeof(timeString), "%d-%m-%Y %I:%M %p", &timeinfo);
    }

    return String(timeString);
  } else {
    // Usar reloj interno basado en millis() para modo offline
    static unsigned long startMillis = millis();
    unsigned long elapsedSeconds = (millis() - startMillis) / 1000;

    unsigned long seconds = elapsedSeconds % 60;
    unsigned long minutes = (elapsedSeconds / 60) % 60;
    unsigned long hours = (elapsedSeconds / 3600) % 24;
    unsigned long days = elapsedSeconds / 86400;

    char timeString[20];
    if (config.timeFormat24h) {
      snprintf(timeString, sizeof(timeString), "+%lud %02lu:%02lu:%02lu", days, hours, minutes, seconds);
    } else {
      bool isPM = hours >= 12;
      hours = hours % 12;
      hours = hours ? hours : 12;  // 0 becomes 12
      snprintf(timeString, sizeof(timeString), "+%lud %02lu:%02lu:%02lu %s", days, hours, minutes, seconds, isPM ? "PM" : "AM");
    }

    return String(timeString);
  }
}

String getFormattedTimeFromEpoch(unsigned long epoch) {
  // Verificar si el valor es válido (timestamp entre 2020 y 2030 aprox)
  if (epoch < 1577836800 || epoch > 1893456000) {  // 2020-01-01 to 2030-01-01
    return "Sin datos";
  }

  struct tm* ti = localtime((time_t*)&epoch);
  if (ti == nullptr) {
    return "Error hora";
  }

  char buffer[10];                                // Reducido para solo hora
  strftime(buffer, sizeof(buffer), "%H:%M", ti);  // Solo hora y minuto
  return String(buffer);
}

String generateGMTOptions(int8_t currentGMT) {
  String options = "";

  // Generar opciones de GMT desde -12 hasta +14
  for (int8_t i = -12; i <= 14; i++) {
    options += "<option value=\"" + String(i) + "\"";
    if (i == currentGMT) {
      options += " selected";
    }
    options += ">GMT";
    if (i > 0) {
      options += "+" + String(i);
    } else if (i == 0) {
      options += "±0";
    } else {
      options += String(i);  // Los negativos ya incluyen el signo -
    }
    options += " (";

    // Agregar ejemplos de ciudades para hacerlo más user-friendly
    if (i == -12) options += "Línea Internacional";
    else if (i == -11) options += "Samoa";
    else if (i == -10) options += "Hawaii";
    else if (i == -9) options += "Alaska";
    else if (i == -8) options += "Pacífico EEUU";
    else if (i == -7) options += "Mountain EEUU";
    else if (i == -6) options += "Central EEUU";
    else if (i == -5) options += "Eastern EEUU";
    else if (i == -4) options += "Atlántico";
    else if (i == -3) options += "Brasilia";
    else if (i == -2) options += "Mid-Atlantic";
    else if (i == -1) options += "Azores";
    else if (i == 0) options += "Londres";
    else if (i == 1) options += "Madrid/Paris";
    else if (i == 2) options += "Europa Este";
    else if (i == 3) options += "Moscú";
    else if (i == 4) options += "Dubai";
    else if (i == 5) options += "Pakistán";
    else if (i == 6) options += "Bangladesh";
    else if (i == 7) options += "Bangkok";
    else if (i == 8) options += "China";
    else if (i == 9) options += "Japón";
    else if (i == 10) options += "Sydney";
    else if (i == 11) options += "N. Caledonia";
    else if (i == 12) options += "N. Zelanda";
    else if (i == 13) options += "Samoa";
    else if (i == 14) options += "Kiribati";

    options += ")</option>";
  }

  return options;
}

//=========================================
// AJUSTE RTC
//=========================================
void handleSetRTC() {
  Serial.println("[WEB] Solicitud de ajuste manual del RTC");
  
  if (!rtcAvailable) {
    Serial.println("[RTC] ❌ No disponible");
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='3;url=/'>";
    html += "<style>body{background:#0f172a;color:white;text-align:center;padding:50px;font-family:Arial}</style>";
    html += "</head><body>";
    html += "<div style='background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #ef4444;'>";
    html += "<h2 style='color:#ef4444;'>❌ Error</h2>";
    html += "<p>RTC DS3231 no disponible</p>";
    html += "<p>Redirigiendo...</p></div></body></html>";
    server.send(200, "text/html", html);
    return;
  }
  
  if (server.hasArg("year") && server.hasArg("month") && server.hasArg("day") &&
      server.hasArg("hour") && server.hasArg("minute")) {
    
    int year = server.arg("year").toInt();
    int month = server.arg("month").toInt();
    int day = server.arg("day").toInt();
    int hour = server.arg("hour").toInt();
    int minute = server.arg("minute").toInt();
    int second = server.hasArg("second") ? server.arg("second").toInt() : 0;
    
    Serial.printf("[RTC] Intentando ajustar: %02d/%02d/%04d %02d:%02d:%02d\n", 
                  day, month, year, hour, minute, second);
    
    bool success = setRTCManually(year, month, day, hour, minute, second);
    
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='3;url=/'>";
    html += "<style>body{background:#0f172a;color:white;text-align:center;padding:50px;font-family:Arial}</style>";
    html += "</head><body>";
    html += "<div style='background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid ";
    html += success ? "#10b981" : "#ef4444";
    html += ";'>";
    
    if (success) {
      html += "<h2 style='color:#10b981;'>✅ RTC Ajustado</h2>";
      html += "<p>Hora configurada correctamente</p>";
      html += "<p>" + String(day) + "/" + String(month) + "/" + String(year);
      html += " " + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + "</p>";
      addSerialLog("[RTC] ✅ Ajustado manualmente");
    } else {
      html += "<h2 style='color:#ef4444;'>⚠️ Error</h2>";
      html += "<p>No se pudo ajustar el RTC</p>";
      addSerialLog("[RTC] ❌ Error ajuste manual");
    }
    
    html += "<p>Redirigiendo...</p></div></body></html>";
    server.send(200, "text/html", html);
    
  } else {
    Serial.println("[RTC] ❌ Faltan parámetros");
    server.send(400, "text/plain", "Faltan parámetros");
  }
}

// ===========================
// TAREAS FREERTOS
// ===========================
void taskReadSensors(void* pv) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(5000);

  while (true) {
    // LECTURA DE SENSORES SIEMPRE ACTIVA - CON O SIN WIFI
    float t = 25.0;
    float h = 50.0;
    float p = 1013.25;  // Presión nivel del mar por defecto
    float gas = 0.0;    // Calidad aire por defecto
    
    // Intentar lectura del BME680
    if (bme.performReading()) {
      t = bme.temperature;
      h = bme.humidity;
      p = bme.pressure / 100.0;  // Convertir Pa a hPa
      gas = bme.gas_resistance / 1000.0;  // Convertir Ohms a kOhms
      
      // Log solo en Serial cada 15 lecturas para no saturar
      static int readCount = 0;
      if (++readCount >= 15) {
        Serial.printf("[BME680] T:%.1f°C H:%.1f%% P:%.1fhPa Gas:%.1fkΩ\n", 
                      t, h, p, gas);
        readCount = 0;
      }
    } else {
      Serial.println("[BME680] ⚠️ Lectura fallida, usando valores por defecto");
    }
    
    float soil1, soil2;

    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(100)) == pdTRUE) {
      // USAR FUNCIÓN UNIFICADA para lectura calibrada
      soil1 = readCalibratedSoil(PIN_SOIL1, config.soil1Cal);
      soil2 = readCalibratedSoil(PIN_SOIL2, config.soil2Cal);
      soil1 += config.offsetSoil1;
      soil2 += config.offsetSoil2;

      xSemaphoreGive(mutexConfig);
    } else {
      // Fallback si no se puede acceder a config
      soil1 = readSoilRawPercent(PIN_SOIL1);
      soil2 = readSoilRawPercent(PIN_SOIL2);
    }
    float light = readLight();
    bool waterStatus = digitalRead(PIN_WATER_SUPPLY) == LOW;
    
    if (!isnan(t) && !isnan(h)) {
  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {
    // APLICAR OFFSETS PRIMERO
    data.temp = t + config.offsetTemp;
    data.hum = h + config.offsetHum;
    data.pressure = p + config.offsetPressure;  
    data.airQuality = gas + config.offsetAirQuality;
    data.soil1 = soil1;
    data.soil2 = soil2;
    data.light = light + config.offsetLight;
    data.waterSupply = waterStatus;
    
    // ✅ CALCULAR VPD UNA SOLA VEZ con valores CALIBRADOS
    data.vpd = calculateVPD(data.temp, data.hum);
    
    xSemaphoreGive(mutexData);
  }
}
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // ✅ Espera hasta próximo ciclo
  }
}


//============================================
// Gestión de Auto Irrigación CON INFERENCIA INTELIGENTE
//============================================
void taskAutoIrrigation(void* pv) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t interval = pdMS_TO_TICKS(2000);  // 2 segundos

  while (true) {
    // 1. OBTENER DATOS RÁPIDAMENTE (timeouts cortos)
    SensorData localData;
    Config localConfig;
    bool dataRetrieved = false;

    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(50)) == pdTRUE) {
      localData = data;
      xSemaphoreGive(mutexData);
      dataRetrieved = true;
    }

    if (dataRetrieved && xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(50)) == pdTRUE) {
      localConfig = config;
      xSemaphoreGive(mutexConfig);
    } else {
      dataRetrieved = false;
    }

    // 2. PROCESAR SOLO SI SE OBTUVIERON DATOS
      if (dataRetrieved && config.irrigationMode != 0) {
      // Verificaciones globales rápidas
      bool waterSupplyBlocking = localConfig.waterSupplyControl && !localData.waterSupply;
      bool weatherBlocking = localConfig.weatherGuard && localConfig.rainExpected;
      bool globalVPDBlocking = localConfig.vpdProtection && isVPDBlocking(localData.vpd);
      bool timeBlocking = isTimeRestricted();
      bool windBlocking = isWindRestricted();

      // Seleccionar umbrales según el modo
      int threshold1ToUse = (localConfig.irrigationMode == 2) ? 
                        localConfig.aiThreshold1 : localConfig.threshold1;
      int threshold2ToUse = (localConfig.irrigationMode == 2) ? 
                        localConfig.aiThreshold2 : localConfig.threshold2;

      // 3. ANÁLISIS INTELIGENTE CON INFERENCIA
      ZoneIrrigationDecision zone1 = analyzeZoneIrrigation(1, localData.soil1, threshold1ToUse);
      ZoneIrrigationDecision zone2 = analyzeZoneIrrigation(2, localData.soil2, threshold2ToUse);
            
      // CALCULAR NECESIDAD DE RIEGO CON INFERENCIA INTELIGENTE
      float irrigationNeed1 = calculateIrrigationNeed(localData, 1, localConfig.threshold1);
      float irrigationNeed2 = calculateIrrigationNeed(localData, 2, localConfig.threshold2);
      
      // APLICAR FACTOR VPD A LA NECESIDAD CALCULADA
      irrigationNeed1 *= zone1.vpdFactor;
      irrigationNeed2 *= zone2.vpdFactor;

      // 4. DECISIONES MEJORADAS CON INFERENCIA INTELIGENTE
      bool shouldStart1 = !localData.pump1 && 
                         (zone1.shouldIrrigate || irrigationNeed1 > localConfig.inferenceThresholdHigh) &&
                         !waterSupplyBlocking && !weatherBlocking && !globalVPDBlocking;
      
      bool shouldStop1 = localData.pump1 && 
                        (localData.soil1 > (localConfig.threshold1 + 5) || 
                         irrigationNeed1 < localConfig.inferenceThresholdLow ||
                         waterSupplyBlocking || globalVPDBlocking);

      bool shouldStart2 = !localData.pump2 && 
                         (zone2.shouldIrrigate || irrigationNeed2 > localConfig.inferenceThresholdHigh) &&
                         !waterSupplyBlocking && !weatherBlocking && !globalVPDBlocking;
      
      bool shouldStop2 = localData.pump2 && 
                        (localData.soil2 > (localConfig.threshold2 + 5) || 
                         irrigationNeed2 < localConfig.inferenceThresholdLow ||
                         waterSupplyBlocking || globalVPDBlocking);

      // 5. EJECUCIÓN ZONA 1 CON INFERENCIA + APRENDIZAJE
      if (shouldStart1) {
        // INICIAR CICLO DE MONITOREO (FASE 2)
        currentCycle1.startTime = millis();
        currentCycle1.initialMoisture = localData.soil1;
        currentCycle1.vpdAtStart = localData.vpd;
        currentCycle1.tempAtStart = localData.temp;
        currentCycle1.completed = false;

        String message = "[SMART-Z1] " + zone1.reason + " | Inferencia: " + String(irrigationNeed1 * 100, 1) + "%";
        Serial.println(message);
        addSerialLog("[Z1] " + zone1.analysis + " | Need: " + String(irrigationNeed1 * 100, 1) + "%");

        digitalWrite(PIN_PUMP1, LOW);

        if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(20)) == pdTRUE) {
          data.pump1 = true;
          data.lastIrrigation1 = millis();
          data.irrigationNeed1 = irrigationNeed1;
          xSemaphoreGive(mutexData);
        }

        if (wifiConnected && localConfig.telegramToken.length() > 0) {
          String telegramMsg = "🌿 *Zona 1 Activada - INFERENCIA INTELIGENTE*\n";
          telegramMsg += "• Suelo: " + String(localData.soil1, 1) + "%\n";
          telegramMsg += "• VPD: " + String(localData.vpd, 2) + " kPa\n";
          telegramMsg += "• Factor VPD: " + String(zone1.vpdFactor, 1) + "x\n";
          telegramMsg += "• *Inferencia: " + String(irrigationNeed1 * 100, 1) + "%*\n";
          telegramMsg += "• " + zone1.reason;
          sendTelegramAlert("🤖 Riego Inteligente", telegramMsg);
        }
      }

      if (shouldStop1) {
        // ANALIZAR EFICIENCIA DEL CICLO (FASE 2)
        analyzeIrrigationEfficiency(1, currentCycle1, localData.soil1);
        
        // OPTIMIZAR SI ES NECESARIO (FASE 2)
        optimizeThresholds();

        String message = "[SMART-Z1] OFF - Inferencia: " + String(irrigationNeed1 * 100, 1) + "%";
        if (globalVPDBlocking) message += " | VPD blocking";
        if (irrigationNeed1 < localConfig.inferenceThresholdLow) message += " | Necesidad baja";
        Serial.println(message);
        addSerialLog(message);

        digitalWrite(PIN_PUMP1, HIGH);

        if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(20)) == pdTRUE) {
          data.pump1 = false;
          data.irrigationNeed1 = irrigationNeed1;
          xSemaphoreGive(mutexData);
        }
      }

      // 6. EJECUCIÓN ZONA 2 CON INFERENCIA + APRENDIZAJE
      if (shouldStart2) {
        // INICIAR CICLO DE MONITOREO (FASE 2)
        currentCycle2.startTime = millis();
        currentCycle2.initialMoisture = localData.soil2;
        currentCycle2.vpdAtStart = localData.vpd;
        currentCycle2.tempAtStart = localData.temp;
        currentCycle2.completed = false;

        String message = "[SMART-Z2] " + zone2.reason + " | Inferencia: " + String(irrigationNeed2 * 100, 1) + "%";
        Serial.println(message);
        addSerialLog("[Z2] " + zone2.analysis + " | Need: " + String(irrigationNeed2 * 100, 1) + "%");

        digitalWrite(PIN_PUMP2, LOW);

        if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(20)) == pdTRUE) {
          data.pump2 = true;
          data.lastIrrigation2 = millis();
          data.irrigationNeed2 = irrigationNeed2;
          xSemaphoreGive(mutexData);
        }

        if (wifiConnected && localConfig.telegramToken.length() > 0) {
          String telegramMsg = "🌿 *Zona 2 Activada - INFERENCIA INTELIGENTE*\n";
          telegramMsg += "• Suelo: " + String(localData.soil2, 1) + "%\n";
          telegramMsg += "• VPD: " + String(localData.vpd, 2) + " kPa\n";
          telegramMsg += "• Factor VPD: " + String(zone2.vpdFactor, 1) + "x\n";
          telegramMsg += "• *Inferencia: " + String(irrigationNeed2 * 100, 1) + "%*\n";
          telegramMsg += "• " + zone2.reason;
          sendTelegramAlert("🤖 Riego Inteligente", telegramMsg);
        }
      }

      if (shouldStop2) {
        // ANALIZAR EFICIENCIA DEL CICLO (FASE 2)
        analyzeIrrigationEfficiency(2, currentCycle2, localData.soil2);
        
        // OPTIMIZAR SI ES NECESARIO (FASE 2)
        optimizeThresholds();

        String message = "[SMART-Z2] OFF - Inferencia: " + String(irrigationNeed2 * 100, 1) + "%";
        if (globalVPDBlocking) message += " | VPD blocking";
        if (irrigationNeed2 < localConfig.inferenceThresholdLow) message += " | Necesidad baja";
        Serial.println(message);
        addSerialLog(message);

        digitalWrite(PIN_PUMP2, HIGH);

        if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(20)) == pdTRUE) {
          data.pump2 = false;
          data.irrigationNeed2 = irrigationNeed2;
          xSemaphoreGive(mutexData);
        }
      }

      // 7. ACTUALIZAR ESTADO VPD
      if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(20)) == pdTRUE) {
        config.vpdProtectionActive = globalVPDBlocking;
        config.timeRestrictionActive = timeBlocking;
        config.windRestrictionActive = windBlocking; 
        xSemaphoreGive(mutexConfig);
      }

      // 8. MOSTRAR ESTADÍSTICAS CADA 5 MINUTOS (FASE 2)
      static unsigned long lastStatsPrint = 0;
      if (millis() - lastStatsPrint > 300000) { // 5 minutos
        printLearningStats();
        lastStatsPrint = millis();
      }
    }

    // 9. ESPERA PRECISA SIN BLOQUEOS
    vTaskDelayUntil(&lastWakeTime, interval);
  }
}

// =====================================
// SINCRONIZACIÓN HORA del RTC
// =====================================
void taskTimeMaintenance(void* pv) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(60000);
  
  Serial.println("[TIME-TASK] ✅ Tarea iniciada");
  
  while (true) {
    updateTimeFromBestSource();
    
    if (currentTimeSource == TIME_SOURCE_RTC_DS3231) {
      syncInternalToRTC();
    }
    
    if (wifiConnected && rtcAvailable && currentTimeSource == TIME_SOURCE_NTP) {
      static unsigned long lastNTPSync = 0;
      if (millis() - lastNTPSync > 21600000 || lastNTPSync == 0) {
        if (syncNTPtoRTC()) {
          lastNTPSync = millis();
          addSerialLog("[TIME] ✅ RTC sincronizado con NTP");
        }
      }
    }
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}


// ============================================
// SISTEMA DE INFERENCIA INTELIGENTE SIMPLIFICADO
// ============================================
float calculateIrrigationNeed(SensorData data, int zone, float threshold) {
  float soilValue = (zone == 1) ? data.soil1 : data.soil2;
  
  // 1. FACTOR HUMEDAD DEL SUELO (Peso: 60%)
  float soilFactor = 0.0;
  if (soilValue < threshold - 15) {
    soilFactor = 1.0;        // Muy seco
  } else if (soilValue < threshold - 10) {
    soilFactor = 0.8;        // Seco
  } else if (soilValue < threshold - 5) {
    soilFactor = 0.6;        // Algo seco
  } else if (soilValue < threshold) {
    soilFactor = 0.3;        // Cercano al threshold
  } else {
    soilFactor = 0.0;        // Suficiente humedad
  }

  // 2. FACTOR VPD - Déficit de presión de vapor (Peso: 25%)
  float vpdFactor = 0.0;
  if (data.vpd > 2.2) {
    vpdFactor = 1.0;         // Estrés hídrico alto
  } else if (data.vpd > 1.8) {
    vpdFactor = 0.8;         // Estrés hídrico moderado
  } else if (data.vpd > 1.4) {
    vpdFactor = 0.4;         // Condiciones normales
  } else if (data.vpd > 1.0) {
    vpdFactor = 0.2;         // Condiciones óptimas
  } else {
    vpdFactor = 0.1;         // Humedad ambiental alta
  }

  // 3. FACTOR TEMPERATURA (Peso: 15%)
  float tempFactor = 0.0;
  if (data.temp > 32) {
    tempFactor = 1.0;        // Muy caliente - mayor evaporación
  } else if (data.temp > 28) {
    tempFactor = 0.7;        // Caliente
  } else if (data.temp > 24) {
    tempFactor = 0.4;        // Templado
  } else if (data.temp > 18) {
    tempFactor = 0.2;        // Óptimo
  } else {
    tempFactor = 0.1;        // Frío - menor necesidad
  }

  // 4. COMBINACIÓN INTELIGENTE CON PESOS
  float irrigationNeed = (soilFactor * 0.60) + (vpdFactor * 0.25) + (tempFactor * 0.15);
  
  // Asegurar que está en el rango 0-1
  return constrain(irrigationNeed, 0.0, 1.0);
}

// ===========================
// MANEJO DE CALIBRACIÓN
// ===========================
void handleSetCalibration() {
  Serial.println("[CALIBRATION] Procesando calibración...");

  if (server.hasArg("sensor") && server.hasArg("offset_value")) {
    int sensor = server.arg("sensor").toInt();
    float offsetValue = server.arg("offset_value").toFloat();

    Serial.printf("[CALIBRATION] Sensor %d, nuevo offset: %.2f\n", sensor, offsetValue);

    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      if (sensor == 1) {
        config.offsetSoil1 = offsetValue;
        Serial.printf("[CALIBRATION] Offset Soil1 actualizado: %.2f\n", config.offsetSoil1);
      } else if (sensor == 2) {
        config.offsetSoil2 = offsetValue;
        Serial.printf("[CALIBRATION] Offset Soil2 actualizado: %.2f\n", config.offsetSoil2);
      }

      saveConfig();
      xSemaphoreGive(mutexConfig);

      addSerialLog("[CAL] ✓ Sensor " + String(sensor) + " calibrado. Offset: " + String(offsetValue, 2));

      // Enviar mensaje de confirmación
      String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
      html += "<meta http-equiv='refresh' content='3;url=/'>";
      html += "<style>body{font-family:Arial;text-align:center;padding:50px;background:#0f172a;color:white;}</style>";
      html += "</head><body>";
      html += "<div style='background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #10b981;'>";
      html += "<h2 style='color:#10b981;'><i class='fas fa-check-circle'></i> Calibración Aplicada</h2>";
      html += "<p>Offset del sensor " + String(sensor) + " actualizado a: <strong>" + String(offsetValue, 2) + "</strong></p>";
      html += "<p>Redirigiendo a la página principal...</p>";
      html += "</div></body></html>";

      server.send(200, "text/html", html);
      return;
    }
  }

  // Si hay error
  server.sendHeader("Location", "/");
  server.send(303);
}

//========================
// Pronóstico del Tiempo - Inicialización
//========================
void taskWeatherGuard(void* pv) {
  unsigned long lastCheckTime = 0;
  const unsigned long CHECK_INTERVAL = 2 * 60 * 60 * 1000;  // cada 2 horas

  while (true) {
    if (!wifiConnected) {
      vTaskDelay(pdMS_TO_TICKS(30000));
      continue;
    }

    bool hasValidConfig = false;

    if (config.weatherProvider == "openmeteo") {
      hasValidConfig = (config.weatherGuard && config.weatherCity.length() > 0);
    } else {
      hasValidConfig = (config.weatherGuard && config.weatherApiKey.length() > 0 && config.weatherCity.length() > 0);
    }

    if (hasValidConfig) {
      unsigned long currentTime = millis();
      bool shouldCheck = false;

      if (config.weatherUpdateRequested) {
        shouldCheck = true;
        config.weatherUpdateRequested = false;
        Serial.println("[WEATHER] Actualización inmediata solicitada");
        addSerialLog("[WEATHER] Consultando pronóstico completo...");
      } else if ((currentTime - lastCheckTime) >= CHECK_INTERVAL || lastCheckTime == 0) {
        shouldCheck = true;
        if (lastCheckTime == 0) {
          Serial.println("[WEATHER] Primera consulta completa al arranque");
        } else {
          Serial.println("[WEATHER] Actualización periódica completa");
        }
      }

      if (shouldCheck) {
        WiFiClientSecure client;
        HTTPClient http;
        client.setInsecure();

        String url = "";
        bool requestSuccess = false;

        // =============================
        // OPEN-METEO API COMPLETA CON ET0 Y DATOS AVANZADOS
        // =============================
        if (config.weatherProvider == "openmeteo") {
          String cityParam = config.weatherCity;
          float latitude = 0, longitude = 0;

          int commaIndex = cityParam.indexOf(',');
          if (commaIndex > 0) {
            latitude = cityParam.substring(0, commaIndex).toFloat();
            longitude = cityParam.substring(commaIndex + 1).toFloat();
          } else {
            Serial.println("[WEATHER-OPENMETEO] Error: Se requieren coordenadas lat,lon");
            addSerialLog("[WEATHER-OPENMETEO] Error: formato lat,lon requerido");
            vTaskDelay(pdMS_TO_TICKS(60000));
            continue;
          }

          // URL completa con todos los parámetros útiles para riego
          url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(latitude, 4) + "&longitude=" + String(longitude, 4) + "&hourly=precipitation,temperature_2m,relative_humidity_2m,et0_fao_evapotranspiration,soil_temperature_0cm,soil_moisture_0_1cm,soil_moisture_9_27cm,shortwave_radiation,uv_index,windspeed_10m,apparent_temperature,vapour_pressure_deficit,surface_pressure" + "&daily=et0_fao_evapotranspiration" + "&timezone=auto&forecast_hours=" + String(config.weatherRainThresholdHours);

          Serial.println("[WEATHER] Open-Meteo URL completa: " + url);
          Serial.println("[WEATHER] Obteniendo datos avanzados + ET0...");

          http.begin(client, url);
          http.setTimeout(20000);  // Timeout mayor para más datos

          int httpCode = http.GET();

          if (httpCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<8192> doc;  // Documento más grande para más datos
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
              bool rainFound = false;
              float maxRain = 0.0;

              // Analizar precipitación (lógica existente)
              JsonArray precipitationArray = doc["hourly"]["precipitation"];
              int hoursToCheck = min((int)precipitationArray.size(), (int)config.weatherRainThresholdHours);

              for (int i = 0; i < hoursToCheck; i++) {
                float precipMm = precipitationArray[i];
                if (precipMm > 0.1) {
                  rainFound = true;
                  maxRain = max(maxRain, precipMm);
                }
              }

              // =============================
              // EXTRAER DATOS AVANZADOS (HORA ACTUAL)
              // =============================
              WeatherData weatherData;

              if (doc["hourly"]["surface_pressure"].size() > 0) {
                weatherData.surface_pressure = doc["hourly"]["surface_pressure"][0];
              }

              if (doc["hourly"]["et0_fao_evapotranspiration"].size() > 0) {
                weatherData.et0_hourly = doc["hourly"]["et0_fao_evapotranspiration"][0];  // Hora actual
              }

              if (doc["daily"]["et0_fao_evapotranspiration"].size() > 0) {
                weatherData.et0_daily = doc["daily"]["et0_fao_evapotranspiration"][0];  // Día actual
              }

              if (doc["hourly"]["soil_temperature_0cm"].size() > 0) {
                weatherData.soil_temperature = doc["hourly"]["soil_temperature_0cm"][0];
              }

              if (doc["hourly"]["soil_moisture_0_1cm"].size() > 0) {
                weatherData.soil_moisture_surface = doc["hourly"]["soil_moisture_0_1cm"][0];
              }

              if (doc["hourly"]["soil_moisture_9_27cm"].size() > 0) {
                weatherData.soil_moisture_9cm = doc["hourly"]["soil_moisture_9_27cm"][0];
              }

              if (doc["hourly"]["shortwave_radiation"].size() > 0) {
                weatherData.solar_radiation = doc["hourly"]["shortwave_radiation"][0];
              }

              if (doc["hourly"]["uv_index"].size() > 0) {
                weatherData.uv_index = doc["hourly"]["uv_index"][0];
              }

              if (doc["hourly"]["windspeed_10m"].size() > 0) {
                weatherData.wind_speed = doc["hourly"]["windspeed_10m"][0];
              }

              if (doc["hourly"]["apparent_temperature"].size() > 0) {
                weatherData.apparent_temperature = doc["hourly"]["apparent_temperature"][0];
              }

              if (doc["hourly"]["vapour_pressure_deficit"].size() > 0) {
                weatherData.vapor_pressure_deficit = doc["hourly"]["vapour_pressure_deficit"][0];
              }

              weatherData.last_update = (unsigned long)time(nullptr);
              weatherData.data_valid = true;

              requestSuccess = true;

              // Actualizar configuración y datos
              if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
                config.rainExpected = rainFound;
                config.lastWeatherCheck = weatherData.last_update;
                config.maxRainExpected = maxRain;
                config.weatherLastStatus = rainFound ? 2 : 1;
                xSemaphoreGive(mutexConfig);
              }

              // Actualizar datos del sensor
              if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {
                data.weather = weatherData;
                xSemaphoreGive(mutexData);
              }

              String log = "[WEATHER-OPENMETEO] Lluvia: " + String(rainFound ? "SÍ" : "NO");
              if (rainFound) log += " (" + String(maxRain, 1) + "mm)";
              log += " | ET0: " + String(weatherData.et0_daily, 1) + "mm/día";
              log += " | VPD: " + String(weatherData.vapor_pressure_deficit, 1) + "kPa";
              log += " | Viento: " + String(weatherData.wind_speed, 1) + "km/h";

              Serial.println(log);
              addSerialLog(log);

              if (rainFound && config.telegramToken.length() > 0) {
                String alert = "🌧️ *Pronóstico de lluvia* (Open-Meteo)\n";
                alert += "Lluvia esperada: " + String(maxRain, 1) + "mm\n";
                alert += "ET0 hoy: " + String(weatherData.et0_daily, 1) + "mm\n";
                alert += "Riego automático pausado";
                sendTelegramAlert("Pronóstico", alert);
              }
            } else {
              Serial.println("[WEATHER-OPENMETEO] Error parseando JSON");
              addSerialLog("[WEATHER-OPENMETEO] Error JSON");
            }
          } else {
            Serial.printf("[WEATHER-OPENMETEO] Error HTTP: %d\n", httpCode);
            addSerialLog("[WEATHER-OPENMETEO] Error consulta: " + String(httpCode));
          }
        }

        // =============================
        // OPENWEATHERMAP API (SOLO LLUVIA)
        // =============================
        else if (config.weatherProvider == "openweathermap") {
          int intervalsNeeded = min(40, (config.weatherRainThresholdHours + 2) / 3 + 1);
          url = "https://api.openweathermap.org/data/2.5/forecast?q=" + urlEncode(config.weatherCity) + "&appid=" + config.weatherApiKey + "&units=metric&lang=es&cnt=" + String(intervalsNeeded);

          Serial.println("[WEATHER] OpenWeatherMap URL: " + url);
          Serial.println("[WEATHER] Analizando próximas " + String(config.weatherRainThresholdHours) + " horas");

          http.begin(client, url);
          http.setTimeout(15000);
          int httpCode = http.GET();

          if (httpCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<8192> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
              bool rainFound = false;
              float maxRain = 0.0;

              JsonArray list = doc["list"];
              for (JsonObject item : list) {
                if (item.containsKey("rain") && item["rain"].containsKey("3h")) {
                  float rain3h = item["rain"]["3h"];
                  if (rain3h > 0.1) {
                    rainFound = true;
                    maxRain = max(maxRain, rain3h);
                  }
                }
                if (item.containsKey("snow") && item["snow"].containsKey("3h")) {
                  float snow3h = item["snow"]["3h"];
                  if (snow3h > 0.1) {
                    rainFound = true;
                    maxRain = max(maxRain, snow3h);
                  }
                }
              }

              requestSuccess = true;

              if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
                config.rainExpected = rainFound;
                config.lastWeatherCheck = (unsigned long)time(nullptr);
                config.maxRainExpected = maxRain;
                config.weatherLastStatus = rainFound ? 2 : 1;
                xSemaphoreGive(mutexConfig);
              }

              // Limpiar datos weather para otros proveedores
              if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {
                data.weather.data_valid = false;
                xSemaphoreGive(mutexData);
              }

              String log = "[WEATHER-OWM] Lluvia: " + String(rainFound ? "SÍ" : "NO");
              if (rainFound) log += " (" + String(maxRain, 1) + "mm)";
              Serial.println(log);
              addSerialLog(log);

              if (rainFound && config.telegramToken.length() > 0) {
                String alert = "🌧️ *Pronóstico de lluvia* (OpenWeatherMap)\n";
                alert += "Se esperan " + String(maxRain, 1) + "mm en las próximas ";
                alert += String(config.weatherRainThresholdHours) + " horas\n";
                alert += "El riego automático se pausará";
                sendTelegramAlert("Pronóstico", alert);
              }
            } else {
              Serial.println("[WEATHER-OWM] Error parseando JSON");
              addSerialLog("[WEATHER-OWM] Error JSON");
            }
          } else {
            Serial.printf("[WEATHER-OWM] Error HTTP: %d\n", httpCode);
            addSerialLog("[WEATHER-OWM] Error consulta: " + String(httpCode));
          }
        }

        // =============================
        // WEATHERAPI.COM API (SOLO LLUVIA)
        // =============================
        else if (config.weatherProvider == "weatherapi") {
          int daysNeeded = min(3, (config.weatherRainThresholdHours / 24) + 1);
          url = "https://api.weatherapi.com/v1/forecast.json?key=" + config.weatherApiKey + "&q=" + urlEncode(config.weatherCity) + "&days=" + String(daysNeeded) + "&aqi=no&alerts=no";

          Serial.println("[WEATHER] WeatherAPI URL: " + url);
          Serial.println("[WEATHER] Analizando próximas " + String(config.weatherRainThresholdHours) + " horas");

          http.begin(client, url);
          http.setTimeout(15000);
          int httpCode = http.GET();

          if (httpCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<3600> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
              bool rainFound = false;
              float maxRain = 0.0;

              unsigned long currentEpoch = time(nullptr);
              unsigned long limitEpoch = currentEpoch + (config.weatherRainThresholdHours * 3600);

              JsonArray forecastDays = doc["forecast"]["forecastday"];
              for (JsonObject day : forecastDays) {
                JsonArray hours = day["hour"];
                for (JsonObject hour : hours) {
                  String timeEpochStr = hour["time_epoch"];
                  unsigned long hourEpoch = timeEpochStr.toInt();

                  if (hourEpoch >= currentEpoch && hourEpoch <= limitEpoch) {
                    float precipMm = hour["precip_mm"];
                    if (precipMm > 0.1) {
                      rainFound = true;
                      maxRain = max(maxRain, precipMm);
                    }
                  }
                }
              }

              requestSuccess = true;

              if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
                config.rainExpected = rainFound;
                config.lastWeatherCheck = currentEpoch;
                config.maxRainExpected = maxRain;
                config.weatherLastStatus = rainFound ? 2 : 1;
                xSemaphoreGive(mutexConfig);
              }

              // Limpiar datos weather para otros proveedores
              if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {
                data.weather.data_valid = false;
                xSemaphoreGive(mutexData);
              }

              String log = "[WEATHER-WAPI] Lluvia: " + String(rainFound ? "SÍ" : "NO");
              if (rainFound) log += " (" + String(maxRain, 1) + "mm)";
              Serial.println(log);
              addSerialLog(log);

              if (rainFound && config.telegramToken.length() > 0) {
                String alert = "🌧️ *Pronóstico de lluvia* (WeatherAPI)\n";
                alert += "Se esperan " + String(maxRain, 1) + "mm en las próximas ";
                alert += String(config.weatherRainThresholdHours) + " horas\n";
                alert += "El riego automático se pausará";
                sendTelegramAlert("Pronóstico", alert);
              }
            } else {
              Serial.println("[WEATHER-WAPI] Error parseando JSON");
              addSerialLog("[WEATHER-WAPI] Error JSON");
            }
          } else {
            Serial.printf("[WEATHER-WAPI] Error HTTP: %d\n", httpCode);
            addSerialLog("[WEATHER-WAPI] Error consulta: " + String(httpCode));
          }
        }

        // Si la petición falló, actualizar estado
        if (!requestSuccess) {
          if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
            config.weatherLastStatus = 0;  // Sin datos
            config.rainExpected = false;
            xSemaphoreGive(mutexConfig);
          }
        }

        http.end();
        lastCheckTime = millis();
        Serial.println("[WEATHER] Próxima actualización completa en 2 horas");
      }
    } else {
      // No hay configuración válida
      if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.weatherLastStatus = -1;  // Desactivado
        config.rainExpected = false;
        xSemaphoreGive(mutexConfig);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10000));  // Esperar 10 segundos
  }
}


// ===========================
// FUNCIONES WIFI
// ===========================
bool connectToWifi() {
  if (ssid_stored.length() == 0) return false;

  String message = "[WIFI] Conectando a: " + ssid_stored;
  Serial.println(message);
  addSerialLog(message);

  WiFi.mode(WIFI_STA);
  if (!config.useDHCP) {
    IPAddress ip, gateway, subnet, dns1, dns2;

    if (ip.fromString(config.staticIP) && gateway.fromString(config.staticGateway) && subnet.fromString(config.staticSubnet)) {

      // DNS opcionales
      if (config.staticDNS1.length() > 0) {
        dns1.fromString(config.staticDNS1);
      } else {
        dns1 = gateway;  // Usar gateway como DNS por defecto
      }

      if (config.staticDNS2.length() > 0) {
        dns2.fromString(config.staticDNS2);
      } else {
        dns2 = dns1;
      }

      // Configurar ANTES de conectar
      WiFi.config(ip, gateway, subnet, dns1, dns2);
      Serial.println("[RED] IP estática configurada ANTES de conectar");
    }
  }

  WiFi.begin(ssid_stored.c_str(), pass_stored.c_str());

  Serial.println("[RED] Aplicando configuración de red...");
  if (!configureNetwork()) {
    Serial.println("[RED] ⚠️ Usando configuración por defecto (DHCP)");
  }

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    setLedStatus(0, 0, (attempts % 2 == 0) ? 255 : 50);
  }

  if (WiFi.status() == WL_CONNECTED) {
    message = "[WIFI] Conectado OK - IP: " + WiFi.localIP().toString();
    Serial.println("\n" + message);
    addSerialLog(message);

    Serial.println("[RED] 📊 Información de red:");
    Serial.println("[RED]   - IP: " + WiFi.localIP().toString());
    Serial.println("[RED]   - Gateway: " + WiFi.gatewayIP().toString());
    Serial.println("[RED]   - Subnet: " + WiFi.subnetMask().toString());
    Serial.println("[RED]   - DNS: " + WiFi.dnsIP().toString());

    addSerialLog(message);
  // 🔥 Configurar mDNS después de conectar
    delay(500);
    setupmDNS();
    return true;
  } else {
    message = "[WIFI] ❌ FALLO conexión";
    Serial.println("\n" + message);
    addSerialLog(message);
    return false;
  }
}

void startConfigPortal() {
  Serial.println("[WIFI] Iniciando portal de configuración...");
  addSerialLog("[WIFI] 📡 Iniciando modo configuración");
  wifiConfigMode = true;
  wifiConnected = false;

  // Apagar WiFi anterior completamente
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);

  // Pequeña pausa para asegurar que el WiFi anterior se apague
  delay(1000);

  // Crear AP
  if (WiFi.softAP("RiegoIoT-Access", "12345678")) {
    Serial.println("[WIFI] AP creado exitosamente");

    // Configurar IP estática
    IPAddress localIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(localIP, gateway, subnet);

    // Iniciar servidores
    dnsServer.start(53, "*", WiFi.softAPIP());

    configServer.on("/", handleConfigPortal);
    configServer.on("/save", handleSaveConfig);
    configServer.on("/scan", handleWifiScan);
    configServer.onNotFound(handleConfigPortal);
    configServer.begin();

    Serial.println("========================================");
    Serial.println("MODO CONFIGURACIÓN ACTIVADO");
    Serial.println("Conéctate a la red: RiegoIoT-Access");
    Serial.println("Contraseña: 12345678");
    Serial.println("Ve a: http://192.168.4.1");
    Serial.println("========================================");

    addSerialLog("[WIFI] 🔗 Conéctate a: RiegoIoT-Access");
    addSerialLog("[WIFI] 🌐 Ve a: http://192.168.4.1");

  } else {
    Serial.println("[WIFI] ERROR: No se pudo crear el AP");
    addSerialLog("[WIFI] ❌ Error creando AP");
    // Intentar reiniciar después de un error crítico
    delay(3000);
    ESP.restart();
  }
}

void handleWifiScan() {
  String html = "";
  int n = WiFi.scanNetworks();

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      String signalBars = "";
      int rssi = WiFi.RSSI(i);

      if (rssi > -55) signalBars = "|||||";
      else if (rssi > -65) signalBars = "||||";
      else if (rssi > -75) signalBars = "|||";
      else if (rssi > -85) signalBars = "||";
      else signalBars = "|";

      String security = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "<span style='color:#10b981'><i class='fas fa-unlock'></i></span>" : "<span style='color:#ef4444'><i class='fas fa-lock'></i></span>";

      html += "<div class='network-item' onclick='selectNetwork(\"" + WiFi.SSID(i) + "\")'>";
      html += "<i class='fas fa-wifi'></i> " + WiFi.SSID(i);
      html += "<span class='signal-strength'>" + signalBars + " " + security + "</span>";
      html += "</div>";
    }
  } else {
    html = "<div class='loading'>No se encontraron redes WiFi</div>";
  }

  configServer.send(200, "text/html", html);
}

void handleSaveConfig() {
  String ssid = configServer.arg("ssid");
  String password = configServer.arg("password");

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='10;url=http://192.168.4.1'>";
  html += "<style>body{font-family:Arial;text-align:center;padding:50px;background:#0f172a;color:white;}</style>";
  html += "</head><body>";
  html += "<div style='background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #10b981;'>";

  if (ssid.length() > 0) {
    // Guardar credenciales WiFi
    saveWifiConfig(ssid, password);

    html += "<h2 style='color:#10b981;'><i class='fas fa-check-circle'></i> Configuración Guardada</h2>";
    html += "<p>Red WiFi: <strong>" + ssid + "</strong></p>";
    html += "<p>Reiniciando y conectando...</p>";
    html += "<p>Si la conexión es exitosa, accede desde tu red local.</p>";
    html += "<p>Pulsa en el enlace <a href='http://miriego.local'>miriego.local<a> para redirigirte.</p>";

    configServer.send(200, "text/html", html);

    // Pequeña pausa para que se envíe la respuesta
    delay(2000);

    // Limpiar servidores del modo AP
    configServer.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);

    // Reiniciar el ESP32 para aplicar la nueva configuración
    ESP.restart();

  } else {
    html += "<h2 style='color:#ef4444;'><i class='fas fa-exclamation-triangle'></i> Error</h2>";
    html += "<p>El nombre de la red WiFi no puede estar vacío</p>";
    html += "<p>Redirigiendo...</p>";
    html += "</div></body></html>";

    configServer.send(200, "text/html", html);
  }
}

// ============================
// FUNCIÓN MODO PRUEBA BOMBAS
// ============================
void handleTestPumps() {
  Serial.println("[TEST] ⚡ Iniciando prueba de bombas por 5 segundos");
  addSerialLog("[TEST] 🔧 Prueba de bombas iniciada");

  // FEEDBACK VISUAL INMEDIATO - LED AMARILLO
  setLedStatus(255, 255, 0);  // Amarillo para modo prueba

  // Enviar página de confirmación inmediatamente
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='7;url=/'>";
  html += "<style>body{font-family:Arial;text-align:center;padding:50px;background:#0f172a;color:white;}</style>";
  html += "</head><body>";
  html += "<div style='background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #f59e0b;'>";
  html += "<h2 style='color:#f59e0b;'><i class='fas fa-vial'></i> Modo Prueba Activado</h2>";
  html += "<p style='font-size:1.1rem;'>Ambas bombas funcionarán durante <strong style='color:#f59e0b;'>5 segundos</strong></p>";
  html += "<div style='background:#dc2626;color:white;padding:15px;border-radius:10px;margin:20px 0;font-weight:bold;'>";
  html += "<i class='fas fa-exclamation-triangle'></i> ¡PRECAUCIÓN! Verifica el sistema antes de proceder";
  html += "</div>";
  html += "<p>Redirigiendo a la página principal en 7 segundos...</p>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);

  // Activar bombas (en background después de enviar la respuesta)
  controlPump(1, true);
  controlPump(2, true);

  // FEEDBACK VISUAL DURANTE PRUEBA - LED NARANJA PARPADEANTE
  unsigned long testStart = millis();
  unsigned long lastBlink = 0;
  bool ledState = true;

  while (millis() - testStart < 5000) {
    // Parpadeo naranja durante la prueba
    if (millis() - lastBlink > 500) {
      if (ledState) {
        setLedStatus(255, 165, 0);  // Naranja
      } else {
        setLedStatus(255, 100, 0);  // Naranja más oscuro
      }
      ledState = !ledState;
      lastBlink = millis();
    }
    delay(100);
  }

  // Apagar bombas y restaurar LED según estado del sistema
  controlPump(1, false);
  controlPump(2, false);

  // RESTAURAR LED SEGÚN ESTADO DEL SISTEMA
  if (wifiConnected) {
    setLedStatus(0, 255, 0);  // Verde si hay WiFi
  } else {
    setLedStatus(255, 165, 0);  // Naranja si no hay WiFi
  }

  Serial.println("[TEST] ✅ Prueba completada - Bombas apagadas");
  addSerialLog("[TEST] ✅ Prueba de bombas completada");

  if (wifiConnected && config.telegramToken.length() > 0) {
    sendTelegramAlert("Modo Prueba", "✅ Prueba de bombas completada correctamente");
  }
}

//=================================
// Función HandleRoot
//=================================
void handleRoot() {
  Serial.println("[WEB] Petición recibida - VERSIÓN OPTIMIZADA");

  // ===========================
  // PROTECCIÓN DE AUTENTICACIÓN WWW
  // ===========================
  if (config.authEnabled && !webAuthenticated) {
    // No autenticado - redirigir a login
    server.sendHeader("Location", "/login");
    server.send(303);
    return;
  }

  // Variables para datos del sistema (igual que tu función original)
  float temp = 0, hum = 0, soil1 = 0, soil2 = 0, light = 0;
  bool pump1 = false, pump2 = false;
  float offsetTemp = 0, offsetHum = 0, offsetSoil1 = 0, offsetSoil2 = 0, offsetLight = 0;
  int threshold1 = 30, threshold2 = 30, lightThreshold = 70;
  int aiThreshold1 = 25, aiThreshold2 = 25;
  uint8_t irrigationMode = 1;
  bool lightProtection = true, lightProtectionActive = false;
  bool waterSupplyControl = true;
  bool tempProtection = true, tempProtectionActive = false;
  float minTempThreshold = 5.0, maxTempThreshold = 35.0;
  bool humidityProtection = true, humidityProtectionActive = false;
  float humidityThreshold = 80.0;
  bool dataSuccess = false;
  int8_t gmtOffset = 1;
  bool daylightSaving = true;
  bool timeFormat24h = true;
  String weatherProvider = "openweathermap";
  String weatherApiKey = "";
  String weatherCity = "";
  uint8_t weatherRainThresholdHours = 4;
  bool weatherGuard = true;
  String telegramToken = config.telegramToken;
  String chatID = config.chatID;
  bool vpdEnabled = true;
  bool vpdProtection = true;
  float vpdValue = 0;


  // Obtener datos del sistema usando semáforos
  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(500)) == pdTRUE) {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      // Datos de sensores
      temp = data.temp;
      hum = data.hum;
      soil1 = data.soil1;
      soil2 = data.soil2;
      light = data.light;
      pump1 = data.pump1;
      pump2 = data.pump2;

      // Datos de configuración
      threshold1 = config.threshold1;
      threshold2 = config.threshold2;
      irrigationMode = config.irrigationMode;
      aiThreshold1 = config.aiThreshold1;
      aiThreshold2 = config.aiThreshold2;
      offsetTemp = config.offsetTemp;
      offsetHum = config.offsetHum;
      offsetSoil1 = config.offsetSoil1;
      offsetSoil2 = config.offsetSoil2;
      offsetLight = config.offsetLight;
      lightProtection = config.lightProtection;
      lightThreshold = config.lightThreshold;
      lightProtectionActive = config.lightProtectionActive;
      waterSupplyControl = config.waterSupplyControl;
      tempProtection = config.tempProtection;
      minTempThreshold = config.minTempThreshold;
      maxTempThreshold = config.maxTempThreshold;
      tempProtectionActive = config.tempProtectionActive;
      humidityProtection = config.humidityProtection;
      humidityThreshold = config.humidityThreshold;
      humidityProtectionActive = config.humidityProtectionActive;
      dataSuccess = true;
      gmtOffset = config.gmtOffset;
      daylightSaving = config.daylightSaving;
      timeFormat24h = config.timeFormat24h;
      weatherProvider = config.weatherProvider;
      weatherApiKey = config.weatherApiKey;
      weatherCity = config.weatherCity;
      weatherRainThresholdHours = config.weatherRainThresholdHours;
      weatherGuard = config.weatherGuard;
      telegramToken = config.telegramToken;
      chatID = config.chatID;
      vpdEnabled = config.vpdEnabled;
      vpdProtection = config.vpdProtection;
      vpdValue = data.vpd;

      xSemaphoreGive(mutexConfig);
    }
    xSemaphoreGive(mutexData);
  }


  // Medición de memoria ANTES de construir la página
  int memBefore = ESP.getFreeHeap();

  // Construir página HTML usando plantillas PROGMEM optimizadas
  String html;
  html.reserve(15000);  // Pre-reservar memoria para evitar fragmentación

  // ============================
  // 1. HEADER CON DATOS DINÁMICOS
  // ============================
  String headerHtml = FPSTR(HTML_HEADER);
  headerHtml.replace("{WIFI_STATUS}", wifiConnected ? "🌐 ONLINE" : "📴 OFFLINE");
  headerHtml.replace("{CURRENT_TIME}", getFormattedTime());
  html += headerHtml;

  // ============================
  // 2. TARJETA DE SENSORES
  // ============================
  String sensorsHtml = FPSTR(HTML_SENSORS_CARD);

  float offsetTemp_val = 0, offsetHum_val = 0, offsetSoil1_val = 0, offsetSoil2_val = 0, offsetLight_val = 0, offsetPressure_val = 0, offsetAirQuality_val = 0;
  String soilProfile_val = "universal";

  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(100)) == pdTRUE) {
    offsetTemp_val = config.offsetTemp;
    offsetHum_val = config.offsetHum;
    offsetPressure_val = config.offsetPressure;
    offsetAirQuality_val = config.offsetAirQuality;
    offsetSoil1_val = config.offsetSoil1;
    offsetSoil2_val = config.offsetSoil2;
    offsetLight_val = config.offsetLight;
    soilProfile_val = config.soilProfile;
    xSemaphoreGive(mutexConfig);
  }

  // Valores calibrados
  sensorsHtml.replace("{TEMP_VALUE}", String(temp, 1));
  sensorsHtml.replace("{HUM_VALUE}", String(hum, 1));
  sensorsHtml.replace("{SOIL1_VALUE}", String(soil1, 1));
  sensorsHtml.replace("{SOIL2_VALUE}", String(soil2, 1));
  sensorsHtml.replace("{LIGHT_VALUE}", String(light, 1));
  sensorsHtml.replace("{PRESSURE_VALUE}", String(data.pressure, 1));
  sensorsHtml.replace("{AIRQUALITY_VALUE}", String(data.airQuality, 1));
  sensorsHtml.replace("{VOC_QUALITY}", getAirQualityText(data.airQuality));
  sensorsHtml.replace("{VOC_COLOR}", getAirQualityColor(data.airQuality));
  AmbientState ambient = getAmbientState(vpdValue, hum, temp, soil1, soil2);
  sensorsHtml.replace("{AMBIENT_ICON}", ambient.icon);
  sensorsHtml.replace("{AMBIENT_TEXT}", ambient.text);
  sensorsHtml.replace("{AMBIENT_DETAIL}", ambient.detail);
  sensorsHtml.replace("{AMBIENT_GRADIENT}", ambient.gradient);

  // Estados del sistema
  sensorsHtml.replace("{WATER_CLASS}", data.waterSupply ? "" : "no-water");
  sensorsHtml.replace("{WATER_ICON_CLASS}", data.waterSupply ? "" : "no-water");
  sensorsHtml.replace("{WATER_STATUS}", data.waterSupply ? "OK" : "FALLO");
  sensorsHtml.replace("{VPD_DISPLAY}", vpdEnabled ? "block" : "none");
  sensorsHtml.replace("{VPD_VALUE}", String(vpdValue, 2));
  sensorsHtml.replace("{VPD_COLOR}", getVPDColor(vpdValue));
  sensorsHtml.replace("{VPD_QUALITY}", getVPDQuality(vpdValue));


  html += sensorsHtml;

  // ============================
  // 3. DATOS METEOROLÓGICOS AVANZADOS
  // ============================
  if (config.weatherProvider == "openmeteo" && data.weather.data_valid) {
    String weatherHtml = FPSTR(HTML_WEATHER_CARD);

    // Reemplazar placeholders
    weatherHtml.replace("{ET0_DAILY}", String(data.weather.et0_daily, 1));
    weatherHtml.replace("{ET0_HOURLY}", String(data.weather.et0_hourly, 2));
    weatherHtml.replace("{SOIL_TEMP}", String(data.weather.soil_temperature, 1));
    weatherHtml.replace("{APPARENT_TEMP}", String(data.weather.apparent_temperature, 1));
    weatherHtml.replace("{SOLAR_RADIATION}", String(data.weather.solar_radiation, 0));
    weatherHtml.replace("{UV_INDEX}", String(data.weather.uv_index, 1));

    // Determinar nivel UV
    String uvLevel = "Bajo";
    if (data.weather.uv_index >= 8) uvLevel = "Muy alto";
    else if (data.weather.uv_index >= 6) uvLevel = "Alto";
    else if (data.weather.uv_index >= 3) uvLevel = "Moderado";
    weatherHtml.replace("{UV_LEVEL}", uvLevel);
    weatherHtml.replace("{SOIL_MOISTURE_9CM}", String(data.weather.soil_moisture_9cm, 3));
    weatherHtml.replace("{WIND_SPEED}", String(data.weather.wind_speed, 1));
    weatherHtml.replace("{WEATHER_UPDATE_TIME}", getFormattedTimeFromEpoch(data.weather.last_update));

    html += weatherHtml;
  }

    // ============================
    // 4. TARJETA DE CONTROL y ESTADO
    // ============================
    bool tempBlocking = tempProtection && (data.temp < minTempThreshold || data.temp > maxTempThreshold);
    bool humBlocking = humidityProtection && (data.hum > humidityThreshold);
    bool vpdBlocking = vpdProtection && isVPDBlocking(data.vpd);
    bool timeBlocking = config.timeRestrictionEnabled && isTimeRestricted();
    bool windBlocking = config.windRestrictionEnabled && isWindRestricted();
    bool systemOk = !tempBlocking && !humBlocking && !vpdBlocking && !timeBlocking && !windBlocking && data.waterSupply;


    String controlHtml = FPSTR(HTML_CONTROL_CARD);

    // Modo Automático
    controlHtml.replace("{MANUAL_CLASS}", config.irrigationMode == 0 ? "btn-toggle-auto-on" : "btn-toggle-auto-off");
    controlHtml.replace("{AUTO_CLASS}", config.irrigationMode == 1 ? "btn-toggle-auto-on" : "btn-toggle-auto-off");
    controlHtml.replace("{IA_CLASS}", config.irrigationMode == 2 ? "btn-toggle-auto-on" : "btn-toggle-auto-off");
    controlHtml.replace("{MANUAL_ICON}", config.irrigationMode == 0 ? "✅" : "");
    controlHtml.replace("{AUTO_ICON}", config.irrigationMode == 1 ? "✅" : "");
    controlHtml.replace("{IA_ICON}", config.irrigationMode == 2 ? "✅" : "");

    // Protección Solar
    controlHtml.replace("{LIGHT_PROTECTION_URL}", lightProtection ? "/lightprotectionoff" : "/lightprotectionon");
    controlHtml.replace("{LIGHT_PROTECTION_CLASS}", lightProtection ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{LIGHT_PROTECTION_STATUS}", lightProtection ? "ACTIVA ✅" : "INACTIVA ❌");

    // Protección Temperatura
    controlHtml.replace("{TEMP_PROTECTION_URL}", tempProtection ? "/tempprotectionoff" : "/tempprotectionon");
    controlHtml.replace("{TEMP_PROTECTION_CLASS}", tempProtection ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{TEMP_PROTECTION_STATUS}", tempProtection ? "ACTIVA ✅" : "INACTIVA ❌");

    // Protección Humedad
    controlHtml.replace("{HUMIDITY_PROTECTION_URL}", humidityProtection ? "/humidityprotectionoff" : "/humidityprotectionon");
    controlHtml.replace("{HUMIDITY_PROTECTION_CLASS}", humidityProtection ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{HUMIDITY_PROTECTION_STATUS}", humidityProtection ? "ACTIVA ✅" : "INACTIVA ❌");

    // Control Suministro Agua
    controlHtml.replace("{WATER_SUPPLY_URL}", waterSupplyControl ? "/watersupplyoff" : "/watersupplyon");
    controlHtml.replace("{WATER_SUPPLY_CLASS}", waterSupplyControl ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{WATER_SUPPLY_STATUS}", waterSupplyControl ? "ACTIVO ✅" : "INACTIVO ❌");

    // Protección VPD
    controlHtml.replace("{VPD_PROTECTION_URL}", vpdProtection ? "/vpdprotectionoff" : "/vpdprotectionon");
    controlHtml.replace("{VPD_PROTECTION_CLASS}", vpdProtection ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{VPD_PROTECTION_STATUS}", vpdProtection ? "ACTIVA ✅" : "INACTIVA ❌");

    // Estados de bombas
    controlHtml.replace("{PUMP1_STATUS_CLASS}", pump1 ? "status-on pump-active" : "status-off");
    controlHtml.replace("{PUMP1_STATUS_TEXT}", pump1 ? "ACTIVA 🔥" : "INACTIVA");
    controlHtml.replace("{PUMP2_STATUS_CLASS}", pump2 ? "status-on pump-active" : "status-off");
    controlHtml.replace("{PUMP2_STATUS_TEXT}", pump2 ? "ACTIVA 🔥" : "INACTIVA");

    // Estado del sistema
    controlHtml.replace("{MANUAL_CLASS}", config.irrigationMode == 0 ? "btn-toggle-auto-on" : "btn-toggle-auto-off");
    controlHtml.replace("{AUTO_CLASS}", config.irrigationMode == 1 ? "btn-toggle-auto-on" : "btn-toggle-auto-off");
    controlHtml.replace("{IA_CLASS}", config.irrigationMode == 2 ? "btn-toggle-auto-on" : "btn-toggle-auto-off");
    controlHtml.replace("{MANUAL_ICON}", config.irrigationMode == 0 ? "✅" : "");
    controlHtml.replace("{AUTO_ICON}", config.irrigationMode == 1 ? "✅" : "");
    controlHtml.replace("{IA_ICON}", config.irrigationMode == 2 ? "✅" : "");

    // Bloqueos
    controlHtml.replace("{TEMP_BLOCK_DISPLAY}", tempBlocking ? "flex" : "none");
    controlHtml.replace("{TEMP_BLOCK_TEXT}", tempBlocking ? "ACTIVO ❄️/🔥" : "");
    controlHtml.replace("{HUM_BLOCK_DISPLAY}", humBlocking ? "flex" : "none");
    controlHtml.replace("{HUM_BLOCK_TEXT}", humBlocking ? "ACTIVO 💦" : "");
    controlHtml.replace("{VPD_BLOCK_DISPLAY}", vpdBlocking ? "flex" : "none");
    controlHtml.replace("{VPD_BLOCK_TEXT}", vpdBlocking ? "ACTIVO 🌿" : "");
    controlHtml.replace("{TIME_BLOCK_DISPLAY}", timeBlocking ? "flex" : "none");
    controlHtml.replace("{TIME_BLOCK_TEXT}", timeBlocking ? "ACTIVO ⏰" : "");
    controlHtml.replace("{WIND_BLOCK_DISPLAY}", windBlocking ? "flex" : "none");
    controlHtml.replace("{WIND_BLOCK_TEXT}", windBlocking ? "ACTIVO 💨" : "");

    // Estados de protecciones
    controlHtml.replace("{LIGHT_PROT_CLASS}", lightProtection ? "status-on" : "status-off");
    controlHtml.replace("{LIGHT_PROT_TEXT}", lightProtection ? "ACTIVA" : "INACTIVA");
    controlHtml.replace("{TEMP_PROT_CLASS}", tempProtection ? "status-on" : "status-off");
    controlHtml.replace("{TEMP_PROT_TEXT}", tempProtection ? "ACTIVA" : "INACTIVA");
    controlHtml.replace("{HUM_PROT_CLASS}", humidityProtection ? "status-on" : "status-off");
    controlHtml.replace("{HUM_PROT_TEXT}", humidityProtection ? "ACTIVA" : "INACTIVA");
    controlHtml.replace("{VPD_PROT_CLASS}", vpdProtection ? "status-on" : "status-off");
    controlHtml.replace("{VPD_PROT_TEXT}", vpdProtection ? "ACTIVA" : "INACTIVA");
    controlHtml.replace("{WATER_CTRL_CLASS}", waterSupplyControl ? "status-on" : "status-off");
    controlHtml.replace("{WATER_CTRL_TEXT}", waterSupplyControl ? "ACTIVO" : "INACTIVO");
    controlHtml.replace("{TIME_RESTRICTION_STATUS_CLASS}", config.timeRestrictionEnabled ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{TIME_RESTRICTION_STATUS_TEXT}", config.timeRestrictionEnabled ? "ACTIVO ✅" : "INACTIVO ❌");
    controlHtml.replace("{WIND_RESTRICTION_STATUS_CLASS}", config.windRestrictionEnabled ? "btn-toggle-on" : "btn-toggle-off");
    controlHtml.replace("{WIND_RESTRICTION_STATUS_TEXT}", config.windRestrictionEnabled ? "ACTIVO ✅" : "INACTIVO ❌");

    // URLs para los nuevos botones
    controlHtml.replace("{TIME_RESTRICTION_URL}", config.timeRestrictionEnabled ? "/timerestrictionoff" : "/timerestrictionon");
    controlHtml.replace("{WIND_RESTRICTION_URL}", config.windRestrictionEnabled ? "/windrestrictionoff" : "/windrestrictionon");

    // Estado general del sistema
    controlHtml.replace("{SYSTEM_STATUS_CLASS}", systemOk ? "status-on" : "status-off");
    controlHtml.replace("{SYSTEM_STATUS_TEXT}", systemOk ? "✅ OPTIMO" : "⚠️ CON RESTRICCIONES");

    // Detalles de configuración
    controlHtml.replace("{THRESHOLD1}", String(threshold1));
    controlHtml.replace("{THRESHOLD2}", String(threshold2));
    controlHtml.replace("{MIN_TEMP}", String(minTempThreshold, 1));
    controlHtml.replace("{MAX_TEMP}", String(maxTempThreshold, 1));
    controlHtml.replace("{HUM_THRESHOLD}", String(humidityThreshold, 0));
    controlHtml.replace("{LIGHT_THRESHOLD}", String(lightThreshold));


    html += controlHtml;

  // ============================
  // 8. TARJETA DE CALIBRACIÓN (NUEVA)
  // ============================
  String calibrationHtml = FPSTR(HTML_CALIBRATION_CARD);

  // Reemplazar valores específicos de calibración
  calibrationHtml.replace("{SOIL1_RAW}", String(soil1 - offsetSoil1_val, 1));
  calibrationHtml.replace("{SOIL1_CAL}", String(soil1, 1));
  calibrationHtml.replace("{SOIL2_RAW}", String(soil2 - offsetSoil2_val, 1));
  calibrationHtml.replace("{SOIL2_CAL}", String(soil2, 1));
  calibrationHtml.replace("{SOIL_PROFILE}", getSubstrateDisplayName(soilProfile_val));
  calibrationHtml.replace("{CALIBRATION_FACTOR}", String(getAutoCalibrationFactor(soilProfile_val), 2));
  calibrationHtml.replace("{OFFSET_SOIL1}", String(offsetSoil1_val, 1));
  calibrationHtml.replace("{OFFSET_SOIL2}", String(offsetSoil2_val, 1));
  calibrationHtml.replace("{OFFSET_TEMP}", String(offsetTemp_val, 1));
  calibrationHtml.replace("{OFFSET_HUM}", String(offsetHum_val, 1));
  calibrationHtml.replace("{OFFSET_LIGHT}", String(offsetLight_val, 1));
  calibrationHtml.replace("{OFFSET_PRESSURE}", String(offsetPressure_val, 1));
  calibrationHtml.replace("{OFFSET_AIRQUALITY}", String(offsetAirQuality_val, 1));
  int inferenceHighPercent = (int)(config.inferenceThresholdHigh * 100);
  int inferenceLowPercent = (int)(config.inferenceThresholdLow * 100);
    calibrationHtml.replace("{INFERENCE_HIGH_VAL}", String(inferenceHighPercent));
  calibrationHtml.replace("{INFERENCE_LOW_VAL}", String(inferenceLowPercent));


  html += calibrationHtml;

  // ============================
  // 7. TARJETA DE AJUSTES
  // ============================
  String settingsHtml = FPSTR(HTML_SETTINGS_CARD);

  // Reemplazar valores de umbrales
  settingsHtml.replace("{THRESHOLD1_VAL}", String(threshold1));
  settingsHtml.replace("{THRESHOLD2_VAL}", String(threshold2));
  settingsHtml.replace("{LIGHT_THRESHOLD_VAL}", String(lightThreshold));
  settingsHtml.replace("{MIN_TEMP_VAL}", String(minTempThreshold, 1));
  settingsHtml.replace("{MAX_TEMP_VAL}", String(maxTempThreshold, 1));
  settingsHtml.replace("{HUM_THRESHOLD_VAL}", String(humidityThreshold, 0));

  // Valores de calibración
  settingsHtml.replace("{OFFSET_TEMP}", String(offsetTemp, 1));
  settingsHtml.replace("{OFFSET_HUM}", String(offsetHum, 1));
  settingsHtml.replace("{OFFSET_SOIL1}", String(offsetSoil1, 1));
  settingsHtml.replace("{OFFSET_SOIL2}", String(offsetSoil2, 1));
  settingsHtml.replace("{OFFSET_LIGHT}", String(offsetLight, 1));
  settingsHtml.replace("{OFFSET_PRESSURE}", String(config.offsetPressure, 1));
  settingsHtml.replace("{OFFSET_AIRQUALITY}", String(config.offsetAirQuality, 1));

  // Valores VPD
  settingsHtml.replace("{VPD_PROT_SELECTED_YES}", vpdProtection ? "selected" : "");
  settingsHtml.replace("{VPD_PROT_SELECTED_NO}", !vpdProtection ? "selected" : "");
  settingsHtml.replace("{VPD_MIN_VAL}", String(config.vpdMinThreshold, 1));
  settingsHtml.replace("{VPD_MAX_VAL}", String(config.vpdMaxThreshold, 1));
  settingsHtml.replace("{VPD_CURRENT_VAL}", String(vpdValue, 2));
  settingsHtml.replace("{VPD_CURRENT_COLOR}", getVPDColor(vpdValue));
  settingsHtml.replace("{VPD_CURRENT_QUALITY}", getVPDQuality(vpdValue));

  // Sustrato
  settingsHtml.replace("{SOIL_UNIVERSAL_SELECTED}", config.soilProfile == "universal" ? "selected" : "");
  settingsHtml.replace("{SOIL_CLAY_SELECTED}", config.soilProfile == "clay" ? "selected" : "");
  settingsHtml.replace("{SOIL_SANDY_SELECTED}", config.soilProfile == "sandy" ? "selected" : "");
  // ... más opciones de sustrato ...
  settingsHtml.replace("{SUBSTRATE_COLOR}", getSubstrateColor(config.soilProfile));
  settingsHtml.replace("{SUBSTRATE_ICON}", getSubstrateIcon(config.soilProfile));
  settingsHtml.replace("{SUBSTRATE_NAME}", getSubstrateDisplayName(config.soilProfile));

  // Restricciones Normativas
  settingsHtml.replace("{TIME_RESTRICTION_SELECTED_YES}", config.timeRestrictionEnabled ? "selected" : "");
  settingsHtml.replace("{TIME_RESTRICTION_SELECTED_NO}", !config.timeRestrictionEnabled ? "selected" : "");
  settingsHtml.replace("{RESTRICTION_START_TIME}", config.restrictionStartTime);
  settingsHtml.replace("{RESTRICTION_END_TIME}", config.restrictionEndTime);

  settingsHtml.replace("{WIND_RESTRICTION_SELECTED_YES}", config.windRestrictionEnabled ? "selected" : "");
  settingsHtml.replace("{WIND_RESTRICTION_SELECTED_NO}", !config.windRestrictionEnabled ? "selected" : "");
  settingsHtml.replace("{MAX_WIND_SPEED}", String(config.maxWindSpeed, 1));
  settingsHtml.replace("{CURRENT_WIND_SPEED}", String(data.weather.wind_speed, 1));

  html += settingsHtml;

  // ============================
  // 8. TARJETA DE CONFIGURACIÓN AVANZADA
  // ============================
  String advConfigHtml = FPSTR(HTML_ADVANCED_CONFIG_CARD);

  // Configurar opciones GMT (necesitarás crear esta función)
  advConfigHtml.replace("{GMT_OPTIONS}", generateGMTOptions(gmtOffset));

  // Horario de verano
  advConfigHtml.replace("{DST_SELECTED_YES}", daylightSaving ? "selected" : "");
  advConfigHtml.replace("{DST_SELECTED_NO}", !daylightSaving ? "selected" : "");

  // Formato de hora
  advConfigHtml.replace("{TIME_FORMAT_24_SELECTED}", timeFormat24h ? "selected" : "");
  advConfigHtml.replace("{TIME_FORMAT_12_SELECTED}", !timeFormat24h ? "selected" : "");

  // Configuración meteorológica
  advConfigHtml.replace("{WEATHER_GUARD_SELECTED_YES}", weatherGuard ? "selected" : "");
  advConfigHtml.replace("{WEATHER_GUARD_SELECTED_NO}", !weatherGuard ? "selected" : "");

  advConfigHtml.replace("{WEATHER_OPENMETEO_SELECTED}", weatherProvider == "openmeteo" ? "selected" : "");
  advConfigHtml.replace("{WEATHER_OPENWEATHER_SELECTED}", weatherProvider == "openweathermap" ? "selected" : "");
  advConfigHtml.replace("{WEATHER_WEATHERAPI_SELECTED}", weatherProvider == "weatherapi" ? "selected" : "");

  advConfigHtml.replace("{WEATHER_CITY_VAL}", weatherCity);
  advConfigHtml.replace("{WEATHER_API_KEY_VAL}", weatherApiKey);
  advConfigHtml.replace("{WEATHER_THRESHOLD_HOURS}", String(weatherRainThresholdHours));

  // Estado meteorológico actual
  String weatherStatusText = "";
  String weatherStatusColor = "#6b7280";  // Gris por defecto

  if (config.weatherLastStatus == -1) {
    weatherStatusText = "DESACTIVADO";
    weatherStatusColor = "#6b7280";
  } else if (config.weatherLastStatus == 0) {
    weatherStatusText = "SIN DATOS";
    weatherStatusColor = "#f59e0b";
  } else if (config.weatherLastStatus == 1) {
    weatherStatusText = "SIN LLUVIA";
    weatherStatusColor = "#10b981";
  } else if (config.weatherLastStatus == 2) {
    weatherStatusText = "LLUVIA ESPERADA";
    weatherStatusColor = "#ef4444";
  }

  advConfigHtml.replace("{WEATHER_STATUS_TEXT}", weatherStatusText);
  advConfigHtml.replace("{WEATHER_STATUS_COLOR}", weatherStatusColor);
  advConfigHtml.replace("{WEATHER_LAST_UPDATE}", getFormattedTimeFromEpoch(config.lastWeatherCheck));
  advConfigHtml.replace("{WEATHER_MAX_RAIN}", String(config.maxRainExpected, 1));

  String weatherProviderName = "";
  if (config.weatherProvider == "openmeteo") {
    weatherProviderName = "Open-Meteo";
  } else if (config.weatherProvider == "openweathermap") {
    weatherProviderName = "OpenWeatherMap";
  } else if (config.weatherProvider == "weatherapi") {
    weatherProviderName = "WeatherAPI";
  } else {
    weatherProviderName = "No configurado";
  }

  advConfigHtml.replace("{WEATHER_PROVIDER_NAME}",
                        weatherProvider == "openmeteo" ? "Open-Meteo" : weatherProvider == "openweathermap" ? "OpenWeatherMap"
                                                                      : weatherProvider == "weatherapi"     ? "WeatherAPI"
                                                                                                            : "No configurado");


// Información RTC
  DateTime rtcNow = rtcAvailable ? rtc.now() : DateTime(2025, 1, 1, 12, 0, 0);
  
  // Estado fuente de tiempo
  String timeSourceStatus = "";
  if (currentTimeSource == TIME_SOURCE_NTP) {
    timeSourceStatus = "<span style='color:#10b981'><i class='fas fa-wifi'></i> NTP (Internet)</span>";
  } else if (currentTimeSource == TIME_SOURCE_RTC_DS3231) {
    timeSourceStatus = "<span style='color:#f59e0b'><i class='fas fa-clock'></i> RTC DS3231</span>";
  } else if (currentTimeSource == TIME_SOURCE_INTERNAL) {
    timeSourceStatus = "<span style='color:#ef4444'><i class='fas fa-exclamation-triangle'></i> Reloj Interno</span>";
  } else {
    timeSourceStatus = "<span style='color:#6b7280'>Sin configurar</span>";
  }
  advConfigHtml.replace("{TIME_SOURCE_STATUS}", timeSourceStatus);
  
  // Estado del RTC
  String rtcStatusInfo = "";
  if (rtcAvailable) {
    rtcStatusInfo += "<p style='font-size:0.85rem;color:#94a3b8;margin:5px 0'>";
    rtcStatusInfo += "RTC: <span style='color:#10b981'>Disponible</span> | ";
    rtcStatusInfo += "Hora: " + String(rtcNow.day()) + "/" + String(rtcNow.month()) + "/" + String(rtcNow.year());
    rtcStatusInfo += " " + String(rtcNow.hour()) + ":" + (rtcNow.minute() < 10 ? "0" : "") + String(rtcNow.minute());
    rtcStatusInfo += " | Sync: " + (lastRTCSync > 0 ? String((millis() - lastRTCSync) / 60000) + "min" : "Nunca");
    rtcStatusInfo += "</p>";
  } else {
    rtcStatusInfo += "<p style='font-size:0.85rem;color:#ef4444;margin:5px 0'>";
    rtcStatusInfo += "RTC: No detectado";
    rtcStatusInfo += "</p>";
  }
  advConfigHtml.replace("{RTC_STATUS_INFO}", rtcStatusInfo);
  
  // Valores formulario
  advConfigHtml.replace("{CURRENT_DAY}", String(rtcNow.day()));
  advConfigHtml.replace("{CURRENT_MONTH}", String(rtcNow.month()));
  advConfigHtml.replace("{CURRENT_YEAR}", String(rtcNow.year()));
  advConfigHtml.replace("{CURRENT_HOUR}", String(rtcNow.hour()));
  advConfigHtml.replace("{CURRENT_MINUTE}", String(rtcNow.minute()));

  // Telegram
  advConfigHtml.replace("{TELEGRAM_TOKEN_VAL}", telegramToken);
  advConfigHtml.replace("{TELEGRAM_CHATID_VAL}", chatID);

  // Reemplazos Pestaña Red
  advConfigHtml.replace("{DHCP_SELECTED}", config.useDHCP ? "selected" : "");
  advConfigHtml.replace("{STATIC_SELECTED}", !config.useDHCP ? "selected" : "");
  advConfigHtml.replace("{STATIC_IP_VAL}", config.staticIP);
  advConfigHtml.replace("{STATIC_GATEWAY_VAL}", config.staticGateway);
  advConfigHtml.replace("{STATIC_SUBNET_VAL}", config.staticSubnet);
  advConfigHtml.replace("{STATIC_DNS1_VAL}", config.staticDNS1);
  advConfigHtml.replace("{STATIC_DNS2_VAL}", config.staticDNS2);

  // Estado actual de red
  advConfigHtml.replace("{CURRENT_NET_MODE}", config.useDHCP ? "DHCP Automático" : "IP Estática");
  advConfigHtml.replace("{CURRENT_IP}", WiFi.localIP().toString());
  advConfigHtml.replace("{CURRENT_GATEWAY}", WiFi.gatewayIP().toString());
  advConfigHtml.replace("{CURRENT_SUBNET}", WiFi.subnetMask().toString());
  // // REEMPLAZOS AUTENTICACIÓN WWW
  advConfigHtml.replace("{AUTH_ENABLED_YES}", config.authEnabled ? "selected" : "");
  advConfigHtml.replace("{AUTH_ENABLED_NO}", !config.authEnabled ? "selected" : "");
  advConfigHtml.replace("{WEB_USER_VAL}", config.webUser);
  advConfigHtml.replace("{SESSION_TIMEOUT_VAL}", String(config.sessionTimeout));

  html += advConfigHtml;

  // ============================
  // 9. TARJETA DE SISTEMA
  // ============================
  String systemHtml = FPSTR(HTML_SYSTEM_CARD);

  systemHtml.replace("{WIFI_STATUS_CLASS}", wifiConnected ? "status-on" : "status-off");
  systemHtml.replace("{WIFI_STATUS_TEXT}", wifiConnected ? "CONECTADO" : "OFFLINE");
  systemHtml.replace("{WIFI_SSID}", wifiConnected ? WiFi.SSID() : "No conectado");
  systemHtml.replace("{WIFI_RSSI_VALUE}", String(wifiConnected ? WiFi.RSSI() : -100));
  systemHtml.replace("{LOCAL_IP}", wifiConnected ? WiFi.localIP().toString() : "No disponible");
  systemHtml.replace("{MAC_ADDRESS}", WiFi.macAddress());
    // ✅ CALCULAR UPTIME EN DÍAS, HORAS, MINUTOS
  unsigned long uptimeMs = millis();
  unsigned long days = uptimeMs / 86400000;
  unsigned long hours = (uptimeMs % 86400000) / 3600000;
  unsigned long minutes = (uptimeMs % 3600000) / 60000;

  String uptimeStr = "";
  if (days > 0) uptimeStr += String(days) + "d ";
  if (hours > 0) uptimeStr += String(hours) + "h ";
  uptimeStr += String(minutes) + "m";
  systemHtml.replace("{UPTIME}", uptimeStr);
  systemHtml.replace("{FREE_MEMORY}", String(ESP.getFreeHeap() / 1024));
  systemHtml.replace("{WEATHER_DISPLAY}", wifiConnected ? "block" : "none");
  systemHtml.replace("{RAIN_AMOUNT}", String(config.maxRainExpected, 2));
  systemHtml.replace("{FREE_MEMORY}", String(ESP.getFreeHeap() / 1024));
  

  html += systemHtml;

  // ============================
  // 10. FOOTER CON JAVASCRIPT
  // ============================
  String footerHtml = FPSTR(HTML_FOOTER);
  footerHtml.replace("{WIFI_RSSI}", String(wifiConnected ? WiFi.RSSI() : -100));
  html += footerHtml;

  // Medición de memoria DESPUÉS de construir la página
  int memAfter = ESP.getFreeHeap();

  // Log de rendimiento de la versión optimizada
  Serial.println("=== RENDIMIENTO OPTIMIZADO ===");
  Serial.printf("Memoria libre antes: %d bytes (%d KB)\n", memBefore, memBefore / 1024);
  Serial.printf("Memoria libre después: %d bytes (%d KB)\n", memAfter, memAfter / 1024);
  Serial.printf("Memoria usada para HTML: %d bytes (%d KB)\n", memBefore - memAfter, (memBefore - memAfter) / 1024);
  Serial.printf("Tamaño HTML generado: %d caracteres\n", html.length());
  Serial.printf("Eficiencia: %.1f%% menos memoria vs versión original\n",
                ((60.0 - (float)(memBefore - memAfter) / 1024) / 60.0) * 100.0);
  Serial.println("================================");

  // Enviar página optimizada al navegador
  html += FPSTR(HTML_LIVE_UPDATE_SCRIPT);
  server.send(200, "text/html", html);
}

// ===========================
// FUNCIONES DE RECONEXIÓN WIFI
// ===========================
void attemptWifiReconnection() {
  Serial.println("[WIFI-RECONNECT] Iniciando proceso de reconexión...");
  addSerialLog("[WIFI] 🔄 Intentando reconexión automática");

  // Intentar con credenciales almacenadas primero
  if (ssid_stored.length() > 0) {
    Serial.printf("[WIFI-RECONNECT] Conectando a: %s\n", ssid_stored.c_str());

    WiFi.disconnect();
    WiFi.begin(ssid_stored.c_str(), pass_stored.c_str());

    Serial.println("[RED-RECONNECT] Aplicando configuración de red...");
    configureNetwork();

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println("\n[WIFI-RECONNECT] ✓ Reconexión exitosa");
      addSerialLog("[WIFI] ✅ Reconectado a: " + ssid_stored);
      setLedStatus(0, 255, 0);  // Verde
      configureNTP();           // Reconfigurar NTP
      // 🔥 Reconfigurar mDNS
      setupmDNS();
      return;
    }
  }

  // Si falla con credenciales almacenadas, intentar con fallback
  if (strlen(WIFI_SSID_FALLBACK) > 0) {
    Serial.printf("[WIFI-RECONNECT] Intentando fallback: %s\n", WIFI_SSID_FALLBACK);

    WiFi.disconnect();
    WiFi.begin(WIFI_SSID_FALLBACK, WIFI_PASS_FALLBACK);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println("\n[WIFI-RECONNECT] ✓ Conexión fallback exitosa");
      addSerialLog("[WIFI] ✅ Conectado via fallback: " + String(WIFI_SSID_FALLBACK));
      setLedStatus(0, 255, 0);
      saveWifiConfig(WIFI_SSID_FALLBACK, WIFI_PASS_FALLBACK);  // Guardar nuevas credenciales
      configureNTP();
      return;
    }
  }

  // Si todo falla, entrar en modo configuración después de 3 intentos fallidos
  static int failedReconnectAttempts = 0;
  failedReconnectAttempts++;

  Serial.printf("[WIFI-RECONNECT] ❌ Reconexión fallida (intento %d/3)\n", failedReconnectAttempts);
  addSerialLog("[WIFI] ❌ Reconexión fallida " + String(failedReconnectAttempts) + "/3");

  if (failedReconnectAttempts >= 3) {
    Serial.println("[WIFI-RECONNECT] Activando modo configuración...");
    addSerialLog("[WIFI] 🚨 Activando modo configuración AP");
    failedReconnectAttempts = 0;
    startConfigPortal();
  }
}



void attemptQuickWifiReconnection() {
  // Reconexión rápida para modo offline
  if (ssid_stored.length() > 0) {
    WiFi.disconnect();
    WiFi.begin(ssid_stored.c_str(), pass_stored.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 5) {
      delay(500);
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println("\n[WIFI] ✓ Reconexión rápida exitosa");
      addSerialLog("[WIFI] ✅ Reconexión rápida exitosa");
      setLedStatus(0, 255, 0);
      configureNTP();
    }
  }
}

void initializeSystemRegardlessOfWifi() {
  Serial.println("[SYSTEM] Iniciando sistema en modo " + String(wifiConnected ? "online" : "offline"));
  addSerialLog("[SYSTEM] Modo: " + String(wifiConnected ? "ONLINE 🌐" : "OFFLINE 📴"));

  // Configurar NTP solo si hay WiFi
  if (wifiConnected) {
    configureNTP();
  } else {
    // Usar hora basada en millis() para modo offline
    Serial.println("[TIME] Usando reloj interno (modo offline)");
    addSerialLog("[TIME] ⏰ Reloj interno activado");
  }
  // Servidor web solo si hay WiFi
  if (wifiConnected) {
    setupmDNS();
  }
}



// ===========================
// VARIABLES GLOBALES PARA MONITORIZACIÓN
// ===========================
unsigned long lastWeatherAttempt = 0;
unsigned long lastTelegramAttempt = 0;
bool weatherInProgress = false;
bool telegramInProgress = false;


// ===========================
// SETUP OPTIMIZADO
// ===========================
void setup() {
  Serial.begin(115200);
  delay(100);

  
  // 2. MUTEX Y CONFIGURACIÓN
  initializeSystemMutex();
  Serial.println("[DEBUG 4] Después de initializeSystemMutex");
  Serial.println("[DEBUG 5] Inicia Telegram en PSRAM");
  initializeTelegramPSRAM(); Serial.println("[DEBUG PSRAM] Buffers PSRAM creados");
  Serial.println("[DEBUG 6] Ahora carga loadSystemConfiguration");
  loadSystemConfiguration();
  Serial.println("[DEBUG 6] Después de loadSystemConfiguration");
  AdaptiveConfig adaptConfig;
  adaptConfig.minThreshold = 20.0;
  adaptConfig.maxThreshold = 70.0;
  adaptConfig.adjustmentStep = 1.5;
  adaptiveSystem.configure(adaptConfig);
  Serial.println("[ADAPTIVE] Sistema bidireccional configurado");
  printSystemConfiguration();

  // 3. RED Y SISTEMA
  initializeNetworkSystem();
  initializeCriticalHardware();

  // 5. SERVICIOS Y CORE
  initializeWebServices();
  initializeCoreSystem();
  
  // 4. HANDLERS WEB
  initializeWebHandlers();
  setupAPIEndpoints();
  
  Serial.println("\n✅ [SYSTEM] Setup optimizado completado correctamente");
  Serial.println("======================================================");
}

//=========================
// LOOP
//========================
void loop() {
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 30000) {  // Cada 15 segundos
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      Serial.println("[WIFI] ❌ Conexión perdida, deteniendo tareas dependientes...");
      wifiConnected = false;
      setLedStatus(255, 165, 0);  // Naranja para reconexión
      addSerialLog("[WIFI] ✗ Conexión perdida - Tareas detenidas");
    }
    lastWifiCheck = millis();
  }


  if (wifiConfigMode) {
    dnsServer.processNextRequest();
    configServer.handleClient();

    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
      static bool ledState = false;
      setLedStatus(ledState ? 255 : 100, ledState ? 165 : 50, 0);
      ledState = !ledState;
      lastBlink = millis();
    }
    return;  //// No procesar el resto del loop en modo configuración
  }

  if (wifiConnected) {
    server.handleClient();
    
    // LED de estado para modo online
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate > 2000) {
      bool anyPumpActive = false;
      if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(50)) == pdTRUE) {
        anyPumpActive = data.pump1 || data.pump2;
        xSemaphoreGive(mutexData);
      } else {
        Serial.println("[WARN] Timeout al acceder a mutex");
      }


      if (anyPumpActive) {
        static bool ledState = false;
        setLedStatus(0, ledState ? 255 : 100, ledState ? 255 : 100);
        ledState = !ledState;
      } else {
        setLedStatus(0, 128, 0);  // Verde fijo
      }
      
      lastStatusUpdate = millis();
    }
    }else {
    // ✅ REINTENTO AUTOMÁTICO cada 2 minutos modo offline
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 120000) {  // 2 minutos
      Serial.println("[WIFI] Intentando reconexión automática...");
      attemptQuickWifiReconnection();
      lastReconnectAttempt = millis();
    }

    // ✅ ACTIVAR AP AUTOMÁTICAMENTE después de 2 minutos offline
    static unsigned long offlineStartTime = 0;
    static unsigned long lastAPCheck = 0;
    
    if (offlineStartTime == 0) {
      offlineStartTime = millis();
    }
    
    if (millis() - lastAPCheck > 60000) {  // Verificar cada minuto
      if (millis() - offlineStartTime > 120000) {  // 2 minutos offline
        Serial.println("[WIFI] 2 minutos offline - Activando modo AP");
        addSerialLog("[WIFI] ⚡ Activando modo AP automáticamente");
        startConfigPortal();
        offlineStartTime = 0;
      }
      lastAPCheck = millis();
    }

    // ✅ LED NARANJA PARPADEANTE para modo offline
    static unsigned long lastOfflineBlink = 0;
    if (millis() - lastOfflineBlink > 2000) {
      static bool offlineLedState = false;
      setLedStatus(offlineLedState ? 255 : 100, offlineLedState ? 165 : 50, 0);
      offlineLedState = !offlineLedState;
      lastOfflineBlink = millis();
    }
  }
}