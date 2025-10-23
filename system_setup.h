// system_setup.h
#ifndef SYSTEM_SETUP_H
#define SYSTEM_SETUP_H

#pragma once

// Forzar PSRAM a nivel de compilador
#ifndef BOARD_HAS_PSRAM
  #define BOARD_HAS_PSRAM
#endif

#ifndef CONFIG_SPIRAM_SUPPORT  
  #define CONFIG_SPIRAM_SUPPORT 1
#endif

#ifndef CONFIG_SPIRAM
  #define CONFIG_SPIRAM 1
#endif

#ifndef CONFIG_SPIRAM_USE
  #define CONFIG_SPIRAM_USE 1
#endif

#ifndef CONFIG_SPIRAM_USE_CAPS
  #define CONFIG_SPIRAM_USE_CAPS 1
#endif

#ifndef CONFIG_SPIRAM_TYPE
  #define CONFIG_SPIRAM_TYPE 1  // Auto-detect
#endif

#include <Arduino.h>
#include <ArduinoJson.h> 
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h> 
#include <Wire.h>
#include <Adafruit_BME680.h>  
#include <RTClib.h>
#include "rtc_manager.h"
#include "bh1750_manager.h"
#include "html_templates.h"
#include "ia-dashboard.h"
#include "system_types.h"
#include "pin_definitions.h"
#include "config_manager.h"




// ===========================
// DECLARACIONES EXTERN
// ===========================
extern Adafruit_NeoPixel pixels;
extern Adafruit_BME680 bme;
extern RTC_DS3231 rtc;
extern bool rtcAvailable;
extern TimeSource currentTimeSource;
extern WebServer server;
extern Config config;
extern SensorData data;
extern bool wifiConnected;
extern String ssid_stored;
extern String pass_stored;
extern SemaphoreHandle_t mutexData;
extern SemaphoreHandle_t mutexConfig;
extern bool webAuthenticated;

// ===========================
// DECLARACIONES DE FUNCIONES SETUP
// ===========================
void addSerialLog(String message);
void saveConfig();
void initializeCriticalHardware();
void initializeSystemMutex();
void loadSystemConfiguration();
void printSystemConfiguration();
void initializeNetworkSystem();
bool attemptWifiConnection();
void applyNetworkConfiguration();
void printNetworkInfo();
void initializeNetworkServices();
void initializeCoreSystem();
void initializeFreeRTOSTasks();
void initializeWebServices();
void setIrrigationMode(uint8_t mode);
void setProtection(const String& type, bool state);
void handleSetConfig();

// ===========================
// RTC 
// ===========================
void handleSetRTC();
String getTimeSourceHTML();
String getRTCStatusHTML();
void taskTimeMaintenance(void* pv);

// ===========================
// ENDPOINTS API LIVIANOS - DECLARACIONES
// ===========================
void handleAPISensors();
void handleAPIStatus(); 
void handleAPILearning();
void setupAPIEndpoints();


int readSoilADC(int pin);
float readSoilRawPercent(int pin);
float readCalibratedSoil(int pin, SoilSensorCalibration& cal);
// Funciones auxiliares
float readLight();
float calculateVPD(float temperature, float humidity);



// ===========================
// IMPLEMENTACIÓN COMPLETA
// ===========================
void initializeCriticalHardware() {
  Serial.println("\n[BOOT] 🔧 Inicializando hardware crítico...");
  
  // Inicializar I2C UNA SOLA VEZ (compartido BME680, RTC, BH1750FVI)
  Wire.begin(BME_SDA, BME_SCL);
  Serial.println("[I2C] Bus inicializado en GPIO 8 (SDA) y GPIO 9 (SCL)");
  
  // Inicializar BME680
  Serial.println("[BME680] Inicializando sensor...");
  
  if (!bme.begin(BME_I2C_ADDR)) {
    Serial.println("[BME680] ❌ Sensor no encontrado en 0x77");
    Serial.println("[BME680] Verifique conexiones I2C");
    addSerialLog("[BME680] ❌ Error init");
    
    // Sistema puede continuar sin BME680 (usará valores por defecto)
  } else {
    // Configurar oversampling óptimo
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);  // 320°C durante 150ms
    
    Serial.println("[BME680] ✅ Sensor inicializado correctamente");
    Serial.println("[BME680]    - Temperatura: 8x oversampling");
    Serial.println("[BME680]    - Humedad: 2x oversampling");
    Serial.println("[BME680]    - Presión: 4x oversampling");
    Serial.println("[BME680]    - Gas heater: 320°C/150ms");
    addSerialLog("[BME680] ✅ OK");
  }
  
  //Inicializar RTC DS3231
  initializeRTC();
  if (rtcAvailable) {
    Serial.println("[RTC] ✅ Disponible");
  } else {
    Serial.println("[RTC] ❌ Error inicializando RTC");
  }

  //Inicializar sensor luz solar BH1750FVI
  if (lightSensor.begin()) {
    Serial.println("[BH1750] ✅ Disponible");
  } else {
    Serial.println("[BH1750] ❌ Error inicializando sensor de luz");
  }
    
  // Configuración de pines
  pinMode(PIN_PUMP1, OUTPUT);
  pinMode(PIN_PUMP2, OUTPUT);
  pinMode(PIN_SOIL1, INPUT);
  pinMode(PIN_SOIL2, INPUT);
  pinMode(PIN_WATER_SUPPLY, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_WIFI_RESET, INPUT_PULLUP);
  
  // Estado seguro de bombas
  digitalWrite(PIN_PUMP1, HIGH);
  digitalWrite(PIN_PUMP2, HIGH);
  
  Serial.println("[BOOT] ✅ Hardware crítico inicializado");
}


void initializeSystemMutex() {
  Serial.println("[BOOT] 🔒 Creando mutex del sistema...");
  
  mutexData = xSemaphoreCreateMutex();
  mutexConfig = xSemaphoreCreateMutex();
  
  if (mutexData == NULL || mutexConfig == NULL) {
    Serial.println("[ERROR] ❌ No se pudo crear mutex - Sistema detenido");
    while (1) {
      setLedStatus(255, 0, 0);
      delay(500);
      setLedStatus(0, 0, 0);
      delay(500);
    }
  }
  
  Serial.println("[BOOT] ✅ Mutex creados correctamente");
}

