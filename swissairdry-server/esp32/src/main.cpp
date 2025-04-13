/**
 * SwissAirDry ESP32 Firmware - Hauptdatei
 * 
 * Diese Datei enthält die Hauptfunktionen der ESP32-Firmware für das
 * SwissAirDry-System zur Überwachung und Steuerung von Trocknungsgeräten.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Eigene Header-Dateien
#include "config.h"
#include "wifi_manager.h"
#include "mqtt_client.h"
#include "sensors.h"
#include "ota_updater.h"

// Globale Variablen
WifiManager wifiManager;
MqttClient mqttClient;
SensorManager sensorManager;
OtaUpdater otaUpdater;
Config config;

AsyncWebServer server(80);

// NTP-Client für Zeitsynchronisierung
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Timer-Variablen
unsigned long lastReadTime = 0;
unsigned long lastReportTime = 0;

/**
 * Initialisierung der Konfiguration aus der gespeicherten JSON-Datei
 */
bool loadConfiguration() {
  // Prüfen, ob SPIFFS initialisiert ist
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS konnte nicht initialisiert werden. Verwendung der Standardkonfiguration.");
    return false;
  }

  // Prüfen, ob die Konfigurationsdatei existiert
  if (!SPIFFS.exists("/config.json")) {
    Serial.println("Konfigurationsdatei nicht gefunden. Verwendung der Standardkonfiguration.");
    return false;
  }

  // Konfigurationsdatei öffnen
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Konfigurationsdatei konnte nicht geöffnet werden. Verwendung der Standardkonfiguration.");
    return false;
  }

  // Größe der Datei prüfen
  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Konfigurationsdatei ist zu groß. Verwendung der Standardkonfiguration.");
    return false;
  }

  // JSON-Puffer erstellen und Inhalt lesen
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error) {
    Serial.print("Fehler beim Parsen der Konfigurationsdatei: ");
    Serial.println(error.c_str());
    return false;
  }

  // Konfiguration aus JSON-Dokument extrahieren
  config.deviceName = doc["device"]["name"] | "SwissAirDry-Gateway";
  config.deviceId = doc["device"]["id"] | "gateway001";
  config.deviceType = doc["device"]["type"] | "esp32-gateway";

  config.wifiSsid = doc["wifi"]["ssid"] | "";
  config.wifiPassword = doc["wifi"]["password"] | "";
  config.apSsid = doc["wifi"]["ap_ssid"] | "SwissAirDry-Setup";
  config.apPassword = doc["wifi"]["ap_password"] | "setup1234";

  config.mqttBroker = doc["mqtt"]["broker"] | "mqtt.swissairdry.com";
  config.mqttPort = doc["mqtt"]["port"] | 1883;
  config.mqttUsername = doc["mqtt"]["username"] | "";
  config.mqttPassword = doc["mqtt"]["password"] | "";
  config.mqttClientId = doc["mqtt"]["client_id"] | "gateway001";
  config.mqttBaseTopic = doc["mqtt"]["base_topic"] | "swissairdry/devices/gateway001";

  config.dhtPin = doc["sensors"]["dht_pin"] | 4;
  config.dhtType = doc["sensors"]["dht_type"] | "DHT22";
  config.useBme280 = doc["sensors"]["use_bme280"] | false;
  config.energyMeterEnabled = doc["sensors"]["energy_meter_enabled"] | false;
  config.energyMeterRxPin = doc["sensors"]["energy_meter_rx_pin"] | 16;
  config.energyMeterTxPin = doc["sensors"]["energy_meter_tx_pin"] | 17;

  config.relayPin = doc["control"]["relay_pin"] | 5;
  config.hasSpeedControl = doc["control"]["has_speed_control"] | false;
  config.speedControlPin = doc["control"]["speed_control_pin"] | 13;

  config.readInterval = doc["system"]["read_interval"] | 60;
  config.reportingInterval = doc["system"]["reporting_interval"] | 300;
  config.otaEnabled = doc["system"]["ota_enabled"] | true;
  config.debugMode = doc["system"]["debug_mode"] | false;

  Serial.println("Konfiguration erfolgreich geladen.");
  return true;
}

