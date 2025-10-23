// config_manager.h
// Gestor de configuración del sistema - Vicente Soriano @2025

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "system_types.h"
#include "system_setup.h"

// ===========================
// ESTRUCTURAS PARA PSRAM
// ===========================
struct TelegramPSRAMData {
    char* telegramToken = nullptr;
    char* chatID = nullptr;
    bool psramAvailable = false;
};

// Variables para Telegram
extern WiFiClientSecure secured_client;
extern UniversalTelegramBot* bot;
extern TelegramPSRAMData telegramPSRAM;
extern const String telegramRootCACert;

//Variable para avisar a webAutheticated en el .ino
extern bool webAuthenticated;

// deficiones reales en el  system_types.h
extern IrrigationCycle currentCycle1;
extern IrrigationCycle currentCycle2;

// definiciones de IA adpatativa
extern SensorHistory sensorHistory;

bool uploadSuccess = false;
bool webAuthenticated = false; // para la web
String uploadMessage = "";


void saveConfig() {
  // IMPORTANTE: Inicializar LittleFS antes de cada operación
  if (!LittleFS.begin()) {
    Serial.println("[CONFIG] Error: No se pudo montar LittleFS para guardar");
    return;
  }

  File f = LittleFS.open("/config.json", "w");
  if (f) {
    // Aumentar el tamaño del documento para acomodar todos los campos
    StaticJsonDocument<3600> doc;

    // Guardar TODOS los campos de configuración
    doc["threshold1"] = config.threshold1;
    doc["threshold2"] = config.threshold2;
    doc["irrigationMode"] = config.irrigationMode;
    doc["aiThreshold1"] = config.aiThreshold1;
    doc["aiThreshold2"] = config.aiThreshold2;
    doc["inferenceThresholdHigh"] = config.inferenceThresholdHigh;
    doc["inferenceThresholdLow"] = config.inferenceThresholdLow;
    doc["offsetTemp"] = config.offsetTemp;
    doc["offsetHum"] = config.offsetHum;
    doc["offsetPressure"] = config.offsetPressure;
    doc["offsetAirQuality"] = config.offsetAirQuality;
    doc["offsetSoil1"] = config.offsetSoil1;
    doc["offsetSoil2"] = config.offsetSoil2;
    doc["offsetLight"] = config.offsetLight;
    doc["lightProtection"] = config.lightProtection;
    doc["lightThreshold"] = config.lightThreshold;
    doc["waterSupplyControl"] = config.waterSupplyControl;
    doc["tempProtection"] = config.tempProtection;
    doc["minTempThreshold"] = config.minTempThreshold;
    doc["maxTempThreshold"] = config.maxTempThreshold;
    doc["humidityProtection"] = config.humidityProtection;
    doc["humidityThreshold"] = config.humidityThreshold;
    // Campos de weather
    doc["weatherGuard"] = config.weatherGuard;
    doc["weatherProvider"] = config.weatherProvider;
    doc["weatherApiKey"] = config.weatherApiKey;
    doc["weatherCity"] = config.weatherCity;
    doc["weatherRainThresholdHours"] = config.weatherRainThresholdHours;
    doc["lastWeatherCheck"] = config.lastWeatherCheck;
    doc["maxRainExpected"] = config.maxRainExpected;
    // Campos de hora
    doc["gmtOffset"] = config.gmtOffset;
    doc["daylightSaving"] = config.daylightSaving;
    doc["timeFormat24h"] = config.timeFormat24h;
    // Campos de Telegram
    doc["telegramToken"] = config.telegramToken;
    doc["chatID"] = config.chatID;
    // Campos VPD
    doc["vpdProtection"] = config.vpdProtection;
    doc["vpdEnabled"] = config.vpdEnabled;
    doc["vpdMinThreshold"] = config.vpdMinThreshold;
    doc["vpdMaxThreshold"] = config.vpdMaxThreshold;
    doc["vpdCriticalHigh"] = config.vpdCriticalHigh;
    doc["vpdCriticalLow"] = config.vpdCriticalLow;
    doc["vpdOptimalLow"] = config.vpdOptimalLow;
    doc["vpdOptimalHigh"] = config.vpdOptimalHigh;
    doc["vpdFactorCriticalHigh"] = config.vpdFactorCriticalHigh;
    doc["vpdFactorCriticalLow"] = config.vpdFactorCriticalLow;
    doc["vpdFactorOptimal"] = config.vpdFactorOptimal;
    doc["vpdFactorNormal"] = config.vpdFactorNormal;
    doc["tempFactorHigh"] = config.tempFactorHigh;
    doc["tempFactorLow"] = config.tempFactorLow;
    doc["tempThresholdHigh"] = config.tempThresholdHigh;
    doc["tempThresholdLow"] = config.tempThresholdLow;
    // Campos de calibración de sustratos
    doc["soilProfile"] = config.soilProfile;
    doc["advancedCalibration"] = config.advancedCalibration;
    doc["universalCalibration"] = config.universalCalibration;
    doc["clayCalibration"] = config.clayCalibration;
    doc["sandyCalibration"] = config.sandyCalibration;
    doc["loamCalibration"] = config.loamCalibration;
    doc["peatCalibration"] = config.peatCalibration;
    doc["cocoCalibration"] = config.cocoCalibration;
    doc["rockwoolCalibration"] = config.rockwoolCalibration;
    doc["perliteCalibration"] = config.perliteCalibration;
    doc["vermiculiteCalibration"] = config.vermiculiteCalibration;
    // Configuración de red
    doc["useDHCP"] = config.useDHCP;
    doc["staticIP"] = config.staticIP;
    doc["staticGateway"] = config.staticGateway;
    doc["staticSubnet"] = config.staticSubnet;
    doc["staticDNS1"] = config.staticDNS1;
    doc["staticDNS2"] = config.staticDNS2;
    // Campos de calibración de sensores de suelo
    doc["soil1Cal_type"] = config.soil1Cal.calibrationType;
    doc["soil1Cal_manual"] = config.soil1Cal.isManuallyCalibrated;
    doc["soil1Cal_dryADC"] = config.soil1Cal.dryPointADC;
    doc["soil1Cal_wetADC"] = config.soil1Cal.wetPointADC;
    doc["soil2Cal_type"] = config.soil2Cal.calibrationType;
    doc["soil2Cal_manual"] = config.soil2Cal.isManuallyCalibrated;
    doc["soil2Cal_dryADC"] = config.soil2Cal.dryPointADC;
    doc["soil2Cal_wetADC"] = config.soil2Cal.wetPointADC;
    doc["soil1Cal_isInverse"] = config.soil1Cal.isInverseSensor;
    doc["soil2Cal_isInverse"] = config.soil2Cal.isInverseSensor;
    //Restricciones normativas
    doc["timeRestrictionEnabled"] = config.timeRestrictionEnabled;
    doc["restrictionStartTime"] = config.restrictionStartTime;
    doc["restrictionEndTime"] = config.restrictionEndTime;
    doc["windRestrictionEnabled"] = config.windRestrictionEnabled;
    doc["maxWindSpeed"] = config.maxWindSpeed;
    // AUTENTICACIÓN WEB - Guardar en archivo
    doc["authEnabled"] = config.authEnabled;
    doc["webUser"] = config.webUser;
    doc["webPassword"] = config.webPassword;
    doc["sessionTimeout"] = config.sessionTimeout;
    
    // Serializar y escribir
    size_t bytesWritten = serializeJson(doc, f);

    // IMPORTANTE: Hacer flush para asegurar que se escriba en flash
    f.flush();
    f.close();

    // Verificar que se escribió correctamente
    Serial.print("[CONFIG] Configuración guardada: ");
    Serial.print(bytesWritten);
    Serial.println(" bytes");

    // Agregar log para el usuario
    addSerialLog("[CONFIG] ✓ Config guardada (" + String(bytesWritten) + " bytes)");
  } else {
    Serial.println("[CONFIG] Error: No se pudo abrir config.json para escritura");
    addSerialLog("[CONFIG] ❌ Error guardando config");
  }

  // Cerrar LittleFS correctamente
  LittleFS.end();
}