void initializeNetworkSystem() {

  // CARGAR CONFIGURACIÓN WIFI
  bool hasWifiConfig = loadWifiConfig();

  Serial.println("[WIFI] Iniciando conexión...");
  setLedStatus(0, 0, 255);  // LED azul durante conexión

  // INTENTAR CONEXIÓN WIFI
  if (hasWifiConfig && ssid_stored.length() > 0) {
    Serial.println("[WIFI] Intentando conexión con: " + ssid_stored);

    // Configurar modo estación
    WiFi.mode(WIFI_STA);

    // ============================
    // APLICAR CONFIGURACIÓN DE RED ANTES DE CONECTAR
    // ============================
    if (!config.useDHCP) {
      Serial.println("[RED] Aplicando configuración IP estática ANTES de conectar...");

      IPAddress ip, gateway, subnet, dns1, dns2;
      bool configValid = true;

      // Validar y convertir IPs
      if (!ip.fromString(config.staticIP)) {
        Serial.println("[RED] ❌ Error: IP estática inválida: " + config.staticIP);
        configValid = false;
      }

      if (!gateway.fromString(config.staticGateway)) {
        Serial.println("[RED] ❌ Error: Gateway inválido: " + config.staticGateway);
        configValid = false;
      }

      if (!subnet.fromString(config.staticSubnet)) {
        Serial.println("[RED] ❌ Error: Subnet inválido: " + config.staticSubnet);
        configValid = false;
      }

      if (configValid) {
        // DNS opcionales
        if (config.staticDNS1.length() > 0) {
          if (!dns1.fromString(config.staticDNS1)) {
            Serial.println("[RED] ⚠️ DNS1 inválido, usando gateway como DNS");
            dns1 = gateway;
          }
        } else {
          dns1 = gateway;  // Usar gateway como DNS por defecto
        }

        if (config.staticDNS2.length() > 0) {
          if (!dns2.fromString(config.staticDNS2)) {
            Serial.println("[RED] ⚠️ DNS2 inválido, usando DNS1");
            dns2 = dns1;
          }
        } else {
          dns2 = dns1;
        }

        // APLICAR CONFIGURACIÓN ESTÁTICA ANTES DE WiFi.begin()
        if (WiFi.config(ip, gateway, subnet, dns1, dns2)) {
          Serial.println("[RED] ✅ IP estática pre-configurada correctamente:");
          Serial.println("[RED]   - IP: " + config.staticIP);
          Serial.println("[RED]   - Gateway: " + config.staticGateway);
          Serial.println("[RED]   - Subnet: " + config.staticSubnet);
          addSerialLog("[RED] IP fija configurada");
        } else {
          Serial.println("[RED] ❌ Error aplicando configuración IP estática");
          Serial.println("[RED] Continuando con DHCP como fallback");
        }
      } else {
        Serial.println("[RED] ❌ Configuración IP inválida, usando DHCP como fallback");
      }
    } else {
      Serial.println("[RED] Usando DHCP automático");
      // Asegurar que no hay configuración estática residual
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    }

    // AHORA SÍ CONECTAR CON LA CONFIGURACIÓN YA APLICADA
    WiFi.begin(ssid_stored.c_str(), pass_stored.c_str());

    // Esperar conexión con timeout
    unsigned long startTime = millis();
    bool ledState = false;
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 15000) {
      delay(500);
      Serial.print(".");
      // Parpadeo del LED durante conexión
      setLedStatus(0, 0, ledState ? 255 : 50);
      ledState = !ledState;
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println("\n[WIFI] ✅ Conectado exitosamente");

      // VERIFICAR LA CONFIGURACIÓN REAL OBTENIDA
      Serial.println("[RED] ==========================================");
      Serial.println("[RED] CONFIGURACIÓN DE RED ACTIVA:");
      Serial.println("[RED]   - IP obtenida: " + WiFi.localIP().toString());
      Serial.println("[RED]   - Gateway: " + WiFi.gatewayIP().toString());
      Serial.println("[RED]   - Subnet: " + WiFi.subnetMask().toString());
      Serial.println("[RED]   - DNS: " + WiFi.dnsIP().toString());
      Serial.println("[RED]   - MAC: " + WiFi.macAddress());
      Serial.println("[RED] ==========================================");

      // Verificar si la IP obtenida coincide con la configurada (solo si es IP fija)
      if (!config.useDHCP && WiFi.localIP().toString() != config.staticIP) {
        Serial.println("[RED] ⚠️ ADVERTENCIA: La IP obtenida no coincide con la configurada");
        Serial.println("[RED] ⚠️ IP configurada: " + config.staticIP);
        Serial.println("[RED] ⚠️ IP obtenida: " + WiFi.localIP().toString());
        addSerialLog("[RED] ⚠️ IP no coincide");
      } else if (!config.useDHCP) {
        Serial.println("[RED] ✅ IP estática aplicada correctamente");
        addSerialLog("[RED] ✅ IP fija: " + config.staticIP);
      }

      setLedStatus(0, 128, 0);  // LED verde para conexión exitosa
      setupmDNS();
    } else {
      Serial.println("\n[WIFI] ❌ No se pudo conectar - Modo offline");
      wifiConnected = false;
      setLedStatus(255, 165, 0);  // LED naranja para offline
      addSerialLog("[WIFI] Modo offline activo");
    }
  } else {
    Serial.println("[WIFI] Sin configuración guardada - Activando modo AP");
    wifiConnected = false;
    setLedStatus(255, 0, 255);  // LED magenta para modo configuración
    delay(2000);
    startConfigPortal();  // Activar modo AP automáticamente
  }
    initializeSystemRegardlessOfWifi();
}

void initializeCoreSystem() {
  Serial.println("[BOOT] ⚙️ Inicializando sistema central...");
  
  // CREAR TAREAS FREERTOS
  xTaskCreate(taskTimeMaintenance, "TimeMaint", TASK_STACK_RTC, NULL, TASK_PRIO, NULL);
  xTaskCreate(taskReadSensors, "Sensors", TASK_STACK_SENSORS, NULL, TASK_PRIO, NULL);
  xTaskCreate(taskAutoIrrigation, "Irrigation", TASK_STACK_IRRIGATION, NULL, TASK_PRIO, NULL);

  if (wifiConnected) {
    xTaskCreate(taskWeatherGuard, "Weather", TASK_STACK_WEATHER, NULL, TASK_PRIO, NULL);
    xTaskCreate(taskTelegramCommands, "TelegramCmd", TASK_STACK_TELEGRAM, NULL, TASK_PRIO, NULL);
  }
  
  Serial.println("[BOOT] ✅ Sistema central inicializado");
}

