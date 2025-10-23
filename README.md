# ESP32-S3 Smart Irrigation System

A feature-rich, autonomous irrigation system built on the ESP32-S3 using the Arduino core and FreeRTOS. It intelligently
 manages plant watering based on a wide array of local and external data sources.

## Key Features

*   **Online/Offline mode
*   **Multi-Sensor Input:** Monitors ambient humidity, soil moisture (2 channels), ambient temperature, atmospheric pressure, and VOC (air quality).
*   **Two zones of irrigation with independent settings
*   **Intelligent Control Modes:** Features Auto, Manual, and an Adaptive (self-learning) mode for dynamic irrigation scheduling.
*   **Advanced Environmental Data:** Uses a local BH1750FVI light sensor and BME680 temp/humidity/pressure/VOC, fetching external data from a weather service if available.
*   **Data-Driven Decisions:** Calculates Vapor Pressure Deficit (VPD) and uses sensor inference for precise auto-watering. 
*   **Includes soil-type compensation and guided calibration for soil moisture sensors.

*   **Connectivity & Control:**
    *   Web interface and Telegram bot for remote control.
    *   RESTful API endpoints (`/api/sensors`, `/api/status`, `/api/learning`).
    *   mDNS support for easy network discovery (e.g., `miriego.local`)

*   **System Management:** Full configuration backup/restore, an RTC (DS3231) for reliable timekeeping, and watering restrictions based on schedule and external wind conditions.
