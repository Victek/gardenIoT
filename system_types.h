// system_types.h
#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>
#include "pin_definitions.h"

// ===========================
// ESTRUCTURAS DE DATOS
// ===========================
struct ZoneLearning {
  float avgIrrigationTime = 0;      // Tiempo promedio de riego
  float avgMoistureRecovery = 0;    // Recuperación promedio de humedad
  float avgEfficiency = 0;          // Eficiencia promedio (%/minuto)
  int successfulCycles = 0;         // Ciclos exitosos
  int totalCycles = 0;              // Total de ciclos
  float efficiencyScore = 0;        // Score de eficiencia (%)
  unsigned long lastOptimization = 0; // Última optimización
};

struct WeatherData {
  float et0_daily = 0;
  float et0_hourly = 0;
  float soil_temperature = 0;
  float soil_moisture_surface = 0;
  float soil_moisture_9cm = 0;
  float solar_radiation = 0;
  float uv_index = 0;
  float wind_speed = 0;
  float apparent_temperature = 0;
  float vapor_pressure_deficit = 0;
  float surface_pressure = 0; 
  unsigned long last_update = 0;
  bool data_valid = false;
};

struct SoilSensorCalibration {
  enum CalType { AUTO_BY_SUBSTRATE, MANUAL_TWO_POINT } calibrationType = AUTO_BY_SUBSTRATE;
  String substratetype = "universal";
  float substrateFactor = 1.0;
  bool isManuallyCalibrated = false;
  bool isInverseSensor = true;  
  int dryPointADC = 4095;
  int wetPointADC = 0;
  bool isCalibrated = false;
  float lastRawADC = 0;
  float lastPercentage = 0;
};

struct Config {
  //Calibración guiada y offsets manuales
  uint8_t irrigationMode = 1;             // Modo de funcionamiento 0-Manual 1-Auto 2-IA
  int threshold1 = 25;                    // Umbral de humedad suelo 1 que permite regar si >
  int threshold2 = 25;                    // Umbral de humedad suelo 2 que permite regar si >
  int aiThreshold1 = 25;                  // Umbrales de partida del modo IA Sensor 1
  int aiThreshold2 = 25;                  // Umbrales de partida del modo IA Sensor 2
  float offsetTemp = 0;
  float offsetHum = 0;
  float offsetLight = 0;
  float offsetSoil1 = 0;
  float offsetSoil2 = 0;
  float offsetPressure = 0;      // Offset presión BME680
  float offsetAirQuality = 0;    // Offset calidad aire BME680
  SoilSensorCalibration soil1Cal;
  SoilSensorCalibration soil2Cal;