void loadConfig() {
  if (!LittleFS.begin()) {
    Serial.println("[CONFIG] LittleFS no montado, formateando...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("[CONFIG] Error crítico: No se puede montar LittleFS");
      return;
    }
  }

  // ===========================
  // DEBUG CONFIGURACIÓN EN PSRAM DE TELEGRAM
  // ===========================
  if (config.telegramToken.length() > 0 && telegramPSRAM.telegramToken) {
    strncpy(telegramPSRAM.telegramToken, config.telegramToken.c_str(), 99);
    telegramPSRAM.telegramToken[99] = '\0';
    Serial.println("[CONFIG] ✅ Token cargado en PSRAM");
  } else {
    Serial.println("[CONFIG] ❌ No se pudo cargar token en PSRAM");
  }
  //===========================================================


  File f = LittleFS.open("/config.json", "r");
  if (f) {
    // Usar el mismo tamaño que en saveConfig
    StaticJsonDocument<3600> doc;
    DeserializationError error = deserializeJson(doc, f);

    if (error) {
      Serial.print("[CONFIG] Error deserializando config: ");
      Serial.println(error.c_str());
      f.close();
      LittleFS.end();
      saveConfig();  // Crear archivo con valores por defecto
      return;
    }

    // Cargar todos los valores con valores por defecto si no existen
    config.threshold1 = doc["threshold1"] | 30;
    config.threshold2 = doc["threshold2"] | 30;
    config.irrigationMode = doc["irrigationMode"] | 1;
    config.aiThreshold1 = doc["aiThreshold1"] | 25;
    config.aiThreshold2 = doc["aiThreshold2"] | 25;
    config.inferenceThresholdHigh = doc["inferenceThresholdHigh"] | 0.7;
    config.inferenceThresholdLow = doc["inferenceThresholdLow"] | 0.3;
    config.offsetTemp = doc["offsetTemp"] | 0.0f;
    config.offsetHum = doc["offsetHum"] | 0.0f;
    config.offsetPressure = doc["offsetPressure"] | 0.0f;
    config.offsetAirQuality = doc["offsetAirQuality"] | 0.0f;
    config.offsetSoil1 = doc["offsetSoil1"] | 0.0f;
    config.offsetSoil2 = doc["offsetSoil2"] | 0.0f;
    config.offsetLight = doc["offsetLight"] | 0.0f;
    config.lightProtection = doc["lightProtection"] | true;
    config.lightThreshold = doc["lightThreshold"] | 70;
    config.waterSupplyControl = doc["waterSupplyControl"] | true;
    config.tempProtection = doc["tempProtection"] | true;
    config.minTempThreshold = doc["minTempThreshold"] | 5.0f;
    config.maxTempThreshold = doc["maxTempThreshold"] | 35.0f;
    config.humidityProtection = doc["humidityProtection"] | true;
    config.humidityThreshold = doc["humidityThreshold"] | 80.0f;
    // Campos de Weather
    config.weatherGuard = doc["weatherGuard"] | true;
    config.weatherProvider = doc["weatherProvider"] | String("openweathermap");
    config.weatherApiKey = doc["weatherApiKey"] | String("");
    config.weatherCity = doc["weatherCity"] | String("");
    config.weatherRainThresholdHours = doc["weatherRainThresholdHours"] | 4;
    config.lastWeatherCheck = doc["lastWeatherCheck"] | 0;
    config.maxRainExpected = doc["maxRainExpected"] | 0.0f;
    // Campos de hora
    config.gmtOffset = doc["gmtOffset"] | 1;
    config.daylightSaving = doc["daylightSaving"] | true;
    config.timeFormat24h = doc["timeFormat24h"] | true;
    // Campos de Telegram
    config.telegramToken = doc["telegramToken"] | "";
    config.chatID = doc["chatID"] | "";
    // Campos de VPD
    config.vpdProtection = doc["vpdProtection"] | true;
    config.vpdEnabled = doc["vpdEnabled"] | true;
    config.vpdMinThreshold = doc["vpdMinThreshold"] | 0.4f;
    config.vpdMaxThreshold = doc["vpdMaxThreshold"] | 1.5f;
    config.vpdCriticalHigh = doc["vpdCriticalHigh"] | 1.8f;
    config.vpdCriticalLow = doc["vpdCriticalLow"] | 0.3f;
    config.vpdOptimalLow = doc["vpdOptimalLow"] | 0.8f;
    config.vpdOptimalHigh = doc["vpdOptimalHigh"] | 1.2f;
    config.vpdFactorCriticalHigh = doc["vpdFactorCriticalHigh"] | 1.5f;
    config.vpdFactorCriticalLow = doc["vpdFactorCriticalLow"] | 0.3f;
    config.vpdFactorOptimal = doc["vpdFactorOptimal"] | 1.2f;
    config.vpdFactorNormal = doc["vpdFactorNormal"] | 1.0f;
    config.tempFactorHigh = doc["tempFactorHigh"] | 1.3f;
    config.tempFactorLow = doc["tempFactorLow"] | 0.7f;
    config.tempThresholdHigh = doc["tempThresholdHigh"] | 28.0f;
    config.tempThresholdLow = doc["tempThresholdLow"] | 18.0f;
    // Campos de calibración de sustratos
    config.soilProfile = doc["soilProfile"] | "universal";
    config.advancedCalibration = doc["advancedCalibration"] | false;
    config.universalCalibration = doc["universalCalibration"] | 1.0f;
    config.clayCalibration = doc["clayCalibration"] | 1.35f;
    config.sandyCalibration = doc["sandyCalibration"] | 0.75f;
    config.loamCalibration = doc["loamCalibration"] | 1.1f;
    config.peatCalibration = doc["peatCalibration"] | 1.25f;
    config.cocoCalibration = doc["cocoCalibration"] | 0.95f;
    config.rockwoolCalibration = doc["rockwoolCalibration"] | 0.85f;
    config.perliteCalibration = doc["perliteCalibration"] | 0.7f;
    config.vermiculiteCalibration = doc["vermiculiteCalibration"] | 1.4f;

    //Configuración de red
    config.useDHCP = doc["useDHCP"] | true;
    config.staticIP = doc["staticIP"] | "192.168.1.100";
    config.staticGateway = doc["staticGateway"] | "192.168.1.1";
    config.staticSubnet = doc["staticSubnet"] | "255.255.255.0";
    config.staticDNS1 = doc["staticDNS1"] | "8.8.8.8";
    config.staticDNS2 = doc["staticDNS2"] | "8.8.4.4";

    // Cargar calibración de sensores de suelo
    config.soil1Cal.calibrationType = doc.containsKey("soil1Cal_type") ? (SoilSensorCalibration::CalType)doc["soil1Cal_type"].as<int>() : SoilSensorCalibration::AUTO_BY_SUBSTRATE;
    config.soil1Cal.isManuallyCalibrated = doc["soil1Cal_manual"] | false;
    config.soil1Cal.dryPointADC = doc["soil1Cal_dryADC"] | 4095;
    config.soil1Cal.wetPointADC = doc["soil1Cal_wetADC"] | 0;
    config.soil2Cal.calibrationType = doc.containsKey("soil2Cal_type") ? (SoilSensorCalibration::CalType)doc["soil2Cal_type"].as<int>() : SoilSensorCalibration::AUTO_BY_SUBSTRATE;
    config.soil2Cal.isManuallyCalibrated = doc["soil2Cal_manual"] | false;
    config.soil2Cal.dryPointADC = doc["soil2Cal_dryADC"] | 4095;
    config.soil2Cal.wetPointADC = doc["soil2Cal_wetADC"] | 0;
    config.soil1Cal.isInverseSensor = doc["soil1Cal_isInverse"] | true;
    config.soil2Cal.isInverseSensor = doc["soil2Cal_isInverse"] | true;

    //Restricciones normativas
    config.timeRestrictionEnabled = doc["timeRestrictionEnabled"] | false;
    config.restrictionStartTime = doc["restrictionStartTime"] | String("08:00");
    config.restrictionEndTime = doc["restrictionEndTime"] | String("22:00");
    config.windRestrictionEnabled = doc["windRestrictionEnabled"] | false;
    config.maxWindSpeed = doc["maxWindSpeed"] | 18.0f;

    // Autenticación de la web
    config.authEnabled = doc["authEnabled"] | true;
    strlcpy(config.webUser, doc["webUser"] | "admin", sizeof(config.webUser));
    strlcpy(config.webPassword, doc["webPassword"] | "admin123", sizeof(config.webPassword));
    config.sessionTimeout = doc["sessionTimeout"] | 30;


    
    f.close();
    LittleFS.end();

    Serial.println("[CONFIG] Configuración cargada correctamente");
    Serial.println("[CONFIG] - Auto Mode: " + String(config.irrigationMode));
    Serial.println("[CONFIG] - Threshold1: " + String(config.threshold1));
    Serial.println("[CONFIG] - Threshold2: " + String(config.threshold2));
    Serial.println("[CONFIG] - GMT Offset: " + String(config.gmtOffset));
    Serial.println("[CONFIG] - Weather Guard: " + String(config.weatherGuard));
    Serial.println("[CONFIG] - Telegram Token: " + String(config.telegramToken.length() > 0 ? "Configurado" : "No configurado"));
    Serial.println("[CONFIG] ✓ Configuración de red cargada:");
    Serial.println("[CONFIG]   - useDHCP: " + String(config.useDHCP ? "Sí" : "No"));
    Serial.println("[CONFIG]   - staticIP: " + config.staticIP);
    Serial.println("[CONFIG]   - Gateway: " + config.staticGateway);

  } else {  // ← Este 'else' corresponde al 'if (f)' original
    Serial.println("[CONFIG] No existe config.json, creando con valores por defecto");
    saveConfig();
  }

  Serial.println("[CONFIG] Valores de calibración cargados:");
  Serial.printf("[CONFIG] Sensor 1: Dry=%d, Wet=%d, Manual=%s\n",
                config.soil1Cal.dryPointADC, config.soil1Cal.wetPointADC,
                config.soil1Cal.isManuallyCalibrated ? "YES" : "NO");
  Serial.printf("[CONFIG] Sensor 2: Dry=%d, Wet=%d, Manual=%s\n",
                config.soil2Cal.dryPointADC, config.soil2Cal.wetPointADC,
                config.soil2Cal.isManuallyCalibrated ? "YES" : "NO");
  // ===========================
  // CARGAR CONFIGURACIÓN EN PSRAM (DESPUÉS DE LittleFS)
  // ===========================
  if (config.telegramToken.length() > 0 && telegramPSRAM.telegramToken) {
    strncpy(telegramPSRAM.telegramToken, config.telegramToken.c_str(), 99);
    telegramPSRAM.telegramToken[99] = '\0';
    Serial.println("[CONFIG] ✅ Token cargado en PSRAM");
  }

  if (config.chatID.length() > 0 && telegramPSRAM.chatID) {
    strncpy(telegramPSRAM.chatID, config.chatID.c_str(), 49);
    telegramPSRAM.chatID[49] = '\0';
    Serial.println("[CONFIG] ✅ ChatID cargado en PSRAM");
  }

}  

