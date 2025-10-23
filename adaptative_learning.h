// adaptive_learning.h
// Sistema de aprendizaje bidireccional para optimización de riego
// Vicente Soriano - 2025
// ✅ Aumenta umbrales si riega muy frecuentemente sin mejoras (baja eficiencia)
// ✅ Disminuye umbrales si el sistema es muy eficiente (puede optimizar más)
// ✅ Considera múltiples métricas: eficiencia, tiempo de riego, recuperación
// ✅ Límites de seguridad configurables

#ifndef ADAPTIVE_LEARNING_H
#define ADAPTIVE_LEARNING_H

#include <Arduino.h>

// ===========================
// CONFIGURACIÓN DEL SISTEMA
// ===========================
struct AdaptiveConfig {
  // Límites de seguridad para thresholds
  float minThreshold = 20.0;        // Nunca bajar de 20%
  float maxThreshold = 70.0;        // Nunca subir de 70%
  
  // Umbrales de eficiencia para decisiones
  float highEfficiencyThreshold = 75.0;   // Eficiencia alta: puede optimizar más
  float lowEfficiencyThreshold = 50.0;    // Eficiencia baja: demasiado riego
  
  // Incrementos/decrementos graduales
  float adjustmentStep = 1.5;       // Paso de ajuste (%)
  float maxAdjustmentPerCycle = 5.0; // Máximo cambio por optimización
  
  // Condiciones para optimización
  int minCyclesForOptimization = 10; // Ciclos mínimos antes de ajustar
  unsigned long optimizationInterval = 86400000; // 24h entre optimizaciones
  
  // Pesos para decisión compuesta
  float weightEfficiency = 0.4;     // Peso de eficiencia
  float weightRecovery = 0.3;       // Peso de recuperación de humedad
  float weightTime = 0.3;           // Peso de tiempo de riego
};

// ===========================
// ESTRUCTURA DE ANÁLISIS
// ===========================
struct ThresholdAnalysis {
  float currentThreshold;
  float recommendedThreshold;
  float adjustmentAmount;
  String reasoning;
  bool shouldAdjust;
  
  // Métricas de soporte
  float efficiencyScore;
  float recoveryRate;
  float avgIrrigationTime;
  int totalCycles;
  int successfulCycles;
};

// ===========================
// SISTEMA ADAPTATIVO MEJORADO
// ===========================
class AdaptiveLearningSystem {
private:
  AdaptiveConfig adaptiveConfig;
  
  // Analizar si el threshold es muy alto (riega tarde)
  bool isThresholdTooHigh(float efficiency, float recovery, float avgTime) {
    // Eficiencia baja + poca recuperación + mucho tiempo = threshold muy alto
    return (efficiency < adaptiveConfig.lowEfficiencyThreshold && 
            recovery < 15.0 && 
            avgTime > 120000); // > 2 minutos
  }
  
  // Analizar si el threshold es muy bajo (riega pronto/mucho)
  bool isThresholdTooLow(float efficiency, float recovery, float avgTime) {
    // Eficiencia alta + buena recuperación + poco tiempo = puede bajar threshold
    return (efficiency > adaptiveConfig.highEfficiencyThreshold && 
            recovery > 20.0 && 
            avgTime < 60000); // < 1 minuto
  }
  
  // Calcular score compuesto para decisión
  float calculateCompositeScore(float efficiency, float recovery, float avgTime) {
    // Normalizar métricas a 0-1
    float normEfficiency = constrain(efficiency / 100.0, 0.0, 1.0);
    float normRecovery = constrain(recovery / 30.0, 0.0, 1.0); // 30% es excelente
    float normTime = constrain((180000 - avgTime) / 180000.0, 0.0, 1.0); // 3min max
    
    // Aplicar pesos
    return (normEfficiency * adaptiveConfig.weightEfficiency) +
           (normRecovery * adaptiveConfig.weightRecovery) +
           (normTime * adaptiveConfig.weightTime);
  }

public:
  AdaptiveLearningSystem() {}
  
  // Configurar parámetros del sistema adaptativo
  void configure(AdaptiveConfig config) {
    adaptiveConfig = config;
  }
  
