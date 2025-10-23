// ia-dashboard.h
#ifndef IA_DASHBOARD_H
#define IA_DASHBOARD_H

#include <Arduino.h>
#include <WebServer.h>

extern WebServer server;

// ia-dashboard.h - VERSIÓN TEMA OSCURO
const char IA_DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Dashboard IA - Sistema Riego Inteligente</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * { 
            margin: 0; 
            padding: 0; 
            box-sizing: border-box; 
        }
        
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: #0f0f0f;
            color: #e0e0e0;
            min-height: 100vh;
        }
        
        .header {
            background: #1a1a1a;
            padding: 1rem 2rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-bottom: 1px solid #333;
            box-shadow: 0 2px 10px rgba(0,0,0,0.3);
        }
        
        .header h1 {
            background: linear-gradient(135deg, #00d4ff, #0099ff);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            font-size: 1.5rem;
            font-weight: 600;
        }
        
        .back-btn {
            background: linear-gradient(135deg, #00d4ff, #0099ff);
            color: #0f0f0f;
            border: none;
            padding: 0.5rem 1.2rem;
            border-radius: 6px;
            cursor: pointer;
            text-decoration: none;
            font-weight: 600;
            transition: all 0.3s ease;
        }
        
        .back-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0, 212, 255, 0.4);
        }
        
        .dashboard-container { 
            padding: 2rem;
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 1.5rem;
            margin-bottom: 2rem;
        }
        
        .stat-card {
            background: #1a1a1a;
            padding: 1.5rem;
            border-radius: 12px;
            text-align: center;
            border: 1px solid #333;
            transition: all 0.3s ease;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        
        .stat-card:hover {
            transform: translateY(-5px);
            border-color: #00d4ff;
            box-shadow: 0 8px 15px rgba(0,0,0,0.2);
        }
        
        .stat-value {
            font-size: 2.2rem;
            font-weight: bold;
            background: linear-gradient(135deg, #00d4ff, #0099ff);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin: 0.5rem 0;
        }
        
        .stat-label {
            color: #888;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .chart-container {
            background: #1a1a1a;
            padding: 1.5rem;
            border-radius: 12px;
            margin-bottom: 1.5rem;
            border: 1px solid #333;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            width: 100%; /* Importante */
            max-width: 100%; /* No sobrepase el container */
        }

        .chart-container h3 {
            color: #00d4ff;
            margin-bottom: 1rem;
            ont-size: 1.1rem;
        }

        .chart-container canvas {
            width: 100% !important;
            height: 500px !important; /* Altura fija razonable */
            max-width: 100%;
        }
        
        /* Scrollbar personalizado para tema oscuro */
        ::-webkit-scrollbar {
            width: 8px;
        }
        
        ::-webkit-scrollbar-track {
            background: #1a1a1a;
        }
        
        ::-webkit-scrollbar-thumb {
            background: #333;
            border-radius: 4px;
        }
        
        ::-webkit-scrollbar-thumb:hover {
            background: #00d4ff;
        }
        
        @media (max-width: 768px) {
            .dashboard-container {
                padding: 1rem;
            }
            
            .header {
                padding: 1rem;
                flex-direction: column;
                gap: 1rem;
            }
            
            .header h1 {
                font-size: 1.3rem;
            }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🤖 Dashboard IA - Riego Inteligente</h1>
        <a href="/" class="back-btn">← Volver al Inicio</a>
    </div>
    
    <div class="dashboard-container">
        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-label">Inferencia Zona 1</div>
                <div class="stat-value" id="inference1">0%</div>
                <div class="stat-label">Necesidad de riego</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Inferencia Zona 2</div>
                <div class="stat-value" id="inference2">0%</div>
                <div class="stat-label">Necesidad de riego</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Eficiencia Global</div>
                <div class="stat-value" id="globalEfficiency">0%</div>
                <div class="stat-label">Sistema de aprendizaje</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Zona 1</div>
                <div class="stat-value" id="zone1-cycles">0</div>
                <div class="stat-label">Ciclos completados</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Zona 2</div>
                <div class="stat-value" id="zone2-cycles">0</div>
                <div class="stat-label">Ciclos completados</div>
            </div>
        </div>

        </div>
        
        <div class="chart-container">
            <h3>📈 Evolución Inferencia Inteligente</h3>
            <canvas id="inferenceChart" height="200"></canvas>
        </div>
    </div>

    <script>
    let inferenceChart;
    let updateInterval;
    
    function initializeCharts() {
        const ctx = document.getElementById('inferenceChart').getContext('2d');
        
        Chart.defaults.color = '#e0e0e0';
        Chart.defaults.borderColor = '#333';
        
        inferenceChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Zona 1 - Inferencia (%)',
                        borderColor: '#FFD700',
                        backgroundColor: 'rgba(255, 215, 0, 0.1)',
                        borderWidth: 3,
                        tension: 0.4,
                        pointBackgroundColor: '#FFD700',
                        data: []
                    },
                    {
                        label: 'Zona 2 - Inferencia (%)',
                        borderColor: '#0099ff',
                        backgroundColor: 'rgba(0, 153, 255, 0.1)',
                        borderWidth: 3,
                        tension: 0.4,
                        pointBackgroundColor: '#0099ff',
                        data: []
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        labels: { color: '#e0e0e0' }
                    },
                    tooltip: {
                        callbacks: {
                            label: function(context) {
                                return context.dataset.label + ': ' + 
                                       context.parsed.y.toFixed(1) + '%';
                            }
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        grid: { color: '#333' },
                        ticks: { 
                            color: '#888',
                            callback: function(value) {
                                return value + '%';
                            }
                        }
                    },
                    x: {
                        grid: { color: '#333' },
                        ticks: { color: '#888' }
                    }
                }
            }
        });
    }
    
    async function updateDashboard() {
        try {
            const response = await fetch('/api/iaData?' + new Date().getTime());
            if (!response.ok) throw new Error('Error HTTP: ' + response.status);
            
            const data = await response.json();
            
            // Actualizar estadísticas principales
            document.getElementById('inference1').textContent = 
                data.inference1.toFixed(1) + '%';
            document.getElementById('inference2').textContent = 
                data.inference2.toFixed(1) + '%';
            document.getElementById('globalEfficiency').textContent = 
                data.global_efficiency.toFixed(1) + '%';
            
            // Actualizar gráfico
            const now = new Date().toLocaleTimeString();
            
            // Limitar a 20 puntos en el gráfico
            if (inferenceChart.data.labels.length > 20) {
                inferenceChart.data.labels.shift();
                inferenceChart.data.datasets[0].data.shift();
                inferenceChart.data.datasets[1].data.shift();
            }
            
            inferenceChart.data.labels.push(now);
            inferenceChart.data.datasets[0].data.push(data.inference1);
            inferenceChart.data.datasets[1].data.push(data.inference2);
            inferenceChart.update('none'); // Sin animación para mejor rendimiento
            
        } catch (error) {
            console.error('Error actualizando dashboard:', error);
        }
    }
    document.getElementById('zone1-cycles').textContent = data.zone1_cycles;
    document.getElementById('zone2-cycles').textContent = data.zone2_cycles;
    document.addEventListener('DOMContentLoaded', function() {
        initializeCharts();
        updateDashboard(); // Primera actualización inmediata
        updateInterval = setInterval(updateDashboard, 3000); // Cada 3 segundos
    });
    
    // Pausar cuando la pestaña no es visible
    document.addEventListener('visibilitychange', function() {
        if (document.hidden) {
            clearInterval(updateInterval);
        } else {
            updateInterval = setInterval(updateDashboard, 3000);
            updateDashboard();
        }
    });
</script>
</body>
</html>
)rawliteral";

void handleIADashboard() {
    String html = FPSTR(IA_DASHBOARD_HTML);
    server.send(200, "text/html", html);
}

#endif