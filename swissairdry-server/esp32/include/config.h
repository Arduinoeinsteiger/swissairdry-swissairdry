/**
 * SwissAirDry ESP32 Firmware - Konfiguration
 * 
 * Diese Datei enthält die Konfigurationsstruktur und -variablen für die
 * ESP32-Firmware des SwissAirDry-Systems.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/**
 * Konfigurationsstruktur für die SwissAirDry ESP32-Firmware
 */
struct Config {
    // Geräteinformationen
    String deviceName;         // Name des Geräts
    String deviceId;           // Eindeutige ID des Geräts
    String deviceType;         // Typ des Geräts (z.B. "esp32-gateway")
    
    // WiFi-Konfiguration
    String wifiSsid;           // SSID des WiFi-Netzwerks
    String wifiPassword;       // Passwort des WiFi-Netzwerks
    String apSsid;             // SSID des Access Points (Konfigurationsmodus)
    String apPassword;         // Passwort des Access Points
    
    // MQTT-Konfiguration
    String mqttBroker;         // MQTT-Broker-Adresse
    int mqttPort;              // MQTT-Broker-Port
    String mqttUsername;       // MQTT-Benutzername
    String mqttPassword;       // MQTT-Passwort
    String mqttClientId;       // MQTT-Client-ID
    String mqttBaseTopic;      // MQTT-Basis-Topic
    
    // Sensor-Konfiguration
    int dhtPin;                // GPIO-Pin für den DHT-Sensor
    String dhtType;            // DHT-Sensortyp (DHT11, DHT22)
    bool useBme280;            // BME280-Sensor verwenden statt DHT
    bool energyMeterEnabled;   // Energiemessung aktivieren
    int energyMeterRxPin;      // RX-Pin für Energiemessmodul
    int energyMeterTxPin;      // TX-Pin für Energiemessmodul
    
    // Steuerungs-Konfiguration
    int relayPin;              // GPIO-Pin für das Relais
    bool hasSpeedControl;      // Geschwindigkeitssteuerung vorhanden
    int speedControlPin;       // GPIO-Pin für Geschwindigkeitssteuerung
    
    // System-Konfiguration
    int readInterval;          // Intervall für Sensorauslesung (Sekunden)
    int reportingInterval;     // Intervall für Datenübertragung (Sekunden)
    bool otaEnabled;           // OTA-Updates aktivieren
    bool debugMode;            // Debug-Modus aktivieren
};

#endif // CONFIG_H