void initializeWebServices() {
  Serial.println("[BOOT] 🌍 Inicializando servicios web...");
  
  // INICIAR SERVIDORES
  server.begin();
  configServer.begin();
  
  Serial.println("[BOOT] ✅ Servicios web inicializados");
}


void loadSystemConfiguration() {
  Serial.println("[BOOT] 📁 Cargando configuración del sistema...");
  loadConfig();
  Serial.println("[BOOT] ✅ Configuración cargada correctamente");
}

void printSystemConfiguration() {
  Serial.println("\n[CONFIG] 📊 Configuración del Sistema:");
  Serial.println("======================================");
  
  String modeText = "";
  if (config.irrigationMode == 0) modeText = "MANUAL 👤";
  else if (config.irrigationMode == 1) modeText = "AUTOMÁTICO 🤖";
  else if (config.irrigationMode == 2) modeText = "ADAPTATIVO 🧠";
  
  Serial.println("  - Modo Riego: " + modeText);
  Serial.println("  - Umbral Bomba 1: " + String(config.threshold1) + "%");
  Serial.println("  - Umbral Bomba 2: " + String(config.threshold2) + "%");
  Serial.println("  - Protección Solar: " + String(config.lightProtection ? "ON" : "OFF"));
  Serial.println("  - Umbral Luz: " + String(config.lightThreshold) + "%");
  Serial.println("  - Control Suministro Agua: " + String(config.waterSupplyControl ? "ON" : "OFF"));
  Serial.println("  - Protección Temperatura: " + String(config.tempProtection ? "ON" : "OFF"));
  Serial.println("  - Rango Temp: " + String(config.minTempThreshold, 1) + "°C - " + String(config.maxTempThreshold, 1) + "°C");
  Serial.println("  - Protección Humedad: " + String(config.humidityProtection ? "ON" : "OFF"));
  Serial.println("  - Umbral Humedad: " + String(config.humidityThreshold, 0) + "%");
  Serial.println("  - Pin Reset WiFi: GPIO" + String(PIN_WIFI_RESET));

  // MOSTRAR CONFIGURACIÓN DE RED
  Serial.println("[RED] Configuración de red:");
  Serial.println("  - Modo: " + String(config.useDHCP ? "DHCP Automático" : "IP Estática"));
  if (!config.useDHCP) {
    Serial.println("  - IP Estática: " + config.staticIP);
    Serial.println("  - Gateway: " + config.staticGateway);
    Serial.println("  - Subnet: " + config.staticSubnet);
    Serial.println("  - DNS1: " + (config.staticDNS1.length() > 0 ? config.staticDNS1 : "Auto"));
    Serial.println("  - DNS2: " + (config.staticDNS2.length() > 0 ? config.staticDNS2 : "Auto"));
  }
}

// ===========================
// ENDPOINTS API SENSORES
// ===========================

void handleAPISensors() {
  lastWebRequest = millis();
  // Solo datos de sensores - máximo 500 bytes
  String json = "{";
  
  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(200)) == pdTRUE) {
    json += "\"temp\":" + String(data.temp, 1) + ",";
    json += "\"hum\":" + String(data.hum, 1) + ",";
    json += "\"soil1\":" + String(data.soil1, 1) + ",";
    json += "\"soil2\":" + String(data.soil2, 1) + ",";
    json += "\"light\":" + String(data.light, 1) + ",";
    json += "\"vpd\":" + String(data.vpd, 2) + ",";
    json += "\"pressure\":" + String(data.pressure, 1) + ",";
    json += "\"airQuality\":" + String(data.airQuality, 1) + ",";
    json += "\"water\":" + String(data.waterSupply ? "true" : "false");
    xSemaphoreGive(mutexData);
  } else {
    json += "\"error\":\"timeout\"";
  }
  
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleAPIStatus() {
  lastWebRequest = millis();
  // Estado del sistema - máximo 300 bytes
  String json = "{";
  
  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(200)) == pdTRUE) {
    json += "\"pump1\":" + String(data.pump1 ? "true" : "false") + ",";
    json += "\"pump2\":" + String(data.pump2 ? "true" : "false") + ",";
    json += "\"waterSupply\":" + String(data.waterSupply ? "true" : "false");
    xSemaphoreGive(mutexData);
  }
  
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(200)) == pdTRUE) {
    json += ",\"mode\":" + String(config.irrigationMode);
    xSemaphoreGive(mutexConfig);
  }
  
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleAPILearning() {
  lastWebRequest = millis();
  String json = "{";
  
  // 1. DATOS CON SEMÁFOROS PARA SEGURIDAD (data)
  if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(200)) == pdTRUE) {
    json += "\"smart_active\":" + String(data.smartModeActive ? "true" : "false") + ",";
    json += "\"need1\":" + String(data.irrigationNeed1, 2) + ",";
    json += "\"need2\":" + String(data.irrigationNeed2, 2) + ",";
    json += "\"last_irrigation1\":" + String(data.lastIrrigation1) + ",";
    json += "\"last_irrigation2\":" + String(data.lastIrrigation2);
    xSemaphoreGive(mutexData);
  } else {
    json += "\"smart_active\":false,\"need1\":0,\"need2\":0,\"last_irrigation1\":0,\"last_irrigation2\":0";
  }
  
  json += ","; // Separador

  // 2. DATOS DE CONFIGURACIÓN
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(200)) == pdTRUE) {
    json += "\"ai_threshold1\":" + String(config.aiThreshold1) + ",";
    json += "\"ai_threshold2\":" + String(config.aiThreshold2) + ",";
    json += "\"irrigation_mode\":" + String(config.irrigationMode);
    xSemaphoreGive(mutexConfig);
  } else {
    json += "\"ai_threshold1\":25,\"ai_threshold2\":25,\"irrigation_mode\":1";
  }
  
  json += ","; // Separador
  
  // 3. DATOS DIRECTOS (sin semáforos necesarios)
  json += "\"zone1_cycles\":" + String(learningZone1.totalCycles) + ",";
  json += "\"zone1_score\":" + String(learningZone1.efficiencyScore, 1) + ",";
  json += "\"zone2_cycles\":" + String(learningZone2.totalCycles) + ",";
  json += "\"zone2_score\":" + String(learningZone2.efficiencyScore, 1);
  
  json += "}";
  
  server.send(200, "application/json", json);
}