  // ===========================
  // ANÁLISIS BIDIRECCIONAL
  // ===========================
  ThresholdAnalysis analyzeZoneThreshold(
    int zone,
    float currentThreshold,
    float efficiencyScore,
    float avgMoistureRecovery,
    float avgIrrigationTime,
    int successfulCycles,
    int totalCycles
  ) {
    ThresholdAnalysis analysis;
    analysis.currentThreshold = currentThreshold;
    analysis.efficiencyScore = efficiencyScore;
    analysis.recoveryRate = avgMoistureRecovery;
    analysis.avgIrrigationTime = avgIrrigationTime;
    analysis.totalCycles = totalCycles;
    analysis.successfulCycles = successfulCycles;
    analysis.shouldAdjust = false;
    analysis.adjustmentAmount = 0.0;
    analysis.recommendedThreshold = currentThreshold;
    
    // Verificar ciclos mínimos
    if (totalCycles < adaptiveConfig.minCyclesForOptimization) {
      analysis.reasoning = "Insuficientes datos (necesita " + 
                          String(adaptiveConfig.minCyclesForOptimization) + " ciclos)";
      return analysis;
    }
    
    // Calcular score compuesto
    float compositeScore = calculateCompositeScore(
      efficiencyScore, 
      avgMoistureRecovery, 
      avgIrrigationTime / 1000.0
    );
    
    // ===========================
    // DECISIÓN BIDIRECCIONAL
    // ===========================
    
    // 🔴 CASO 1: EFICIENCIA MUY BAJA - AUMENTAR THRESHOLD
    if (efficiencyScore < adaptiveConfig.lowEfficiencyThreshold) {
      // Riega demasiado frecuente sin resultados
      float adjustment = adaptiveConfig.adjustmentStep;
      
      // Ajustar más agresivamente si es muy malo
      if (efficiencyScore < 40.0) {
        adjustment *= 1.5;
      }
      
      analysis.adjustmentAmount = min(adjustment, adaptiveConfig.maxAdjustmentPerCycle);
      analysis.recommendedThreshold = currentThreshold + analysis.adjustmentAmount;
      analysis.shouldAdjust = true;
      analysis.reasoning = "🔴 Eficiencia baja (" + String(efficiencyScore, 1) + 
                          "%) - Aumentar threshold para reducir riegos innecesarios";
    }
    
    // 🟢 CASO 2: EFICIENCIA ALTA - PUEDE DISMINUIR THRESHOLD
    else if (efficiencyScore > adaptiveConfig.highEfficiencyThreshold && 
             avgMoistureRecovery > 15.0) {
      // Sistema muy eficiente, puede optimizar más
      float adjustment = adaptiveConfig.adjustmentStep * 0.75; // Más conservador al bajar
      
      // Si es EXCELENTE, puede bajar más
      if (efficiencyScore > 85.0 && avgMoistureRecovery > 20.0) {
        adjustment *= 1.3;
      }
      
      analysis.adjustmentAmount = -min(adjustment, adaptiveConfig.maxAdjustmentPerCycle);
      analysis.recommendedThreshold = currentThreshold + analysis.adjustmentAmount;
      analysis.shouldAdjust = true;
      analysis.reasoning = "🟢 Eficiencia alta (" + String(efficiencyScore, 1) + 
                          "%) - Disminuir threshold para riego más anticipativo";
    }
    
    // 🟡 CASO 3: EFICIENCIA MEDIA - AJUSTE FINO
    else if (efficiencyScore >= 60.0 && efficiencyScore <= 70.0) {
      // Zona de ajuste fino basado en otros factores
      
      if (avgIrrigationTime > 120000 && avgMoistureRecovery < 12.0) {
        // Riega mucho tiempo con poca ganancia - SUBIR LEVEMENTE
        analysis.adjustmentAmount = adaptiveConfig.adjustmentStep * 0.5;
        analysis.recommendedThreshold = currentThreshold + analysis.adjustmentAmount;
        analysis.shouldAdjust = true;
        analysis.reasoning = "🟡 Tiempo excesivo con baja recuperación - Ajuste fino +";
      }
      else if (avgIrrigationTime < 45000 && avgMoistureRecovery > 18.0) {
        // Riega poco tiempo con buena ganancia - BAJAR LEVEMENTE
        analysis.adjustmentAmount = -adaptiveConfig.adjustmentStep * 0.5;
        analysis.recommendedThreshold = currentThreshold + analysis.adjustmentAmount;
        analysis.shouldAdjust = true;
        analysis.reasoning = "🟡 Recuperación rápida y eficiente - Ajuste fino -";
      }
      else {
        analysis.reasoning = "✅ Threshold óptimo - Sin cambios necesarios";
      }
    }
    
    // 🔵 CASO 4: ESTABLE - MANTENER
    else {
      analysis.reasoning = "✅ Rendimiento estable - Threshold adecuado";
    }
    
    // ===========================
    // APLICAR LÍMITES DE SEGURIDAD
    // ===========================
    if (analysis.shouldAdjust) {
      analysis.recommendedThreshold = constrain(
        analysis.recommendedThreshold,
        adaptiveConfig.minThreshold,
        adaptiveConfig.maxThreshold
      );
      
      // Verificar que el cambio sea significativo
      if (abs(analysis.recommendedThreshold - currentThreshold) < 0.5) {
        analysis.shouldAdjust = false;
        analysis.reasoning += " | Cambio insignificante, mantener";
      }
    }
    
    return analysis;
  }
  
