// rtc_manager.h
#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
#include <time.h>

// ===========================
// VARIABLES GLOBALES RTC
// ===========================
extern RTC_DS3231 rtc;
extern bool rtcAvailable;
extern unsigned long lastRTCSync;

// ===========================
// ENUMERACIONES
// ===========================
enum TimeSource {
  TIME_SOURCE_NONE,
  TIME_SOURCE_NTP,
  TIME_SOURCE_RTC_DS3231,
  TIME_SOURCE_INTERNAL
};

extern TimeSource currentTimeSource;

// ===========================
// DECLARACIONES DE FUNCIONES
// ===========================
void initializeRTC();
bool syncNTPtoRTC();
bool syncRTCtoSystem();
void syncInternalToRTC();
String getFormattedTimeRTC();
bool setRTCManually(int year, int month, int day, int hour, int minute, int second);
TimeSource getBestTimeSource();
void updateTimeFromBestSource();
String getTimeSourceName();
unsigned long getRTCEpoch();

// ===========================
// IMPLEMENTACIÓN
// ===========================

RTC_DS3231 rtc;
bool rtcAvailable = false;
unsigned long lastRTCSync = 0;
TimeSource currentTimeSource = TIME_SOURCE_NONE;

// Inicializar RTC DS3231
void initializeRTC() {
  Serial.println("[RTC] Inicializando DS3231...");
  
  // Intentar inicializar RTC en el bus I2C existente
  if (!rtc.begin()) {
    Serial.println("[RTC] ❌ DS3231 no detectado en I2C");
    rtcAvailable = false;
    return;
  }
  
  rtcAvailable = true;
  Serial.println("[RTC] ✅ DS3231 detectado correctamente");
  
  // Verificar si el RTC perdió alimentación
  if (rtc.lostPower()) {
    Serial.println("[RTC] ⚠️ RTC perdió alimentación - necesita ajuste");
    // Se ajustará con NTP o manualmente
  } else {
    // Leer hora actual del RTC
    DateTime now = rtc.now();
    Serial.printf("[RTC] Hora actual RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
                  now.day(), now.month(), now.year(),
                  now.hour(), now.minute(), now.second());
  }
  
  // Verificar temperatura del RTC (diagnóstico)
  float temp = rtc.getTemperature();
  Serial.printf("[RTC] Temperatura del módulo: %.2f°C\n", temp);
}

// Sincronizar hora NTP al RTC
bool syncNTPtoRTC() {
  if (!rtcAvailable) {
    Serial.println("[RTC] No disponible para sincronizar");
    return false;
  }
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[RTC] ❌ No se pudo obtener hora NTP");
    return false;
  }
  
  // Ajustar RTC con hora NTP
  DateTime ntpTime(timeinfo.tm_year + 1900, 
                   timeinfo.tm_mon + 1, 
                   timeinfo.tm_mday,
                   timeinfo.tm_hour, 
                   timeinfo.tm_min, 
                   timeinfo.tm_sec);
  
  rtc.adjust(ntpTime);
  lastRTCSync = millis();
  
  Serial.println("[RTC] ✅ Sincronizado con NTP");
  Serial.printf("[RTC] Nueva hora: %02d/%02d/%04d %02d:%02d:%02d\n",
                ntpTime.day(), ntpTime.month(), ntpTime.year(),
                ntpTime.hour(), ntpTime.minute(), ntpTime.second());
  
  return true;
}

// Sincronizar RTC al sistema ESP32
bool syncRTCtoSystem() {
  if (!rtcAvailable) {
    return false;
  }
  
  DateTime now = rtc.now();
  
  // Convertir a struct tm para configTime
  struct tm t;
  t.tm_year = now.year() - 1900;
  t.tm_mon = now.month() - 1;
  t.tm_mday = now.day();
  t.tm_hour = now.hour();
  t.tm_min = now.minute();
  t.tm_sec = now.second();
  t.tm_isdst = -1;
  
  time_t epoch = mktime(&t);
  struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
  settimeofday(&tv, NULL);
  
  Serial.println("[RTC] ✅ Sistema ESP32 sincronizado con RTC");
  return true;
}

// Sincronizar reloj interno al RTC (cuando no hay WiFi)
void syncInternalToRTC() {
  if (!rtcAvailable) {
    return;
  }
  
  // Actualizar cada 10 minutos para compensar drift del RTC
  if (millis() - lastRTCSync > 600000) {
    syncRTCtoSystem();
    lastRTCSync = millis();
  }
}

// Obtener hora formateada desde RTC
String getFormattedTimeRTC() {
  if (!rtcAvailable) {
    return "RTC no disponible";
  }
  
  DateTime now = rtc.now();
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d %02d:%02d",
           now.day(), now.month(), now.year(),
           now.hour(), now.minute());
  
  return String(buffer);
}

// Ajustar RTC manualmente
bool setRTCManually(int year, int month, int day, int hour, int minute, int second) {
  if (!rtcAvailable) {
    Serial.println("[RTC] No disponible para ajuste manual");
    return false;
  }
  
  // Validar rangos
  if (year < 2025 || year > 2099 || 
      month < 1 || month > 12 || 
      day < 1 || day > 31 ||
      hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 ||
      second < 0 || second > 59) {
    Serial.println("[RTC] ❌ Valores fuera de rango");
    return false;
  }
  
  DateTime newTime(year, month, day, hour, minute, second);
  rtc.adjust(newTime);
  
  // Sincronizar con sistema ESP32
  syncRTCtoSystem();
  
  Serial.printf("[RTC] ✅ Ajustado manualmente: %02d/%02d/%04d %02d:%02d:%02d\n",
                day, month, year, hour, minute, second);
  
  return true;
}

// Determinar la mejor fuente de tiempo disponible
TimeSource getBestTimeSource() {
  extern bool wifiConnected;
  
  // Prioridad: NTP > RTC DS3231 > Reloj Interno
  if (wifiConnected) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      return TIME_SOURCE_NTP;
    }
  }
  
  if (rtcAvailable) {
    DateTime now = rtc.now();
    // Verificar que el RTC tiene una fecha válida (no 2000-01-01)
    if (now.year() >= 2025) {
      return TIME_SOURCE_RTC_DS3231;
    }
  }
  
  return TIME_SOURCE_INTERNAL;
}

// Actualizar hora desde la mejor fuente
void updateTimeFromBestSource() {
  TimeSource newSource = getBestTimeSource();
  
  if (newSource != currentTimeSource) {
    currentTimeSource = newSource;
    Serial.println("[TIME] Cambio de fuente: " + getTimeSourceName());
    
    if (currentTimeSource == TIME_SOURCE_RTC_DS3231) {
      syncRTCtoSystem();
    }
  }
  
  // Sincronización periódica
  if (currentTimeSource == TIME_SOURCE_RTC_DS3231) {
    syncInternalToRTC();
  }
}

// Obtener nombre de fuente de tiempo
String getTimeSourceName() {
  switch (currentTimeSource) {
    case TIME_SOURCE_NTP:
      return "NTP (Internet)";
    case TIME_SOURCE_RTC_DS3231:
      return "RTC DS3231";
    case TIME_SOURCE_INTERNAL:
      return "Reloj Interno";
    default:
      return "Sin configurar";
  }
}

// Obtener epoch desde RTC
unsigned long getRTCEpoch() {
  if (!rtcAvailable) {
    return 0;
  }
  
  DateTime now = rtc.now();
  return now.unixtime();
}

#endif // RTC_MANAGER_H