/**
 * Webserver für Konfigurationsoberfläche einrichten
 */
void setupWebServer() {
  // Wurzelverzeichnis - Konfigurationsseite ausliefern
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // CSS und JavaScript Dateien ausliefern
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/styles.css", "text/css");
  });

  // Aktuelle Konfiguration als JSON abrufen
  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<1024> doc;

    JsonObject device = doc.createNestedObject("device");
    device["name"] = config.deviceName;
    device["id"] = config.deviceId;
    device["type"] = config.deviceType;

    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["ssid"] = config.wifiSsid;
    wifi["password"] = ""; // Aus Sicherheitsgründen nicht zurückgeben
    wifi["ap_ssid"] = config.apSsid;
    wifi["ap_password"] = ""; // Aus Sicherheitsgründen nicht zurückgeben

    JsonObject mqtt = doc.createNestedObject("mqtt");
    mqtt["broker"] = config.mqttBroker;
    mqtt["port"] = config.mqttPort;
    mqtt["username"] = config.mqttUsername;
    mqtt["password"] = ""; // Aus Sicherheitsgründen nicht zurückgeben
    mqtt["client_id"] = config.mqttClientId;
    mqtt["base_topic"] = config.mqttBaseTopic;

    JsonObject sensors = doc.createNestedObject("sensors");
    sensors["dht_pin"] = config.dhtPin;
    sensors["dht_type"] = config.dhtType;
    sensors["use_bme280"] = config.useBme280;
    sensors["energy_meter_enabled"] = config.energyMeterEnabled;
    sensors["energy_meter_rx_pin"] = config.energyMeterRxPin;
    sensors["energy_meter_tx_pin"] = config.energyMeterTxPin;

    JsonObject control = doc.createNestedObject("control");
    control["relay_pin"] = config.relayPin;
    control["has_speed_control"] = config.hasSpeedControl;
    control["speed_control_pin"] = config.speedControlPin;

    JsonObject system = doc.createNestedObject("system");
    system["read_interval"] = config.readInterval;
    system["reporting_interval"] = config.reportingInterval;
    system["ota_enabled"] = config.otaEnabled;
    system["debug_mode"] = config.debugMode;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Konfiguration aktualisieren
  server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Konfiguration wird aktualisiert...");
    // Die eigentliche Verarbeitung erfolgt im Handler für den Anforderungskörper
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // JSON-Daten parsen
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
      Serial.print("Fehler beim Parsen der Konfigurationsdaten: ");
      Serial.println(error.c_str());
      return;
    }

    // Konfiguration aus JSON-Dokument aktualisieren
    if (doc.containsKey("device")) {
      if (doc["device"].containsKey("name")) config.deviceName = doc["device"]["name"].as<String>();
      if (doc["device"].containsKey("id")) config.deviceId = doc["device"]["id"].as<String>();
      if (doc["device"].containsKey("type")) config.deviceType = doc["device"]["type"].as<String>();
    }

    if (doc.containsKey("wifi")) {
      if (doc["wifi"].containsKey("ssid")) config.wifiSsid = doc["wifi"]["ssid"].as<String>();
      if (doc["wifi"].containsKey("password") && doc["wifi"]["password"].as<String>() != "")
        config.wifiPassword = doc["wifi"]["password"].as<String>();
      if (doc["wifi"].containsKey("ap_ssid")) config.apSsid = doc["wifi"]["ap_ssid"].as<String>();
      if (doc["wifi"].containsKey("ap_password") && doc["wifi"]["ap_password"].as<String>() != "")
        config.apPassword = doc["wifi"]["ap_password"].as<String>();
    }

    if (doc.containsKey("mqtt")) {
      if (doc["mqtt"].containsKey("broker")) config.mqttBroker = doc["mqtt"]["broker"].as<String>();
      if (doc["mqtt"].containsKey("port")) config.mqttPort = doc["mqtt"]["port"].as<int>();
      if (doc["mqtt"].containsKey("username")) config.mqttUsername = doc["mqtt"]["username"].as<String>();
      if (doc["mqtt"].containsKey("password") && doc["mqtt"]["password"].as<String>() != "")
        config.mqttPassword = doc["mqtt"]["password"].as<String>();
      if (doc["mqtt"].containsKey("client_id")) config.mqttClientId = doc["mqtt"]["client_id"].as<String>();
      if (doc["mqtt"].containsKey("base_topic")) config.mqttBaseTopic = doc["mqtt"]["base_topic"].as<String>();
    }

    if (doc.containsKey("sensors")) {
      if (doc["sensors"].containsKey("dht_pin")) config.dhtPin = doc["sensors"]["dht_pin"].as<int>();
      if (doc["sensors"].containsKey("dht_type")) config.dhtType = doc["sensors"]["dht_type"].as<String>();
      if (doc["sensors"].containsKey("use_bme280")) config.useBme280 = doc["sensors"]["use_bme280"].as<bool>();
      if (doc["sensors"].containsKey("energy_meter_enabled")) config.energyMeterEnabled = doc["sensors"]["energy_meter_enabled"].as<bool>();
      if (doc["sensors"].containsKey("energy_meter_rx_pin")) config.energyMeterRxPin = doc["sensors"]["energy_meter_rx_pin"].as<int>();
      if (doc["sensors"].containsKey("energy_meter_tx_pin")) config.energyMeterTxPin = doc["sensors"]["energy_meter_tx_pin"].as<int>();
    }

    if (doc.containsKey("control")) {
      if (doc["control"].containsKey("relay_pin")) config.relayPin = doc["control"]["relay_pin"].as<int>();
      if (doc["control"].containsKey("has_speed_control")) config.hasSpeedControl = doc["control"]["has_speed_control"].as<bool>();
      if (doc["control"].containsKey("speed_control_pin")) config.speedControlPin = doc["control"]["speed_control_pin"].as<int>();
    }

    if (doc.containsKey("system")) {
      if (doc["system"].containsKey("read_interval")) config.readInterval = doc["system"]["read_interval"].as<int>();
      if (doc["system"].containsKey("reporting_interval")) config.reportingInterval = doc["system"]["reporting_interval"].as<int>();
      if (doc["system"].containsKey("ota_enabled")) config.otaEnabled = doc["system"]["ota_enabled"].as<bool>();
      if (doc["system"].containsKey("debug_mode")) config.debugMode = doc["system"]["debug_mode"].as<bool>();
    }

    // Konfiguration in Datei speichern
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Fehler beim Öffnen der Konfigurationsdatei zum Schreiben.");
      return;
    }

    // Aktualisierte Konfiguration serialisieren und speichern
    serializeJson(doc, configFile);
    configFile.close();

    Serial.println("Konfiguration erfolgreich aktualisiert.");
    
    // Neustart in 3 Sekunden, um die neuen Einstellungen zu übernehmen
    delay(1000);
    ESP.restart();
  });

  // Aktuelle Sensordaten abrufen
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument data = sensorManager.readSensorData();
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
  });

  // Neustart des ESP32 auslösen
  server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Neustart wird durchgeführt...");
    delay(1000);
    ESP.restart();
  });

  // Dateien im SPIFFS-Dateisystem auflisten
  server.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<1024> doc;
    JsonArray files = doc.createNestedArray("files");

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
      JsonObject fileObj = files.createNestedObject();
      fileObj["name"] = String(file.name());
      fileObj["size"] = file.size();
      file = root.openNextFile();
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Unbekannte Pfade auf die Hauptseite umleiten
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  // Server starten
  server.begin();
  Serial.println("HTTP-Server gestartet");
}