  // ===========================
  // APLICAR OPTIMIZACIÓN
  // ===========================
  bool applyOptimization(
    int zone,
    float& thresholdToAdjust,
    unsigned long& lastOptimizationTime,
    float efficiencyScore,
    float avgMoistureRecovery,
    float avgIrrigationTime,
    int successfulCycles,
    int totalCycles
  ) {
    unsigned long currentTime = millis();
    
    // Verificar intervalo de tiempo
    if (currentTime - lastOptimizationTime < adaptiveConfig.optimizationInterval &&
        totalCycles < adaptiveConfig.minCyclesForOptimization * 2) {
      return false; // Muy pronto para optimizar
    }
    
    // Analizar zona
    ThresholdAnalysis analysis = analyzeZoneThreshold(
      zone,
      thresholdToAdjust,
      efficiencyScore,
      avgMoistureRecovery,
      avgIrrigationTime,
      successfulCycles,
      totalCycles
    );
    
    // Aplicar cambios si es necesario
    if (analysis.shouldAdjust) {
      float oldThreshold = thresholdToAdjust;
      thresholdToAdjust = analysis.recommendedThreshold;
      lastOptimizationTime = currentTime;
      
      Serial.printf("\n╔════════════════════════════════════════╗\n");
      Serial.printf("║  🧠 OPTIMIZACIÓN ZONA %d - BIDIRECCIONAL ║\n", zone);
      Serial.printf("╠════════════════════════════════════════╣\n");
      Serial.printf("║ Threshold: %.1f%% → %.1f%% (Δ %.1f%%)   \n", 
                    oldThreshold, thresholdToAdjust, analysis.adjustmentAmount);
      Serial.printf("║ Eficiencia: %.1f%% | Score: %.2f        \n", 
                    efficiencyScore, calculateCompositeScore(
                      efficiencyScore, avgMoistureRecovery, avgIrrigationTime / 1000.0));
      Serial.printf("║ Recuperación: %.1f%% | Tiempo: %.1fs   \n", 
                    avgMoistureRecovery, avgIrrigationTime / 1000.0);
      Serial.printf("║ Ciclos: %d/%d exitosos                 \n", 
                    successfulCycles, totalCycles);
      Serial.printf("╠════════════════════════════════════════╣\n");
      Serial.printf("║ 📊 %s\n", analysis.reasoning.c_str());
      Serial.printf("╚════════════════════════════════════════╝\n\n");
      
      return true;
    }
    
    return false;
  }
  
  // Obtener estado del sistema
  void printSystemStatus(int zone, ThresholdAnalysis analysis) {
    Serial.printf("[ADAPTIVE-Z%d] Status: %s\n", zone, analysis.reasoning.c_str());
    Serial.printf("[ADAPTIVE-Z%d] Composite Score: %.2f | Threshold: %.1f%%\n",
                  zone, 
                  calculateCompositeScore(analysis.efficiencyScore, 
                                         analysis.recoveryRate, 
                                         analysis.avgIrrigationTime / 1000.0),
                  analysis.currentThreshold);
  }
};

#endif // ADAPTIVE_LEARNING_H