// ===========================
// VALIDACIÓN DE DIRECCIONES IP (Usadas en handleSetConfig();)
// ===========================
bool isValidIP(const String& ip) {
  IPAddress testIP;
  return testIP.fromString(ip);
}

bool isValidSubnet(const String& subnet) {
  IPAddress testSubnet;
  if (!testSubnet.fromString(subnet)) return false;

  // Validar que sea una máscara de red válida
  uint32_t mask = (uint32_t)testSubnet;
  uint32_t neg = ~mask;
  return ((neg + 1) & neg) == 0;  // Debe ser una máscara contigua
}


void handleSetConfig() {
  bool configChanged = false;
  bool weatherConfigChanged = false;
  bool ntpReconfigure = false;
  bool networkConfigChanged = false;

  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(500)) == pdTRUE) {
    
    // ===========================
    // RESTRICCIONES NORMATIVAS
    // ===========================
    if (server.hasArg("timeRestriction")) {
      config.timeRestrictionEnabled = server.arg("timeRestriction").toInt();
      configChanged = true;
    }
    if (server.hasArg("startTime")) {
      config.restrictionStartTime = server.arg("startTime");
      configChanged = true;
    }
    if (server.hasArg("endTime")) {
      config.restrictionEndTime = server.arg("endTime");
      configChanged = true;
    }
    if (server.hasArg("windRestriction")) {
      config.windRestrictionEnabled = server.arg("windRestriction").toInt();
      configChanged = true;
    }
    if (server.hasArg("maxWind")) {
      config.maxWindSpeed = constrain(server.arg("maxWind").toFloat(), 0.0, 100.0);
      configChanged = true;
    }
    
    // ===========================
    // UMBRALES DE RIEGO
    // ===========================
    if (server.hasArg("th1")) {
      config.threshold1 = constrain(server.arg("th1").toInt(), 0, 100);
      configChanged = true;
    }
    if (server.hasArg("th2")) {
      config.threshold2 = constrain(server.arg("th2").toInt(), 0, 100);
      configChanged = true;
    }
    if (server.hasArg("lt")) {
      config.lightThreshold = constrain(server.arg("lt").toInt(), 0, 100);
      configChanged = true;
    }
    
    // ===========================
    // PROTECCIONES AMBIENTALES
    // ===========================
    if (server.hasArg("minTemp")) {
      config.minTempThreshold = server.arg("minTemp").toFloat();
      configChanged = true;
    }
    if (server.hasArg("maxTemp")) {
      config.maxTempThreshold = server.arg("maxTemp").toFloat();
      configChanged = true;
    }
    if (server.hasArg("humidityThreshold")) {
      config.humidityThreshold = server.arg("humidityThreshold").toFloat();
      configChanged = true;
    }
    
    // ===========================
    // CALIBRACIÓN DE SUELO
    // ===========================
    if (server.hasArg("soilProfile")) {
      config.soilProfile = server.arg("soilProfile");
      configChanged = true;
    }
    if (server.hasArg("advancedCal")) {
      config.advancedCalibration = server.arg("advancedCal").toInt();
      configChanged = true;
    }
    
    // ===========================
    // OFFSETS DE SENSORES
    // ===========================
    if (server.hasArg("s1")) {
      config.offsetSoil1 = constrain(server.arg("s1").toFloat(), -50.0, 50.0);
      configChanged = true;
      Serial.printf("[CONFIG] Offset Soil1 actualizado: %.2f\n", config.offsetSoil1);
    }
    if (server.hasArg("s2")) {
      config.offsetSoil2 = constrain(server.arg("s2").toFloat(), -50.0, 50.0);
      configChanged = true;
      Serial.printf("[CONFIG] Offset Soil2 actualizado: %.2f\n", config.offsetSoil2);
    }
    if (server.hasArg("t")) {
      config.offsetTemp = constrain(server.arg("t").toFloat(), -10.0, 10.0);
      configChanged = true;
    }
    if (server.hasArg("h")) {
      config.offsetHum = constrain(server.arg("h").toFloat(), -20.0, 20.0);
      configChanged = true;
    }
    if (server.hasArg("p")) {
      config.offsetPressure = constrain(server.arg("p").toFloat(), -50.0, 50.0);
      configChanged = true;
      Serial.printf("[CONFIG] Offset Pressure actualizado: %.2f\n", config.offsetPressure);
    }
    if (server.hasArg("aq")) {
      config.offsetAirQuality = constrain(server.arg("aq").toFloat(), -100.0, 100.0);
      configChanged = true;
      Serial.printf("[CONFIG] Offset AirQuality actualizado: %.2f\n", config.offsetAirQuality);
    }
    if (server.hasArg("l")) {
      config.offsetLight = constrain(server.arg("l").toFloat(), -20.0, 20.0);
      configChanged = true;
    }
    
    // ===========================
    // PROTECCIÓN VPD
    // ===========================
    if (server.hasArg("vpdProt")) {
      config.vpdProtection = server.arg("vpdProt").toInt();
      configChanged = true;
    }
    if (server.hasArg("vpdMin")) {
      config.vpdMinThreshold = constrain(server.arg("vpdMin").toFloat(), 0.1, 2.0);
      configChanged = true;
    }
    if (server.hasArg("vpdMax")) {
      config.vpdMaxThreshold = constrain(server.arg("vpdMax").toFloat(), 0.5, 3.0);
      configChanged = true;
    }
    
    // ===========================
    // CONFIGURACIÓN METEOROLÓGICA
    // ===========================
    if (server.hasArg("wg")) {
      bool newValue = server.arg("wg").toInt();
      if (config.weatherGuard != newValue) {
        config.weatherGuard = newValue;
        configChanged = true;
        weatherConfigChanged = true;
      }
    }
    if (server.hasArg("wp")) {
      String newValue = server.arg("wp");
      if (config.weatherProvider != newValue) {
        config.weatherProvider = newValue;
        configChanged = true;
        weatherConfigChanged = true;
      }
    }
    if (server.hasArg("wak")) {
      String newValue = server.arg("wak");
      if (config.weatherApiKey != newValue) {
        config.weatherApiKey = newValue;
        configChanged = true;
        weatherConfigChanged = true;
      }
    }
    if (server.hasArg("wc")) {
      String newValue = server.arg("wc");
      if (config.weatherCity != newValue) {
        config.weatherCity = newValue;
        configChanged = true;
        weatherConfigChanged = true;
      }
    }
    if (server.hasArg("wrth")) {
      uint8_t newValue = constrain(server.arg("wrth").toInt(), 1, 48);
      if (config.weatherRainThresholdHours != newValue) {
        config.weatherRainThresholdHours = newValue;
        configChanged = true;
        weatherConfigChanged = true;
      }
    }
    
  // ===========================
  // TELEGRAM - ACTUALIZAR PSRAM
  // ===========================
  if (server.hasArg("tg_token")) {
  String newToken = server.arg("tg_token");
  config.telegramToken = newToken;
  
  // Actualizar buffer PSRAM
  if (telegramPSRAM.telegramToken) {
    strncpy(telegramPSRAM.telegramToken, newToken.c_str(), 99);
    telegramPSRAM.telegramToken[99] = '\0'; // Null-terminate
    Serial.println("[CONFIG] ✅ Token Telegram actualizado en PSRAM");
  }
  configChanged = true;
  }

  if (server.hasArg("tg_chatid")) {
  String newChatID = server.arg("tg_chatid");
  config.chatID = newChatID;
  
  // Actualizar buffer PSRAM
  if (telegramPSRAM.chatID) {
    strncpy(telegramPSRAM.chatID, newChatID.c_str(), 49);
    telegramPSRAM.chatID[49] = '\0'; // Null-terminate
    Serial.println("[CONFIG] ✅ ChatID actualizado en PSRAM");
  }
  configChanged = true;
  }

    bool pumpNotifications = server.hasArg("tg_pump");
    bool alertNotifications = server.hasArg("tg_alert");
    (void)pumpNotifications;
    (void)alertNotifications;
    
    // ===========================
    // CONFIGURACIÓN DE HORA
    // ===========================
    if (server.hasArg("gmt")) {
      config.gmtOffset = constrain(server.arg("gmt").toInt(), -12, 12);
      configChanged = true;
      ntpReconfigure = true;
    }
    if (server.hasArg("dst")) {
      config.daylightSaving = server.arg("dst").toInt();
      configChanged = true;
      ntpReconfigure = true;
    }
    if (server.hasArg("tf")) {
      config.timeFormat24h = server.arg("tf").toInt();
      configChanged = true;
    }
    
    // ===========================
    // CONFIGURACIÓN DE RED
    // ===========================
    if (server.hasArg("useDHCP")) {
      bool newUseDHCP = server.arg("useDHCP").toInt();
      if (config.useDHCP != newUseDHCP) {
        config.useDHCP = newUseDHCP;
        configChanged = true;
        networkConfigChanged = true;
        Serial.println("[CONFIG] Modo DHCP: " + String(config.useDHCP ? "Activado" : "Desactivado"));
      }
    }
    if (server.hasArg("staticIP")) {
      String newIP = server.arg("staticIP");
      if (config.staticIP != newIP && isValidIP(newIP)) {
        config.staticIP = newIP;
        configChanged = true;
        networkConfigChanged = true;
        Serial.println("[CONFIG] IP estática: " + config.staticIP);
      }
    }
    if (server.hasArg("staticGateway")) {
      String newGateway = server.arg("staticGateway");
      if (config.staticGateway != newGateway && isValidIP(newGateway)) {
        config.staticGateway = newGateway;
        configChanged = true;
        networkConfigChanged = true;
        Serial.println("[CONFIG] Gateway: " + config.staticGateway);
      }
    }
    if (server.hasArg("staticSubnet")) {
      String newSubnet = server.arg("staticSubnet");
      if (config.staticSubnet != newSubnet && isValidSubnet(newSubnet)) {
        config.staticSubnet = newSubnet;
        configChanged = true;
        networkConfigChanged = true;
        Serial.println("[CONFIG] Subnet: " + config.staticSubnet);
      }
    }
    if (server.hasArg("staticDNS1")) {
      String newDNS1 = server.arg("staticDNS1");
      if (config.staticDNS1 != newDNS1 && (newDNS1.length() == 0 || isValidIP(newDNS1))) {
        config.staticDNS1 = newDNS1;
        configChanged = true;
        networkConfigChanged = true;
        Serial.println("[CONFIG] DNS1: " + config.staticDNS1);
      }
    }
    if (server.hasArg("staticDNS2")) {
      String newDNS2 = server.arg("staticDNS2");
      if (config.staticDNS2 != newDNS2 && (newDNS2.length() == 0 || isValidIP(newDNS2))) {
        config.staticDNS2 = newDNS2;
        configChanged = true;
        networkConfigChanged = true;
        Serial.println("[CONFIG] DNS2: " + config.staticDNS2);
      }
    }
    
    // ===========================
    // UMBRALES DE INFERENCIA INTELIGENTE
    // ===========================
    if (server.hasArg("inf_high")) {
      int highValue = constrain(server.arg("inf_high").toInt(), 50, 90);
      config.inferenceThresholdHigh = highValue / 100.0;  // Convertir 50-90 a 0.5-0.9
      configChanged = true;
      Serial.println("[CONFIG] Umbral inferencia ALTO: " + String(config.inferenceThresholdHigh * 100) + "%");
      addSerialLog("[CONFIG] Umbral IA Alto: " + String(highValue) + "%");
    }
    
    if (server.hasArg("inf_low")) {
      int lowValue = constrain(server.arg("inf_low").toInt(), 10, 50);
      config.inferenceThresholdLow = lowValue / 100.0;  // Convertir 10-50 a 0.1-0.5
      configChanged = true;
      Serial.println("[CONFIG] Umbral inferencia BAJO: " + String(config.inferenceThresholdLow * 100) + "%");
      addSerialLog("[CONFIG] Umbral IA Bajo: " + String(lowValue) + "%");
    }
    
    // Validar que HIGH siempre sea mayor que LOW (mínimo 20% de diferencia)
    if (config.inferenceThresholdHigh <= config.inferenceThresholdLow) {
      Serial.println("[WARNING] Umbrales inválidos, ajustando...");
      config.inferenceThresholdHigh = config.inferenceThresholdLow + 0.2;
      addSerialLog("[CONFIG] ⚠️ Umbrales ajustados automáticamente");
    }
    
    // ===========================
    // AUTENTICACIÓN WEB
    // ===========================
    if (server.hasArg("authEnabled")) {
      config.authEnabled = (server.arg("authEnabled") == "1");
      configChanged = true;
      Serial.printf("[CONFIG] Autenticación web: %s\n", config.authEnabled ? "ACTIVADA" : "DESACTIVADA");
    }
    
    if (server.hasArg("webUser")) {
      String newUser = server.arg("webUser");
      if (newUser.length() > 0 && newUser.length() < 32) {
        strlcpy(config.webUser, newUser.c_str(), sizeof(config.webUser));
        configChanged = true;
        Serial.printf("[CONFIG] Usuario web: %s\n", config.webUser);
      }
    }
    
    if (server.hasArg("webPassword")) {
      String newPass = server.arg("webPassword");
      // Solo cambiar si NO está vacío (permite no cambiar contraseña)
      if (newPass.length() > 0) {
        if (newPass.length() >= 4 && newPass.length() < 64) {
          strlcpy(config.webPassword, newPass.c_str(), sizeof(config.webPassword));
          configChanged = true;
          webAuthenticated = false;  // Forzar nuevo login con nueva contraseña
          Serial.println("[CONFIG] ✅ Contraseña web actualizada");
        }
      }
    }
    
    if (server.hasArg("sessionTimeout")) {
      int timeout = server.arg("sessionTimeout").toInt();
      if (timeout >= 5 && timeout <= 1440) {
        config.sessionTimeout = timeout;
        configChanged = true;
      }
    }
    
    // ===========================
    // GUARDAR Y PROCESAR CAMBIOS
    // ===========================
    if (configChanged) {
      saveConfig();
      Serial.println("[CONFIG] Configuración actualizada y guardada");
      addSerialLog("[CONFIG] ✓ Ajustes aplicados");

      if (weatherConfigChanged) {
        config.weatherUpdateRequested = true;
        Serial.println("[CONFIG] Solicitando actualización meteorológica inmediata");
        addSerialLog("[WEATHER] Actualización programada");
      }
      
      if (ntpReconfigure && wifiConnected) {
        configureNTP();
      }
      
      if (networkConfigChanged && wifiConnected) {
        Serial.println("[CONFIG] 🔄 Reconfigurando conexión de red...");
        configureNetwork();
        Serial.println("[CONFIG] 📊 Nueva configuración de red:");
        Serial.println("[CONFIG]   - IP: " + WiFi.localIP().toString());
        Serial.println("[CONFIG]   - Gateway: " + WiFi.gatewayIP().toString());
        Serial.println("[CONFIG] ✅ Configuración de red actualizada");
        addSerialLog("[CONFIG] ✓ Ajustes de red aplicados");
      }
    }
    
    xSemaphoreGive(mutexConfig);
    
  } else {
    Serial.println("[CONFIG] ERROR: No se pudo obtener mutex de configuración");
    addSerialLog("[CONFIG] ❌ Error accediendo configuración");
  }

  // Redireccionar a la página principal
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleConfigPortal() {
  String html = R"=====(
  <!DOCTYPE html>
  <html lang="es">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuración WiFi - Sistema Riego</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
        :root {
            --primary: #6366f1;
            --primary-dark: #4f46e5;
            --secondary: #06b6d4;
            --success: #10b981;
            --dark: #1f2937;
            --light: #f8fafc;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        body {
            background: linear-gradient(135deg, #1e293b 0%, #0f172a 100%);
            color: #f8fafc;
            min-height: 100vh;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }

        .container {
            background: linear-gradient(135deg, rgba(30, 41, 59, 0.9), rgba(15, 23, 42, 0.9));
            border-radius: 20px;
            padding: 25px;
            box-shadow: 10px 10px 20px rgba(0, 0, 0, 0.2), 
                       -10px -10px 20px rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            width: 100%;
            max-width: 45%;
        }

        .header {
            text-align: center;
            margin-bottom: 20px;
        }

        .header h1 {
            font-size: 1.8rem;
            background: linear-gradient(135deg, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 10px;
        }

        .header p {
            color: #94a3b8;
            font-size: 0.9rem;
        }

        .form-group {
            margin-bottom: 18px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            color: #e2e8f0;
            font-weight: 500;
            font-size: 0.9rem;
        }

        input {
            width: 100%;
            padding: 14px;
            border: none;
            border-radius: 12px;
            background: rgba(15, 23, 42, 0.6);
            color: #f8fafc;
            box-shadow: inset 3px 3px 5px rgba(0, 0, 0, 0.2), 
                       inset -3px -3px 5px rgba(255, 255, 255, 0.05);
            transition: all 0.3s ease;
            font-size: 16px; /* Mejor para móviles */
        }

        input:focus {
            outline: none;
            box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.3),
                       inset 3px 3px 5px rgba(0, 0, 0, 0.2);
            background: rgba(15, 23, 42, 0.8);
        }

        .btn {
            width: 100%;
            padding: 16px;
            border: none;
            border-radius: 12px;
            background: linear-gradient(135deg, var(--primary), var(--primary-dark));
            color: white;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 6px 6px 12px rgba(0, 0, 0, 0.25),
                       -6px -6px 12px rgba(255, 255, 255, 0.08);
            margin: 12px 0;
            font-size: 1rem;
        }

        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 8px 8px 20px rgba(0, 0, 0, 0.3);
        }

        .btn-scan {
            background: linear-gradient(135deg, var(--secondary), #0891b2);
        }

        .network-list {
            margin-top: 20px;
            max-height: 200px;
            overflow-y: auto;
        }

        .network-item {
            padding: 14px;
            margin: 8px 0;
            background: rgba(15, 23, 42, 0.6);
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: inset 2px 2px 4px rgba(0, 0, 0, 0.1);
            font-size: 0.9rem;
        }

        .network-item:hover {
            background: rgba(15, 23, 42, 0.8);
            transform: translateX(5px);
        }

        .signal-strength {
            float: right;
            font-size: 0.8rem;
        }

        .loading {
            text-align: center;
            padding: 20px;
            color: #94a3b8;
        }

        .footer {
            text-align: center;
            margin-top: 25px;
            color: #64748b;
            font-size: 0.8rem;
        }

        /* MEJORAS RESPONSIVE PARA MÓVIL */
        @media (max-width: 768px) {
            .container {
                padding: 20px;
                border-radius: 15px;
                max-width: 90%;
            }
            
            .header h1 {
                font-size: 1.5rem;
            }
            
            input {
                padding: 16px;
                font-size: 16px; /* Evita zoom en iOS */
            }
            
            .btn {
                padding: 18px;
                font-size: 1.1rem;
            }
            
            .network-item {
                padding: 16px;
                font-size: 1rem;
            }
        }

        @media (max-width: 360px) {
            .header h1 {
                font-size: 1.3rem;
            }
            
            .header p {
                font-size: 0.8rem;
            }
            
            input, .btn {
                padding: 14px;
            }
        }
    </style>
  </head>
  <body>
    <div class="container">
        <div class="header">
            <h1><i class="fas fa-wifi"></i> Configuración WiFi</h1>
            <p>Conecta tu sistema de riego inteligente</p>
        </div>

        <form action='/save' method='POST'>
            <div class="form-group">
                <label for="ssid"><i class="fas fa-network-wired"></i> Nombre de la red WiFi</label>
                <input type="text" id="ssid" name="ssid" placeholder="Entra el SSID" required>
            </div>
            
            <div class="form-group">
                <label for="password"><i class="fas fa-key"></i> Contraseña</label>
                <input type="password" id="password" name="password" placeholder="Escribe la contraseña">
            </div>

            <button type="submit" class="btn">
                <i class="fas fa-save"></i> Guardar y Conectar
            </button>
        </form>

        <button class="btn btn-scan" onclick="scanNetworks()">
            <i class="fas fa-search"></i> Escanear Redes Disponibles
        </button>

        <div class="network-list" id="networks">
            <!-- Las redes aparecerán aquí -->
        </div>

        <div class="footer">
            <p>🌿 Sistema Riego Pro v1.0</p>
            <p>IP: 192.168.4.1</p>
            <p>Conectado a: RiegoIoT-Access</p>
        </div>
    </div>

    <script>
        function scanNetworks() {
            const networksDiv = document.getElementById('networks');
            networksDiv.innerHTML = '<div class="loading"><i class="fas fa-spinner fa-spin"></i> Buscando redes disponibles...</div>';
            
            fetch('/scan')
                .then(response => response.text())
                .then(data => {
                    networksDiv.innerHTML = data;
                })
                .catch(error => {
                    networksDiv.innerHTML = '<div class="loading">Error al escanear redes</div>';
                });
        }

        function selectNetwork(ssid) {
            document.getElementById('ssid').value = ssid;
            document.getElementById('password').focus();
            
            const networks = document.querySelectorAll('.network-item');
            networks.forEach(net => net.style.background = 'rgba(15, 23, 42, 0.6)');
            event.target.closest('.network-item').style.background = 'rgba(99, 102, 241, 0.2)';
        }
    </script>
  </body>
  </html>
  )=====";

  configServer.send(200, "text/html", html);
}

