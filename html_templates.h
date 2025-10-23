// html_templates.h
#ifndef HTML_TEMPLATES_H
#define HTML_TEMPLATES_H

#include <Arduino.h>

// ============================
// HEADER CON CSS
// ============================
const char HTML_HEADER[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control de Riego Inteligente - {WIFI_STATUS}</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
        :root {
            --primary: #6366f1;
            --primary-dark: #4f46e5;
            --secondary: #06b6d4;
            --success: #10b981;
            --warning: #f59e0b;
            --danger: #ef4444;
            --dark: #1f2937;
            --light: #f8fafc;
            --gray: #64748b;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        body {
            background: linear-gradient(135deg, #1e293b 0%, #0f172a 100%);
            color: var(--light);
            min-height: 100vh;
            padding: 20px;
            overflow-x: hidden;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            padding: 30px;
            background: linear-gradient(135deg, rgba(30, 41, 59, 0.9), rgba(15, 23, 42, 0.9));
            border-radius: 25px;
            box-shadow: 
                12px 12px 24px rgba(0, 0, 0, 0.25),
                -12px -12px 24px rgba(255, 255, 255, 0.05),
                inset 4px 4px 8px rgba(0, 0, 0, 0.15),
                inset -4px -4px 8px rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(15px);
            position: relative;
            overflow: hidden;
        }

        .header h1 {
            font-size: 2.8rem;
            background: linear-gradient(135deg, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 15px;
            text-shadow: 0 0 30px rgba(99, 102, 241, 0.4);
            font-weight: 700;
        }

        .header p {
            color: #94a3b8;
            font-size: 1.1rem;
            font-weight: 300;
        }
        .ia-dashboard-btn {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            padding: 0.4rem 0.8rem;
            border-radius: 20px;
            text-decoration: none;
            font-size: 0.9rem;
            margin-left: 1rem;
            transition: all 0.3s ease;
        }
        .ia-dashboard-btn:hover {
            transform: scale(1.05);
            box-shadow: 0 3px 10px rgba(0,0,0,0.2);
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(380px, 1fr));
            gap: 30px;
            margin-bottom: 40px;
        }

        .card {
            background: linear-gradient(135deg, rgba(30, 41, 59, 0.9), rgba(15, 23, 42, 0.9));
            border-radius: 25px;
            padding: 30px;
            box-shadow: 
                10px 10px 20px rgba(0, 0, 0, 0.2),
                -10px -10px 20px rgba(255, 255, 255, 0.05),
                inset 4px 4px 8px rgba(0, 0, 0, 0.15),
                inset -4px -4px 8px rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(12px);
            position: relative;
            overflow: visible;
        }

        .card h2 {
            color: var(--primary);
            margin-bottom: 25px;
            font-size: 1.6rem;
            display: flex;
            align-items: center;
            gap: 12px;
            text-shadow: 0 2px 4px rgba(0,0,0,0.3);
        }

        /* Estilos para el sistema de pestañas */
        .tabs {
            display: flex;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            margin-bottom: 15px;
            flex-wrap: wrap;
        }

        .tablink {
            background: rgba(15, 23, 42, 0.7);
            border: none;
            padding: 8px 10px;
            cursor: pointer;
            font-size: 0.8rem;
            color: #94a3b8;
            flex: 1;
            text-align: center;
            transition: all 0.3s ease;
            border-radius: 5px 5px 0 0;
            margin: 0 1px;
            min-width: 80px;
            max-width: 90px;
        }

        .tablink:hover {
            background: rgba(99, 102, 241, 0.2);
            color: #e2e8f0;
        }

        .tablink.active {
            background: rgba(99, 102, 241, 0.4);
            color: white;
            border-bottom: 2px solid var(--primary);
        }

        .tabcontent {
            display: none;
            animation: fadeEffect 0.5s;
        }

        @keyframes fadeEffect {
            from {opacity: 0;}
            to {opacity: 1;}
        }

        /* Ajustes para los formularios dentro de las pestañas */
        .tabcontent input, 
        .tabcontent select {
            margin: 8px 0;
            padding: 12px;
        }

        .tabcontent .window-title {
            margin: 15px 0 10px 0;
            font-size: 1rem;
        }

        .tabcontent .status-row {
            padding: 10px 0;
        }

        .toggle-buttons-container {
            display: flex;
            flex-direction: column;
            gap: 10px;
            margin: 15px 0;
        }

        .toggle-btn {
            display: flex !important;
            align-items: center;
            justify-content: space-between;
            padding: 16px 20px !important;
            text-decoration: none;
            border: none;
            border-radius: 15px;
            font-weight: 600;
            transition: all 0.3s ease;
            box-shadow: 4px 4px 8px rgba(0, 0, 0, 0.2),
                       -4px -4px 8px rgba(255, 255, 255, 0.05);
            border: 2px solid transparent;
        }

        .toggle-btn:hover {
            transform: translateY(-2px);
            box-shadow: 6px 6px 12px rgba(0, 0, 0, 0.25);
        }

        .toggle-btn i {
            font-size: 1.3rem;
            width: 30px;
            text-align: center;
        }

        .toggle-label {
            flex-grow: 1;
            text-align: left;
            margin-left: 12px;
            font-size: 0.95rem;
        }

        .toggle-status {
            padding: 8px 16px;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: bold;
            min-width: 90px;
            text-align: center;
            text-shadow: 0 1px 2px rgba(0,0,0,0.3);
        }

        /* Estados de los botones toggle */
        .btn-toggle-on {
            background: linear-gradient(135deg, #10b981, #059669) !important;
            color: white !important;
            border-color: rgba(16, 185, 129, 0.3) !important;
        }

        .btn-toggle-on .toggle-status {
            background: rgba(255, 255, 255, 0.2);
            color: #ffffff;
        }

        .btn-toggle-off {
            background: linear-gradient(135deg, #ef4444, #dc2626) !important;
            color: white !important;
            border-color: rgba(239, 68, 68, 0.3) !important;
        }

        .btn-toggle-off .toggle-status {
            background: rgba(255, 255, 255, 0.2);
            color: #ffffff;
        }

        .btn-toggle-auto-on {
            background: linear-gradient(135deg, #3b82f6, #1d4ed8) !important;
            color: white !important;
        }

        .btn-toggle-auto-off {
            background: linear-gradient(135deg, #6b7280, #4b5563) !important;
            color: white !important;
        }

        /* Botón de prueba especial */
        .test-btn {
            background: linear-gradient(135deg, #f59e0b, #d97706) !important;
            color: white !important;
            border: 2px solid #f59e0b !important;
            position: relative;
            text-align: center;
            flex-wrap: nowrap;
            min-width: 300px !important;  /* Ancho mínimo para que no se vea pequeño */
            margin: 0 auto !important;    /* Centrado automático */
            display: flex !important;
            align-items: center;
            justify-content: center;
            padding: 16px 20px !important;
        }

        .test-btn .toggle-label {
            margin: 0 12px !important;
            text-align: center;
            flex-grow: 0 !important;
        }

        .test-btn .warning-badge {
            margin-left: 8px !important;
            white-space: nowrap;
        }

        /* Opcional: Si quieres que ocupe todo el ancho disponible */
        .test-btn-container {
            display: flex;
            justify-content: center;
            width: 100%;
            margin: 15px 0;
        }

        .warning-badge {
            background: #dc2626;
            color: white;
            padding: 6px 12px;
            border-radius: 15px;
            font-size: 0.75rem;
            font-weight: bold;
            margin-left: 10px;
            animation: pulse-warning 2s infinite;
        }

        @keyframes pulse-warning {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.8; transform: scale(1.05); }
        }

        .test-btn:hover {
            background: linear-gradient(135deg, #fbbf24, #f59e0b) !important;
            transform: translateY(-2px);
        }

        /* Responsive para botones toggle */
        @media (max-width: 768px) {
            .toggle-btn {
                padding: 14px 16px !important;
            }
            
            .toggle-label {
                font-size: 0.9rem;
            }
            
            .toggle-status {
                min-width: 80px;
                font-size: 0.75rem;
                padding: 6px 12px;
            }
            
            .warning-badge {
                font-size: 0.7rem;
                padding: 4px 8px;
            }
        }

        .sensor-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));
            gap: 12px;
            align-items: start;
        }

        .sensor-item {
            background: rgba(15, 23, 42, 0.7);
            padding: 10px;                    /* ← Reducir padding */
            border-radius: 20px;
            text-align: center;
            box-shadow: 
                inset 4px 4px 8px rgba(0, 0, 0, 0.2),
                inset -4px -4px 8px rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.08);
            position: relative;
            overflow: visible;                /* ← Cambiar de hidden a visible */
            min-height: 140px;                /* ← Usar min-height en lugar de height */
            max-height: 150px;
            display: flex;
            flex-direction: column;
            justify-content: center;          /* ← Centrar en lugar de space-between */
            gap: 0px;                         /* ← Añadir espacio entre elementos */
        }

        /* Todos los estilos de iconos de sensores animados */
        .sensor-temp-icon {
            position: relative;
            font-size: 1.6rem;
            margin-bottom: 10px;
        }

        .sensor-temp-icon::before {
            content: "🌡️";
            font-size: 2rem;
            filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
        }

        .sensor-temp-icon::after {
            content: "";
            position: absolute;
            bottom: -6px;
            left: 50%;
            width: 24px;
            height: 4px;
            background: linear-gradient(90deg, #ff6b6b, #ff8e8e);
            border-radius: 3px;
            transform: translateX(-50%);
            animation: temp-pulse 2s ease-in-out infinite;
        }

        @keyframes temp-pulse {
            0%, 100% { opacity: 0.7; width: 24px; }
            50% { opacity: 1; width: 30px; }
        }

        .sensor-humidity-icon {
            position: relative;
            font-size: 1.6rem;
            margin-bottom: 10px;
        }

        .sensor-humidity-icon::before {
            content: "💧";
            font-size: 2rem;
            filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
        }

        .sensor-humidity-icon::after {
            content: "💧";
            font-size: 1.3rem;
            position: absolute;
            top: -10px;
            right: -10px;
            opacity: 0.7;
            animation: droplet-fall 3s ease-in-out infinite;
        }

        @keyframes droplet-fall {
            0%, 100% { transform: translateY(0) scale(1); opacity: 0.7; }
            50% { transform: translateY(6px) scale(1.1); opacity: 1; }
        }

        .sensor-soil-icon {
            position: relative;
            font-size: 1.6rem;
            margin-bottom: 10px;
        }

        .sensor-soil-icon::before {
            content: "🌱";
            font-size: 2rem;
            filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
        }

        .sensor-soil-icon::after {
            content: "💧";
            font-size: 1.1rem;
            position: absolute;
            top: -6px;
            right: -6px;
            text-shadow: 0 2px 4px rgba(0,0,0,0.3);
            animation: drip 2s ease-in-out infinite;
        }

        @keyframes drip {
            0%, 100% { transform: translateY(0) scale(1); }
            50% { transform: translateY(3px) scale(1.1); }
        }

        .sensor-light-icon {
            position: relative;
            font-size: 1.6rem;
            margin-bottom: 10px;
        }

        .sensor-light-icon::before {
            content: "☀️";
            font-size: 2rem;
            filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
        }

        .sensor-light-icon::after {
            content: "✨";
            font-size: 1rem;
            position: absolute;
            top: -6px;
            right: -6px;
            animation: sparkle 2s ease-in-out infinite;
        }

        @keyframes sparkle {
            0%, 100% { opacity: 0.5; transform: scale(1) rotate(0deg); }
            50% { opacity: 1; transform: scale(1.3) rotate(25deg); }
        }

        .water-supply-icon {
            position: relative;
            font-size: 1.6rem;
            margin-bottom: 10px;
        }     

        .water-supply-icon::before {
            content: "💧";
            font-size: 2rem;
            filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
            animation: water-drip 2s ease-in-out infinite;
        }

        .water-supply-icon::after {
            content: "❌";
            font-size: 1.2rem;
            position: absolute;
            top: -5px;
            right: -8px;
            opacity: 0;
            transform: scale(0.5);
            transition: all 0.3s ease;
        }

        .water-supply-icon.no-water::after {
            opacity: 1;
            transform: scale(1);
            text-shadow: 0 0 10px rgba(239, 68, 68, 0.8);
        }

        @keyframes water-drip {
            0%, 100% { 
                transform: scale(1) translateY(0);
                text-shadow: 0 0 15px rgba(6, 182, 212, 0.6);
            }
            50% { 
                transform: scale(1.1) translateY(2px);
                text-shadow: 0 0 25px rgba(6, 182, 212, 1);
            }
        }

        .water-supply-icon.no-water::before {
            animation: water-alert 1.5s ease-in-out infinite;
            filter: grayscale(0.8) drop-shadow(0 3px 6px rgba(0,0,0,0.4));
        }

        @keyframes water-alert {
            0%, 100% { 
                transform: scale(1);
                text-shadow: 0 0 10px rgba(239, 68, 68, 0.6);
            }
            50% { 
                transform: scale(1.1);
                text-shadow: 0 0 20px rgba(239, 68, 68, 1);
            }
        }

        /* Colores de sensores */
        .sensor-item.temp-sensor {
            background: linear-gradient(135deg, rgba(255, 107, 107, 0.8), rgba(255, 142, 142, 0.8));
            border: 1px solid rgba(255, 200, 200, 0.3);
        }

        .sensor-item.humidity-sensor {
            background: linear-gradient(135deg, rgba(6, 182, 212, 0.8), rgba(99, 102, 241, 0.8));
            border: 1px solid rgba(200, 220, 255, 0.3);
        }

        .sensor-item.soil-sensor {
            background: linear-gradient(135deg, rgba(139, 69, 19, 0.8), rgba(101, 67, 33, 0.8));
            border: 1px solid rgba(210, 180, 140, 0.4);
        }

        .sensor-item.light-sensor {
            background: linear-gradient(135deg, rgba(245, 158, 11, 0.8), rgba(249, 115, 22, 0.8));
            border: 1px solid rgba(255, 220, 150, 0.4);
        }

        .sensor-item.water-sensor {
            background: linear-gradient(135deg, rgba(6, 182, 212, 0.8), rgba(99, 102, 241, 0.8));
            border: 1px solid rgba(200, 220, 255, 0.3);
        }

        .sensor-item.water-sensor.no-water {
            background: linear-gradient(135deg, rgba(239, 68, 68, 0.8), rgba(245, 158, 11, 0.8));
            border: 1px solid rgba(255, 200, 200, 0.3);
        }
        
        .sensor-value {
            font-size: 2rem;
            font-weight: bold;
            margin: 8px 0;
            color: #ffffff;
            text-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }

        .sensor-label {
            font-size: 0.8rem;
            color: var(--light);
            text-transform: uppercase;
            letter-spacing: 1px;
            font-weight: 600;
            margin-top: 2px;  /* REDUCIDO de 5px a 2px */
            margin-bottom: 0; /* AGREGADO para eliminar espacio inferior */
            line-height: 1.2; /* MEJORADO para texto más compacto */
            text-shadow: 0 1px 2px rgba(0,0,0,0.3);
        }

        /* VPD Sensor */
        .sensor-item.vpd-sensor {
            background: linear-gradient(135deg, rgba(16, 185, 129, 0.8), rgba(5, 150, 105, 0.8));
            border: 1px solid rgba(200, 255, 220, 0.4);
        }

        .sensor-vpd-icon {
          position: relative;
          font-size: 1.6rem;
          margin-bottom: 10px;
        }

        .sensor-vpd-icon::before {
          content: "🌿";
          font-size: 2rem;
          filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
        }

        .sensor-vpd-icon::after {
          content: "💨";
          font-size: 1.1rem;
          position: absolute;
          top: -6px;
          right: -6px;
          animation: vpd-flow 3s ease-in-out infinite;
        }

        @keyframes vpd-flow {
          0%, 100% { 
          opacity: 0.6; 
          transform: translateX(0) scale(1); 
        }
        50% { 
        opacity: 1; 
        transform: translateX(3px) scale(1.1); 
        }
        }

        .vpd-quality {
          color: white;
          text-shadow: 0 1px 2px rgba(0,0,0,0.5);
        }

        /* Resto de estilos de status, botones, etc. */
        .status-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 14px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.15);
        }

        .status-row:last-child {
            border-bottom: none;
        }

        .status-badge {
            padding: 10px 18px;
            border-radius: 25px;
            font-size: 0.95rem;
            font-weight: 700;
            box-shadow: 
                inset 4px 4px 8px rgba(0, 0, 0, 0.25),
                inset -4px -4px 8px rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.15);
            min-width: 100px;
            text-align: center;
        }

        .status-on {
            background: linear-gradient(135deg, var(--success), #059669);
            color: white;
            text-shadow: 0 1px 2px rgba(0,0,0,0.3);
        }

        .status-off {
            background: linear-gradient(135deg, var(--danger), #dc2626);
            color: white;
            text-shadow: 0 1px 2px rgba(0,0,0,0.3);
        }

        .btn-group {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-bottom: 20px;
        }

        .btn {
            padding: 16px 20px;
            border: none;
            border-radius: 18px;
            font-weight: 700;
            cursor: pointer;
            text-decoration: none;
            text-align: center;
            display: block;
            box-shadow: 
                6px 6px 12px rgba(0, 0, 0, 0.25),
                -6px -6px 12px rgba(255, 255, 255, 0.08),
                inset 3px 3px 6px rgba(0, 0, 0, 0.15),
                inset -3px -3px 6px rgba(255, 255, 255, 0.08);
            position: relative;
            overflow: hidden;
            border: 1px solid rgba(255, 255, 255, 0.15);
            font-size: 0.95rem;
        }

        .btn-success {
            background: linear-gradient(135deg, var(--success), #059669);
            color: white;
        }

        .btn-danger {
            background: linear-gradient(135deg, var(--danger), #dc2626);
            color: white;
        }

        .btn-primary {
            background: linear-gradient(135deg, var(--primary), var(--primary-dark));
            color: white;
        }

        .btn-warning {
            background: linear-gradient(135deg, var(--warning), #d97706);
            color: white;
        }

        .btn-outline {
            background: transparent !important;
            border: 2px solid var(--primary) !important;
            color: var(--primary) !important;
            box-shadow: 
                4px 4px 8px rgba(0, 0, 0, 0.2),
                -4px -4px 8px rgba(255, 255, 255, 0.05);
        }

        .btn-outline:hover {
            background: var(--primary) !important;
            color: white !important;
            transform: translateY(-3px);
        }

        /* Estilos para calibración de sensores de humedad */
        .calibration-step {
            border: 2px solid rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 15px;
            background: rgba(15, 23, 42, 0.5);
            transition: all 0.3s ease;
        }

        .calibration-step.active {
            border-color: #6366f1;
            background: rgba(99, 102, 241, 0.1);
            box-shadow: 0 0 15px rgba(99, 102, 241, 0.3);
        }

        .step-header {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
            font-weight: 600;
            flex-wrap: wrap;
        }

        .step-number {
            background: #6366f1;
            color: white;
            width: 30px;
            height: 30px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            margin-right: 15px;
            font-weight: bold;
        }

        .step-title {
            flex-grow: 1;
            color: #6366f1;
            font-size: 1.1rem;
        }

        .step-status {
            padding: 5px 12px;
            border-radius: 15px;
            font-size: 0.8rem;
            background: #6b7280;
            color: white;
            min-width: 80px;
            text-align: center;
        }

        .step-status.completed {
            background: #10b981;
        }

        .step-content p {
            margin: 8px 0;
            font-size: 0.9rem;
            line-height: 1.4;
        }

        .step-controls {
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
        }

        .calibration-results {
            background: rgba(15, 23, 42, 0.7);
            padding: 15px;
            border-radius: 10px;
            margin-bottom: 15px;
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        .result-item {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            font-size: 0.9rem;
            padding: 5px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .result-item:last-child {
            border-bottom: none;
            margin-bottom: 0;
        }

        /* Responsive */
        @media (max-width: 768px) {
            .calibration-step {
                padding: 15px;
            }
            
            .step-header {
                flex-direction: column;
                align-items: flex-start;
            }
            
            .step-number {
                margin-bottom: 10px;
            }
        }

        input, select {
            width: 100%;
            padding: 16px;
            margin: 10px 0;
            border: none;
            border-radius: 15px;
            background: rgba(15, 23, 42, 0.7);
            color: var(--light);
            box-shadow: 
                inset 4px 4px 8px rgba(0, 0, 0, 0.2),
                inset -4px -4px 8px rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            font-size: 0.95rem;
        }

        input:focus, select:focus {
            outline: none;
            box-shadow: 
                0 0 0 3px rgba(99, 102, 241, 0.3),
                inset 4px 4px 8px rgba(0, 0, 0, 0.2),
                inset -4px -4px 8px rgba(255, 255, 255, 0.05);
            background: rgba(15, 23, 42, 0.9);
        }

        .signal-strength {
            display: flex;
            align-items: center;
            gap: 6px;
        }

        .signal-bar {
            width: 5px;
            background: var(--success);
            border-radius: 3px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }

        /* Responsive design */
        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
                gap: 20px;
            }
            
            .header h1 {
                font-size: 2.2rem;
            }
            
            .btn-group {
              grid-template-columns: 1fr;
            }
                  
            .sensor-grid {
                grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
                gap: 15px;
            }

            .tabs {
                flex-direction: column;
            }
            
            .tablink {
                border-radius: 5px;
                margin: 2px 0;
                text-align: left;
                padding: 12px 15px;
            }
            
            .tabcontent {
                padding: 5px;
            }
            
            .tabcontent input, 
            .tabcontent select {
                padding: 14px;
                font-size: 16px;
            }
        }

        /* Estilos específicos adicionales */
        .pump-icon {
            position: relative;
            font-size: 1.6rem;
            margin-right: 12px;
        }

        .pump-icon::before {
            content: "💧";
            font-size: 1.8rem;
            filter: drop-shadow(0 3px 6px rgba(0,0,0,0.4));
            animation: water-flow 2s ease-in-out infinite;
        }

        .pump-icon::after {
            content: "⚡";
            font-size: 1rem;
            position: absolute;
            top: -5px;
            right: -8px;
            animation: energy-pulse 1.5s ease-in-out infinite;
        }

        @keyframes water-flow {
            0%, 100% { 
                transform: scale(1) rotate(0deg);
                text-shadow: 0 0 10px rgba(6, 182, 212, 0.5);
            }
            50% { 
                transform: scale(1.1) rotate(5deg);
                text-shadow: 0 0 20px rgba(6, 182, 212, 0.8);
            }
        }

        @keyframes energy-pulse {
            0%, 100% { 
                opacity: 0.7;
                transform: scale(1) rotate(0deg);
            }
            50% { 
                opacity: 1;
                transform: scale(1.3) rotate(15deg);
                text-shadow: 0 0 15px rgba(255, 230, 0, 0.8);
            }
        }

        .pump-active .pump-icon::before {
            animation: water-flow-active 1s ease-in-out infinite;
            color: #10b981;
        }

        @keyframes water-flow-active {
            0%, 100% { 
                transform: scale(1.2) rotate(0deg);
                text-shadow: 0 0 15px rgba(16, 185, 129, 0.6);
            }
            50% { 
                transform: scale(1.4) rotate(10deg);
                text-shadow: 0 0 25px rgba(16, 185, 129, 1);
            }
        }

        .pump-active .pump-icon::after {
            animation: energy-pulse-active 0.8s ease-in-out infinite;
        }

        @keyframes energy-pulse-active {
            0%, 100% { 
                opacity: 1;
                transform: scale(1.5) rotate(0deg);
                text-shadow: 0 0 20px rgba(255, 230, 0, 1);
            }
            50% { 
                opacity: 0.8;
                transform: scale(1.8) rotate(20deg);
                text-shadow: 0 0 30px rgba(255, 230, 0, 1);
            }
        }

        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.08); }
            100% { transform: scale(1); }
        }

        .glow {
            animation: glow 2s ease-in-out infinite alternate;
        }

        @keyframes glow {
            from {
                box-shadow: 0 0 15px rgba(99, 102, 241, 0.4),
                            6px 6px 12px rgba(0, 0, 0, 0.25),
                            -6px -6px 12px rgba(255, 255, 255, 0.08);
            }
            to {
                box-shadow: 0 0 25px rgba(99, 102, 241, 0.7),
                            8px 8px 20px rgba(0, 0, 0, 0.3),
                            -8px -8px 20px rgba(255, 255, 255, 0.1);
            }
        }

        .window-title {
            font-size: 1.1rem;
            font-weight: 600;
            color: var(--primary);
            margin: 25px 0 15px 0;
            padding-bottom: 10px;
            border-bottom: 2px solid rgba(99, 102, 241, 0.3);
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .log-window {
            background: rgba(15, 23, 42, 0.6);
            border-radius: 12px;
            padding: 15px;
            height: 200px;
            overflow-y: auto;
            border: 1px solid rgba(255, 255, 255, 0.1);
            margin-bottom: 20px;
        }

        .log-content {
            font-family: 'Courier New', monospace;
            font-size: 0.75rem;
            color: #94a3b8;
            line-height: 1.3;
        }

        
  .mode-title {
    color: #7d8ff5;
    font-size: 14px;
    font-weight: 600;
    margin-bottom: 15px;
    display: flex;
    align-items: center;
    gap: 8px;
  }

  .mode-title::before {
    content: "🎮";
    font-size: 16px;
  }

  .mode-container {
    display: flex;
    gap: 10px;
    flex-wrap: nowrap; /* Forzar una sola línea */
  }

  .mode-button {
    flex: 1;
    min-width: 0; /* Permitir que se compriman */
    padding: 12px 16px;
    border: none;
    border-radius: 12px;
    font-size: 13px;
    font-weight: 600;
    cursor: pointer;
    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    position: relative;
    overflow: hidden;
    color: rgba(255, 255, 255, 0.6);
    background: rgba(255, 255, 255, 0.08);
    box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
    text-transform: uppercase;
    letter-spacing: 0.3px;
    white-space: nowrap;
  }

  .mode-button::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
    transition: left 0.5s;
  }

  .mode-button:hover::before {
    left: 100%;
  }

  .mode-button:hover {
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);
  }

  .mode-button:active {
    transform: translateY(0px);
  }

  /* Botón MANUAL */
  .mode-button.manual {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
  }

  .mode-button.manual:hover {
    background: linear-gradient(135deg, #7c8ff0 0%, #8659b0 100%);
    box-shadow: 0 6px 20px rgba(102, 126, 234, 0.4);
  }

  .mode-button.manual.active {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.3), 0 6px 20px rgba(102, 126, 234, 0.4);
    color: white;
  }

  /* Botón AUTO */
  .mode-button.auto {
    background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
    color: white;
  }

  .mode-button.auto:hover {
    background: linear-gradient(135deg, #60a5fa 0%, #3b82f6 100%);
    box-shadow: 0 6px 20px rgba(59, 130, 246, 0.4);
  }

  .mode-button.auto.active {
    background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
    box-shadow: 0 0 0 2px rgba(59, 130, 246, 0.3), 0 6px 20px rgba(59, 130, 246, 0.4);
    color: white;
  }

  /* Botón ADAPTATIVO */
  .mode-button.adaptativo {
    background: linear-gradient(135deg, #8b5cf6 0%, #7c3aed 100%);
    color: white;
  }

  .mode-button.adaptativo:hover {
    background: linear-gradient(135deg, #a78bfa 0%, #8b5cf6 100%);
    box-shadow: 0 6px 20px rgba(139, 92, 246, 0.4);
  }

  .mode-button.adaptativo.active {
    background: linear-gradient(135deg, #8b5cf6 0%, #7c3aed 100%);
    box-shadow: 0 0 0 2px rgba(139, 92, 246, 0.3), 0 6px 20px rgba(139, 92, 246, 0.4);
    color: white;
  }

  /* Iconos de modo */
  .mode-button .icon {
    display: inline-block;
    margin-right: 6px;
    font-size: 16px;
  }

  /* Estado activo destacado */
  .mode-button.active {
    transform: scale(1.03);
  }

  .mode-button.active::after {
    content: '✓';
    position: absolute;
    top: 6px;
    right: 6px;
    font-size: 12px;
    background: rgba(255, 255, 255, 0.25);
    width: 18px;
    height: 18px;
    border-radius: 50%;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  /* Indicador de estado */
  .mode-button .status-dot {
    display: inline-block;
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: rgba(255, 255, 255, 0.4);
    margin-left: 6px;
    animation: pulse 2s infinite;
  }

  .mode-button.active .status-dot {
    background: #4ade80;
    box-shadow: 0 0 8px #4ade80;
  }


  @keyframes pulse {
    0%, 100% {
      opacity: 1;
    }
    50% {
      opacity: 0.5;
    }
  }


        .restrictions-window {
            background: rgba(15, 23, 42, 0.7);
            border-radius: 15px;
            padding: 20px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            margin-bottom: 20px;
            box-shadow: 
                inset 3px 3px 6px rgba(0, 0, 0, 0.2),
                inset -3px -3px 6px rgba(255, 255, 255, 0.05);
        }

        .restriction-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }

        .restriction-item:last-child {
            border-bottom: none;
        }

        .restriction-status {
            padding: 8px 16px;
            border-radius: 20px;
            font-size: 0.85rem;
            font-weight: 600;
            min-width: 80px;
            text-align: center;
        }

        .status-active {
            background: linear-gradient(135deg, var(--danger), #dc2626);
            color: white;
            box-shadow: 0 2px 8px rgba(239, 68, 68, 0.3);
        }

        .status-inactive {
            background: linear-gradient(135deg, var(--success), #059669);
            color: white;
            box-shadow: 0 2px 8px rgba(16, 185, 129, 0.3);
        }

        .status-warning {
            background: linear-gradient(135deg, var(--warning), #d97706);
            color: white;
            box-shadow: 0 2px 8px rgba(245, 158, 11, 0.3);
        }

        .restriction-icon {
            font-size: 1.2rem;
            margin-right: 12px;
            width: 30px;
            text-align: center;
        }

        .status-dot {
            display: inline-block;
            width: 8px;
            height: 8px;
            border-radius: 50%;
            margin-left: 5px;
        }

        .test-btn {
            background: linear-gradient(135deg, #f59e0b, #d97706) !important;
            color: white !important;
            border: 2px solid #f59e0b !important;
            position: relative;
        }

        .warning-badge {
          background: #dc2626;
          color: white;
          padding: 4px 8px;
          border-radius: 12px;
          font-size: 0.7rem;
          font-weight: bold;
          margin-left: 10px;
          animation: pulse-warning 2s infinite;
        }

        /* ========================================
        SISTEMA DE TOOLTIPS
        =========================================== */
        [data-tooltip] {
            position: relative !important;
        }

        [data-tooltip]:hover {
            cursor: help;
        }

        /* Tooltip bubble */
        [data-tooltip]::before {
            content: attr(data-tooltip);
            position: absolute;
            bottom: calc(100% + 10px);
            left: 50%;
            transform: translateX(-50%) translateY(-10px);
            background: rgba(15, 23, 42, 0.98);
            color: #ffffff;
            padding: 12px 16px;
            border-radius: 8px;
            font-size: 0.85rem;
            line-height: 1.5;
            white-space: normal;
            max-width: 280px;
            width: max-content;
            z-index: 10000;
            pointer-events: none;
            opacity: 0;
            visibility: hidden;
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
            border: 1px solid rgba(99, 102, 241, 0.5);
            text-align: center;
            font-weight: 400;
        }

        /* Tooltip arrow */
        [data-tooltip]::after {
            content: '';
            position: absolute;
            bottom: calc(100% + 4px);
            left: 50%;
            transform: translateX(-50%);
            border: 6px solid transparent;
            border-top-color: rgba(15, 23, 42, 0.98);
            opacity: 0;
            visibility: hidden;
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            z-index: 9999;
        }

        /* Show on hover */
        [data-tooltip]:hover::before {
            opacity: 1;
            visibility: visible;
            transform: translateX(-50%) translateY(0);
        }

        [data-tooltip]:hover::after {
            opacity: 1;
            visibility: visible;
        }

        .toggle-btn-wrapper {
            display: block;
            width: 100%;
        }

        .toggle-btn-wrapper .toggle-btn {
            width: 100%;
            display: flex !important;
        }

      @keyframes pulse-warning {
          0%, 100% { opacity: 1; }
          50% { opacity: 0.7; }
      }

      .test-btn:hover {
          background: linear-gradient(135deg, #fbbf24, #f59e0b) !important;
          transform: translateY(-2px);
      }

       @media (max-width: 600px) {
        .mode-container {
        flex-direction: column;
        }
    
        .mode-button {
        width: 100%;
        min-width: auto;
        }
        }

        /* DEMO: Controles para probar */
        .demo-controls {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
            text-align: center;
        }

        .demo-info {
            color: rgba(255, 255, 255, 0.5);
            font-size: 13px;
            margin-top: 10px;
        }
    }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1><i class="fas fa-seedling"></i> Sistema de Riego Inteligente</h1>
            <p>versión 1.0 - {CURRENT_TIME}</p>
        <a href="/ia-dashboard">🤖 IA Dashboard</a>    
        </div>
        <div class="grid">
)=====";

// ============================
// 1. TARJETA DE SENSORES EN TIEMPO REAL
// ============================
const char HTML_SENSORS_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Sensores en Tiempo Real -->
            <div class="card">
                <h2><i class="fas fa-gauge-high"></i> Sensores locales 
                    <span style="font-size: 0.8rem; color: #10b981; margin-left: 10px;">
                        <i class="fas fa-sync-alt fa-spin" id="sync-icon"></i> 
                        <span id="update-status">Actualizando...</span>
                    </span>
                </h2>
                
                <div class="sensor-grid">
                    <!-- Sensor de Temperatura -->
                    <!-- Sensor de Temperatura -->
                        <div class="sensor-item temp-sensor" data-tooltip="Temperatura actual. Afecta a la evaporación y el crecimiento de las plantas.">
                        <div class="sensor-temp-icon"></div>
                        <div class="sensor-value" id="temp-value">{TEMP_VALUE}°C</div>
                        <div class="sensor-label">Temp. Ambiente</div>
                    </div>

                    <!-- Sensor de Humedad -->
                        <div class="sensor-item humidity-sensor" data-tooltip="Afecta al crecimiento de las plantas, plagas y hongos.">
                        <div class="sensor-humidity-icon"></div>
                        <div class="sensor-value" id="hum-value">{HUM_VALUE}%</div>
                        <div class="sensor-label">Humedad Ambiente</div>
                    </div>

                    <!-- Sensor de Suelo 1 -->
                    <div class="sensor-item soil-sensor" data-tooltip="Sensor de humedad del suelo en zona 1">
                        <div class="sensor-soil-icon"></div>
                        <div class="sensor-value" id="soil1-value">{SOIL1_VALUE}%</div>
                        <div class="sensor-label">Hum. Suelo 1</div>
                    </div>

                    <!-- Sensor de Suelo 2 -->
                    <div class="sensor-item soil-sensor" data-tooltip="Sensor de humedad del suelo en zona 2">
                        <div class="sensor-soil-icon"></div>
                        <div class="sensor-value" id="soil2-value">{SOIL2_VALUE}%</div>
                        <div class="sensor-label">Hum. Suelo 2</div>
                    </div>

                    <!-- Sensor de Luz Solar -->
                    <div class="sensor-item light-sensor" data-tooltip="Radiación solar medida en Lux, no tapar el sensor, afecta al riego">
                        <div class="sensor-light-icon"></div>
                        <div class="sensor-value" id="light-value">{LIGHT_VALUE}%</div>
                        <div class="sensor-label">Luz Solar</div>
                    </div>

                    <!-- Sensor de Suministro de Agua -->
                    <div class="sensor-item water-sensor {WATER_CLASS}" id="water-sensor" data-tooltip="Verifica si hay agua en deposito o flujo para evitar daños al sistema">
                        <div class="water-supply-icon {WATER_ICON_CLASS}" id="water-icon"></div>
                        <div class="sensor-value" id="water-value">{WATER_STATUS}</div>
                        <div class="sensor-label">Suministro Agua</div>
                    </div>

                    <!-- Sensor VPD -->
                    <div class="sensor-item vpd-sensor" style="display: {VPD_DISPLAY};" id="vpd-container" data-tooltip="Ecuación de Magnus-Tetens. VPD = Presión diferencial de vapor. La planta evapotranspira en función de: Temperatura, humedad del sustrato, humedad ambiente, radiación solar, presión atmosférica, viento. El valor nos dice si debemos regar o no, pero el sistema adaptativo tomará la decisión.">
                        <div class="sensor-vpd-icon"></div>
                        <div class="sensor-value" id="vpd-value">{VPD_VALUE}</div>
                        <div class="sensor-label">VPD</div>
                        <div class="vpd-quality" id="vpd-quality" style="font-size: 0.7rem; margin-top: 5px; font-weight: bold; color: {VPD_COLOR};">
                            {VPD_QUALITY}
                        </div>
                    </div>
                    <!-- Sensor Presión Atmosférica -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, #0ea5e9, #0284c7);" data-tooltip="Mide la presión atmosférica y se utiliza para el calculo del riego inteligente y adaptativo">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">🌡️</div>
                        <div class="sensor-value" id="pressure-value">--</div>
                        <div class="sensor-label">Presión (mbar)</div>
                    </div>
                    <!-- Sensor Calidad Aire -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, #8b5cf6, #7c3aed);" data-tooltip="Mide la volatilidad de los compuestos orgánicos (pudrimiento de las raices, hongos, contaminación)">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">💨</div>
                        <div class="sensor-value" id="airquality-value">--</div>
                        <div class="sensor-label">Calidad Aire</div>
                        <div class="vpd-quality" id="vpd-quality" style="font-size: 0.7rem; margin-top: 5px; font-weight: bold; color: {VOC_COLOR};">
                            {VOC_QUALITY}
                        </div>
                    </div>
                    <!-- ICONO RESUMEN AMBIENTAL -->
                    <div class="sensor-item" id="ambient-summary" style="{AMBIENT_GRADIENT}" data-tooltip="Es el resumen de todos los sensores, ecuaciones, valores y nos informa si necesita riego SI o NO. Sirve para el sistema inteligente y para los que no entendemos nada.">
                        <div style="font-size: 2.5rem; margin-bottom: 8px;" id="ambient-icon">{AMBIENT_ICON}</div>
                        <div class="sensor-value" id="ambient-recommendation" style="font-size: 0.85rem; line-height: 1.2;">{AMBIENT_TEXT}</div>
                        <div class="sensor-label">Estado General</div>
                        <div style="font-size: 0.75rem; margin-top: 5px; color: rgba(255,255,255,0.8);" id="ambient-detail">
                            {AMBIENT_DETAIL}
                        </div>
                    </div>
                </div>
                
                <!-- Indicadores de estado -->
                <div style="display: flex; justify-content: space-between; align-items: center; margin-top: 15px; padding: 10px; background: rgba(15, 23, 42, 0.5); border-radius: 10px;">
                    <div style="font-size: 0.8rem; color: #64748b;">
                        <i class="fas fa-clock"></i> Última actualización: 
                        <span id="last-update">{CURRENT_TIME}</span>
                    </div>
                    <div style="font-size: 0.8rem; color: #64748b;">
                        <i class="fas fa-microchip"></i> Intervalo: 
                        <span id="update-interval">3 segundos</span>
                    </div>
                </div>
            </div>
)=====";

// ============================
// 3. TARJETA DE CONTROL Y ESTADO
// ============================
const char HTML_CONTROL_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Control y Estado -->
            <div class="card">
                <h2><i class="fas fa-gamepad"></i> CONTROL Y ESTADO</h2>
                
                <!-- CONTROL DE ZONAS -->
                <div class="window-title">
                    <i class="fas fa-tint"></i> CONTROL DE ZONAS
                </div>
                <div class="btn-group">
                    <a href='/pump1on' class='btn btn-success'><i class="fas fa-play"></i> Activa Zona 1</a>
                    <a href='/pump1off' class='btn btn-danger'><i class="fas fa-stop"></i> Detiene Zona 1</a>
                    <a href='/pump2on' class='btn btn-success'><i class="fas fa-play"></i> Activa Zona 2</a>
                    <a href='/pump2off' class='btn btn-danger'><i class="fas fa-stop"></i> Detiene Zona 2</a>
                </div>

                <!-- Estado Actual de las Zonas -->
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-top: 15px; margin-bottom: 20px;">
                    <div style="background: rgba(15, 23, 42, 0.7); padding: 12px; border-radius: 10px; text-align: center;">
                        <div style="display: flex; align-items: center; justify-content: center; margin-bottom: 8px;">
                            <div class="pump-icon" style="margin-right: 8px;"></div>
                            <span style="font-weight: 600;">Zona 1</span>
                        </div>
                        <div class="status-badge {PUMP1_STATUS_CLASS}" id="pump1-status-indicator" style="font-size: 0.9rem;">
                            {PUMP1_STATUS_TEXT}
                        </div>
                    </div>
                    <div style="background: rgba(15, 23, 42, 0.7); padding: 12px; border-radius: 10px; text-align: center;">
                        <div style="display: flex; align-items: center; justify-content: center; margin-bottom: 8px;">
                            <div class="pump-icon" style="margin-right: 8px;"></div>
                            <span style="font-weight: 600;">Zona 2</span>
                        </div>
                        <div class="status-badge {PUMP2_STATUS_CLASS}" id="pump2-status-indicator" style="font-size: 0.9rem;">
                            {PUMP2_STATUS_TEXT}
                        </div>
                    </div>
                </div>

                <!-- MODO DE OPERACIÓN -->
                <div class="mode-section">
                <div class="mode-title">MODO DE OPERACIÓN</div>

                    <div class="mode-container">
                    <div class="toggle-btn-wrapper" data-tooltip="Puede activar/desactivar las zonas siempre que haya agua. Para hacerlo sin agua desactiva el botón de suministro de agua">
                    <button class="mode-button manual" onclick="setMode('manual')">
                        <span class="text">MANUAL</span>
                        <span class="status-dot"></span>
                    </button>
                    </div>
                    
                    <div class="toggle-btn-wrapper" data-tooltip="Riega de acuerdo al ajuste de umbrales y condiciones atmosféricas">
                    <button class="mode-button auto active" onclick="setMode('auto')">
                        <span class="text">AUTOMÁTICO</span>
                        <span class="status-dot"></span>
                    </button>
                    </div>
                    
                    <div class="toggle-btn-wrapper" data-tooltip="Riega de acuerdo a lo aprendido del riego automático y optimiza la salud de las plantas">
                    <button class="mode-button adaptativo" onclick="setMode('adaptativo')">
                        <span class="text">ADAPTATIVO</span>
                        <span class="status-dot"></span>
                    </button>
                    </div>
                    </div>
                </div>

                <!-- MODO DE PRUEBA -->
                <div class="window-title">
                    <i class="fas fa-vial"></i> MODO DE PRUEBA
                </div>
                <div class="toggle-btn-wrapper" data-tooltip="Realiza una prueba de las zonas de 5 segundos. Atención!! Si no hay agua puede perjudicar a los mecanismos">
                <div style="display: flex; justify-content: center; margin-bottom: 20px;">
                    <a href='/testpumps' class='btn btn-warning test-btn' style="min-width: 300px; justify-content: center !important;">
                        <i class="fas fa-play-circle"></i> 
                        <span class="toggle-label">Prueba Zonas (5 seg)</span>
                        <span class="warning-badge">¡PRECAUCIÓN!</span>
                    </a>
                    </div>
                </div>

                <!-- PROTECCIONES DEL SISTEMA -->
                <div class="window-title">
                    <i class="fas fa-shield-alt"></i> PROTECCIONES DEL SISTEMA
                </div>

                <div class="toggle-buttons-container">
                <!-- Protección Solar -->
                <div class="toggle-btn-wrapper" data-tooltip="Activa o desactiva la protección por exceso de radiación solar. Si está activa, no regará cuando la intensidad lumínica supere el umbral configurado. Tarjeta 'Ajustes', pestaña 'Umbral'. Para desactivarlo pulsa el botón">
                    <a href='{LIGHT_PROTECTION_URL}' class='btn {LIGHT_PROTECTION_CLASS} toggle-btn'>
                        <i class='fas fa-sun'></i>
                        <span class="toggle-label">Protección Solar</span>
                        <span class='toggle-status'>{LIGHT_PROTECTION_STATUS}</span>
                    </a>
                </div>

                <!-- Protección Temperatura -->
                <div class="toggle-btn-wrapper" data-tooltip="Protege el sistema cuando la temperatura está fuera del rango configurado (mínimo-máximo). Tarjeta 'Ajustes', pestaña 'Umbral'. Para desactivarlo pulsa el botón">
                    <a href='{TEMP_PROTECTION_URL}' class='btn {TEMP_PROTECTION_CLASS} toggle-btn'>
                        <i class='fas fa-thermometer-half'></i>
                        <span class="toggle-label">Protección Temperatura</span>
                        <span class='toggle-status'>{TEMP_PROTECTION_STATUS}</span>
                    </a>
                </div>

                <!-- Protección Humedad -->
                <div class="toggle-btn-wrapper" data-tooltip="Evita el riego cuando la humedad ambiente supera el umbral configurado. Tarjeta 'Ajustes', pestaña 'Umbral'. Para desactivarlo pulsa el botón">
                    <a href='{HUMIDITY_PROTECTION_URL}' class='btn {HUMIDITY_PROTECTION_CLASS} toggle-btn'>
                        <i class='fas fa-tint'></i>
                        <span class="toggle-label">Protección Humedad</span>
                        <span class='toggle-status'>{HUMIDITY_PROTECTION_STATUS}</span>
                    </a>
                </div>

                <!-- Control Suministro Agua -->
                <div class="toggle-btn-wrapper" data-tooltip="Verifica que hay agua disponible antes de activar las bombas para evitar daños al sistema. Para desactivarlo pulsa el botón">
                    <a href='{WATER_SUPPLY_URL}' class='btn {WATER_SUPPLY_CLASS} toggle-btn'>
                        <i class='fas fa-faucet'></i>
                        <span class="toggle-label">Control Suministro Agua</span>
                        <span class='toggle-status'>{WATER_SUPPLY_STATUS}</span>
                    </a>
                </div>

                <!-- Protección VPD -->
                <div class="toggle-btn-wrapper" data-tooltip="Utiliza el VPD (Déficit de Presión de Vapor) para decidir si las plantas necesitan riego según las condiciones ambientales. Tarjeta 'Ajustes', pestaña 'VPD'. Para desactivarlo pulsa el botón">
                    <a href='{VPD_PROTECTION_URL}' class='btn {VPD_PROTECTION_CLASS} toggle-btn'>
                        <i class='fas fa-leaf'></i>
                        <span class="toggle-label">Protección VPD</span>
                        <span class='toggle-status'>{VPD_PROTECTION_STATUS}</span>
                    </a>
                </div>
                
                <!-- Restricción Horaria -->
                <div class="toggle-btn-wrapper" data-tooltip="Permite configurar horarios específicos donde NO se permite el riego automático. Tarjeta 'Ajustes', pestaña 'Normativas'. Para desactivarlo pulsa el botón">
                    <a href='{TIME_RESTRICTION_URL}' class='btn {TIME_RESTRICTION_STATUS_CLASS} toggle-btn'>
                        <i class='fas fa-clock'></i>
                        <span class="toggle-label">Protección Horaria</span>
                        <span class='toggle-status'>{TIME_RESTRICTION_STATUS_TEXT}</span>
                    </a>
                </div>

                <!-- Restricción Viento -->
                <div class="toggle-btn-wrapper" data-tooltip="Evita el riego cuando hay viento excesivo para prevenir pérdida de agua por evaporación o deriva. Tarjeta 'Ajustes', pestaña 'Normativas'. Para desactivarlo pulsa el botón">
                    <a href='{WIND_RESTRICTION_URL}' class='btn {WIND_RESTRICTION_STATUS_CLASS} toggle-btn'>
                        <i class='fas fa-wind'></i>
                        <span class="toggle-label">Protección Viento</span>
                        <span class='toggle-status'>{WIND_RESTRICTION_STATUS_TEXT}</span>
                    </a>
                </div>
            </div>
        </div>
                
)=====";

// ============================
// 4. TARJETA DE AJUSTES CON PESTAÑAS
// ============================
const char HTML_SETTINGS_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Ajustes Combinada -->
            <div class="card">
                <h2><i class="fas fa-cog"></i> Ajustes</h2>
                
                <!-- Sistema de pestañas -->
                <div class="tabs">
                    <button class="tablink active" onclick="openTab(event, 'tabUmbrales')">
                        <i class="fas fa-sliders-h"></i> Umbral
                    </button>
                    <button class="tablink" onclick="openTab(event, 'tabSalud')">
                        <i class="fas fa-heartbeat"></i> VPD
                    </button>
                    <button class="tablink" onclick="openTab(event, 'tabSoilCalibration')">
                        <i class="fas fa-seedling"></i> Suelo
                    </button>
                    <button class="tablink" onclick="openTab(event, 'tabNormativas')">
                        <i class="fas fa-gavel"></i> Normativas
                    </button>
                </div>
                
                <!-- Pestaña Umbrales -->
                <div id="tabUmbrales" class="tabcontent" style="display:block;">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-sliders-h"></i> CONFIGURACIÓN DE UMBRALES
                        </div>
                        
                        <label>Si humedad del suelo supera el valor, zona 1 nula:</label>
                        <input type='number' name='th1' min='0' max='100' value='{THRESHOLD1_VAL}'>
                        
                        <label>Si humedad del suelo supera el valor, zona 2 nula:</label>
                        <input type='number' name='th2' min='0' max='100' value='{THRESHOLD2_VAL}'>
                        
                        <label>Si radiación solar supera este valor no regará:</label>
                        <input type='number' name='lt' min='0' max='100' value='{LIGHT_THRESHOLD_VAL}'>
                                                              
                        <label>Por debajo de esta temperatura ambiente no regará:</label>
                        <input type='number' name='minTemp' step='0.1' value='{MIN_TEMP_VAL}'>
                        
                        <label>Por encima de esta temperatura ambiente no regará:</label>
                        <input type='number' name='maxTemp' step='0.1' value='{MAX_TEMP_VAL}'>
                                                              
                        <label>Por encima de esta humedad ambiente no regará:</label>
                        <input type='number' name='humidityThreshold' min='0' max='100' step='1' value='{HUM_THRESHOLD_VAL}'>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar cambios
                        </button>
                    </form>
                </div>
                
                

                <!-- Pestaña Salud Vegetal VPD -->
                <div id="tabSalud" class="tabcontent">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-heartbeat"></i> CONFIGURACIÓN VPD
                        </div>
                
                        <label>Protección VPD:</label>
                        <select name='vpdProt'>
                            <option value='1' {VPD_PROT_SELECTED_YES}>Sí - Bloquear si VPD incorrecto</option>
                            <option value='0' {VPD_PROT_SELECTED_NO}>No - Ignorar VPD</option>
                        </select>
                        
                        <label>VPD Mínimo Óptimo (kPa):</label>
                        <input type='number' name='vpdMin' step='0.1' min='0.1' max='2.0' value='{VPD_MIN_VAL}'>
                        
                        <label>VPD Máximo Óptimo (kPa):</label>
                        <input type='number' name='vpdMax' step='0.1' min='0.5' max='3.0' value='{VPD_MAX_VAL}'>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar VPD
                        </button>
                    </form>
                    
                    <!-- Panel informativo VPD actual -->
                    <div style='background: rgba(16, 185, 129, 0.1); border-radius: 12px; padding: 15px; margin-top: 20px;'>
                        <h5 style='color: #10b981; margin-bottom: 10px;'>
                            <i class='fas fa-leaf'></i> Estado VPD Actual
                        </h5>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Valor:</strong> {VPD_CURRENT_VAL} kPa
                        </p>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Calidad:</strong> <span style='color: {VPD_CURRENT_COLOR};'> {VPD_CURRENT_QUALITY} </span>
                        </p>
                        <p style='font-size: 0.85rem; color: #6b7280; margin-top: 8px;'>
                            <i class='fas fa-info-circle'></i> VPD óptimo: 0.8-1.2 kPa | Crítico: <0.3 o >1.8 kPa
                        </p>
                    </div>
                </div>

                <!-- Pestaña Calibración Sustrato -->
                <div id="tabSoilCalibration" class="tabcontent">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-seedling"></i> CALIBRACIÓN DE SUSTRATO
                        </div>
                        
                        <label>Tipo de Sustrato:</label>
                        <select name='soilProfile' onchange='showSubstrateInfo(this.value)'>
                            <option value='universal' {SOIL_UNIVERSAL_SELECTED}>Universal (Genérico)</option>
                            <option value='clay' {SOIL_CLAY_SELECTED}>Arcilla - Alta retención</option>
                            <option value='sandy' {SOIL_SANDY_SELECTED}>Arenoso - rápido drenaje</option>
                            <option value='loam' {SOIL_LOAM_SELECTED}>Franco - Balance perfecto</option>
                            <option value='peat' {SOIL_PEAT_SELECTED}>Turba - Orgánico</option>
                            <option value='coco' {SOIL_COCO_SELECTED}>Fibra de Coco - Aireación</option>
                            <option value='rockwool' {SOIL_ROCKWOOL_SELECTED}>Lana de Roca - Hidroponía</option>
                            <option value='perlite' {SOIL_PERLITE_SELECTED}>Perlita - Ultra drenaje</option>
                            <option value='vermiculite' {SOIL_VERMICULITE_SELECTED}>Vermiculita - Super absorbente</option>
                        </select>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar cambios
                        </button>
                    </form>
                    
                    <!-- Indicadores visuales de sustrato actual -->
                    <div style="margin-top: 20px;">
                        <div class="substrate-indicator" style="font-size: 0.8rem; padding: 8px 12px; border-radius: 15px; display: inline-block; 
                              background: {SUBSTRATE_COLOR};
                              color: white; font-weight: 500; margin-right: 10px;">
                            <i class="{SUBSTRATE_ICON}" style="margin-right: 5px;"></i>
                            {SUBSTRATE_NAME} - Sensor 1
                        </div>
                        
                        <div class="substrate-indicator" style="font-size: 0.8rem; padding: 8px 12px; border-radius: 15px; display: inline-block; 
                              background: {SUBSTRATE_COLOR};
                              color: white; font-weight: 500;">
                            <i class="{SUBSTRATE_ICON}" style="margin-right: 5px;"></i>
                            {SUBSTRATE_NAME} - Sensor 2
                        </div>
                    </div>
                </div>

                    <!-- Pestaña Normativas -->
                    <div id="tabNormativas" class="tabcontent">
                        <form action='/setconfig' method='get'>
                            <div class="window-title">
                                <i class="fas fa-gavel"></i> RESTRICCIONES NORMATIVAS
                            </div>
                            
                            <h4 style="color: #6366f1; margin: 20px 0 10px 0;">
                                <i class="fas fa-clock"></i> Restricción Horaria
                            </h4>
                            <label>Activar restricción horaria:</label>
                            <select name='timeRestriction'>
                                <option value='1' {TIME_RESTRICTION_SELECTED_YES}>Sí - Aplicar horario prohibido</option>
                                <option value='0' {TIME_RESTRICTION_SELECTED_NO}>No - Permitir riego a cualquier hora</option>
                            </select>
                            
                            <label>Hora inicio restricción:</label>
                            <input type='time' name='startTime' value='{RESTRICTION_START_TIME}'>
                            
                            <label>Hora fin restricción:</label>
                            <input type='time' name='endTime' value='{RESTRICTION_END_TIME}'>
                            
                            <div style='background: rgba(99, 102, 241, 0.1); border-radius: 8px; padding: 10px; margin: 15px 0; font-size: 0.85rem;'>
                                <p><i class='fas fa-info-circle'></i> <strong>Ejemplo:</strong> 12:00 - 18:00 prohíbe riego entre mediodía y 6 de la tarde</p>
                                <p><strong>Cruza medianoche:</strong> 22:00 - 06:00 prohíbe riego de 10 PM a 6 AM</p>
                            </div>
                            
                            <h4 style="color: #6366f1; margin: 20px 0 10px 0;">
                                <i class="fas fa-wind"></i> Restricción por Viento
                            </h4>
                            <label>Activar restricción por viento:</label>
                            <select name='windRestriction'>
                                <option value='1' {WIND_RESTRICTION_SELECTED_YES}>Sí - No regar con viento fuerte</option>
                                <option value='0' {WIND_RESTRICTION_SELECTED_NO}>No - Ignorar velocidad del viento</option>
                            </select>
                            
                            <label>No regar si velocidad del viento excede de:</label>
                            <input type='number' name='maxWind' step='0.1' min='0' max='100' value='{MAX_WIND_SPEED}'> km/h
                            
                            <div style='background: rgba(16, 185, 129, 0.1); border-radius: 8px; padding: 10px; margin: 15px 0; font-size: 0.85rem;'>
                                <p><i class='fas fa-info-circle'></i> <strong>Recomendación:</strong> 10-15 km/h para evitar dispersión del agua</p>
                                <p><strong>Viento actual:</strong> <span id="current-wind-speed">{CURRENT_WIND_SPEED}</span> km/h</p>
                            </div>
                            
                            <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                                <i class="fas fa-save"></i> Aplicar cambios
                            </button>
                        </form>
                    </div>
                </div>
)=====";

// ====================================
// 5. TARJETA DE CONFIGURACIÓN AVANZADA (COMPLETA CON SCRIPTS)
// =====================================
const char HTML_ADVANCED_CONFIG_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Configuración Avanzada -->
            <div class="card">
                <h2><i class="fas fa-cog"></i> Configuración Avanzada</h2>
                
                <!-- Sistema de pestañas PRINCIPAL -->
                <div class="tabs">
                    <button class="tablink active" onclick="openTab(event, 'tabHora')">
                        <i class="fas fa-clock"></i> Hora
                    </button>
                    <button class="tablink" onclick="openTab(event, 'tabWeather')">
                        <i class="fas fa-cloud-sun"></i> Meteorología
                    </button>
                    <button class="tablink" onclick="openTab(event, 'tabTelegram')">
                        <i class="fab fa-telegram"></i> Telegram
                    </button>
                    <button class="tablink" onclick="openTab(event, 'tabRed')">
                        <i class="fas fa-network-wired"></i> Red
                    </button>
                </div>
                
                <!-- Pestaña Hora -->
                <div id="tabHora" class="tabcontent" style="display:block;">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-clock"></i> CONFIGURACIÓN DE HORA
                        </div>
                        
                        <label>Zona Horaria (GMT):</label>
                        <select name='gmt'>
                            {GMT_OPTIONS}
                        </select>
                        
                        <label>Horario de Verano:</label>
                        <select name='dst'>
                            <option value='1' {DST_SELECTED_YES}>Activado (+1 hora)</option>
                            <option value='0' {DST_SELECTED_NO}>Desactivado</option>
                        </select>
                        
                        <label>Formato de Hora:</label>
                        <select name='tf'>
                            <option value='1' {TIME_FORMAT_24_SELECTED}>24 horas</option>
                            <option value='0' {TIME_FORMAT_12_SELECTED}>12 horas AM/PM</option>
                        </select>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar Cambios
                        </button>
                    </form>
                    
                    <!-- Panel informativo RTC -->
                    <div style='background: rgba(16, 185, 129, 0.1); border-radius: 12px; padding: 15px; margin-top: 20px;'>
                        <h5 style='color: #10b981; margin-bottom: 10px;'>
                            <i class='fas fa-clock'></i> Estado del reloj del sistema
                        </h5>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Fuente Activa:</strong> {TIME_SOURCE_STATUS}
                        </p>
                        {RTC_STATUS_INFO}
                    </div>
                    
                    <!-- Ajuste Manual RTC -->
                    <form action='/setrtc' method='get'>
                        <div class="window-title" style="margin-top: 25px;">
                            <i class="fas fa-tools"></i> AJUSTE MANUAL RTC
                        </div>
                        
                        <div style='background: rgba(245, 158, 11, 0.1); border-radius: 8px; padding: 10px; margin: 15px 0; font-size: 0.85rem;'>
                            <p style='margin: 0; color: #f59e0b;'>
                                <i class='fas fa-exclamation-triangle'></i>
                                <strong>Importante:</strong> Solo usar cuando no haya conexión a Internet
                            </p>
                        </div>
                        
                        <div style='display:grid; grid-template-columns:repeat(3,1fr); gap:10px; margin-bottom:10px;'>
                            <div><label>Día:</label><input type='number' name='day' min='1' max='31' value='{CURRENT_DAY}' required></div>
                            <div><label>Mes:</label><input type='number' name='month' min='1' max='12' value='{CURRENT_MONTH}' required></div>
                            <div><label>Año:</label><input type='number' name='year' min='2025' max='2099' value='{CURRENT_YEAR}' required></div>
                        </div>
                        
                        <div style='display:grid; grid-template-columns:repeat(3,1fr); gap:10px;'>
                            <div><label>Hora:</label><input type='number' name='hour' min='0' max='23' value='{CURRENT_HOUR}' required></div>
                            <div><label>Min:</label><input type='number' name='minute' min='0' max='59' value='{CURRENT_MINUTE}' required></div>
                            <div><label>Seg:</label><input type='number' name='second' min='0' max='59' value='0'></div>
                        </div>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px; background: linear-gradient(135deg, #f59e0b, #d97706);'>
                            <i class='fas fa-clock'></i> Ajustar RTC Manualmente
                        </button>
                    </form>
                </div>
                
                <!-- Pestaña Weather -->
                <div id="tabWeather" class="tabcontent">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-cloud-sun"></i> PREVISIÓN METEOROLÓGICA
                        </div>
                        
                        <label>Activar Pronóstico:</label>
                        <select name='wg'>
                            <option value='1' {WEATHER_GUARD_SELECTED_YES}>Sí - Bloquear si llueve</option>
                            <option value='0' {WEATHER_GUARD_SELECTED_NO}>No - Ignorar pronóstico</option>
                        </select>
                        
                        <label>Proveedor de Datos:</label>
                        <select name='wp' onchange='updateWeatherProviderInfo(this.value)'>
                            <option value='openmeteo' {WEATHER_OPENMETEO_SELECTED}>Open-Meteo (GRATIS - Más preciso)</option>
                            <option value='openweathermap' {WEATHER_OPENWEATHER_SELECTED}>OpenWeatherMap (Estándar)</option>
                            <option value='weatherapi' {WEATHER_WEATHERAPI_SELECTED}>WeatherAPI (Moderno)</option>
                        </select>
                        
                        <label id='cityLabel'>Ciudad o Coordenadas:</label>
                        <input type='text' name='wc' id='weatherCity' placeholder='Madrid,ES o 40.4168,-3.7038' value='{WEATHER_CITY_VAL}'>
                        
                        <div id='apiKeyGroup'>
                            <label>API Key:</label>
                            <input type='password' name='wak' id='weatherApiKey' placeholder='Tu clave API' value='{WEATHER_API_KEY_VAL}'>
                        </div>
                        
                        <label>Ventana de Pronóstico (horas):</label>
                        <input type='number' name='wrth' min='1' max='48' value='{WEATHER_THRESHOLD_HOURS}'>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar cambios
                        </button>
                    </form>
                    
                    <!-- Estado actual del weather -->
                    <div style='background: rgba(16, 185, 129, 0.1); border-radius: 12px; padding: 15px; margin-top: 15px;'>
                        <h5 style='color: #10b981; margin-bottom: 10px;'>
                            <i class='fas fa-cloud-rain'></i> Estado Actual
                        </h5>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Proveedor:</strong> {WEATHER_PROVIDER_NAME}
                        </p>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Estado:</strong> 
                            <span style='color: {WEATHER_STATUS_COLOR};'>
                                {WEATHER_STATUS_TEXT}
                            </span>
                        </p>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Última actualización:</strong> {WEATHER_LAST_UPDATE}
                        </p>
                        <p style='font-size: 0.9rem; margin: 5px 0;'>
                            <strong>Lluvia máxima esperada:</strong> {WEATHER_MAX_RAIN} mm
                        </p>
                    </div>
                </div>

                <!-- Pestaña Telegram -->
                <div id="tabTelegram" class="tabcontent">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fab fa-telegram"></i> NOTIFICACIONES TELEGRAM
                        </div>
                        
                        <label>Bot Token:</label>
                        <input type='password' name='tg_token' placeholder='123456:ABC-DEF...' value='{TELEGRAM_TOKEN_VAL}'>
                        
                        <label>Chat ID:</label>
                        <input type='text' name='tg_chatid' placeholder='123456789' value='{TELEGRAM_CHATID_VAL}'>
                        
                        <label>Notificaciones Activas:</label>
                        <div style='margin: 10px 0;'>
                            <label style='display: flex; align-items: center; margin: 5px 0;'>
                                <input type='checkbox' name='tg_pump' checked style='width: auto; margin-right: 10px;'>
                                Activación/desactivación de bombas
                            </label>
                            <label style='display: flex; align-items: center; margin: 5px 0;'>
                                <input type='checkbox' name='tg_alert' checked style='width: auto; margin-right: 10px;'>
                                Alertas de sistema (fallos, restricciones)
                            </label>
                        </div>
                                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar cambios
                        </button>
                        
                        <a href='/testtelegram' class='btn btn-warning' style='margin-top: 10px;'>
                            <i class="fas fa-paper-plane"></i> Enviar Mensaje de Prueba
                        </a>
                    </form>
                    
                    <!-- Info box -->
                    <div style='background: rgba(99, 102, 241, 0.1); border-radius: 12px; padding: 15px; margin-top: 20px;'>
                        <p style='font-size: 0.85rem; margin: 5px 0;'>
                            <strong>Cómo configurar:</strong>
                        </p>
                        <ol style='font-size: 0.85rem; margin: 5px 0 5px 20px;'>
                            <li>Crea un bot con @BotFather en Telegram</li>
                            <li>Copia el token que te proporciona</li>
                            <li>Inicia conversación con tu bot</li>
                            <li>Obtén tu Chat ID con @userinfobot</li>
                        </ol>
                    </div>
                </div>
                
                <!-- Pestaña Red (NUEVA - después de Telegram) -->
                <div id="tabRed" class="tabcontent">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-network-wired"></i> CONFIGURACIÓN DE RED
                        </div>
                        
                        <label>Tipo de Configuración IP:</label>
                        <select name='useDHCP' id='useDHCP' onchange='toggleStaticIP()'>
                            <option value='1' {DHCP_SELECTED}>DHCP Automático</option>
                            <option value='0' {STATIC_SELECTED}>IP Estática Fija</option>
                        </select>
                        
                        <!-- Campos IP Estática (ocultos inicialmente) -->
                        <div id='staticIPFields' style='display: none;'>
                            <label>Dirección IP:</label>
                            <input type='text' name='staticIP' placeholder='192.168.1.100' value='{STATIC_IP_VAL}'>
                            
                            <label>Puerta de Enlace (Gateway):</label>
                            <input type='text' name='staticGateway' placeholder='192.168.1.1' value='{STATIC_GATEWAY_VAL}'>
                            
                            <label>Máscara de Subred:</label>
                            <input type='text' name='staticSubnet' placeholder='255.255.255.0' value='{STATIC_SUBNET_VAL}'>
                            
                            <label>DNS Primario (opcional):</label>
                            <input type='text' name='staticDNS1' placeholder='8.8.8.8' value='{STATIC_DNS1_VAL}'>
                            
                            <label>DNS Secundario (opcional):</label>
                            <input type='text' name='staticDNS2' placeholder='8.8.4.4' value='{STATIC_DNS2_VAL}'>
                            
                            <div style='background: rgba(59, 130, 246, 0.1); padding: 12px; border-radius: 8px; margin: 10px 0;'>
                                <p style='font-size: 0.85rem; margin: 0; color: #3b82f6;'>
                                    <i class='fas fa-info-circle'></i> 
                                    <strong>Recomendación:</strong> La IP debe estar fuera del rango DHCP del router para evitar conflictos.
                                </p>
                            </div>
                        </div>
                            <!-- SEGURIDAD WEB -->
                        <div style='margin-top: 30px; border-top: 2px solid rgba(99, 102, 241, 0.3); padding-top: 20px;'>
                            <div class="window-title">
                                <i class="fas fa-shield-alt"></i> SEGURIDAD WEB
                            </div>
                            
                            <label>Autenticación Web:</label>
                            <select name='authEnabled' id='authEnabled' onchange='toggleAuthFields()'>
                                <option value='1' {AUTH_ENABLED_YES}>Activada (Recomendado)</option>
                                <option value='0' {AUTH_ENABLED_NO}>Desactivada</option>
                            </select>
                            
                            <div id='authFields'>
                                <label>Usuario:</label>
                                <input type='text' name='webUser' value='{WEB_USER_VAL}'>
                                
                                <label>Nueva Contraseña (dejar vacío para no cambiar):</label>
                                <input type='password' name='webPassword' placeholder='Nueva contraseña'>
                                
                                <label>Timeout de Sesión (minutos):</label>
                                <input type='number' name='sessionTimeout' min='5' max='1440' value='{SESSION_TIMEOUT_VAL}'>
                                
                                <div style='background: rgba(245, 158, 11, 0.1); padding: 12px; border-radius: 8px; margin: 15px 0; border-left: 3px solid #f59e0b;'>
                                    <p style='font-size: 0.85rem; margin: 0; color: #fbbf24;'>
                                        <i class='fas fa-exclamation-triangle'></i> 
                                        <strong>Importante:</strong> Anota las nuevas credenciales antes de guardar.
                                    </p>
                                </div>
                            </div>
                        </div>
                        
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar cambios
                        </button>
                        
                        <div style='background: rgba(16, 185, 129, 0.1); border-radius: 12px; padding: 15px; margin-top: 20px;'>
                            <h5 style='color: #10b981; margin-bottom: 10px;'>
                                <i class='fas fa-network-wired'></i> Estado Actual de Red
                            </h5>
                            <p style='font-size: 0.9rem; margin: 5px 0;'>
                                <strong>Modo:</strong> {CURRENT_NET_MODE}
                            </p>
                            <p style='font-size: 0.9rem; margin: 5px 0;'>
                                <strong>IP Actual:</strong> {CURRENT_IP}
                            </p>
                            <p style='font-size: 0.9rem; margin: 5px 0;'>
                                <strong>Gateway:</strong> {CURRENT_GATEWAY}
                            </p>
                            <p style='font-size: 0.9rem; margin: 5px 0;'>
                                <strong>Máscara:</strong> {CURRENT_SUBNET}
                            </p>
                        </div>
                    </form>
                </div>
            </div>

            <script>
                function toggleStaticIP() {
                    var useDHCP = document.getElementById('useDHCP').value;
                    var staticFields = document.getElementById('staticIPFields');
                    
                    if (useDHCP === '1') {
                        staticFields.style.display = 'none';
                    } else {
                        staticFields.style.display = 'block';
                    }
                }

                // Función para mostrar/ocultar campos de autenticación
                function toggleAuthFields() {
                    var authEnabled = document.getElementById('authEnabled');
                    var authFields = document.getElementById('authFields');
                    
                    if (authEnabled && authFields) {
                        if (authEnabled.value === '1') {
                            authFields.style.display = 'block';
                        } else {
                            authFields.style.display = 'none';
                        }
                    }
                }

                function updateWeatherProviderInfo(provider) {
                    var cityLabel = document.getElementById('cityLabel');
                    var apiKeyGroup = document.getElementById('apiKeyGroup');
                    var weatherCity = document.getElementById('weatherCity');
                    
                    if (provider === 'openmeteo') {
                        cityLabel.innerHTML = 'Ciudad o Coordenadas:';
                        weatherCity.placeholder = 'Madrid,ES o 40.4168,-3.7038';
                        apiKeyGroup.style.display = 'none';
                    } else {
                        cityLabel.innerHTML = 'Ciudad:';
                        weatherCity.placeholder = 'Madrid,ES';
                        apiKeyGroup.style.display = 'block';
                    }
                }

                function showSubstrateInfo(substrate) {
                    // Función para mostrar información del sustrato seleccionado
                    var info = '';
                    switch(substrate) {
                        case 'clay':
                            info = 'Arcilla: Alta retención de agua, riegos más espaciados';
                            break;
                        case 'sandy':
                            info = 'Arenoso: Drenaje rápido, riegos más frecuentes';
                            break;
                        case 'loam':
                            info = 'Franco: Balance perfecto, riegos moderados';
                            break;
                        case 'peat':
                            info = 'Turba: Orgánico, alta retención pero buen drenaje';
                            break;
                        default:
                            info = 'Selecciona un tipo de sustrato para ver sus características';
                    }
                    // Aquí podrías mostrar la info en un div específico
                    console.log(info);
                }

                // Inicializar al cargar
                document.addEventListener('DOMContentLoaded', function() {
                    toggleStaticIP(); // Estado inicial correcto
                    toggleAuthFields(); // Estado inicial autenticación
                    
                    // Inicializar información del proveedor meteorológico
                    var weatherProvider = document.querySelector('select[name="wp"]');
                    if (weatherProvider) {
                        updateWeatherProviderInfo(weatherProvider.value);
                    }
                });
                // Función para las pestañas (debe estar en el footer general)
                function openTab(evt, tabName) {
                    var tabContainer = evt.currentTarget.closest('.card');
                    var tabcontent = tabContainer.querySelectorAll('.tabcontent');
                    for (var i = 0; i < tabcontent.length; i++) {
                        tabcontent[i].style.display = "none";
                    }
                    var tablinks = tabContainer.querySelectorAll('.tablink');
                    for (var i = 0; i < tablinks.length; i++) {
                        tablinks[i].className = tablinks[i].className.replace(" active", "");
                    }
                    document.getElementById(tabName).style.display = "block";
                    evt.currentTarget.className += " active";
                }
            </script>
)=====";

// ============================
// 6. TARJETA DE INFORMACIÓN DEL SISTEMA
// ============================
const char HTML_SYSTEM_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Sistema -->
            <div class="card">
                <h2><i class="fas fa-microchip"></i> Sistema</h2>
                <div class="status-row">
                    <span><i class="fas fa-wifi"></i> Estado WiFi</span>
                    <span class='status-badge {WIFI_STATUS_CLASS}'>{WIFI_STATUS_TEXT}</span>
                </div>
                <div class="status-row">
                    <span><i class="fas fa-network-wired"></i> Red WiFi Actual</span>
                    <span>{WIFI_SSID}</span>
                </div>
                <div class="status-row">
                    <span><i class="fas fa-signal"></i> Intensidad Señal WiFi</span>
                    <span class="signal-strength">
                        <div class="signal-bar" style="height: 10px;"></div>
                        <div class="signal-bar" style="height: 14px;"></div>
                        <div class="signal-bar" style="height: 18px;"></div>
                        <div class="signal-bar" style="height: 22px;"></div>
                        {WIFI_RSSI_VALUE} dBm
                    </span>
                </div>
                <div class="status-row">
                    <span><i class="fas fa-ip-address"></i> Dirección IP local</span>
                    <span>{LOCAL_IP}</span>
                </div>
                <div class="status-row">
                  <span><i class="fas fa-network-wired"></i> Dirección MAC</span>
                  <span>{MAC_ADDRESS}</span>
                </div>
                <div class="status-row">
                    <span><i class="fas fa-microchip"></i> Memoria libre</span>
                    <span>{FREE_MEMORY} KB</span>
                </div>
                <div class="status-row">
                    <span><i class="fas fa-clock"></i> Tiempo Funcionamiento</span>
                    <span>{UPTIME}</span>
                </div>
                <div id="weatherInfo" style="display: {WEATHER_DISPLAY};">
                 <div class="status-row">
                    <span><i class="fas fa-tint"></i> Cantidad lluvia esperada</span>
                    <span>{RAIN_AMOUNT} mm</span>
                </div>
                
                <!-- GESTIÓN DEL SISTEMA -->
                <div class="window-title">
                    <i class="fas fa-tools"></i> GESTIÓN DEL SISTEMA
                </div>
                
                <!-- Grid 2x2 para botones -->
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 20px;">
                    <!-- Fila 1 -->
                    <a href='/resetwifi' class='btn btn-warning' onclick='return confirm("¿Resetear WiFi?")' 
                       style="padding: 14px 10px; font-size: 0.85rem;">
                        <i class="fas fa-sync-alt"></i><br>Reset WiFi
                    </a>
                    <a href='/factoryreset' class='btn btn-danger' onclick='return confirmFactoryReset()' 
                       style="padding: 14px 10px; font-size: 0.85rem;">
                        <i class="fas fa-exclamation-triangle"></i><br>Valores Fábrica
                    </a>
                    
                    <!-- Fila 2 -->
                    <a href='/importconfig' class='btn btn-primary' 
                       style="padding: 14px 10px; font-size: 0.85rem;">
                        <i class="fas fa-file-import"></i><br>Restaurar Config.
                    </a>
                    <a href='/exportconfig' class='btn btn-success' download
                       style="padding: 14px 10px; font-size: 0.85rem;">
                        <i class="fas fa-file-export"></i><br>Backup Config.
                    </a>
                </div>
            </div>
)=====";


// ============================
// 7. TARJETA DE DATOS METEOROLÓGICOS AVANZADOS
// ============================
const char HTML_WEATHER_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Datos Meteorológicos Avanzados -->
            <div class="card">
                <h2><i class="fas fa-satellite"></i> Datos Meteo. Avanzados</h2>
                <div class="sensor-grid">
                    <!-- ET0 Diaria -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(16, 185, 129, 0.8), rgba(5, 150, 105, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">💧</div>
                        <div class="sensor-value">{ET0_DAILY}</div>
                        <div class="sensor-label">ET0 Diaria (mm)</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Agua necesaria l/m2 hoy
                        </div>
                    </div>

                    <!-- ET0 Horaria -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(6, 182, 212, 0.8), rgba(8, 145, 178, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">🕐</div>
                        <div class="sensor-value">{ET0_HOURLY}</div>
                        <div class="sensor-label">ET0 Hora (mm/h)</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Evapotranspiración actual
                        </div>
                    </div>

                    <!-- Temperatura del Suelo -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(139, 69, 19, 0.8), rgba(101, 67, 33, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">🌡️</div>
                        <div class="sensor-value">{SOIL_TEMP}°C</div>
                        <div class="sensor-label">Temp. Suelo</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Temperatura superficie
                        </div>
                    </div>

                    <!-- Sensación Térmica -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(239, 68, 68, 0.8), rgba(220, 38, 38, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">🌡️</div>
                        <div class="sensor-value">{APPARENT_TEMP}°C</div>
                        <div class="sensor-label">Sensación Térmica</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Temperatura percibida
                        </div>
                    </div>

                    <!-- Humedad del Suelo a 9 cm -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(34, 197, 94, 0.8), rgba(22, 163, 74, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">💦</div>
                        <div class="sensor-value">{SOIL_MOISTURE_9CM}</div>
                        <div class="sensor-label">Humedad Suelo</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Humedad a 9cm bajo suelo 
                        </div>
                    </div>

                    <!-- Radiación Solar -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(245, 158, 11, 0.8), rgba(249, 115, 22, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">☀️</div>
                        <div class="sensor-value">{SOLAR_RADIATION}</div>
                        <div class="sensor-label">Radiación (W/m²)</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Intensidad solar actual
                        </div>
                    </div>

                    <!-- Índice UV -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(168, 85, 247, 0.8), rgba(147, 51, 234, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">🌞</div>
                        <div class="sensor-value">{UV_INDEX}</div>
                        <div class="sensor-label">Índice UV</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            {UV_LEVEL}
                        </div>
                    </div>

                    <!-- Velocidad del Viento -->
                    <div class="sensor-item" style="background: linear-gradient(135deg, rgba(99, 102, 241, 0.8), rgba(79, 70, 229, 0.8));">
                        <div style="font-size: 1.6rem; margin-bottom: 15px;">💨</div>
                        <div class="sensor-value">{WIND_SPEED}</div>
                        <div class="sensor-label">Viento (km/h)</div>
                        <div style="font-size: 0.7rem; margin-top: 5px; color: #e2e8f0;">
                            Aumenta evaporación
                        </div>
                    </div>
                </div>
    
                <!-- Información adicional -->
                <div style="background: rgba(16, 185, 129, 0.1); border-radius: 12px; padding: 15px; margin-top: 20px;">
                    <h5 style="color: #10b981; margin-bottom: 10px;">
                        <i class="fas fa-info-circle"></i> Información ET0
                    </h5>
                    <p style="font-size: 0.85rem; margin: 5px 0;">
                        <strong>ET0 Diaria:</strong> Cantidad total de agua que las plantas necesitan hoy
                    </p>
                    <p style="font-size: 0.85rem; margin: 5px 0;">
                        <strong>Ejemplo:</strong> ET0 = 5mm → Aplicar 5 litros por m² de cultivo
                    </p>
                    <p style="font-size: 0.85rem; margin: 5px 0;">
                        <strong>Última actualización:</strong> {WEATHER_UPDATE_TIME} h
                    </p>
                </div>
            </div>
)=====";

// ============================
// TARJETA DE CALIBRACIÓN DE SENSORES
// ============================
const char HTML_CALIBRATION_CARD[] PROGMEM = R"=====(
            <!-- Tarjeta de Calibración de Sensores en Tiempo Real -->
            <div class="card" style="position: relative;">
                <h2><i class="fas fa-sliders-h"></i> Calibración de Sensores
                    <span style="font-size: 0.8rem; color: #f59e0b; margin-left: 10px;">
                        <i class="fas fa-sync-alt fa-spin" id="cal-sync-icon"></i> 
                        <span id="cal-update-status">Monitor activo</span>
                    </span>
                </h2>
                
                <!-- Sistema Actual en Tiempo Real -->
                <div style="background: rgba(99, 102, 241, 0.1); border-radius: 12px; padding: 15px; margin-bottom: 20px;">
                    <h4 style="color: #6366f1; margin-bottom: 10px;">
                        <i class="fas fa-chart-line"></i> Sistema Actual - Valores
                    </h4>
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px; font-size: 0.9rem;">
                        <div>
                            <strong>Sensor 1:</strong> 
                            <span id="cal-soil1-raw-adc">{SOIL1_RAW}</span> raw → 
                            <span id="cal-soil1-cal">{SOIL1_CAL}</span>% cal
                        </div>
                        <div>
                            <strong>Sensor 2:</strong> 
                            <span id="cal-soil2-raw-adc">{SOIL2_RAW}</span> raw → 
                            <span id="cal-soil2-cal">{SOIL2_CAL}</span>% cal
                        </div>
                        <div>
                            <strong>Perfil:</strong> <span id="cal-profile">{SOIL_PROFILE}</span>
                        </div>
                        <div>
                            <strong>Factor:</strong> <span id="cal-factor">{CALIBRATION_FACTOR}</span>x
                        </div>
                    </div>
                </div>

                <!-- Pestañas - CAMBIO AQUÍ: usar clases únicas -->
                <div class="tabs">
                    <button class="tablink cal-tablink active" onclick="openCalibrationTab(event, 'tabCalSoil')">
                        <i class="fas fa-seedling"></i> Sensores Suelo
                    </button>
                    <button class="tablink cal-tablink" onclick="openCalibrationTab(event, 'tabCalManual')">
                        <i class="fas fa-tools"></i> Ajustes Manuales
                    </button>
                </div>
                
                <!-- Pestaña Sensores Suelo en Tiempo Real -->
                <div id="tabCalSoil" class="tabcontent cal-tabcontent" style="display:block;">
                    <!-- Selector de Sensor -->
                    <div class="window-title">
                        <i class="fas fa-microchip"></i> SELECCIONAR SENSOR - CALIBRACIÓN EN TIEMPO REAL
                    </div>
                    <div class="btn-group">
                        <button class="btn btn-outline sensor-select active" data-sensor="1" onclick="selectSensor(1)">
                            <i class="fas fa-1"></i> Sensor Zona 1 
                        </button>
                        <button class="btn btn-outline sensor-select" data-sensor="2" onclick="selectSensor(2)">
                            <i class="fas fa-2"></i> Sensor Zona 2 
                        </button>
                    </div>

                    <!-- Lectura en Tiempo Real con Gráfico Visual -->
                    <div id="sensor-indicator" style="background: rgba(16, 185, 129, 0.1); border-radius: 10px; padding: 20px; margin: 15px 0;">
                        <div style="display: flex; align-items: center; justify-content: space-between; width: 100%; margin-bottom: 15px;">
                            <div>
                                <i class="fas fa-microchip"></i> 
                                <strong id="sensor-title">Sensor Zona 1</strong>
                            </div>
                            <div class="sensor-value" id="cal-sensor-value" style="font-size: 2rem;">{SOIL1_CAL}%</div>
                        </div>
                        
                        <!-- Barra de progreso visual -->
                        <div style="background: rgba(0,0,0,0.2); border-radius: 10px; height: 20px; margin: 10px 0; position: relative;">
                            <div id="sensor-progress" style="background: linear-gradient(90deg, #ef4444, #f59e0b, #10b981); 
                                 height: 100%; border-radius: 10px; width: 50%; transition: width 0.5s ease;"></div>
                            <div style="position: absolute; top: 0; left: 0; right: 0; bottom: 0; display: flex; justify-content: space-between; padding: 0 10px; align-items: center; font-size: 0.7rem; color: white; text-shadow: 0 1px 2px black;">
                                <span>0%</span>
                                <span id="progress-value">50%</span>
                                <span>100%</span>
                            </div>
                        </div>
                        
                        <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px; font-size: 0.8rem; color: #64748b;">
                            <div>
                                <i class="fas fa-microchip"></i> ADC Raw: <span id="cal-sensor-raw-adc">----</span>
                            </div>
                            <div>
                                <i class="fas fa-wave-square"></i> Raw %: <span id="cal-sensor-raw-percent">{SOIL1_RAW}</span>%
                            </div>
                            <div>
                                <i class="fas fa-sliders-h"></i> Calibrado: <span id="cal-sensor-cal-adc">{SOIL1_CAL}</span>%
                            </div>
                            <div>
                                <i class="fas fa-history"></i> Actualizado: <span id="cal-sensor-time">ahora</span>
                            </div>
                        </div>
                    </div>
                        
                    <!-- Proceso de Calibración con Valores en Tiempo Real -->
                    <div class="window-title">
                        <i class="fas fa-list-ol"></i> CALIBRACIÓN GUIADA - VALORES EN TIEMPO REAL
                    </div>
                    
                    <!-- Paso 1: Aire -->
                    <div class="calibration-step active" data-step="1" id="step-air">
                        <div class="step-header">
                            <span class="step-number">1</span>
                            <span class="step-title">Punto SECO (Aire - 0% Humedad)</span>
                            <span class="step-status" id="status-air">Pendiente</span>
                        </div>
                        <div class="step-content">
                            <p>🔹 <strong>Instrucciones:</strong> Sensor COMPLETAMENTE SECO en el aire</p>
                            <p>🔹 <strong>Comportamiento esperado:</strong> 
                                <span style="color: #06b6d4;">• Sensores NORMALES: Valor bajo de ADC</span><br>
                                <span style="color: #f59e0b;">• Sensores INVERSOS: Valor alto de ADC</span>
                            </p>
                            <p>🔹 <strong>Valor ADC RAW en tiempo real:</strong> 
                                <span id="current-air-value-adc">{SOIL1_RAW}</span>%</p>
                            <div class="step-controls">
                                <button class="btn btn-primary calibrate-btn" onclick="setCalibrationPoint('air')">
                                    <i class="fas fa-bullseye"></i> Establecer como valor en SECO (0%)
                                </button>
                            </div>
                        </div>
                    </div>

                    <!-- Paso 2: Agua -->
                    <div class="calibration-step" data-step="2" id="step-water">
                        <div class="step-header">
                            <span class="step-number">2</span>
                            <span class="step-title">Punto HÚMEDO (Agua - 100% Humedad)</span>
                            <span class="step-status" id="status-water">Pendiente</span>
                        </div>
                        <div class="step-content">
                            <p>🔹 <strong>Instrucciones:</strong> Sumerge solo la sonda en agua limpia</p>
                            <p>🔹 <strong>Comportamiento esperado:</strong> 
                            <span style="color: #06b6d4;">• Sensores NORMALES: Valor alto de ADC</span><br>
                             <span style="color: #f59e0b;">• Sensores INVERSOS: Valor bajo de ADC</span>
                            </p>
                            <p>🔹 <strong>Precaución:</strong> ¡NO sumerjas la electrónica!</p>

                            <p>🔹 <strong>Valor ADC RAW en tiempo real:</strong> 
                                <span id="current-water-value-adc">{SOIL1_RAW}</span>%</p>
                            <div class="step-controls">
                                <button class="btn btn-primary calibrate-btn" onclick="setCalibrationPoint('water')">
                                    <i class="fas fa-bullseye"></i> Establecer como valor en AGUA (100%)
                                </button>
                            </div>
                        </div>
                    </div>

                    <!-- Paso 3: Resultados en Tiempo Real -->
                    <div class="calibration-step" data-step="3" id="step-results">
                        <div class="step-header">
                            <span class="step-number">3</span>
                            <span class="step-title">Resultados y Aplicación</span>
                            <span class="step-status" id="status-results">Pendiente</span>
                        </div>
                        <div class="step-content">
                            <div class="calibration-results">
                                <div class="result-item">
                                    <span>Punto SECO (0%):</span>
                                    <span id="cal-point-air">--</span>
                                </div>
                                <div class="result-item">
                                    <span>Punto HÚMEDO (100%):</span>
                                    <span id="cal-point-water">--</span>
                                </div>
                                <div class="result-item">
                                    <span>Nuevo offset calculado:</span>
                                    <span id="cal-new-offset">--</span>
                                </div>
                                <div class="result-item">
                                    <span>Precisión estimada:</span>
                                    <span id="cal-precision">--</span>
                                </div>
                            </div>
                            
                            <form action='/setcalibration' method='get' style="margin-top: 15px;">
                                <input type="hidden" name="sensor" id="cal-sensor" value="1">
                                <input type="hidden" name="offset_value" id="cal-offset-value">
                                
                                <button type="submit" class="btn btn-success" id="apply-calibration" disabled>
                                    <i class="fas fa-check"></i> Aplicar Calibración
                                </button>
                            </form>
                        </div>
                    </div>
                </div>
                
                <!-- Pestaña Ajustes Manuales -->
                <div id="tabCalManual" class="tabcontent cal-tabcontent" style="display:none;">
                    <form action='/setconfig' method='get'>
                        <div class="window-title">
                            <i class="fas fa-tools"></i> AJUSTES MANUALES - ACTUALIZADOS EN TIEMPO REAL
                        </div>
                        
                        <h4 style="color: #6366f1; margin: 15px 0;">Sensores de Suelo (Valores Actuales)</h4>
                        <label>Offset Sensor Suelo 1 (Actual: <span id="current-offset1">{OFFSET_SOIL1}</span>):</label>
                        <input type='number' name='s1' step='0.1' value='{OFFSET_SOIL1}' id="offset-input1">
                        
                        <label>Offset Sensor Suelo 2 (Actual: <span id="current-offset2">{OFFSET_SOIL2}</span>):</label>
                        <input type='number' name='s2' step='0.1' value='{OFFSET_SOIL2}' id="offset-input2">
                        
                        <h4 style="color: #6366f1; margin: 15px 0;">Sensores Ambientales</h4>
                        <label>Offset Temperatura (Actual: <span id="current-offset-temp">{OFFSET_TEMP}</span>):</label>
                        <input type='number' name='t' step='0.1' value='{OFFSET_TEMP}' id="offset-input-temp">
                        
                        <label>Offset Humedad Ambiente (Actual: <span id="current-offset-hum">{OFFSET_HUM}</span>):</label>
                        <input type='number' name='h' step='0.1' value='{OFFSET_HUM}' id="offset-input-hum">
                        
                        <label>Offset Radiación Solar (Actual: <span id="current-offset-light">{OFFSET_LIGHT}</span>):</label>
                        <input type='number' name='l' step='0.1' value='{OFFSET_LIGHT}' id="offset-input-light">

                        <label>Offset Presión Atmosférica (Actual: <span id="current-offset-pressure">{OFFSET_PRESSURE}</span>):</label>
                        <input type='number' name='p' step='0.1' value='{OFFSET_PRESSURE}' id="offset-input-pressure">
                        
                        <label>Offset Calidad Aire (Actual: <span id="current-offset-airquality">{OFFSET_AIRQUALITY}</span>):</label>
                        <input type='number' name='aq' step='1' value='{OFFSET_AIRQUALITY}' id="offset-input-airquality">

                        <h4 style="color: #8b5cf6; margin: 25px 0 15px 0; padding-top: 20px; border-top: 2px solid rgba(139, 92, 246, 0.3);">
                        <i class="fas fa-brain"></i> Sistema de Inferencia Inteligente
                        </h4>
                        
                        <div style="background: rgba(139, 92, 246, 0.1); padding: 15px; border-radius: 10px; margin-bottom: 15px;">
                            <p style="color: #a78bfa; margin: 0; font-size: 0.9rem; line-height: 1.5;">
                                <i class="fas fa-info-circle"></i> 
                                <strong>¿Qué son estos umbrales?</strong><br>
                                Controlan la sensibilidad del sistema adaptativo. La <strong>Necesidad de Riego</strong> 
                                se calcula entre 0-100% usando múltiples factores (humedad, VPD, temperatura, etc.).
                            </p>
                        </div>
                        
                        <label>
                            <i class="fas fa-play-circle" style="color: #10b981;"></i> 
                            Umbral ALTO - Activación (Necesidad para iniciar riego):
                        </label>
                        <div style="display: flex; align-items: center; gap: 10px;">
                            <input type='range' name='inf_high' min='50' max='90' step='5' 
                                   value='{INFERENCE_HIGH_VAL}' id="inference-high-slider"
                                   oninput="updateInferenceDisplay('high', this.value)"
                                   style="flex: 1;">
                            <span id="inference-high-display" style="min-width: 60px; font-weight: bold; color: #10b981;">
                                {INFERENCE_HIGH_VAL}%
                            </span>
                        </div>
                        <small style="color: #94a3b8; display: block; margin-top: 5px;">
                            📊 Recomendado: 60-75%. Valores altos = riego más conservador
                        </small>
                        
                        <label style="margin-top: 15px;">
                            <i class="fas fa-stop-circle" style="color: #ef4444;"></i>
                            Umbral BAJO - Desactivación (Necesidad para detener riego):
                        </label>
                        <div style="display: flex; align-items: center; gap: 10px;">
                            <input type='range' name='inf_low' min='10' max='50' step='5' 
                                   value='{INFERENCE_LOW_VAL}' id="inference-low-slider"
                                   oninput="updateInferenceDisplay('low', this.value)"
                                   style="flex: 1;">
                            <span id="inference-low-display" style="min-width: 60px; font-weight: bold; color: #ef4444;">
                                {INFERENCE_LOW_VAL}%
                            </span>
                        </div>
                        <small style="color: #94a3b8; display: block; margin-top: 5px;">
                            📊 Recomendado: 20-35%. Valores bajos = detención más rápida
                        </small>
                        
                        <div style="background: rgba(251, 191, 36, 0.1); padding: 12px; border-radius: 8px; margin-top: 15px; border-left: 3px solid #fbbf24;">
                            <p style="margin: 0; font-size: 0.85rem; color: #fbbf24;">
                                <i class="fas fa-lightbulb"></i> 
                                <strong>Importante:</strong> La diferencia entre ambos umbrales crea una "zona de histéresis" 
                                que evita activaciones/desactivaciones constantes del sistema.
                            </p>
                        </div>
                        <button type='submit' class='btn btn-primary' style='margin-top: 20px;'>
                            <i class="fas fa-save"></i> Aplicar cambios
                        </button>
                    </form>
                </div>
            </div>
)=====";

// ============================
// FOOTER CON JAVASCRIPT
// ============================
const char HTML_FOOTER[] PROGMEM = R"=====(
        </div>
    </div>

    <script>
        // Función para pestañas principales (Ajustes y Configuración Avanzada)
        function openTab(evt, tabName) {
            // Obtener solo la tarjeta específica donde se hizo clic
            var tabContainer = evt.currentTarget.closest('.card');
            
            // Solo afectar elementos dentro de esta tarjeta específica
            var tabcontent = tabContainer.querySelectorAll('.tabcontent');
            var tablinks = tabContainer.querySelectorAll('.tablink');
            
            // Ocultar todas las pestañas de ESTA tarjeta
            for (var i = 0; i < tabcontent.length; i++) {
                tabcontent[i].style.display = "none";
            }
            
            // Quitar active de todos los links de ESTA tarjeta
            for (var i = 0; i < tablinks.length; i++) {
                tablinks[i].className = tablinks[i].className.replace(" active", "");
            }
            
            // Mostrar la pestaña seleccionada
            var targetTab = document.getElementById(tabName);
            if (targetTab) {
                targetTab.style.display = "block";
            }
            
            // Marcar como activo el link clickeado
            evt.currentTarget.className += " active";
        }

            // Función específica para calibración con selector más específico
            function openCalibrationTab(evt, tabName) {
                var calibrationCard = evt.currentTarget.closest('.card');
                var tabcontent = calibrationCard.querySelectorAll('.cal-tabcontent');
                var tablinks = calibrationCard.querySelectorAll('.cal-tablink');
                
                for (var i = 0; i < tabcontent.length; i++) {
                    tabcontent[i].style.display = "none";
                }
                
                for (var i = 0; i < tablinks.length; i++) {
                    tablinks[i].className = tablinks[i].className.replace(" active", "");
                }
                
                var targetTab = document.getElementById(tabName);
                if (targetTab) {
                    targetTab.style.display = "block";
                }
                evt.currentTarget.className += " active";
            }
        
            // Función para actualizar displays de umbrales de inferencia en tiempo real
            function updateInferenceDisplay(type, value) {
                const displayId = 'inference-' + type + '-display';
                const element = document.getElementById(displayId);
                
                if (element) {
                    element.textContent = value + '%';
                    
                    // Opcional: cambiar color según el valor
                    if (type === 'high') {
                        if (value >= 75) element.style.color = '#ef4444'; // Rojo - muy conservador
                        else if (value >= 65) element.style.color = '#10b981'; // Verde - óptimo
                        else element.style.color = '#fbbf24'; // Amarillo - agresivo
                    } else {
                        if (value <= 20) element.style.color = '#ef4444'; // Rojo - muy agresivo
                        else if (value <= 35) element.style.color = '#10b981'; // Verde - óptimo
                        else element.style.color = '#fbbf24'; // Amarillo - conservador
                    }
                }
                
                console.log('Umbral de inferencia ' + type + ' actualizado a: ' + value + '%');
            }

        function updateSignalStrength() {
            const rssi = {WIFI_RSSI};
            const bars = document.querySelectorAll('.signal-bar');
            const heights = [10, 14, 18, 22];
            
            bars.forEach((bar, index) => {
                if (rssi >= -55 - (index * 10)) {
                    bar.style.background = '#10b981';
                    bar.style.height = heights[index] + 'px';
                } else if (rssi >= -70 - (index * 10)) {
                    bar.style.background = '#f59e0b';
                    bar.style.height = heights[index] - 2 + 'px';
                } else {
                    bar.style.background = '#ef4444';
                    bar.style.height = heights[index] - 4 + 'px';
                }
            });
        }

        function confirmFactoryReset() {
            if (confirm("⚠️ ADVERTENCIA ⚠️\\n\\n¿Estás seguro de que quieres BORRAR TODA la configuración?")) {
                if (confirm("⚠️ SEGUNDA CONFIRMACIÓN ⚠️\\n\\n¿Realmente deseas continuar?")) {
                    return true;
                }
            }
            return false;
        }

        document.addEventListener('DOMContentLoaded', function() {
            updateSignalStrength();
        });
     
    </script>
</body>
</html>
)=====";

const char HTML_LIVE_UPDATE_SCRIPT[] PROGMEM = R"=====(
<script>
// ============================================
// SISTEMA DE ACTUALIZACIÓN EN TIEMPO REAL COMPLETO
// ============================================

let currentSensor = 1;
let calData = { air: null, water: null };
let updateInterval;
let isTabVisible = true;
let errorCount = 0;
const MAX_ERRORS = 3;

async function updateLiveData() {
    if (!isTabVisible) return;
    
    try {
        console.log('🔃 Solicitando datos al servidor...');
        
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 5000);
        
        const response = await fetch('/api/sensorData?' + new Date().getTime(), {
            signal: controller.signal,
            cache: 'no-cache'
        });
        
        clearTimeout(timeoutId);
        
        if (!response.ok) {
            throw new Error(`Error HTTP: ${response.status}`);
        }
        
        const text = await response.text();
        console.log('📥 Respuesta recibida:', text.substring(0, 200) + '...');
        
        const data = JSON.parse(text);
        
        // VERIFICAR DATOS CRÍTICOS
        if (typeof data.temp !== 'number' || typeof data.soil1 !== 'number') {
            throw new Error('Datos inválidos en respuesta');
        }
        
        errorCount = 0;
        updateSensorCard(data);
        updateCalibrationCard(data);
        updateUIStatus('success', 'Actualizado');
        
    } catch (error) {
        errorCount++;
        console.error('❌ Error updateLiveData:', error.message);
        
        if (error.name === 'AbortError') {
            updateUIStatus('error', '⏰ Timeout del servidor');
        } else {
            updateUIStatus('error', '❌ Error de conexión');
        }
        
        if (errorCount >= 2) {
            clearInterval(updateInterval);
            updateInterval = setInterval(updateLiveData, 8000);
            updateUIStatus('warning', '🔄 Modo recuperación (8s)');
        }
    }
}

// FUNCIÓN DE SENSORES CON VALIDACIÓN ROBUSTA
function updateSensorCard(data) {
    if (!data) {
        console.error('❌ Data es null o undefined');
        return;
    }
    
    console.log('🔄 Actualizando UI con datos:', {
        temp: data.temp,
        soil1: data.soil1,
        soil2: data.soil2
    });
    
    // VALORES PRINCIPALES
    safeUpdateElement('soil1-value', data.soil1, '%', 1);
    safeUpdateElement('soil2-value', data.soil2, '%', 1);
    safeUpdateElement('light-value', data.light, '%', 1);
    safeUpdateElement('vpd-value', data.vpd, ' kPa', 2);
    safeUpdateElement('pressure-value', data.pressure, ' mbar', 1);
    safeUpdateElement('airquality-value', data.airQuality, ' kΩ', 1);
    
    if (data.soil1_raw !== undefined) {
        updateElement('soil1-raw', `RAW: ${data.soil1_raw} | Offset: ${data.offset_soil1 || 0}`);
    }
    if (data.soil2_raw !== undefined) {
        updateElement('soil2-raw', `RAW: ${data.soil2_raw} | Offset: ${data.offset_soil2 || 0}`);
    }
    
    // ESTADOS
    if (typeof data.pump1 === 'boolean') updatePumpStatus(1, data.pump1);
    if (typeof data.pump2 === 'boolean') updatePumpStatus(2, data.pump2);
    if (typeof data.waterSupply === 'boolean') updateWaterSupply(data.waterSupply);
    if (typeof data.vpd === 'number') updateVPDQuality(data.vpd);

        
    // BME680
    if (typeof data.pressure === 'number') {
        document.getElementById('pressure-value').textContent = data.pressure.toFixed(1);
    }
    if (typeof data.airQuality === 'number') {
        document.getElementById('airquality-value').textContent = data.airQuality.toFixed(1);
    }

    if (data.vocQuality && data.vocColor) {
    const vocQuality = document.getElementById('voc-quality');
    if (vocQuality) {
        vocQuality.textContent = data.vocQuality;
        vocQuality.style.color = data.vocColor;
    }
}
    
    updateElement('last-update', new Date().toLocaleTimeString());
    
}

// FUNCIÓN AUXILIAR SEGURA
function safeUpdateElement(id, value, suffix = '', decimals = 1) {
    if (typeof value === 'number' && !isNaN(value)) {
        updateElement(id, value.toFixed(decimals) + suffix);
    } else {
        console.warn(`Valor inválido para ${id}:`, value);
        updateElement(id, '--');
    }
}

// FUNCIÓN CALIBRACIÓN SEGURA
function updateCalibrationCard(data) {
    if (!data) return;
    
    // CORREGIR: Asegurar que currentSensor esté definida
    if (typeof currentSensor === 'undefined' || currentSensor === null) {
        currentSensor = 1; // Sensor 1 por defecto
    }
    
    // Actualizar valores específicos por sensor (para ambos sensores)
    safeUpdateElement('cal-soil1-raw-adc', data.soil1_raw_adc, '', 0);
    safeUpdateElement('cal-soil2-raw-adc', data.soil2_raw_adc, '', 0);
    safeUpdateElement('cal-soil1-raw-percent', data.soil1_raw_percent, '%', 1);
    safeUpdateElement('cal-soil2-raw-percent', data.soil2_raw_percent, '%', 1);
    
    // Actualizar perfil y factor si existen
    if (data.soil_profile) updateElement('cal-profile', data.soil_profile);
    if (data.calibration_factor) safeUpdateElement('cal-factor', data.calibration_factor, 'x', 2);
    
    // CALCULAR VALORES DEL SENSOR ACTUAL SELECCIONADO
    const sensorRawADC = currentSensor === 1 ? data.soil1_raw_adc : data.soil2_raw_adc;
    const sensorRawPercent = currentSensor === 1 ? data.soil1_raw_percent : data.soil2_raw_percent;
    const sensorCalibrated = currentSensor === 1 ? data.soil1 : data.soil2;
    
    // ACTUALIZAR ELEMENTOS DEL GAUGE DE CALIBRACIÓN
    safeUpdateElement('cal-sensor-raw-adc', sensorRawADC, '', 0);
    safeUpdateElement('cal-sensor-raw-percent', sensorRawPercent, '', 1);
    safeUpdateElement('cal-sensor-value', sensorCalibrated, '', 1);
    safeUpdateElement('cal-sensor-cal-adc', sensorCalibrated, '', 1);
    
    // Actualizar tiempo y barra de progreso
    updateElement('cal-sensor-time', new Date().toLocaleTimeString());
    if (typeof sensorCalibrated === 'number') updateProgressBar(sensorCalibrated);
    
    // Actualizar valores en tiempo real para calibración guiada
    updateElement('current-air-value-adc', sensorRawPercent + '%');
    updateElement('current-water-value-adc', sensorRawPercent + '%');
}

// FUNCIÓN updateElement MEJORADA
function updateElement(id, content) {
    try {
        const element = document.getElementById(id);
        if (element && element.textContent !== content) {
            element.textContent = content;
        }
    } catch (error) {
        console.error(`Error actualizando elemento ${id}:`, error);
    }
}

// Función para actualizar barra de progreso
function updateProgressBar(value) {
    const progress = document.getElementById('sensor-progress');
    const progressValue = document.getElementById('progress-value');
    
    if (progress && progressValue) {
        const width = Math.max(0, Math.min(100, value));
        progress.style.width = `${width}%`;
        progressValue.textContent = `${value.toFixed(1)}%`;
        
        // Cambiar color según el valor
        if (value < 30) {
            progress.style.background = 'linear-gradient(90deg, #ef4444, #f59e0b)';
        } else if (value < 70) {
            progress.style.background = 'linear-gradient(90deg, #f59e0b, #10b981)';
        } else {
            progress.style.background = '#10b981';
        }
    }
}

//=====================================
// Función para seleccionar sensor en calibración
//=====================================
function selectSensor(sensorNum) {
    currentSensor = sensorNum;
    
    // Actualizar botones
    document.querySelectorAll('.sensor-select').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.classList.add('active');
    
    // Actualizar título
    updateElement('sensor-title', `Sensor de Suelo ${sensorNum}`);
    updateElement('sensor-gpio', `GPIO${sensorNum === 1 ? '5' : '15'}`);
    updateElement('cal-sensor', sensorNum);

    // Resetear offset del sensor seleccionado a 0 antes de calibrar
    fetch('/resetoffset?sensor=' + sensorNum, {method: 'GET'})
        .then(() => {
            console.log('Offset del sensor ' + sensorNum + ' reseteado a 0');
        })
        .catch(err => console.error('Error reseteando offset:', err));
    
    // Resetear calibración anterior
    calData = { air: null, water: null };
    resetCalibrationSteps();
    
    // Forzar actualización inmediata
    updateLiveData();
}

function setCalibrationPoint(point) {
    console.log('=== ESTABLECIENDO PUNTO DE CALIBRACIÓN ===');
    console.log('Sensor:', currentSensor, 'Punto:', point);
    
    // PRIMERO: Activar modo manual si es el primer punto
    if (point === 'air') {
        console.log('Activando modo manual...');
        fetch('/startCalibration?sensor=' + currentSensor, {method: 'GET'})
            .then(response => response.text())
            .then(data => {
                console.log('Modo manual activado:', data);
                return establecerPuntoCalibration(point);
            })
            .catch(error => {
                console.error('Error activando modo manual:', error);
            });
    } else {
        establecerPuntoCalibration(point);
    }
}

function establecerPuntoCalibration(point) {
    fetch('/setCalibrationPoint?sensor=' + currentSensor + '&point=' + point, {method: 'GET'})
        .then(response => response.text())
        .then(data => {
            console.log('Punto establecido:', data);
            
            // OBTENER EL VALOR REAL DEL ADC desde los datos más recientes
            setTimeout(() => {
                updateLiveData();
                
                // Esperar a que se actualicen los datos y luego obtener el valor ADC
                setTimeout(() => {
                    // Forzar obtención del valor ADC actual
                    fetch('/api/sensorData?' + new Date().getTime())
                        .then(response => response.json())
                        .then(data => {
                            const adcValue = currentSensor === 1 ? data.soil1_raw_adc : data.soil2_raw_adc;
                            console.log('Valor ADC capturado:', adcValue);
                            
                            // Ahora sí actualizar con el valor real
                            updateCalibrationStepUI(point, adcValue);
                            showNextCalibrationStep(point);
                        })
                        .catch(error => console.error('Error obteniendo ADC:', error));
                }, 1000);
            }, 500);
        })
        .catch(error => {
            console.error('Error estableciendo punto:', error);
        });
}

function showNextCalibrationStep(currentPoint) {
    if (currentPoint === 'air') {
        const waterStep = document.getElementById('step-water');
        if (waterStep) {
            waterStep.classList.add('active');
            setTimeout(() => waterStep.scrollIntoView({ behavior: 'smooth' }), 300);
        }
    } else if (currentPoint === 'water') {
        const resultsStep = document.getElementById('step-results');
        if (resultsStep) {
            resultsStep.classList.add('active');
            setTimeout(() => {
                resultsStep.scrollIntoView({ behavior: 'smooth' });
                calculateFinalCalibration();
            }, 300);
        }
    }
}

// Función auxiliar para actualizar la interfaz del paso de calibración guiado Nº3
function updateCalibrationStepUI(point, rawValue) {
    const stepElement = document.getElementById(`step-${point}`);
    const statusElement = document.getElementById(`status-${point}`);
    
    if (stepElement && statusElement) {
        // Marcar como completado
        statusElement.textContent = 'Completado ✓';
        statusElement.className = 'step-status completed';
        statusElement.style.background = '#10b981';
        statusElement.style.color = 'white';
        
        // Mostrar valor en el título
        const titleElement = stepElement.querySelector('.step-title');
        if (titleElement) {
            const existingValue = titleElement.querySelector('.captured-value');
            if (existingValue) existingValue.remove();
            
            const valueSpan = document.createElement('span');
            valueSpan.className = 'captured-value';
            valueSpan.style.color = '#10b981';
            valueSpan.style.fontSize = '0.9em';
            valueSpan.style.marginLeft = '10px';
            valueSpan.textContent = `(${rawValue} ADC)`;
            titleElement.appendChild(valueSpan);
        }
        
        // ACTUALIZAR VENTANA DE RESULTADOS
        if (point === 'air') {
            updateElement('cal-point-air', rawValue + ' ADC (punto seco)');
        } else if (point === 'water') {
            updateElement('cal-point-water', rawValue + ' ADC (punto húmedo)');
            
            // Si tenemos ambos puntos, calcular
            calculateFinalCalibration();
        }
    }
}

// Nueva función para calcular y mostrar resultados finales
function calculateFinalCalibration() {
    console.log('Calculando calibración final...');
    
    // Obtener valores de la interfaz
    const airText = document.getElementById('cal-point-air')?.textContent || '';
    const waterText = document.getElementById('cal-point-water')?.textContent || '';
    
    // Extraer valores numéricos
    const airValue = parseInt(airText.split(' ')[0]) || 0;
    const waterValue = parseInt(waterText.split(' ')[0]) || 0;
    
    console.log('Valores capturados - Aire:', airValue, 'Agua:', waterValue);
    
    if (airValue > 0 && waterValue > 0 && airValue !== waterValue) {
        // Calcular diferencia y tipo de sensor
        const difference = Math.abs(airValue - waterValue);
        const isInverse = airValue > waterValue;
        
        // Mostrar información en la ventana de resultados
        updateElement('cal-new-offset', 'Calculado automáticamente');
        
        let precisionText = '';
        if (difference > 1000) {
            precisionText = '🎯 Excelente diferencia (' + difference + ' ADC)';
        } else if (difference > 500) {
            precisionText = '✅ Buena diferencia (' + difference + ' ADC)';
        } else {
            precisionText = '⚠️ Diferencia pequeña (' + difference + ' ADC)';
        }
        
        precisionText += ' | Sensor: ' + (isInverse ? 'INVERSO' : 'NORMAL');
        updateElement('cal-precision', precisionText);
        
        // Activar botón de aplicación
        const applyBtn = document.getElementById('apply-calibration');
        if (applyBtn) {
            applyBtn.disabled = false;
            applyBtn.style.background = '#10b981';
            applyBtn.innerHTML = '<i class="fas fa-check"></i> Aplicar Calibración Automática';
        }
        
        // Marcar paso como completado
        const resultsStatus = document.getElementById('status-results');
        if (resultsStatus) {
            resultsStatus.textContent = 'Completado ✓';
            resultsStatus.className = 'step-status completed';
            resultsStatus.style.background = '#10b981';
        }
        
        console.log('Calibración calculada correctamente');
    } else {
        console.log('No se pueden calcular los resultados - valores insuficientes');
        updateElement('cal-precision', 'Error: Valores insuficientes');
    }
}

// Función auxiliar mejorada
function updateElement(id, content) {
    const element = document.getElementById(id);
    if (element) {
        element.textContent = content;
        console.log('Actualizado', id, ':', content);
    } else {
        console.error('Elemento no encontrado:', id);
    }
}


// Calcular calibración automática
function calculateCalibration() {
    const airRaw = calData.air;    // Valor RAW cuando está seco (0% real)
    const waterRaw = calData.water; // Valor RAW cuando está húmedo (100% real)
    
    console.log("=== CÁLCULO DE CALIBRACIÓN (SENSOR INVERSO) ===");
    console.log("Punto seco (0% real):", airRaw, "% raw");
    console.log("Punto húmedo (100% real):", waterRaw, "% raw");
    
    // VALIDACIÓN CORREGIDA: Para sensores inversos, airRaw DEBE ser MAYOR que waterRaw
    if (airRaw === waterRaw) {
        updateElement('cal-precision', '❌ Error: Valores idénticos');
        updateElement('status-results', 'Error en calibración');
        document.getElementById('apply-calibration').disabled = true;
        return;
    }
    
    const isInverseSensor = airRaw > waterRaw;
    const rawDifference = Math.abs(airRaw - waterRaw);
    
    if (rawDifference < 10) {
        updateElement('cal-precision', '❌ Error: Diferencia muy pequeña (<10%)');
        updateElement('status-results', 'Diferencia insuficiente');
        document.getElementById('apply-calibration').disabled = true;
        return;
    }
    
    // ✅ FÓRMULA CORRECTA PARA SENSORES INVERSOS
    let scale, offset;
    
    if (isInverseSensor) {
        // SENSOR INVERSO: 0% = valor ALTO, 100% = valor BAJO
        // Fórmula: porcentaje = (valor_seco - valor_actual) / (valor_seco - valor_húmedo) * 100
        scale = 100.0 / (airRaw - waterRaw);
        offset = airRaw * scale;  // Offset positivo
    } else {
        // SENSOR NORMAL: 0% = valor BAJO, 100% = valor ALTO  
        scale = 100.0 / (waterRaw - airRaw);
        offset = -airRaw * scale; // Offset negativo
    }
    
    console.log("Tipo sensor:", isInverseSensor ? "INVERSO" : "NORMAL");
    console.log("Escala calculada:", scale.toFixed(3));
    console.log("Offset calculado:", offset.toFixed(2));
    
    // Mostrar resultados
    updateElement('cal-point-air', airRaw.toFixed(1) + '% raw');
    updateElement('cal-point-water', waterRaw.toFixed(1) + '% raw');
    updateElement('cal-new-offset', offset.toFixed(2));
    
    // VERIFICACIÓN CON EJEMPLOS
    let testDry, testWet;
    
    if (isInverseSensor) {
        testDry = (airRaw - airRaw) * scale;      // Debería ser 0%
        testWet = (airRaw - waterRaw) * scale;    // Debería ser 100%
    } else {
        testDry = (airRaw - airRaw) * scale;      // Debería ser 0%  
        testWet = (waterRaw - airRaw) * scale;    // Debería ser 100%
    }
    
    console.log("Prueba punto seco:", testDry.toFixed(1) + '% (debería ser 0%)');
    console.log("Prueba punto húmedo:", testWet.toFixed(1) + '% (debería ser 100%)');
    
    const precisionDry = Math.abs(testDry);
    const precisionWet = Math.abs(100 - testWet);
    const avgPrecision = ((precisionDry + precisionWet) / 2).toFixed(1);
    
    // Información del tipo de sensor
    const sensorTypeInfo = isInverseSensor ? 
        "🔁 SENSOR INVERSO (valor baja con humedad)" : 
        "➡️ SENSOR NORMAL (valor sube con humedad)";
    
    // Evaluar calidad
    let precisionText, precisionColor;
    if (avgPrecision < 2) {
        precisionText = `🎯 Excelente (${avgPrecision}% error) | ${sensorTypeInfo}`;
        precisionColor = '#059669';
    } else if (avgPrecision < 5) {
        precisionText = `✅ Buena (${avgPrecision}% error) | ${sensorTypeInfo}`;
        precisionColor = '#10b981';
    } else if (avgPrecision < 10) {
        precisionText = `⚠️ Aceptable (${avgPrecision}% error) | ${sensorTypeInfo}`;
        precisionColor = '#f59e0b';
    } else {
        precisionText = `❌ Pobre (${avgPrecision}% error) | ${sensorTypeInfo}`;
        precisionColor = '#ef4444';
    }
    
    updateElement('cal-precision', precisionText);
    document.getElementById('cal-precision').style.color = precisionColor;
    
    // Información adicional
    const infoText = `Diferencia RAW: ${rawDifference.toFixed(1)}% | Tipo: ${isInverseSensor ? 'Inverso' : 'Normal'}`;
    updateElement('status-results', infoText);
    
    // Habilitar botón de aplicación
    const applyBtn = document.getElementById('apply-calibration');
    const offsetInput = document.getElementById('cal-offset-value');
    
    if (applyBtn && offsetInput) {
        applyBtn.disabled = false;
        offsetInput.value = offset.toFixed(2);
        
        applyBtn.innerHTML = `<i class="fas fa-check"></i> Aplicar Offset: ${offset.toFixed(1)}`;
        applyBtn.style.background = precisionColor;
        applyBtn.style.color = 'white';
    }
}

// Validar valores de calibración en tiempo real
function validateCalibrationValues() {
    if (calData.air === null || calData.water === null) return;
    
    const diff = calData.water - calData.air;
    const statusElement = document.getElementById('status-results');
    
    if (!statusElement) return;
    
    if (diff < 15) {
        statusElement.innerHTML = '⚠️ Diferencia pequeña';
        statusElement.style.background = '#f59e0b';
    } else if (diff < 30) {
        statusElement.innerHTML = '✅ Diferencia aceptable';
        statusElement.style.background = '#10b981';
    } else {
        statusElement.innerHTML = '🎯 Excelente diferencia';
        statusElement.style.background = '#059669';
    }
    // Añadir información del tipo de sensor
    statusText += ` | Tipo: ${isInverse ? 'Inverso' : 'Normal'}`;
    
    statusElement.innerHTML = statusText;
    statusElement.style.background = statusColor;
}

// Resetear pasos de calibración
function resetCalibrationSteps() {
    // Resetear estados visuales
    const steps = ['air', 'water', 'results'];
    steps.forEach(step => {
        const stepElement = document.getElementById(`step-${step}`);
        const statusElement = document.getElementById(`status-${step}`);
        
        if (stepElement && statusElement) {
            if (step !== 'air') stepElement.classList.remove('active');
            statusElement.textContent = step === 'results' ? 'Pendiente' : 'Pendiente';
            statusElement.className = 'step-status';
        }
    });
    
    // Resetear resultados
    updateElement('cal-point-air', '--');
    updateElement('cal-point-water', '--');
    updateElement('cal-new-offset', '--');
    updateElement('cal-precision', '--');
    
    // Deshabilitar botón de aplicación
    const applyBtn = document.getElementById('apply-calibration');
    if (applyBtn) {
        applyBtn.disabled = true;
        applyBtn.innerHTML = '<i class="fas fa-check"></i> Aplicar Calibración';
        applyBtn.style.background = '';
    }
}

// Funciones auxiliares
function updateElement(id, content) {
    const element = document.getElementById(id);
    if (element && element.textContent !== content) {
        element.textContent = content;
    }
}

function updatePumpStatus(pumpNumber, isOn) {
    const pumpElement = document.getElementById(`pump${pumpNumber}-status`);
    if (pumpElement) {
        pumpElement.className = isOn ? 'status-badge status-on pump-active' : 'status-badge status-off';
        pumpElement.innerHTML = isOn ? 'ACTIVA 🔥' : 'INACTIVA';
    }
}

function updateWaterSupply(waterSupply) {
    const waterSensor = document.getElementById('water-sensor');
    const waterValue = document.getElementById('water-value');
    const waterIcon = document.getElementById('water-icon');
    
    if (waterSensor && waterValue && waterIcon) {
        if (waterSupply) {
            waterSensor.classList.remove('no-water');
            waterIcon.classList.remove('no-water');
            waterValue.textContent = 'OK';
        } else {
            waterSensor.classList.add('no-water');
            waterIcon.classList.add('no-water');
            waterValue.textContent = 'FALLO';
        }
    }
}

function updateVPDQuality(vpd) {
    const vpdQuality = document.getElementById('vpd-quality');
    if (vpdQuality) {
        let quality, color;
        
        if (vpd < 0.4) { quality = 'MUY BAJO'; color = '#ef4444'; }
        else if (vpd < 0.8) { quality = 'BAJO'; color = '#f59e0b'; }
        else if (vpd <= 1.2) { quality = 'ÓPTIMO'; color = '#10b981'; }
        else if (vpd <= 1.5) { quality = 'ALTO'; color = '#f59e0b'; }
        else { quality = 'MUY ALTO'; color = '#ef4444'; }
        
        vpdQuality.textContent = quality;
        vpdQuality.style.color = color;
    }
}

function updateUIStatus(type, message) {
    const syncIcon = document.getElementById('sync-icon');
    const statusElement = document.getElementById('update-status');
    
    if (syncIcon && statusElement) {
        if (type === 'success') {
            syncIcon.style.color = '#10b981';
            syncIcon.className = 'fas fa-check-circle';
            statusElement.textContent = message;
            // Mensaje de éxito breve
            setTimeout(() => {
                statusElement.textContent = 'Actualizando...';
                syncIcon.className = 'fas fa-sync-alt fa-spin';
                syncIcon.style.color = '';
            }, 2000);
        } else if (type === 'error') {
            syncIcon.style.color = '#ef4444';
            syncIcon.className = 'fas fa-exclamation-triangle';
            statusElement.textContent = message;
            // Error breve
            setTimeout(() => {
                statusElement.textContent = 'Actualizando...';
                syncIcon.className = 'fas fa-sync-alt fa-spin';
                syncIcon.style.color = '';
            }, 3000);
        }
    }
}

// Para los botones de modo de operación
function setMode(mode) {
    document.querySelectorAll('.mode-button').forEach(btn => {
        btn.classList.remove('active');
    });
    
    // Activar el seleccionado visualmente (temporalmente)
    const selectedBtn = document.querySelector('.mode-button.' + mode);
    if (selectedBtn) {
        selectedBtn.classList.add('active');
    }
    
    // Mapear nombres a endpoints reales
    let endpoint;
    switch(mode) {
        case 'manual':
            endpoint = '/modemanual';
            break;
        case 'auto':
            endpoint = '/modeauto';
            break;
        case 'adaptativo':
            endpoint = '/modeia';
            break;
        default:
            endpoint = '/modeauto';
    }
    
    // Enviar comando al ESP32
    fetch(endpoint)
    .then(response => {
        if (response.ok || response.status === 303) {
            updateUIStatus('success', 'Modo ' + mode.toUpperCase() + ' activado');
            
            // Recargar la página después de un éxito para actualizar todo
            setTimeout(() => {
                window.location.reload();
            }, 1000);
            return;
        }
        
        if (response.status === 400) {
            // ERROR ESPECÍFICO - Datos insuficientes para modo adaptativo
            return response.text().then(html => {
                showAdaptiveModePopup(html);
                throw new Error('Datos insuficientes');
            });
        }
        
        throw new Error(`Error HTTP: ${response.status}`);
    })
    .catch(error => {
        console.error('❌ Error cambiando modo:', error);
        
        // Solo mostrar error en UI si NO es el caso de datos insuficientes (que ya mostró popup)
        if (!error.message.includes('Datos insuficientes')) {
            updateUIStatus('error', 'Error cambiando modo');
        }
        
        // Revertir selección visual
        if (selectedBtn) {
            selectedBtn.classList.remove('active');
        }
        
        // Restaurar el modo anterior
        setTimeout(() => {
            fetch('/api/currentMode')
            .then(response => response.json())
            .then(data => {
                setInitialMode(data.mode);
            });
        }, 1000);
    });
}

// Función para mostrar popup del modo adaptativo
function showAdaptiveModePopup(html) {
    // Crear popup
    const popup = document.createElement('div');
    popup.style.cssText = `
        position: fixed;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        background: #1e293b;
        color: white;
        padding: 30px;
        border-radius: 15px;
        border-left: 5px solid #f59e0b;
        box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        z-index: 10000;
        max-width: 500px;
        text-align: center;
    `;
    
    // Parsear el HTML de error del ESP32
    const parser = new DOMParser();
    const doc = parser.parseFromString(html, 'text/html');
    const content = doc.querySelector('div') || doc.body;
    
    popup.innerHTML = content.innerHTML;
    
    // Agregar botón de cerrar
    const closeBtn = document.createElement('button');
    closeBtn.textContent = 'Cerrar';
    closeBtn.style.cssText = `
        background: #6366f1;
        color: white;
        border: none;
        padding: 10px 20px;
        border-radius: 8px;
        cursor: pointer;
        margin-top: 20px;
        font-weight: bold;
    `;
    closeBtn.onclick = () => {
        document.body.removeChild(popup);
        document.body.removeChild(overlay);
    };
    
    popup.appendChild(closeBtn);
    
    // Crear overlay
    const overlay = document.createElement('div');
    overlay.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: rgba(0,0,0,0.7);
        z-index: 9999;
    `;
    overlay.onclick = () => {
        document.body.removeChild(popup);
        document.body.removeChild(overlay);
    };
    
    // Agregar al DOM
    document.body.appendChild(overlay);
    document.body.appendChild(popup);
}



// Función para inicializar el estado de los botones de modo al cargar la página
function initializeModeButtons() {
    // Obtener el modo actual del sistema (esto requiere un endpoint adicional)
    fetch('/api/currentMode')
    .then(response => response.json())
    .then(data => {
        if (data.mode !== undefined) {
            setInitialMode(data.mode);
        }
    })
    .catch(error => {
        console.log('No se pudo obtener el modo actual, usando valores por defecto');
        // Por defecto, modo automático activo
        setInitialMode(1);
    });
}

// Función auxiliar para establecer el modo inicial
function setInitialMode(modeNumber) {
    let modeName = '';
    switch(modeNumber) {
        case 0: modeName = 'manual'; break;
        case 1: modeName = 'auto'; break;
        case 2: modeName = 'adaptativo'; break;
        default: modeName = 'auto';
    }
    
    // Solo establecer visualmente, sin enviar comando al servidor
    document.querySelectorAll('.mode-button').forEach(btn => {
        btn.classList.remove('active');
    });
    
    const initialBtn = document.querySelector('.mode-button.' + modeName);
    if (initialBtn) {
        initialBtn.classList.add('active');
    }
    
    console.log('Modo inicial establecido:', modeName);
}

// Control de visibilidad de pestaña (PAUSA AUTOMÁTICA)
function handleVisibilityChange() {
    if (document.hidden) {
        // Pestaña no visible - PAUSAR
        isTabVisible = false;
        clearInterval(updateInterval);
        updateUIStatus('warning', 'Pausado (pestaña oculta)');
    } else {
        // Pestaña visible - REANUDAR
        isTabVisible = true;
        clearInterval(updateInterval);
        updateInterval = setInterval(updateLiveData, 3000);
        updateLiveData(); // Actualización inmediata
        updateUIStatus('success', 'Reanudado');
    }
}

// Inicialización COMPLETA
document.addEventListener('DOMContentLoaded', function() {
    console.log('Sistema de tiempo real iniciado');
    
    // 1. Configurar detector de visibilidad
    document.addEventListener('visibilitychange', handleVisibilityChange);
    
    // 2. Inicializar botones de modo
    initializeModeButtons();
    
    // 3. Iniciar actualización periódica
    updateInterval = setInterval(updateLiveData, 3000);
    updateLiveData(); // Primera actualización inmediata
    
    // 4. Inicializar calibración
    resetCalibrationSteps();
    
    console.log('✅ Sistema completamente inicializado');
});
</script>
)=====";

#endif  // HTML_TEMPLATES_H