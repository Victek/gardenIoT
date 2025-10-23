// vpd_functions.h
// Funciones VPD con corrección por presión barométrica
// Vicente Soriano @2025

#ifndef VPD_FUNCTIONS_H
#define VPD_FUNCTIONS_H

#include <Arduino.h>

// Declaraciones externas
extern struct Config config;
extern SemaphoreHandle_t mutexConfig;

// ═══════════════════════════════════════════════════════════════
// CÁLCULO VPD - Ecuación de Tetens
// ═══════════════════════════════════════════════════════════════
float calculateVPD(float temperature, float humidity) {
  if (isnan(temperature) || isnan(humidity) || 
      humidity < 0 || humidity > 100) {
    return -1.0;
  }

  // Ecuación de Magnus-Tetens directa
  float es = 0.6108 * exp((17.27 * temperature) / (temperature + 237.3));
  float ea = (humidity / 100.0) * es;
  float vpd = es - ea;

  // Evitar valores negativos por errores de redondeo
  return (vpd < 0) ? 0 : vpd;  // kPa
}

// ═══════════════════════════════════════════════════════════════
// FUNCIONES AUXILIARES VPD
// ═════════════════════════════════════════════════════════════== 

String getVPDQuality(float vpd) {
  if (vpd < 0) return "ERROR";
  if (vpd < 0.4) return "MUY BAJO";
  if (vpd < 0.8) return "BAJO"; 
  if (vpd < 1.2) return "ÓPTIMO";
  if (vpd < 1.6) return "ALTO";
  return "MUY ALTO";
}

String getVPDColor(float vpd) {
  if (vpd < 0) return "#6b7280";     // Gris (error)
  if (vpd < 0.4) return "#3b82f6";   // Azul (muy bajo)
  if (vpd < 0.8) return "#06b6d4";   // Cian (bajo)
  if (vpd < 1.2) return "#10b981";   // Verde (óptimo)
  if (vpd < 1.6) return "#f59e0b";   // Amarillo (alto)
  return "#ef4444";                  // Rojo (muy alto)
}

bool isVPDBlocking(float vpd) {
  // Bloquear riego en VPD extremos
  return (vpd < 0.3 || vpd > 1.8);
}

#endif // VPD_FUNCTIONS_H