void handleExportConfig() {
  Serial.println("[CONFIG] Exportando configuración COMPLETA con aprendizaje...");
  
  if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(1000)) == pdTRUE) {
    if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(1000)) == pdTRUE) {
      
      DynamicJsonDocument doc(3600); // Aumentado para más datos
      
      // ===========================
      // METADATOS
      // ===========================
      doc["version"] = "2.1"; // Versión actualizada
      doc["exported_at"] = getFormattedTime();
      doc["device_mac"] = WiFi.macAddress();
      doc["uptime_minutes"] = millis() / 60000;
      
      // ===========================
      // CONFIGURACIÓN BÁSICA
      // ===========================
      doc["threshold1"] = config.threshold1;
      doc["threshold2"] = config.threshold2;
      doc["irrigationMode"] = config.irrigationMode;
      doc["aiThreshold1"] = config.aiThreshold1;
      doc["aiThreshold2"] = config.aiThreshold2;
      doc["inferenceThresholdHigh"] = config.inferenceThresholdHigh;
      doc["inferenceThresholdLow"] = config.inferenceThresholdLow;
      doc["offsetTemp"] = config.offsetTemp;
      doc["offsetHum"] = config.offsetHum;
      doc["offsetPressure"] = config.offsetPressure;
      doc["offsetAirQuality"] = config.offsetAirQuality;
      doc["offsetSoil1"] = config.offsetSoil1;
      doc["offsetSoil2"] = config.offsetSoil2;
      doc["offsetLight"] = config.offsetLight;
      doc["lightProtection"] = config.lightProtection;
      doc["lightThreshold"] = config.lightThreshold;
      doc["waterSupplyControl"] = config.waterSupplyControl;
      doc["tempProtection"] = config.tempProtection;
      doc["minTempThreshold"] = config.minTempThreshold;
      doc["maxTempThreshold"] = config.maxTempThreshold;
      doc["humidityProtection"] = config.humidityProtection;
      doc["humidityThreshold"] = config.humidityThreshold;
      doc["weatherGuard"] = config.weatherGuard;
      doc["weatherProvider"] = config.weatherProvider;
      doc["weatherApiKey"] = config.weatherApiKey;
      doc["weatherCity"] = config.weatherCity;
      doc["weatherRainThresholdHours"] = config.weatherRainThresholdHours;
      doc["gmtOffset"] = config.gmtOffset;
      doc["daylightSaving"] = config.daylightSaving;
      doc["timeFormat24h"] = config.timeFormat24h;
      doc["telegramToken"] = config.telegramToken;
      doc["chatID"] = config.chatID;
      
      // VPD
      doc["vpdProtection"] = config.vpdProtection;
      doc["vpdEnabled"] = config.vpdEnabled;
      doc["vpdMinThreshold"] = config.vpdMinThreshold;
      doc["vpdMaxThreshold"] = config.vpdMaxThreshold;
      doc["vpdCriticalHigh"] = config.vpdCriticalHigh;
      doc["vpdCriticalLow"] = config.vpdCriticalLow;
      doc["vpdOptimalLow"] = config.vpdOptimalLow;
      doc["vpdOptimalHigh"] = config.vpdOptimalHigh;
      doc["vpdFactorCriticalHigh"] = config.vpdFactorCriticalHigh;
      doc["vpdFactorCriticalLow"] = config.vpdFactorCriticalLow;
      doc["vpdFactorOptimal"] = config.vpdFactorOptimal;
      doc["vpdFactorNormal"] = config.vpdFactorNormal;
      doc["tempFactorHigh"] = config.tempFactorHigh;
      doc["tempFactorLow"] = config.tempFactorLow;
      doc["tempThresholdHigh"] = config.tempThresholdHigh;
      doc["tempThresholdLow"] = config.tempThresholdLow;
      
      // Sustratos
      doc["soilProfile"] = config.soilProfile;
      doc["advancedCalibration"] = config.advancedCalibration;
      
      // Red
      doc["useDHCP"] = config.useDHCP;
      doc["staticIP"] = config.staticIP;
      doc["staticGateway"] = config.staticGateway;
      doc["staticSubnet"] = config.staticSubnet;
      doc["staticDNS1"] = config.staticDNS1;
      doc["staticDNS2"] = config.staticDNS2;
      
      // Calibración sensores
      doc["soil1Cal_type"] = config.soil1Cal.calibrationType;
      doc["soil1Cal_manual"] = config.soil1Cal.isManuallyCalibrated;
      doc["soil1Cal_dryADC"] = config.soil1Cal.dryPointADC;
      doc["soil1Cal_wetADC"] = config.soil1Cal.wetPointADC;
      doc["soil2Cal_type"] = config.soil2Cal.calibrationType;
      doc["soil2Cal_manual"] = config.soil2Cal.isManuallyCalibrated;
      doc["soil2Cal_dryADC"] = config.soil2Cal.dryPointADC;
      doc["soil2Cal_wetADC"] = config.soil2Cal.wetPointADC;
      
      // Restricciones normativas
      doc["timeRestrictionEnabled"] = config.timeRestrictionEnabled;
      doc["restrictionStartTime"] = config.restrictionStartTime;
      doc["restrictionEndTime"] = config.restrictionEndTime;
      doc["windRestrictionEnabled"] = config.windRestrictionEnabled;
      doc["maxWindSpeed"] = config.maxWindSpeed;

      // AUTENTICACIÓN WEB - Guardar en archivo
      doc["authEnabled"] = config.authEnabled;
      doc["webUser"] = config.webUser;
      doc["webPassword"] = config.webPassword;
      doc["sessionTimeout"] = config.sessionTimeout;
            
      // ===========================
      // 🆕 DATOS DE APRENDIZAJE ZONA 1
      // ===========================
      JsonObject zone1 = doc.createNestedObject("learningZone1");
      zone1["avgIrrigationTime"] = learningZone1.avgIrrigationTime;
      zone1["avgMoistureRecovery"] = learningZone1.avgMoistureRecovery;
      zone1["avgEfficiency"] = learningZone1.avgEfficiency;
      zone1["successfulCycles"] = learningZone1.successfulCycles;
      zone1["totalCycles"] = learningZone1.totalCycles;
      zone1["efficiencyScore"] = learningZone1.efficiencyScore;
      zone1["lastOptimization"] = learningZone1.lastOptimization;
      
      // ===========================
      // 🆕 DATOS DE APRENDIZAJE ZONA 2
      // ===========================
      JsonObject zone2 = doc.createNestedObject("learningZone2");
      zone2["avgIrrigationTime"] = learningZone2.avgIrrigationTime;
      zone2["avgMoistureRecovery"] = learningZone2.avgMoistureRecovery;
      zone2["avgEfficiency"] = learningZone2.avgEfficiency;
      zone2["successfulCycles"] = learningZone2.successfulCycles;
      zone2["totalCycles"] = learningZone2.totalCycles;
      zone2["efficiencyScore"] = learningZone2.efficiencyScore;
      zone2["lastOptimization"] = learningZone2.lastOptimization;
      
      // ===========================
      // 🆕 CICLO ACTUAL ZONA 1
      // ===========================
      JsonObject cycle1 = doc.createNestedObject("currentCycle1");
      cycle1["startTime"] = currentCycle1.startTime;
      cycle1["initialMoisture"] = currentCycle1.initialMoisture;
      cycle1["finalMoisture"] = currentCycle1.finalMoisture;
      cycle1["vpdAtStart"] = currentCycle1.vpdAtStart;
      cycle1["tempAtStart"] = currentCycle1.tempAtStart;
      cycle1["completed"] = currentCycle1.completed;
      cycle1["efficiency"] = currentCycle1.efficiency;
      
      // ===========================
      // 🆕 CICLO ACTUAL ZONA 2
      // ===========================
      JsonObject cycle2 = doc.createNestedObject("currentCycle2");
      cycle2["startTime"] = currentCycle2.startTime;
      cycle2["initialMoisture"] = currentCycle2.initialMoisture;
      cycle2["finalMoisture"] = currentCycle2.finalMoisture;
      cycle2["vpdAtStart"] = currentCycle2.vpdAtStart;
      cycle2["tempAtStart"] = currentCycle2.tempAtStart;
      cycle2["completed"] = currentCycle2.completed;
      cycle2["efficiency"] = currentCycle2.efficiency;
      
      // ===========================
      // 🆕 HISTORIAL DE SENSORES
      // ===========================
      JsonObject sensorHist = doc.createNestedObject("sensorHistory");
      sensorHist["lastTemp"] = sensorHistory.lastTemp;
      sensorHist["lastHum"] = sensorHistory.lastHum;
      sensorHist["lastLight"] = sensorHistory.lastLight;
      sensorHist["lastUpdate"] = sensorHistory.lastUpdate;
      sensorHist["tempChangeRate"] = sensorHistory.tempChangeRate;
      sensorHist["humChangeRate"] = sensorHistory.humChangeRate;
      sensorHist["lightChangeRate"] = sensorHistory.lightChangeRate;
      
      // ===========================
      // 🆕 ESTADO INFERENCIA INTELIGENTE
      // ===========================
      doc["smartModeActive"] = data.smartModeActive;
      doc["lastIrrigation1"] = data.lastIrrigation1;
      doc["lastIrrigation2"] = data.lastIrrigation2;
      doc["irrigationNeed1"] = data.irrigationNeed1;
      doc["irrigationNeed2"] = data.irrigationNeed2;
      
      xSemaphoreGive(mutexData);
      xSemaphoreGive(mutexConfig);
      
      String jsonOutput;
      serializeJsonPretty(doc, jsonOutput);
      
      server.sendHeader("Content-Disposition", "attachment; filename=riego_backup_completo_" + String(millis()) + ".json");
      server.send(200, "application/json", jsonOutput);
      
      Serial.println("[CONFIG] ✅ Backup completo exportado con aprendizaje");
      Serial.printf("[CONFIG] Tamaño: %d bytes | Zonas: Z1=%d ciclos, Z2=%d ciclos\n", 
                    jsonOutput.length(), learningZone1.totalCycles, learningZone2.totalCycles);
      addSerialLog("[CONFIG] Backup completo creado");
      
    } else {
      xSemaphoreGive(mutexConfig);
      server.send(500, "text/plain", "Error: No se pudo acceder a los datos");
    }
    } else {
    server.send(500, "text/plain", "Error: No se pudo acceder a la configuración");
  }
}