  //<-----Restricciones globales --->
  //<-------Restricciones horarias (externo) -----
  bool timeRestrictionEnabled = false;     // Activar restricción horaria
  String restrictionStartTime = "";   // Hora inicio restricción (formato HH:MM)
  String restrictionEndTime = "";     // Hora fin restricción (formato HH:MM)
  bool timeRestrictionActive = false;      // Estado actual (calculado)
  //<-----Viento (externo)----->
  bool windRestrictionEnabled = false;     // Activar restricción viento (Meteo-Open)
  float maxWindSpeed = 18.0;              // Velocidad máxima permitida (km/h)
  bool windRestrictionActive = false;      // Estado actual (calculado)
  //<------Luz Solar (interno)----->
  bool lightProtection = true;            // Por excesiva Radiación solar
  int lightThreshold = 80;                // Umbral de luz (%)
  bool lightProtectionActive = false;     // Estado actual de la protección
  //<------Suministro Agua (interno)----->
  bool waterSupplyControl = true;         // Por falta de agua
  //<------Temperatura (interno)------>
  bool tempProtection = true;             // Por temperatura ambiente (sensor local) 
  float minTempThreshold = 5.0;           // Temp. mínima que permite regar por encima del valor
  float maxTempThreshold = 35.0;          // Temp. máxima que deshabilita por encima para regar
  bool tempProtectionActive = false;      // Estado actual de la protección
  //<------Humedad ambiente (interno)---->
  bool humidityProtection = true;         // Protección por humedad ambiente (sensor local)
  float humidityThreshold = 80.0;         // Umbral de humedad (%)
  bool humidityProtectionActive = false;  // Estado actual de la protección
  //<----- PREVISION DEL TIEMPO (externo)----->
  bool weatherGuard = true;                    // Función activada on/off
  String weatherProvider = "";                 // "openmeteo" | "openweathermap" | "weatherapi" | "accuweather"
  String weatherApiKey = "";                   // clave token API del servicio elegido si es necesario
  String weatherCity = "";                     // Ciudad o long/lat de la ubicación del sistema
  uint8_t weatherRainThresholdHours = 4;       // Ventana de Pronóstico (1-48 h)
  bool rainExpected = false;                   // Valor de inicio previsión de lluvia
  int weatherLastStatus = -1;                  // -1=desactivado, 0=sin datos, 1=sin lluvia, 2=con lluvia                 // flag calculado por la tarea
  unsigned long lastWeatherCheck = 0;          // epoch UTC del servidor de Meteo
  float maxRainExpected = 0.0;                 // mm de lluvia en la ventana de tiempo seleccionada
  bool weatherUpdateRequested = false;         // para preguntar si hay un cambio de configuración
  //<----- Presión Diferencial de Vapor (interno)----->
  bool vpdProtection = true;                   // Protección VPD activada
  bool vpdProtectionActive = false;            // Estado actual de bloqueo VPD
  bool vpdEnabled = true;                      // Mostrar VPD en UX
  float vpdMinThreshold = 0.4;                 // VPD mínimo óptimo (kPa)
  float vpdMaxThreshold = 1.5;                 // VPD máximo óptimo (kPa)
  float vpdCriticalHigh = 1.8;                 // VPD crítico alto - riego urgente
  float vpdCriticalLow = 0.3;                  // VPD crítico bajo - no regar
  float vpdOptimalLow = 0.8;                   // Inicio rango óptimo
  float vpdOptimalHigh = 1.2;                  // Final rango óptimo
  float vpdFactorCriticalHigh = 1.5;           // Multiplicador VPD alto
  float vpdFactorCriticalLow = 0.3;            // Multiplicador VPD bajo
  float vpdFactorOptimal = 1.2;                // Multiplicador VPD óptimo
  float vpdFactorNormal = 1.0;                 // Multiplicador VPD normal
  float tempFactorHigh = 1.3;                  // Multiplicador temp alta (>28°C)
  float tempFactorLow = 0.7;                   // Multiplicador temp baja (<18°C)
  float tempThresholdHigh = 28.0;              // Umbral temperatura alta
  float tempThresholdLow = 18.0;               // Umbral temperatura baja
    
  //<------- Ajuste Hora ------>
  int8_t gmtOffset = 1;                        // GMT offset en horas (-12 a +12) = Zona Horaria
  bool daylightSaving = true;                  // Horario de verano activo
  bool timeFormat24h = true;                   // true = 24h, false = 12h AM/PM
  //<----- Telegram ----------->
  String telegramToken = "";                   // Bot API
  String chatID = "";                          // ID del chat
  //<----Perfiles de suelo-sustrato-->
  String soilProfile = "universal";            // Sustrato por defecto
  bool advancedCalibration = false;            // Permite calibración del tipo de sustrato
  float universalCalibration = 1.0;            // Factor permeabilidad para el tipo de sustrato
  float clayCalibration = 1.35;                // lo mismo
  float sandyCalibration = 0.75;               // lo mismo
  float loamCalibration = 1.1;                 // lo mismo
  float peatCalibration = 1.25;                // lo mismo
  float cocoCalibration = 0.95;                // lo mismo
  float rockwoolCalibration = 0.85;            // lo mismo
  float perliteCalibration = 0.7;              // lo mismo
  float vermiculiteCalibration = 1.4;          // lo mismo
  //<---- Configuración de Red ---->
  bool useDHCP = true;                         // true = DHCP automático, false = IP fija
  String staticIP = "192.168.1.100";           // IP local
  String staticGateway = "192.168.1.1";        // Puerta de enlace
  String staticSubnet = "255.255.255.0";       // Mascara de subred
  String staticDNS1 = "8.8.8.8";               // DNS 1
  String staticDNS2 = "8.8.4.4";               // DNS 2
  // UMBRALES DE INFERENCIA INTELIGENTE
  float inferenceThresholdHigh = 0.7;  // 70% necesidad para activar (0.0-1.0)
  float inferenceThresholdLow = 0.3;   // 30% necesidad para mantener (0.0-1.0)
  // USER/PASS ACCESO WEB
  bool authEnabled = true;              // ¿Pedir login para acceder?
  char webUser[32] = "admin";           // Usuario para login
  char webPassword[64] = "admin123";    // Contraseña para login
  uint32_t sessionTimeout = 30; 
  
};