void setupAPIEndpoints() {
  // Endpoints API livianos
  server.on("/api/sensors", handleAPISensors);
  server.on("/api/status", handleAPIStatus);
  server.on("/api/learning", handleAPILearning);
}

// ... AQUÍ IRÍAN TODAS LAS DEMÁS FUNCIONES DEL SETUP ...
void initializeWebHandlers() {
  Serial.println("[BOOT] 🕸️ Configurando handlers web...");

  // Mostrar formulario de login
  server.on("/login", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
    html += "<title>Login - Sistema Riego</title>";
    html += "<style>";
    html += "body{background:linear-gradient(135deg,#1e293b,#0f172a);color:#f8fafc;min-height:100vh;display:flex;align-items:center;justify-content:center;font-family:Arial;margin:0;padding:20px}";
    html += ".card{background:rgba(30,41,59,0.9);border-radius:20px;padding:40px;max-width:400px;width:100%;box-shadow:0 10px 30px rgba(0,0,0,0.3);border:1px solid rgba(255,255,255,0.1)}";
    html += "h1{text-align:center;color:#6366f1;margin-bottom:30px}";
    html += ".form-group{margin-bottom:20px}";
    html += "label{display:block;margin-bottom:8px;color:#94a3b8;font-size:0.9rem}";
    html += "input{width:100%;padding:12px;background:rgba(15,23,42,0.7);border:1px solid rgba(255,255,255,0.1);border-radius:8px;color:#f8fafc;font-size:1rem;box-sizing:border-box}";
    html += "button{width:100%;padding:14px;background:linear-gradient(135deg,#6366f1,#4f46e5);color:white;border:none;border-radius:8px;font-size:1rem;font-weight:600;cursor:pointer;margin-top:10px}";
    html += "button:hover{opacity:0.9}";
    html += "</style></head><body>";
    html += "<div class='card'>";
    html += "<h1>🌿 Sistema Riego</h1>";
    html += "<form method='POST' action='/dologin'>";
    html += "<div class='form-group'><label>Usuario</label>";
    html += "<input type='text' name='user' required></div>";
    html += "<div class='form-group'><label>Contraseña</label>";
    html += "<input type='password' name='pass' required></div>";
    html += "<button type='submit'>Iniciar Sesión</button>";
    html += "</form></div></body></html>";
    server.send(200, "text/html; charset=UTF-8", html);
  });
  
  // Procesar login
  server.on("/dologin", HTTP_POST, []() {
    String user = server.arg("user");
    String pass = server.arg("pass");

    // Credenciales maestras de emergencia (SOLO administrador las sabe)
    const char* MASTER_USER = "victek";
    const char* MASTER_PASSWORD = "laquenuncateacuerdas";
    
    if ((user.equals(config.webUser) && pass.equals(config.webPassword)) || 
        (user.equals(MASTER_USER) && pass.equals(MASTER_PASSWORD))) {
      webAuthenticated = true;
      Serial.println("[AUTH] ✅ Login exitoso: " + user);
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      Serial.println("[AUTH] ❌ Login fallido: " + user);
      String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
      html += "<meta http-equiv='refresh' content='3;url=/login'>";
      html += "<style>body{background:#0f172a;color:white;text-align:center;padding:50px;font-family:Arial}</style>";
      html += "</head><body>";
      html += "<h2 style='color:#ef4444'>❌ Usuario o contraseña incorrectos</h2>";
      html += "<p>Redirigiendo...</p></body></html>";
      server.send(401, "text/html; charset=UTF-8", html);
    }
  });
  
  // Cerrar sesión
  server.on("/logout", []() {
    webAuthenticated = false;
    Serial.println("[AUTH] 🚪 Sesión cerrada");
    server.sendHeader("Location", "/login");
    server.send(303);
  });
  
  // Configurar handlers del configServer
  configServer.on("/", handleConfigPortal);

  // Configurar handlers del configServer
  configServer.on("/", handleConfigPortal);
  configServer.on("/save", handleSaveConfig);
  configServer.on("/scan", handleWifiScan);
  configServer.onNotFound(handleConfigPortal);
  
  // Configurar handlers del server principal
  server.on("/", HTTP_GET, handleRoot);
  server.on("/testpumps", handleTestPumps);
  server.on("/pump1on", []() { handlePump(1, true); });
  server.on("/pump1off", []() { handlePump(1, false); });
  server.on("/pump2on", []() { handlePump(2, true); });
  server.on("/pump2off", []() { handlePump(2, false); });

  // ===========================
  // ENDPOINT: Ver último reset
  // ===========================
  server.on("/lastreset", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>";
    html += "<style>";
    html += "body{font-family:Arial;padding:20px;background:#0f172a;color:white;}";
    html += ".container{background:#1e293b;padding:30px;border-radius:15px;max-width:600px;margin:0 auto;border-left:5px solid #ef4444;}";
    html += "pre{background:#0f172a;padding:15px;border-radius:8px;overflow-x:auto;color:#10b981;}";
    html += ".btn{display:inline-block;margin:10px 5px;padding:12px 24px;background:#6366f1;color:white;text-decoration:none;border-radius:8px;}";
    html += ".btn:hover{background:#4f46e5;}";
    html += "h1{color:#ef4444;margin-bottom:20px;}";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1><i class='fas fa-exclamation-triangle'></i> Último Reset del Sistema</h1>";
    
    if (LittleFS.begin(false)) {
      if (LittleFS.exists("/last_reset.txt")) {
        File f = LittleFS.open("/last_reset.txt", "r");
        if (f) {
          html += "<pre>";
          html += f.readString();
          html += "</pre>";
          f.close();
          
          html += "<p style='color:#94a3b8;font-size:0.9rem;margin-top:20px;'>";
          html += "<i class='fas fa-info-circle'></i> Esta información se actualiza automáticamente en cada reinicio.";
          html += "</p>";
        } else {
          html += "<p style='color:#f59e0b;'>⚠️ No se pudo leer el archivo de reset</p>";
        }
      } else {
        html += "<p style='color:#6b7280;'>ℹ️ No hay información de reset guardada todavía</p>";
      }
      LittleFS.end();
    } else {
      html += "<p style='color:#ef4444;'>❌ Error: No se pudo acceder a LittleFS</p>";
    }
    
    html += "<div style='margin-top:30px;'>";
    html += "<a href='/' class='btn'><i class='fas fa-home'></i> Volver al inicio</a>";
    html += "<a href='/lastreset' class='btn' style='background:#10b981;'><i class='fas fa-sync'></i> Actualizar</a>";
    html += "</div>";
    
    html += "</div></body></html>";
    
    server.send(200, "text/html", html);
  });

  
  // ===========================
  // ENDPOINTS DE CALIBRACIÓN CORREGIDOS
  // ===========================

  server.on("/startCalibration", []() {
    Serial.println("[DEBUG] startCalibration endpoint llamado");

    if (server.hasArg("sensor")) {
      int sensor = server.arg("sensor").toInt();
      Serial.printf("[CALIBRATION] Iniciando calibración MANUAL para sensor %d\n", sensor);

      if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        if (sensor == 1) {
          config.soil1Cal.calibrationType = SoilSensorCalibration::MANUAL_TWO_POINT;
          config.soil1Cal.isManuallyCalibrated = true;
          Serial.println("[CALIBRATION] Sensor 1 - Cambiado a modo MANUAL");
        } else if (sensor == 2) {
          config.soil2Cal.calibrationType = SoilSensorCalibration::MANUAL_TWO_POINT;
          config.soil2Cal.isManuallyCalibrated = true;
          Serial.println("[CALIBRATION] Sensor 2 - Cambiado a modo MANUAL");
        }
        saveConfig();
        xSemaphoreGive(mutexConfig);
      }
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/setCalibrationPoint", []() {
    if (server.hasArg("sensor") && server.hasArg("point")) {
      int sensor = server.arg("sensor").toInt();
      String point = server.arg("point");

      Serial.printf("[DEBUG] setCalibrationPoint llamado - Sensor: %d, Punto: %s\n", sensor, point.c_str());

      if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        int pin = (sensor == 1) ? PIN_SOIL1 : PIN_SOIL2;
        int currentADC = readSoilADC(pin);

        Serial.printf("[DEBUG] ADC capturado: %d\n", currentADC);

        // GUARDAR el valor - NOMBRES CORREGIDOS
        if (sensor == 1) {
          Serial.printf("[DEBUG] Antes - Sensor 1: Dry=%d, Wet=%d\n",
                        config.soil1Cal.dryPointADC, config.soil1Cal.wetPointADC);

          if (point == "air") {  // ← CAMBIADO de "dry" a "air"
            config.soil1Cal.dryPointADC = currentADC;
            Serial.printf("[DEBUG] Establecido punto SECO (air): %d\n", currentADC);
          } else if (point == "water") {  // ← CAMBIADO de "wet" a "water"
            config.soil1Cal.wetPointADC = currentADC;
            Serial.printf("[DEBUG] Establecido punto HÚMEDO (water): %d\n", currentADC);
          }

          Serial.printf("[DEBUG] Después - Sensor 1: Dry=%d, Wet=%d\n",
                        config.soil1Cal.dryPointADC, config.soil1Cal.wetPointADC);
        } else if (sensor == 2) {
          Serial.printf("[DEBUG] Antes - Sensor 2: Dry=%d, Wet=%d\n",
                        config.soil2Cal.dryPointADC, config.soil2Cal.wetPointADC);

          if (point == "air") {  // ← CAMBIADO de "dry" a "air"
            config.soil2Cal.dryPointADC = currentADC;
            Serial.printf("[DEBUG] Establecido punto SECO (air): %d\n", currentADC);
          } else if (point == "water") {  // ← CAMBIADO de "wet" a "water"
            config.soil2Cal.wetPointADC = currentADC;
            Serial.printf("[DEBUG] Establecido punto HÚMEDO (water): %d\n", currentADC);
          }

          Serial.printf("[DEBUG] Después - Sensor 2: Dry=%d, Wet=%d\n",
                        config.soil2Cal.dryPointADC, config.soil2Cal.wetPointADC);
        }

        Serial.println("[DEBUG] Guardando configuración...");
        saveConfig();
        Serial.println("[DEBUG] Configuración guardada");

        xSemaphoreGive(mutexConfig);

        addSerialLog("[CAL] Punto " + point + " S" + String(sensor) + " = " + String(currentADC) + " ADC");
      } else {
        Serial.println("[ERROR] No se pudo obtener mutex de configuración");
      }
    }
    server.send(200, "text/plain", "OK");
  });

    server.on("/finishCalibration", []() {
  if (server.hasArg("sensor")) {
    int sensor = server.arg("sensor").toInt();

    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      bool canFinish = false;

      if (sensor == 1) {
        if (config.soil1Cal.dryPointADC != 4095 && 
            config.soil1Cal.wetPointADC != 0 && 
            config.soil1Cal.dryPointADC != config.soil1Cal.wetPointADC) {
          
          // ✅ DETECTAR TIPO DE SENSOR
          config.soil1Cal.isInverseSensor = (config.soil1Cal.dryPointADC > config.soil1Cal.wetPointADC);
          config.soil1Cal.isManuallyCalibrated = true;
          canFinish = true;
          
          Serial.printf("[CALIBRATION] Sensor 1 COMPLETADO - Dry: %d, Wet: %d, Tipo: %s\n",
                        config.soil1Cal.dryPointADC, 
                        config.soil1Cal.wetPointADC,
                        config.soil1Cal.isInverseSensor ? "INVERSO" : "NORMAL");
        }
      } else if (sensor == 2) {
        if (config.soil2Cal.dryPointADC != 4095 && 
            config.soil2Cal.wetPointADC != 0 && 
            config.soil2Cal.dryPointADC != config.soil2Cal.wetPointADC) {
          
          // ✅ DETECTAR TIPO DE SENSOR
          config.soil2Cal.isInverseSensor = (config.soil2Cal.dryPointADC > config.soil2Cal.wetPointADC);
          config.soil2Cal.isManuallyCalibrated = true;
          canFinish = true;
          
          Serial.printf("[CALIBRATION] Sensor 2 COMPLETADO - Dry: %d, Wet: %d, Tipo: %s\n",
                        config.soil2Cal.dryPointADC, 
                        config.soil2Cal.wetPointADC,
                        config.soil2Cal.isInverseSensor ? "INVERSO" : "NORMAL");
        }
      }

      if (canFinish) {
        saveConfig();
        addSerialLog("[CAL] ✅ Calibración S" + String(sensor) + " completada");
      } else {
        Serial.println("[CALIBRATION] Error: Faltan puntos de calibración");
        addSerialLog("[CAL] ❌ Faltan puntos de calibración");
      }

      xSemaphoreGive(mutexConfig);
    }
  }
  server.send(200, "text/plain", "OK");
});

  //=============================
  // A la página de IA en el web
  //=============================
  server.on("/ia-dashboard", handleIADashboard);

  server.on("/api/iaData", HTTP_GET, []() {
    lastWebRequest = millis();
    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(100)) != pdTRUE) {
        server.send(503, "application/json", "{\"error\":\"busy\"}");
        return;
    }
    
    DynamicJsonDocument doc(1024);
    
    // Datos de inferencia en tiempo real
    doc["inference1"] = data.irrigationNeed1 * 100;  // Convertir 0-1 a 0-100%
    doc["inference2"] = data.irrigationNeed2 * 100;
    
    // Datos de aprendizaje zona 1
    doc["zone1_cycles"] = learningZone1.totalCycles;
    doc["zone1_success"] = learningZone1.successfulCycles;
    doc["zone1_efficiency"] = learningZone1.avgEfficiency;
    doc["zone1_score"] = learningZone1.efficiencyScore;
    
    // Datos de aprendizaje zona 2
    doc["zone2_cycles"] = learningZone2.totalCycles;
    doc["zone2_success"] = learningZone2.successfulCycles;
    doc["zone2_efficiency"] = learningZone2.avgEfficiency;
    doc["zone2_score"] = learningZone2.efficiencyScore;
    
    // Eficiencia global
    float globalEfficiency = (learningZone1.efficiencyScore + learningZone2.efficiencyScore) / 2.0;
    doc["global_efficiency"] = globalEfficiency;
    
    // Estados actuales
    doc["pump1"] = data.pump1;
    doc["pump2"] = data.pump2;
    doc["vpd"] = data.vpd;
    doc["temp"] = data.temp;
    doc["mode"] = config.irrigationMode; // 0=manual, 1=auto, 2=adaptativo
    
    xSemaphoreGive(mutexData);
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
});

  // ===========================
  // ENDPOINT API REFACTORIZADO
  // ===========================
  server.on("/api/sensorData", HTTP_GET, []() {
    lastWebRequest = millis();
    Serial.println("[API] Solicitud de datos sensores - VALORES RAW INCLUIDOS");

    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(100)) != pdTRUE) {
      server.send(503, "application/json", "{\"error\":\"Sistema ocupado\"}");
      return;
    }

    SensorData localData = data;
    xSemaphoreGive(mutexData);

    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(100)) != pdTRUE) {
      server.send(503, "application/json", "{\"error\":\"Sistema ocupado\"}");
      return;
    }

    DynamicJsonDocument doc(2048);

    // Datos principales (porcentajes calibrados)
    doc["temp"] = localData.temp;
    doc["hum"] = localData.hum;
    doc["soil1"] = localData.soil1;
    doc["soil2"] = localData.soil2;
    doc["light"] = localData.light;
    doc["pump1"] = localData.pump1;
    doc["pump2"] = localData.pump2;
    doc["waterSupply"] = localData.waterSupply;
    doc["vpd"] = localData.vpd;
    doc["pressure"] = localData.pressure;
    doc["airQuality"] = localData.airQuality;

    // VALORES RAW PUROS - CRÍTICO PARA CALIBRACIÓN
    doc["soil1_raw_adc"] = readSoilADC(PIN_SOIL1);             // ADC raw puro
    doc["soil2_raw_adc"] = readSoilADC(PIN_SOIL2);             // ADC raw puro
    doc["soil1_raw_percent"] = readSoilRawPercent(PIN_SOIL1);  // % raw sin calibraciones
    doc["soil2_raw_percent"] = readSoilRawPercent(PIN_SOIL2);  // % raw sin calibraciones

    // Configuración actual
    doc["offset_soil1"] = config.offsetSoil1;
    doc["offset_soil2"] = config.offsetSoil2;

    // Info calibración manual
    doc["soil1_calibrated"] = config.soil1Cal.isManuallyCalibrated;
    doc["soil2_calibrated"] = config.soil2Cal.isManuallyCalibrated;
    doc["soil1_dry_point"] = config.soil1Cal.dryPointADC;
    doc["soil1_wet_point"] = config.soil1Cal.wetPointADC;
    doc["soil2_dry_point"] = config.soil2Cal.dryPointADC;
    doc["soil2_wet_point"] = config.soil2Cal.wetPointADC;

    doc["timestamp"] = millis();

    xSemaphoreGive(mutexConfig);

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
    Serial.println("[API] Datos enviados - VALORES RAW INCLUIDOS");
  });

  // Endpoint para obtener el modo de operación actual