void handleImportConfig() {
  String html = R"=====(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Restaurar Configuración</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #1e293b 0%, #0f172a 100%);
            color: #f8fafc;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }
        .container {
            background: rgba(30, 41, 59, 0.9);
            border-radius: 20px;
            padding: 30px;
            max-width: 600px;
            box-shadow: 10px 10px 20px rgba(0, 0, 0, 0.3);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }
        h1 {
            color: #6366f1;
            text-align: center;
            margin-bottom: 10px;
        }
        .subtitle {
            text-align: center;
            color: #94a3b8;
            margin-bottom: 30px;
        }
        .file-drop-zone {
            border: 3px dashed #6366f1;
            border-radius: 15px;
            padding: 40px;
            text-align: center;
            background: rgba(99, 102, 241, 0.05);
            cursor: pointer;
            transition: all 0.3s;
            margin: 20px 0;
        }
        .file-drop-zone:hover {
            background: rgba(99, 102, 241, 0.1);
            border-color: #818cf8;
            transform: scale(1.02);
        }
        .file-drop-zone.dragover {
            background: rgba(99, 102, 241, 0.2);
            border-color: #a5b4fc;
        }
        .file-icon {
            font-size: 4rem;
            color: #6366f1;
            margin-bottom: 15px;
        }
        .file-info {
            display: none;
            margin: 20px 0;
            padding: 15px;
            background: rgba(16, 185, 129, 0.1);
            border-radius: 10px;
            border-left: 4px solid #10b981;
        }
        .warning {
            background: rgba(239, 68, 68, 0.1);
            border-left: 4px solid #ef4444;
            padding: 15px;
            margin: 20px 0;
            border-radius: 8px;
        }
        .btn {
            width: 100%;
            padding: 15px;
            border: none;
            border-radius: 10px;
            font-weight: bold;
            cursor: pointer;
            margin: 10px 0;
            font-size: 1rem;
            transition: all 0.3s;
        }
        .btn-primary {
            background: linear-gradient(135deg, #6366f1, #4f46e5);
            color: white;
        }
        .btn-primary:hover:not(:disabled) {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(99, 102, 241, 0.4);
        }
        .btn-primary:disabled {
            background: #6b7280;
            cursor: not-allowed;
            opacity: 0.5;
        }
        .btn-secondary {
            background: transparent;
            border: 2px solid #6b7280;
            color: #6b7280;
        }
        .btn-secondary:hover {
            background: #6b7280;
            color: white;
        }
        input[type="file"] {
            display: none;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1><i class="fas fa-file-import"></i> Restaurar Configuración</h1>
        <p class="subtitle">Importa un archivo de backup para restaurar el sistema</p>
        
        <div class="warning">
            <strong><i class="fas fa-exclamation-triangle"></i> Advertencia:</strong> 
            Esto sobrescribirá TODA la configuración actual (excepto WiFi).
        </div>
        
        <form id="uploadForm" action="/upload" method="POST" enctype="multipart/form-data">
            <div class="file-drop-zone" id="dropZone" onclick="document.getElementById('fileInput').click()">
                <div class="file-icon">
                    <i class="fas fa-cloud-upload-alt"></i>
                </div>
                <p style="margin: 10px 0; font-size: 1.1rem;"><strong>Haz clic o arrastra el archivo aquí</strong></p>
                <p style="font-size: 0.9rem; color: #94a3b8;">Archivo JSON de configuración</p>
                <input type="file" id="fileInput" name="config" accept=".json" onchange="handleFileSelect(this)">
            </div>
            
            <div class="file-info" id="fileInfo">
                <i class="fas fa-file-check" style="color: #10b981;"></i>
                <span id="fileName" style="margin-left: 10px; font-weight: bold;"></span>
            </div>
            
            <button type="submit" class="btn btn-primary" id="uploadBtn" disabled>
                <i class="fas fa-upload"></i> Restaurar Configuración
            </button>
            <a href="/" class="btn btn-secondary" style="text-decoration:none; display:block; text-align:center;">
                <i class="fas fa-times"></i> Cancelar
            </a>
        </form>
    </div>

    <script>
        const dropZone = document.getElementById('dropZone');
        const fileInput = document.getElementById('fileInput');
        const fileInfo = document.getElementById('fileInfo');
        const fileName = document.getElementById('fileName');
        const uploadBtn = document.getElementById('uploadBtn');
        
        ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
            dropZone.addEventListener(eventName, preventDefaults, false);
        });

        function preventDefaults(e) {
            e.preventDefault();
            e.stopPropagation();
        }

        ['dragenter', 'dragover'].forEach(eventName => {
            dropZone.addEventListener(eventName, () => dropZone.classList.add('dragover'), false);
        });

        ['dragleave', 'drop'].forEach(eventName => {
            dropZone.addEventListener(eventName, () => dropZone.classList.remove('dragover'), false);
        });

        dropZone.addEventListener('drop', function(e) {
            const files = e.dataTransfer.files;
            fileInput.files = files;
            handleFileSelect(fileInput);
        }, false);

        function handleFileSelect(input) {
            if (input.files && input.files[0]) {
                const file = input.files[0];
                fileName.textContent = file.name + ' (' + (file.size / 1024).toFixed(1) + ' KB)';
                fileInfo.style.display = 'block';
                uploadBtn.disabled = false;
            }
        }
    </script>
</body>
</html>
)=====";
  
  server.send(200, "text/html", html);
}