struct SensorData {
  float temp = 0;
  float hum = 0;
  float soil1 = 0;
  float soil2 = 0;
  float light = 0;
  bool pump1 = false;
  bool pump2 = false;
  bool waterSupply = false;
  float vpd = 0;
  float pressure = 0;       // Presión atmosférica hPa
  float airQuality = 0;     // Calidad aire kΩ
  WeatherData weather;
  float irrigationNeed1 = 0;
  float irrigationNeed2 = 0;
  unsigned long lastIrrigation1 = 0;
  unsigned long lastIrrigation2 = 0;
  bool smartModeActive = false;
};

// Estructura para estado ambiental
struct AmbientState {
  String icon;
  String text;
  String detail;
  String gradient;
};

// ===========================
// CONSTANTES FREERTOS
// ===========================
#define TASK_STACK_SENSORS     4096
#define TASK_STACK_IRRIGATION  8192
#define TASK_STACK_RTC         2048
#define TASK_STACK_WEATHER     8192   
#define TASK_STACK_TELEGRAM    22128
#define TASK_PRIO 1

// ===========================
// GESTIÓN DE TAREAS FREERTOS
// ===========================
extern TaskHandle_t taskWeatherHandle;
extern TaskHandle_t taskTelegramHandle;
extern bool weatherTaskRunning;
extern bool telegramTaskRunning;


// ===========================
// DECLARACIONES EXTERN 
// ===========================
extern ZoneLearning learningZone1, learningZone2;
extern const char* MDNS_HOSTNAME;
extern WebServer server;
extern DNSServer dnsServer;
extern WebServer configServer;
extern SemaphoreHandle_t mutexData;
extern SemaphoreHandle_t mutexConfig;
extern Config config;
extern SensorData data;
extern Adafruit_NeoPixel pixels;
extern bool wifiConnected;
extern String ssid_stored;
extern String pass_stored;
extern unsigned long lastWebRequest;

struct IrrigationCycle {
  unsigned long startTime = 0;
  float initialMoisture = 0;
  float finalMoisture = 0;
  float vpdAtStart = 0;
  float tempAtStart = 0;
  bool completed = false;
  float efficiency = 0;
};

// VARIABLES GLOBALES PARA INFERENCIA INTELIGENTE
struct SensorHistory {
  float lastTemp = 0;
  float lastHum = 0;
  float lastLight = 0;
  unsigned long lastUpdate = 0;
  float tempChangeRate = 0;
  float humChangeRate = 0;
  float lightChangeRate = 0;
};

// ===========================
// DECLARACIONES DE FUNCIONES
// ===========================

// Sistema y configuración
void testNeopixel();
void setLedStatus(int red, int green, int blue);
void loadConfig();
bool loadWifiConfig();
bool setupmDNS();  // Cambiado de void a bool
void startConfigPortal();
void initializeSystemRegardlessOfWifi();
bool configureNetwork();

// Handlers web
void handleConfigPortal();
void handleSaveConfig();
void handleWifiScan();
void handleRoot();
void handleTestPumps();
void handlePump(int pump, bool state);
void handleSetConfig();
void handleAuto(bool on);
void handleResetWifi();
void handleWaterSupplyOn();
void handleWaterSupplyOff();
void handleLogs();
void handleWifiResetButton();
void handleFactoryReset();
void handleTestTelegram();
void handleSetCalibration();
void handleExportConfig();
void handleImportConfig();
void handleUploadComplete();
void handleUploadConfig();

// Utilidades y tiempo
void addSerialLog(String message);
String getRestrictionsStatus();
void configureNTP();
String getFormattedTime();
String getFormattedTimeFromEpoch(unsigned long epoch);

// Sensores y datos
void updateSensorHistory(float temp, float hum, float light);
void updateZoneLearning(int zone, float moistureGain, unsigned long irrigationTime, float efficiency);
void analyzeIrrigationEfficiency(int zone, IrrigationCycle& cycle, float finalMoisture);

// Control de hardware
void controlPump(int pump, bool on);
void setupVPDHandlers();

// Telegram
bool sendTelegramMessage(String message);
void sendTelegramAlert(String alertType, String details);
void handleTelegramCommand(String chat_id, String text);

// WiFi
void attemptWifiReconnection();
void attemptQuickWifiReconnection();
void deleteWifiConfig();
void saveWifiConfig(String ssid, String password);

// Tareas FreeRTOS
void taskReadSensors(void* pvParameters);
void taskAutoIrrigation(void* pvParameters);
void taskWeatherGuard(void* pvParameters);
void taskTelegramCommands(void* pvParameters);

// Configuración del sistema
void deleteAllConfig();



#endif