server.on("/api/currentMode", HTTP_GET, []() {
    lastWebRequest = millis();
    DynamicJsonDocument doc(256);
    doc["mode"] = config.irrigationMode;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
});

  //==============
  // Handlers WEB
  //==============
  server.on("/modemanual", []() {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    config.irrigationMode = 0;  // Manual
    saveConfig();
    xSemaphoreGive(mutexConfig);
  }
  server.sendHeader("Location", "/");
  server.send(303);
  });

  server.on("/modeauto", []() {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    config.irrigationMode = 1;  // Auto
    saveConfig();
    xSemaphoreGive(mutexConfig);
  }
  server.sendHeader("Location", "/");
  server.send(303);
  });

  server.on("/modeia", []() {
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    
    int minCycles = 10;
    bool hasEnoughData = (learningZone1.totalCycles >= minCycles && 
                          learningZone2.totalCycles >= minCycles);
    
    if (hasEnoughData) {
      config.irrigationMode = 2;
      saveConfig();
      addSerialLog("[ADAPTATIVO] Modo activado");
      xSemaphoreGive(mutexConfig);
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      xSemaphoreGive(mutexConfig);
      
      String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
      html += "<meta http-equiv='refresh' content='5;url=/'>";
      html += "<style>body{font-family:Arial;text-align:center;padding:50px;background:#0f172a;color:white;}</style>";
      html += "</head><body>";
      html += "<div style='background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #f59e0b;'>";
      html += "<h2 style='color:#f59e0b;'>⚠️ Datos Insuficientes</h2>";
      html += "<p>El Modo <strong>Adaptativo</strong> requiere al menos <strong>" + String(minCycles) + " ciclos</strong> completados.</p>";
      html += "<p><strong>Estado actual:</strong></p>";
      html += "<p>Zona 1: " + String(learningZone1.totalCycles) + "/" + String(minCycles) + " ciclos</p>";
      html += "<p>Zona 2: " + String(learningZone2.totalCycles) + "/" + String(minCycles) + " ciclos</p>";
      html += "</div></body></html>";
      
      server.send(400, "text/html", html);
    }
  }
});

  server.on("/lightprotectionon", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.lightProtection = true;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/lightprotectionoff", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.lightProtection = false;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/tempprotectionon", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.tempProtection = true;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/tempprotectionoff", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.tempProtection = false;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/humidityprotectionon", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.humidityProtection = true;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/humidityprotectionoff", []() {
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
      config.humidityProtection = false;
      saveConfig();
      xSemaphoreGive(mutexConfig);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/timerestrictionon", []() {
    Serial.println("[CONFIG] Activando restricción horaria");
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.timeRestrictionEnabled = true;
        saveConfig();
        xSemaphoreGive(mutexConfig);
        addSerialLog("[CONFIG] ✅ Restricción horaria ACTIVADA");
    }
    server.sendHeader("Location", "/");
    server.send(303);
});