void printLearningStats() {
  Serial.println("\n=== SISTEMA DE APRENDIZAJE ===");
  Serial.printf("Zona 1 - Score: %.1f%% | Ciclos: %d/%d | Eff Prom: %.2f%%/min\n",
                learningZone1.efficiencyScore, learningZone1.successfulCycles,
                learningZone1.totalCycles, learningZone1.avgEfficiency);
  Serial.printf("Zona 2 - Score: %.1f%% | Ciclos: %d/%d | Eff Prom: %.2f%%/min\n",
                learningZone2.efficiencyScore, learningZone2.successfulCycles,
                learningZone2.totalCycles, learningZone2.avgEfficiency);
  Serial.println("==============================\n");
  }

void handleUploadConfig() {
  HTTPUpload& upload = server.upload();
  static String receivedData = "";
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("[CONFIG] Recibiendo backup completo: " + String(upload.filename));
    receivedData = "";
    uploadSuccess = false;
    uploadMessage = "";
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    receivedData += String((char*)upload.buf).substring(0, upload.currentSize);
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    Serial.println("[CONFIG] Procesando backup completo...");
    
    DynamicJsonDocument doc(3600); // Aumentado para más datos
    DeserializationError error = deserializeJson(doc, receivedData);
    
    if (error) {
      Serial.println("[CONFIG] ❌ Error JSON inválido");
      uploadSuccess = false;
      uploadMessage = "Error: Archivo JSON inválido - " + String(error.c_str());
      receivedData = "";
      return;
    }
    
    // Verificar versión del backup
    String backupVersion = doc["version"] | "1.0";
    Serial.println("[CONFIG] Versión backup: " + backupVersion);
    
    if (xSemaphoreTake(mutexConfig, pdMS_TO_TICKS(2000)) == pdTRUE) {
      if (xSemaphoreTake(mutexData, pdMS_TO_TICKS(2000)) == pdTRUE) {
        
        // ===========================
        // RESTAURAR CONFIGURACIÓN BÁSICA
        // ===========================
        config.threshold1 = doc["threshold1"] | config.threshold1;
        config.threshold2 = doc["threshold2"] | config.threshold2;
        config.irrigationMode = doc["irrigationMode"] | config.irrigationMode;
        config.aiThreshold1 = doc["aiThreshold1"] | config.aiThreshold1;
        config.aiThreshold2 = doc["aiThreshold2"] | config.aiThreshold2;
        config.inferenceThresholdHigh = doc["inferenceThresholdHigh"] | config.inferenceThresholdHigh;
        config.inferenceThresholdLow = doc["inferenceThresholdLow"] | config.inferenceThresholdLow;
        config.authEnabled = doc["authEnabled"] | true;
        strlcpy(config.webUser, doc["webUser"] | "admin", sizeof(config.webUser));
        strlcpy(config.webPassword, doc["webPassword"] | "admin123", sizeof(config.webPassword));
        config.sessionTimeout = doc["sessionTimeout"] | 30;
        config.offsetTemp = doc["offsetTemp"] | config.offsetTemp;
        config.offsetHum = doc["offsetHum"] | config.offsetHum;
        config.offsetSoil1 = doc["offsetSoil1"] | config.offsetSoil1;
        config.offsetSoil2 = doc["offsetSoil2"] | config.offsetSoil2;
        config.offsetLight = doc["offsetLight"] | config.offsetLight;
        config.lightProtection = doc["lightProtection"] | config.lightProtection;
        config.lightThreshold = doc["lightThreshold"] | config.lightThreshold;
        config.waterSupplyControl = doc["waterSupplyControl"] | config.waterSupplyControl;
        config.tempProtection = doc["tempProtection"] | config.tempProtection;
        config.minTempThreshold = doc["minTempThreshold"] | config.minTempThreshold;
        config.maxTempThreshold = doc["maxTempThreshold"] | config.maxTempThreshold;
        config.humidityProtection = doc["humidityProtection"] | config.humidityProtection;
        config.humidityThreshold = doc["humidityThreshold"] | config.humidityThreshold;
        config.weatherGuard = doc["weatherGuard"] | config.weatherGuard;
        config.weatherProvider = doc["weatherProvider"] | config.weatherProvider;
        config.weatherApiKey = doc["weatherApiKey"] | config.weatherApiKey;
        config.weatherCity = doc["weatherCity"] | config.weatherCity;
        config.weatherRainThresholdHours = doc["weatherRainThresholdHours"] | config.weatherRainThresholdHours;
        config.gmtOffset = doc["gmtOffset"] | config.gmtOffset;
        config.daylightSaving = doc["daylightSaving"] | config.daylightSaving;
        config.timeFormat24h = doc["timeFormat24h"] | config.timeFormat24h;
        config.telegramToken = doc["telegramToken"] | config.telegramToken;
        config.chatID = doc["chatID"] | config.chatID;
        config.vpdProtection = doc["vpdProtection"] | config.vpdProtection;
        config.vpdEnabled = doc["vpdEnabled"] | config.vpdEnabled;
        config.vpdMinThreshold = doc["vpdMinThreshold"] | config.vpdMinThreshold;
        config.vpdMaxThreshold = doc["vpdMaxThreshold"] | config.vpdMaxThreshold;
        config.vpdCriticalHigh = doc["vpdCriticalHigh"] | config.vpdCriticalHigh;
        config.vpdCriticalLow = doc["vpdCriticalLow"] | config.vpdCriticalLow;
        config.vpdOptimalLow = doc["vpdOptimalLow"] | config.vpdOptimalLow;
        config.vpdOptimalHigh = doc["vpdOptimalHigh"] | config.vpdOptimalHigh;
        config.soilProfile = doc["soilProfile"] | config.soilProfile;
        config.useDHCP = doc["useDHCP"] | config.useDHCP;
        config.staticIP = doc["staticIP"] | config.staticIP;
        config.staticGateway = doc["staticGateway"] | config.staticGateway;
        config.staticSubnet = doc["staticSubnet"] | config.staticSubnet;
        config.staticDNS1 = doc["staticDNS1"] | config.staticDNS1;
        config.staticDNS2 = doc["staticDNS2"] | config.staticDNS2;
        config.timeRestrictionEnabled = doc["timeRestrictionEnabled"] | config.timeRestrictionEnabled;
        config.restrictionStartTime = doc["restrictionStartTime"] | config.restrictionStartTime;
        config.restrictionEndTime = doc["restrictionEndTime"] | config.restrictionEndTime;
        config.windRestrictionEnabled = doc["windRestrictionEnabled"] | config.windRestrictionEnabled;
        config.maxWindSpeed = doc["maxWindSpeed"] | config.maxWindSpeed;
        
        // Calibración sensores
        config.soil1Cal.calibrationType = doc.containsKey("soil1Cal_type") ? 
          (SoilSensorCalibration::CalType)doc["soil1Cal_type"].as<int>() : 
          SoilSensorCalibration::AUTO_BY_SUBSTRATE;
        config.soil1Cal.isManuallyCalibrated = doc["soil1Cal_manual"] | config.soil1Cal.isManuallyCalibrated;
        config.soil1Cal.dryPointADC = doc["soil1Cal_dryADC"] | config.soil1Cal.dryPointADC;
        config.soil1Cal.wetPointADC = doc["soil1Cal_wetADC"] | config.soil1Cal.wetPointADC;
        config.soil2Cal.calibrationType = doc.containsKey("soil2Cal_type") ? 
          (SoilSensorCalibration::CalType)doc["soil2Cal_type"].as<int>() : 
          SoilSensorCalibration::AUTO_BY_SUBSTRATE;
        config.soil2Cal.isManuallyCalibrated = doc["soil2Cal_manual"] | config.soil2Cal.isManuallyCalibrated;
        config.soil2Cal.dryPointADC = doc["soil2Cal_dryADC"] | config.soil2Cal.dryPointADC;
        config.soil2Cal.wetPointADC = doc["soil2Cal_wetADC"] | config.soil2Cal.wetPointADC;
        
        // ===========================
        // 🆕 RESTAURAR APRENDIZAJE ZONA 1
        // ===========================
        if (doc.containsKey("learningZone1")) {
          JsonObject zone1 = doc["learningZone1"];
          learningZone1.avgIrrigationTime = zone1["avgIrrigationTime"] | learningZone1.avgIrrigationTime;
          learningZone1.avgMoistureRecovery = zone1["avgMoistureRecovery"] | learningZone1.avgMoistureRecovery;
          learningZone1.avgEfficiency = zone1["avgEfficiency"] | learningZone1.avgEfficiency;
          learningZone1.successfulCycles = zone1["successfulCycles"] | learningZone1.successfulCycles;
          learningZone1.totalCycles = zone1["totalCycles"] | learningZone1.totalCycles;
          learningZone1.efficiencyScore = zone1["efficiencyScore"] | learningZone1.efficiencyScore;
          learningZone1.lastOptimization = zone1["lastOptimization"] | learningZone1.lastOptimization;
          Serial.printf("[RESTORE] Zona 1: %d ciclos, Score: %.1f%%\n", 
                        learningZone1.totalCycles, learningZone1.efficiencyScore);
        }
        
        // ===========================
        // 🆕 RESTAURAR APRENDIZAJE ZONA 2
        // ===========================
        if (doc.containsKey("learningZone2")) {
          JsonObject zone2 = doc["learningZone2"];
          learningZone2.avgIrrigationTime = zone2["avgIrrigationTime"] | learningZone2.avgIrrigationTime;
          learningZone2.avgMoistureRecovery = zone2["avgMoistureRecovery"] | learningZone2.avgMoistureRecovery;
          learningZone2.avgEfficiency = zone2["avgEfficiency"] | learningZone2.avgEfficiency;
          learningZone2.successfulCycles = zone2["successfulCycles"] | learningZone2.successfulCycles;
          learningZone2.totalCycles = zone2["totalCycles"] | learningZone2.totalCycles;
          learningZone2.efficiencyScore = zone2["efficiencyScore"] | learningZone2.efficiencyScore;
          learningZone2.lastOptimization = zone2["lastOptimization"] | learningZone2.lastOptimization;
          Serial.printf("[RESTORE] Zona 2: %d ciclos, Score: %.1f%%\n", 
                        learningZone2.totalCycles, learningZone2.efficiencyScore);
        }
        
        // ===========================
        // 🆕 RESTAURAR CICLOS ACTUALES
        // ===========================
        if (doc.containsKey("currentCycle1")) {
          JsonObject cycle1 = doc["currentCycle1"];
          currentCycle1.startTime = cycle1["startTime"] | currentCycle1.startTime;
          currentCycle1.initialMoisture = cycle1["initialMoisture"] | currentCycle1.initialMoisture;
          currentCycle1.finalMoisture = cycle1["finalMoisture"] | currentCycle1.finalMoisture;
          currentCycle1.vpdAtStart = cycle1["vpdAtStart"] | currentCycle1.vpdAtStart;
          currentCycle1.tempAtStart = cycle1["tempAtStart"] | currentCycle1.tempAtStart;
          currentCycle1.completed = cycle1["completed"] | currentCycle1.completed;
          currentCycle1.efficiency = cycle1["efficiency"] | currentCycle1.efficiency;
        }
        
        if (doc.containsKey("currentCycle2")) {
          JsonObject cycle2 = doc["currentCycle2"];
          currentCycle2.startTime = cycle2["startTime"] | currentCycle2.startTime;
          currentCycle2.initialMoisture = cycle2["initialMoisture"] | currentCycle2.initialMoisture;
          currentCycle2.finalMoisture = cycle2["finalMoisture"] | currentCycle2.finalMoisture;
          currentCycle2.vpdAtStart = cycle2["vpdAtStart"] | currentCycle2.vpdAtStart;
          currentCycle2.tempAtStart = cycle2["tempAtStart"] | currentCycle2.tempAtStart;
          currentCycle2.completed = cycle2["completed"] | currentCycle2.completed;
          currentCycle2.efficiency = cycle2["efficiency"] | currentCycle2.efficiency;
        }
        
        // ===========================
        // 🆕 RESTAURAR HISTORIAL SENSORES
        // ===========================
        if (doc.containsKey("sensorHistory")) {
          JsonObject sensorHist = doc["sensorHistory"];
          sensorHistory.lastTemp = sensorHist["lastTemp"] | sensorHistory.lastTemp;
          sensorHistory.lastHum = sensorHist["lastHum"] | sensorHistory.lastHum;
          sensorHistory.lastLight = sensorHist["lastLight"] | sensorHistory.lastLight;
          sensorHistory.lastUpdate = sensorHist["lastUpdate"] | sensorHistory.lastUpdate;
          sensorHistory.tempChangeRate = sensorHist["tempChangeRate"] | sensorHistory.tempChangeRate;
          sensorHistory.humChangeRate = sensorHist["humChangeRate"] | sensorHistory.humChangeRate;
          sensorHistory.lightChangeRate = sensorHist["lightChangeRate"] | sensorHistory.lightChangeRate;
        }
        
        // ===========================
        // 🆕 RESTAURAR ESTADO INFERENCIA
        // ===========================
        data.smartModeActive = doc["smartModeActive"] | data.smartModeActive;
        data.lastIrrigation1 = doc["lastIrrigation1"] | data.lastIrrigation1;
        data.lastIrrigation2 = doc["lastIrrigation2"] | data.lastIrrigation2;
        data.irrigationNeed1 = doc["irrigationNeed1"] | data.irrigationNeed1;
        data.irrigationNeed2 = doc["irrigationNeed2"] | data.irrigationNeed2;

        saveConfig();
        
        xSemaphoreGive(mutexData);
        xSemaphoreGive(mutexConfig);
        
        Serial.println("[CONFIG] ✅ Configuración completa restaurada");
        Serial.println("[LEARNING] Aprendizaje restaurado:");
        printLearningStats();
        addSerialLog("[CONFIG] Config completa restaurada desde backup");
        
        uploadSuccess = true;
        uploadMessage = "Configuración y aprendizaje restaurados correctamente";
        
      } else {
        xSemaphoreGive(mutexConfig);
        uploadSuccess = false;
        uploadMessage = "Error: Timeout accediendo a datos";
      }
    } else {
      uploadSuccess = false;
      uploadMessage = "Error: Timeout accediendo a configuración";
    }
    
    receivedData = "";
  }
}