/**
 * Setup - wird einmalig beim Start ausgeführt
 */
void setup() {
  // Serielle Verbindung initialisieren
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n");
  Serial.println("===========================================");
  Serial.println("SwissAirDry ESP32 Firmware");
  Serial.println("Version: 1.0.0");
  Serial.println("===========================================");
  
  // SPIFFS initialisieren
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS-Initialisierung fehlgeschlagen!");
    delay(3000);
    ESP.restart();
    return;
  }
  
  // Konfiguration laden
  if (!loadConfiguration()) {
    Serial.println("Verwende Standard-Konfiguration");
  }
  
  // Ausgabe der Konfiguration im Debug-Modus
  if (config.debugMode) {
    Serial.println("Aktuelle Konfiguration:");
    Serial.println("Gerätename: " + config.deviceName);
    Serial.println("Geräte-ID: " + config.deviceId);
    Serial.println("MQTT-Broker: " + config.mqttBroker);
    Serial.println("MQTT-Basis-Topic: " + config.mqttBaseTopic);
  }
  
  // WiFi initialisieren
  wifiManager.init(config.wifiSsid, config.wifiPassword, config.apSsid, config.apPassword);
  
  // Verbindung zum WLAN herstellen
  if (wifiManager.connect()) {
    // MQTT-Client initialisieren
    mqttClient.init(
      config.mqttBroker,
      config.mqttPort,
      config.mqttClientId,
      config.mqttUsername,
      config.mqttPassword,
      config.mqttBaseTopic
    );
    
    // NTP-Client starten
    timeClient.begin();
    timeClient.setTimeOffset(3600); // UTC+1 (MEZ)
    timeClient.update();
    
    // OTA-Updates aktivieren, wenn in der Konfiguration aktiviert
    if (config.otaEnabled) {
      otaUpdater.init(config.deviceName);
    }
  } else {
    Serial.println("Wi-Fi-Verbindung fehlgeschlagen, AP-Modus aktiviert");
  }
  
  // Sensoren initialisieren
  sensorManager.init(
    config.dhtPin,
    config.dhtType,
    config.useBme280,
    config.energyMeterEnabled,
    config.energyMeterRxPin,
    config.energyMeterTxPin
  );
  
  // GPIO für Relais (Gerätesteuerung) konfigurieren
  if (config.relayPin > 0) {
    pinMode(config.relayPin, OUTPUT);
    digitalWrite(config.relayPin, LOW); // Standardmäßig ausgeschaltet
  }
  
  // GPIO für Geschwindigkeitssteuerung konfigurieren
  if (config.hasSpeedControl && config.speedControlPin > 0) {
    pinMode(config.speedControlPin, OUTPUT);
    analogWrite(config.speedControlPin, 0); // Standardmäßig aus
  }
  
  // Webserver für Konfiguration einrichten
  setupWebServer();
  
  // Online-Status an MQTT-Broker senden
  if (mqttClient.isConnected()) {
    mqttClient.publishStatus("online");
  }
  
  Serial.println("Setup abgeschlossen");
}