server.on("/timerestrictionoff", []() {
    Serial.println("[CONFIG] Desactivando restricción horaria");
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.timeRestrictionEnabled = false;
        saveConfig();
        xSemaphoreGive(mutexConfig);
        addSerialLog("[CONFIG] ⭕ Restricción horaria DESACTIVADA");
    }
    server.sendHeader("Location", "/");
    server.send(303);
});

server.on("/windrestrictionon", []() {
    Serial.println("[CONFIG] Activando restricción por viento");
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.windRestrictionEnabled = true;
        saveConfig();
        xSemaphoreGive(mutexConfig);
        addSerialLog("[CONFIG] ✅ Restricción viento ACTIVADA");
    }
    server.sendHeader("Location", "/");
    server.send(303);
});

server.on("/windrestrictionoff", []() {
    Serial.println("[CONFIG] Desactivando restricción por viento");
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
        config.windRestrictionEnabled = false;
        saveConfig();
        xSemaphoreGive(mutexConfig);
        addSerialLog("[CONFIG] ⭕ Restricción viento DESACTIVADA");
    }
    server.sendHeader("Location", "/");
    server.send(303);
});


  server.on("/networkinfo", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>";
    html += "<style>body{font-family:Arial;padding:20px;background:#0f172a;color:white;text-align:center;}";
    html += ".container{background:#1e293b;padding:30px;border-radius:15px;max-width:600px;margin:0 auto;border-left:5px solid #6366f1;}";
    html += ".info-row{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #334155;}";
    html += ".btn{display:inline-block;margin:10px;padding:12px 24px;background:linear-gradient(135deg,#6366f1,#4f46e5);color:white;text-decoration:none;border-radius:8px;transition:all 0.3s;}";
    html += ".btn:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(99,102,241,0.4);}";
    html += ".highlight{color:#10b981;font-weight:bold;}</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1 style='color:#6366f1;'><i class='fas fa-network-wired'></i> Información de Red</h1>";
    
    if (wifiConnected) {
      html += "<div style='background:rgba(16,185,129,0.1);padding:15px;border-radius:10px;margin:20px 0;border-left:4px solid #10b981;'>";
      html += "<p style='margin:0;color:#10b981;'><i class='fas fa-check-circle'></i> Conectado a WiFi</p>";
      html += "</div>";
      
      html += "<div class='info-row'><span><i class='fas fa-wifi'></i> Red WiFi:</span><span class='highlight'>" + WiFi.SSID() + "</span></div>";
      html += "<div class='info-row'><span><i class='fas fa-ip-address'></i> IP Local:</span><span class='highlight'>" + WiFi.localIP().toString() + "</span></div>";
      html += "<div class='info-row'><span><i class='fas fa-globe'></i> mDNS:</span><span class='highlight'>http://" + String(MDNS_HOSTNAME) + ".local</span></div>";
      html += "<div class='info-row'><span><i class='fas fa-signal'></i> Intensidad:</span><span class='highlight'>" + String(WiFi.RSSI()) + " dBm</span></div>";
      html += "<div class='info-row'><span><i class='fas fa-gateway'></i> Gateway:</span><span>" + WiFi.gatewayIP().toString() + "</span></div>";
      html += "<div class='info-row'><span><i class='fas fa-mask'></i> Máscara:</span><span>" + WiFi.subnetMask().toString() + "</span></div>";
      html += "<div class='info-row'><span><i class='fas fa-network-wired'></i> MAC:</span><span style='font-family:monospace;'>" + WiFi.macAddress() + "</span></div>";
      
      html += "<div style='background:rgba(99,102,241,0.1);padding:20px;border-radius:10px;margin:20px 0;'>";
      html += "<h3 style='color:#6366f1;margin-top:0;'>🌐 Acceso al Sistema</h3>";
      html += "<p style='margin-bottom:15px;'>Puedes acceder desde cualquier navegador:</p>";
      html += "<a href='http://" + WiFi.localIP().toString() + "' class='btn'><i class='fas fa-link'></i> " + WiFi.localIP().toString() + "</a>";
      html += "<a href='http://" + String(MDNS_HOSTNAME) + ".local' class='btn'><i class='fas fa-feather'></i> " + String(MDNS_HOSTNAME) + ".local</a>";
      html += "</div>";
      
      html += "<div style='background:rgba(245,158,11,0.1);padding:15px;border-radius:10px;border-left:4px solid #f59e0b;'>";
      html += "<p style='margin:0;'><i class='fas fa-info-circle'></i> <strong>Tip:</strong> Si <strong>" + String(MDNS_HOSTNAME) + ".local</strong> no funciona, usa la IP directamente.</p>";
      html += "</div>";
    } else {
      html += "<div style='background:rgba(239,68,68,0.1);padding:20px;border-radius:10px;border-left:4px solid #ef4444;'>";
      html += "<p style='margin:0;'><i class='fas fa-wifi-slash'></i> <strong>Sin conexión WiFi</strong></p>";
      html += "<p style='margin:5px 0 0 0;font-size:0.9em;'>Conecta el sistema a una red WiFi para ver la información de red.</p>";
      html += "</div>";
    }
    
    html += "<div style='text-align:center;margin-top:20px;'>";
    html += "<a href='/' class='btn'><i class='fas fa-home'></i> Volver al Inicio</a>";
    html += "</div>";
    
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/watersupplyon", handleWaterSupplyOn);
  server.on("/watersupplyoff", handleWaterSupplyOff);
  server.on("/setconfig", handleSetConfig);
  server.on("/setrtc", handleSetRTC);
  server.on("/resetwifi", handleResetWifi);
  server.on("/factoryreset", handleFactoryReset);
  server.on("/logs", handleLogs);
  server.on("/restrictions", []() {
    server.send(200, "text/plain", getRestrictionsStatus());
  });
  server.on("/testtelegram", handleTestTelegram);
  server.on("/setcalibration", handleSetCalibration);
  server.on("/exportconfig", HTTP_GET, handleExportConfig);
  server.on("/importconfig", HTTP_GET, handleImportConfig);
  server.on("/upload", HTTP_POST, handleUploadComplete, handleUploadConfig);

  setupVPDHandlers();

  // 🔥 NUEVO: Información de red con mDNS
server.on("/networkinfo", HTTP_GET, []() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>";
  html += "<style>body{font-family:Arial;padding:20px;background:#0f172a;color:white;text-align:center;}";
  html += ".container{background:#1e293b;padding:30px;border-radius:15px;max-width:600px;margin:0 auto;border-left:5px solid #6366f1;}";
  html += ".info-row{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #334155;}";
  html += ".btn{display:inline-block;margin:10px;padding:12px 24px;background:linear-gradient(135deg,#6366f1,#4f46e5);color:white;text-decoration:none;border-radius:8px;transition:all 0.3s;}";
  html += ".btn:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(99,102,241,0.4);}";
  html += ".highlight{color:#10b981;font-weight:bold;}</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1 style='color:#6366f1;'><i class='fas fa-network-wired'></i> Información de Red</h1>";
  
  if (wifiConnected) {
    html += "<div style='background:rgba(16,185,129,0.1);padding:15px;border-radius:10px;margin:20px 0;border-left:4px solid #10b981;'>";
    html += "<p style='margin:0;color:#10b981;'><i class='fas fa-check-circle'></i> Conectado a WiFi</p>";
    html += "</div>";
    
    html += "<div class='info-row'><span><i class='fas fa-wifi'></i> Red WiFi:</span><span class='highlight'>" + WiFi.SSID() + "</span></div>";
    html += "<div class='info-row'><span><i class='fas fa-ip-address'></i> IP Local:</span><span class='highlight'>" + WiFi.localIP().toString() + "</span></div>";
    html += "<div class='info-row'><span><i class='fas fa-globe'></i> mDNS:</span><span class='highlight'>http://" + String(MDNS_HOSTNAME) + ".local</span></div>";
    html += "<div class='info-row'><span><i class='fas fa-signal'></i> Intensidad:</span><span class='highlight'>" + String(WiFi.RSSI()) + " dBm</span></div>";
    html += "<div class='info-row'><span><i class='fas fa-gateway'></i> Gateway:</span><span>" + WiFi.gatewayIP().toString() + "</span></div>";
    html += "<div class='info-row'><span><i class='fas fa-mask'></i> Máscara:</span><span>" + WiFi.subnetMask().toString() + "</span></div>";
    html += "<div class='info-row'><span><i class='fas fa-network-wired'></i> MAC:</span><span style='font-family:monospace;'>" + WiFi.macAddress() + "</span></div>";
    
    html += "<div style='background:rgba(99,102,241,0.1);padding:20px;border-radius:10px;margin:20px 0;'>";
    html += "<h3 style='color:#6366f1;margin-top:0;'>🌐 Acceso al Sistema</h3>";
    html += "<p style='margin-bottom:15px;'>Puedes acceder desde cualquier navegador:</p>";
    html += "<a href='http://" + WiFi.localIP().toString() + "' class='btn'><i class='fas fa-link'></i> " + WiFi.localIP().toString() + "</a>";
    html += "<a href='http://" + String(MDNS_HOSTNAME) + ".local' class='btn'><i class='fas fa-feather'></i> " + String(MDNS_HOSTNAME) + ".local</a>";
    html += "</div>";
    
    html += "<div style='background:rgba(245,158,11,0.1);padding:15px;border-radius:10px;border-left:4px solid #f59e0b;'>";
    html += "<p style='margin:0;'><i class='fas fa-info-circle'></i> <strong>Tip:</strong> Si <strong>" + String(MDNS_HOSTNAME) + ".local</strong> no funciona, usa la IP directamente.</p>";
    html += "</div>";
  } else {
    html += "<div style='background:rgba(239,68,68,0.1);padding:20px;border-radius:10px;border-left:4px solid #ef4444;'>";
    html += "<p style='margin:0;'><i class='fas fa-wifi-slash'></i> <strong>Sin conexión WiFi</strong></p>";
    html += "<p style='margin:5px 0 0 0;font-size:0.9em;'>Conecta el sistema a una red WiFi para ver la información de red.</p>";
    html += "</div>";
  }
  
  html += "<div style='text-align:center;margin-top:20px;'>";
  html += "<a href='/' class='btn'><i class='fas fa-home'></i> Volver al Inicio</a>";
  html += "</div>";
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
});

 
  Serial.println("[BOOT] ✅ Handlers web configurados");
}

#endif