// Handler principal que envía la respuesta
void handleUploadComplete() {
  String html;
  
  if (uploadSuccess) {
    html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>";
    html += "<style>";
    html += "body{font-family:Arial;text-align:center;padding:50px;background:#0f172a;color:white;}";
    html += ".container{background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #10b981;}";
    html += "h2{color:#10b981;margin-bottom:20px;}";
    html += ".btn{display:inline-block;margin-top:20px;padding:15px 30px;background:#6366f1;color:white;text-decoration:none;border-radius:10px;transition:all 0.3s;}";
    html += ".btn:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(99,102,241,0.4);}";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h2><i class='fas fa-check-circle'></i> Configuración Restaurada</h2>";
    html += "<p>" + uploadMessage + "</p>";
    html += "<p>Todos los ajustes se han aplicado correctamente</p>";
    html += "<a href='/' class='btn'><i class='fas fa-home'></i> Volver al inicio</a>";
    html += "</div></body></html>";
    
    server.sendHeader("Content-Type", "text/html; charset=UTF-8");
    server.send(200, "text/html; charset=UTF-8", html);
    
  } else {
    html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'>";
    html += "<style>";
    html += "body{font-family:Arial;text-align:center;padding:50px;background:#0f172a;color:white;}";
    html += ".container{background:#1e293b;padding:30px;border-radius:15px;max-width:500px;margin:0 auto;border-left:5px solid #ef4444;}";
    html += "h2{color:#ef4444;margin-bottom:20px;}";
    html += ".btn{display:inline-block;margin-top:20px;padding:15px 30px;background:#6366f1;color:white;text-decoration:none;border-radius:10px;}";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h2><i class='fas fa-exclamation-triangle'></i> Error en la Restauración</h2>";
    html += "<p>" + uploadMessage + "</p>";
    html += "<a href='/importconfig' class='btn'><i class='fas fa-redo'></i> Intentar de nuevo</a>";
    html += "</div></body></html>";
    
    server.sendHeader("Content-Type", "text/html; charset=UTF-8");
    server.send(400, "text/html; charset=UTF-8", html);
  }
}

// Definiciones reales
WiFiClientSecure secured_client;
UniversalTelegramBot* bot = NULL;
TelegramPSRAMData telegramPSRAM;
const String telegramRootCACert = "";

// ===========================
// GESTIÓN DE CONFIGURACIÓN PRINCIPAL
// ===========================
void saveConfig();
void loadConfig();
void handleSetConfig();

// ===========================
// BACKUP Y RESTAURACIÓN
// ===========================
void handleExportConfig();
void handleImportConfig();
void handleUploadConfig();
void handleUploadComplete();

#endif