/**
 * Hauptschleife - wird kontinuierlich ausgeführt
 */
void loop() {
  // NTP-Zeit aktualisieren
  timeClient.update();
  
  // OTA-Updates überwachen, wenn aktiviert
  if (config.otaEnabled) {
    otaUpdater.handle();
  }
  
  // WLAN-Verbindung überwachen
  wifiManager.handle();
  
  // MQTT-Verbindung überwachen
  if (WiFi.status() == WL_CONNECTED) {
    mqttClient.handle();
  }
  
  // Sensordaten in regelmäßigen Abständen auslesen
  unsigned long currentMillis = millis();
  
  // Sensordaten auslesen
  if (currentMillis - lastReadTime >= config.readInterval * 1000) {
    lastReadTime = currentMillis;
    
    // Sensordaten auslesen
    JsonDocument sensorData = sensorManager.readSensorData();
    
    // Daten an MQTT-Broker senden, wenn es Zeit für den Bericht ist
    if (currentMillis - lastReportTime >= config.reportingInterval * 1000) {
      lastReportTime = currentMillis;
      
      if (mqttClient.isConnected()) {
        mqttClient.publishData(sensorData);
      } else {
        Serial.println("MQTT nicht verbunden, Daten können nicht gesendet werden");
        // Hier könnte eine lokale Speicherung der Daten für späteres Senden implementiert werden
      }
    }
    
    // Im Debug-Modus die Sensordaten auf der seriellen Schnittstelle ausgeben
    if (config.debugMode) {
      String output;
      serializeJson(sensorData, output);
      Serial.println("Sensordaten: " + output);
    }
  }
  
  // Kleine Pause, um die CPU nicht zu stark zu belasten
  delay(100);
}