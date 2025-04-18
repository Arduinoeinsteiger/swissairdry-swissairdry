/*
 * SwissAirDry ESP32-C6 Multifunktionssystem mit Farbdisplay
 * 
 * Unterstützt:
 * - 1.47" LCD-Farbdisplay (172x320)
 * - SD-Kartenspeicher
 * - QR-Code Generierung für einfache Verbindung
 * - MQTT für Sensor- und Steuerungsdaten
 * - API-Integration mit lokaler Zwischenspeicherung
 * - OTA Updates
 * 
 * Hardwarevoraussetzungen:
 * - ESP32-C6 Board mit 1.47" LCD-Display
 * - SD-Kartenslot
 */

// ----- BIBLIOTHEKEN -----
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <TFT_eSPI.h>         // Display-Treiber (muss für das spezifische Display konfiguriert sein)
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>     // MQTT
#include <HTTPClient.h>
#include <qrcode.h>           // QR-Code Generator
#include <TimeLib.h>
#include <Update.h>
#include <WebServer.h>
#include <DNSServer.h>

// ----- KONFIGURATION -----
// Diese Einstellungen können über die Weboberfläche angepasst werden
#define EEPROM_SIZE 2048
#define JSON_CONFIG_SIZE 1024
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_TITLE TFT_ORANGE
#define COLOR_ERROR TFT_RED
#define COLOR_SUCCESS TFT_GREEN
#define COLOR_WARNING TFT_YELLOW
#define COLOR_INFO TFT_BLUE
#define COLOR_STATUS_BAR TFT_DARKGREY
#define QR_CODE_SIZE 4        // Größenskalierung für QR-Codes

// Standard-WLAN-Einstellungen
#define DEFAULT_AP_SSID "SwissAirDry-Setup"
#define DEFAULT_AP_PASSWORD "12345678"
#define CONFIG_AP_IP IPAddress(192, 168, 4, 1)
#define DNS_PORT 53

// Pins
#define SD_CS_PIN 10          // SD-Karten Chip Select Pin (anpassen je nach Board)
#define RELAY_PIN 5           // Relais für die Steuerung

// Zeitintervalle
#define API_UPDATE_INTERVAL 60000        // API-Update alle 60 Sekunden
#define DISPLAY_UPDATE_INTERVAL 1000     // Display-Update alle 1 Sekunde
#define SENSOR_READ_INTERVAL 5000        // Sensor-Leseintervall alle 5 Sekunden
#define LOG_INTERVAL 300000             // Daten-Logging alle 5 Minuten

// ----- GLOBALE OBJEKTE -----
TFT_eSPI tft = TFT_eSPI();              // Display-Objekt
WebServer server(80);                    // Webserver für Konfiguration
WiFiClient wifiClient;                   // WiFi-Client für MQTT und HTTP
PubSubClient mqttClient(wifiClient);     // MQTT-Client
DNSServer dnsServer;                     // DNS-Server für Captive Portal
TaskHandle_t dataLoggingTask;            // Task-Handle für Datenprotokollierung

// ----- GLOBALE VARIABLEN -----
// Konfiguration
struct Config {
  char version[8] = "1.0.0";             // Konfigurationsversion
  char deviceName[32] = "SwissAirDry";   // Gerätename
  char wifiSSID[33] = "";                // WLAN-SSID
  char wifiPassword[65] = "";            // WLAN-Passwort
  bool apMode = true;                    // AP-Modus aktiviert
  
  // MQTT-Einstellungen
  char mqttServer[65] = "";              // MQTT-Server
  int mqttPort = 1883;                   // MQTT-Port
  char mqttUser[33] = "";                // MQTT-Benutzername
  char mqttPassword[65] = "";            // MQTT-Passwort
  char mqttTopic[65] = "swissairdry/";   // MQTT-Basistopic
  
  // API-Einstellungen
  char apiServer[65] = "";               // API-Server
  char apiKey[65] = "";                  // API-Schlüssel
  int apiUpdateInterval = 60;            // Update-Intervall in Sekunden
  
  // Relay-Einstellungen
  bool relayEnabled = true;              // Relais aktiviert
  int relayMode = 0;                     // 0=Manuell, 1=Zeit, 2=Sensor
  int relayOnHour = 8;                   // Einschaltzeit (Stunde)
  int relayOnMinute = 0;                 // Einschaltzeit (Minute)
  int relayOffHour = 18;                 // Ausschaltzeit (Stunde)
  int relayOffMinute = 0;                // Ausschaltzeit (Minute)
  float relayThreshold = 60.0;           // Schwellenwert für sensorbasierte Steuerung
  
  // Sensoreinstellungen
  bool temperatureSensor = true;         // Temperatursensor vorhanden
  bool humiditySensor = true;            // Feuchtigkeitssensor vorhanden
  bool energySensor = true;              // Energiesensor vorhanden
  int sensorReadInterval = 5;            // Leseintervall in Sekunden
  
  // Logging-Einstellungen
  bool sdLogging = true;                 // SD-Kartenprotokollierung aktiviert
  int logInterval = 300;                 // Protokollintervall in Sekunden
};

Config config;                           // Konfigurationsobjekt

// Status
bool wifiConnected = false;              // WLAN-Verbindung
bool mqttConnected = false;              // MQTT-Verbindung
bool sdCardAvailable = false;            // SD-Karte verfügbar
bool configMode = false;                 // Konfigurationsmodus aktiv
bool relayState = false;                 // Relais-Status
String networkIP = "";                   // IP-Adresse
String firmwareVersion = "1.0.0";        // Firmware-Version
unsigned long lastApiUpdate = 0;         // Letztes API-Update
unsigned long lastDisplayUpdate = 0;     // Letztes Display-Update
unsigned long lastSensorRead = 0;        // Letzter Sensorwert
unsigned long lastLog = 0;               // Letztes Logging
int displayPage = 0;                     // Aktuelle Displayseite
int errorCode = 0;                       // Fehlercode
String errorMessage = "";                // Fehlermeldung

// Sensordaten
struct SensorData {
  float temperature = 0.0;               // Temperatur in °C
  float humidity = 0.0;                  // Luftfeuchtigkeit in %
  float energy = 0.0;                    // Energieverbrauch in kWh
  float power = 0.0;                     // Aktuelle Leistung in W
  long totalRuntime = 0;                 // Gesamtlaufzeit in Sekunden
  long currentRuntime = 0;               // Aktuelle Laufzeit in Sekunden
  bool dataValid = false;                // Daten sind gültig
};

SensorData sensorData;                   // Sensordaten

// ----- FUNKTIONEN -----

// ----- SETUP -----
void setup() {
  // Serielle Schnittstelle initialisieren
  Serial.begin(115200);
  Serial.println("\nSwissAirDry ESP32-C6 mit Farbdisplay startet...");
  
  // Pins initialisieren
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Relais aus
  
  // EEPROM initialisieren
  EEPROM.begin(EEPROM_SIZE);
  
  // Konfiguration laden
  loadConfig();
  
  // Display initialisieren
  tft.begin();
  tft.setRotation(1);  // Rotation einstellen (0-3)
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Startbildschirm anzeigen
  showSplashScreen();
  
  // SD-Karte initialisieren
  initSDCard();
  
  // Mit WLAN verbinden oder AP-Modus starten
  if (!config.apMode && strlen(config.wifiSSID) > 0) {
    connectWiFi();
  } else {
    startAPMode();
  }
  
  // Webserver-Routen definieren
  setupWebServer();
  
  // MQTT initialisieren, wenn konfiguriert
  if (strlen(config.mqttServer) > 0) {
    setupMQTT();
  }
  
  // OTA initialisieren
  setupOTA();
  
  // Datenprotokollierung als separaten Task starten
  xTaskCreatePinnedToCore(
    dataLoggingTaskFunction,   // Task-Funktion
    "DataLogging",             // Name
    10000,                     // Stack-Größe
    NULL,                      // Parameter
    1,                         // Priorität
    &dataLoggingTask,          // Task-Handle
    0                          // Core
  );
  
  Serial.println("Setup abgeschlossen");
}

// ----- LOOP -----
void loop() {
  // DNS-Server für Captive Portal
  if (config.apMode) {
    dnsServer.processNextRequest();
  }
  
  // Webserver-Anfragen bearbeiten
  server.handleClient();
  
  // OTA-Updates prüfen
  ArduinoOTA.handle();
  
  // MQTT bearbeiten, wenn verbunden
  if (mqttConnected) {
    mqttClient.loop();
    
    // MQTT-Verbindung wiederherstellen, wenn unterbrochen
    if (!mqttClient.connected()) {
      reconnectMQTT();
    }
  }
  
  // WLAN-Verbindung überprüfen
  if (!config.apMode && WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }
  
  // Sensordaten lesen
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
    readSensors();
    lastSensorRead = millis();
  }
  
  // API aktualisieren
  if (wifiConnected && (millis() - lastApiUpdate >= API_UPDATE_INTERVAL)) {
    updateAPI();
    lastApiUpdate = millis();
  }
  
  // Display aktualisieren
  if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Relais-Steuerung basierend auf der Konfiguration
  controlRelay();
  
  // Kurze Pause zur CPU-Entlastung
  delay(10);
}

// ----- KONFIGURATION -----
// Konfiguration aus EEPROM laden
void loadConfig() {
  Serial.println("Lade Konfiguration...");
  
  if (EEPROM.read(0) == 'S' && EEPROM.read(1) == 'A' && EEPROM.read(2) == 'D') {
    // Konfiguration aus EEPROM lesen
    EEPROM.get(10, config);
    Serial.println("Konfiguration geladen");
  } else {
    // Standardkonfiguration verwenden
    Serial.println("Keine Konfiguration gefunden, verwende Standardwerte");
    
    // Eindeutige Geräte-ID generieren
    uint32_t chipId = (uint32_t)(ESP.getEfuseMac() >> 32);
    sprintf(config.deviceName, "SwissAirDry-%08X", chipId);
    
    // Konfiguration speichern
    saveConfig();
  }
}

// Konfiguration im EEPROM speichern
void saveConfig() {
  Serial.println("Speichere Konfiguration...");
  
  // Signatur schreiben
  EEPROM.write(0, 'S');
  EEPROM.write(1, 'A');
  EEPROM.write(2, 'D');
  
  // Konfiguration speichern
  EEPROM.put(10, config);
  EEPROM.commit();
  
  Serial.println("Konfiguration gespeichert");
}

// ----- NETZWERK -----
// Mit WLAN verbinden
void connectWiFi() {
  Serial.print("Verbinde mit WLAN ");
  Serial.print(config.wifiSSID);
  Serial.println("...");
  
  // Verbindungsstatus auf dem Display anzeigen
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("WLAN-Verbindung");
  tft.setCursor(10, 40);
  tft.print("SSID: ");
  tft.println(config.wifiSSID);
  
  // Mit WLAN verbinden
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config.deviceName);
  WiFi.begin(config.wifiSSID, config.wifiPassword);
  
  // Auf Verbindung warten
  int dots = 0;
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    
    // Fortschritt auf dem Display anzeigen
    tft.setCursor(10, 70);
    tft.print("Verbinde");
    for (int i = 0; i < dots; i++) {
      tft.print(".");
    }
    tft.print("     ");  // Um überschüssige Punkte zu löschen
    dots = (dots + 1) % 6;
    
    timeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    networkIP = WiFi.localIP().toString();
    
    Serial.println();
    Serial.print("Verbunden mit IP: ");
    Serial.println(networkIP);
    
    // Verbindung erfolgreich
    tft.fillRect(0, 70, tft.width(), 30, COLOR_BACKGROUND);
    tft.setCursor(10, 70);
    tft.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
    tft.print("Verbunden!");
    tft.setCursor(10, 100);
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    tft.print("IP: ");
    tft.println(networkIP);
    delay(2000);  // Kurz anzeigen
  } else {
    wifiConnected = false;
    
    Serial.println();
    Serial.println("WLAN-Verbindung fehlgeschlagen");
    
    // Verbindung fehlgeschlagen
    tft.fillRect(0, 70, tft.width(), 30, COLOR_BACKGROUND);
    tft.setCursor(10, 70);
    tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    tft.println("Verbindung fehlgeschlagen!");
    delay(2000);  // Kurz anzeigen
    
    // AP-Modus starten
    startAPMode();
  }
}

// AP-Modus starten
void startAPMode() {
  Serial.println("Starte Access Point...");
  
  // AP-Modus starten
  String apName = String(config.deviceName);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(CONFIG_AP_IP, CONFIG_AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName.c_str(), DEFAULT_AP_PASSWORD);
  
  networkIP = WiFi.softAPIP().toString();
  config.apMode = true;
  configMode = true;
  
  Serial.print("AP gestartet. IP: ");
  Serial.println(networkIP);
  
  // DNS-Server starten, um alle Domains zum ESP umzuleiten (Captive Portal)
  dnsServer.start(DNS_PORT, "*", CONFIG_AP_IP);
  
  // AP-Informationen auf dem Display anzeigen
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("WLAN-Zugangspunkt");
  
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setCursor(10, 40);
  tft.print("Name: ");
  tft.println(apName);
  tft.setCursor(10, 70);
  tft.print("Passwort: ");
  tft.println(DEFAULT_AP_PASSWORD);
  tft.setCursor(10, 100);
  tft.print("IP: ");
  tft.println(networkIP);
  
  // QR-Code für WLAN-Zugang anzeigen
  displayWifiQR(apName, DEFAULT_AP_PASSWORD);
}

// WiFi-Verbindung wiederherstellen
void reconnectWiFi() {
  static unsigned long lastReconnectAttempt = 0;
  
  // Nicht zu oft versuchen
  if (millis() - lastReconnectAttempt < 30000) return;
  
  lastReconnectAttempt = millis();
  
  Serial.println("WLAN-Verbindung verloren, versuche neu zu verbinden...");
  
  // Kurze Statusanzeige
  showToastMessage("WLAN-Verbindung wird wiederhergestellt...", COLOR_WARNING);
  
  // Verbindung neu herstellen
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(config.wifiSSID, config.wifiPassword);
  
  // Kurz warten und Status prüfen
  delay(5000);
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    networkIP = WiFi.localIP().toString();
    showToastMessage("WLAN-Verbindung wiederhergestellt", COLOR_SUCCESS);
  } else {
    wifiConnected = false;
  }
}

// ----- WEBSERVER -----
void setupWebServer() {
  // Webserver-Routen definieren
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/restart", handleRestart);
  server.on("/reset", handleReset);
  server.on("/update", HTTP_GET, handleUpdateForm);
  server.on("/update", HTTP_POST, handleUpdateResult, handleUpdateUpload);
  server.on("/toggle", handleToggle);
  server.on("/data", handleData);
  server.on("/logs", handleLogs);
  server.on("/style.css", handleCSS);
  server.on("/script.js", handleJS);
  
  // Captive Portal - alle nicht definierten Adressen umleiten
  server.onNotFound([]() {
    if (captivePortal()) { return; }
    handleNotFound();
  });
  
  // Webserver starten
  server.begin();
  Serial.println("Webserver gestartet");
}

// Captive Portal Umleitung
bool captivePortal() {
  if (!config.apMode) return false;
  
  // Wenn es sich nicht um eine IP-Adresse handelt, zum Portal umleiten
  if (server.hostHeader() != networkIP) {
    server.sendHeader("Location", String("http://") + networkIP, true);
    server.send(302, "text/plain", "");
    return true;
  }
  return false;
}

// Hauptseite anzeigen
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - " + String(config.deviceName) + "</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>SwissAirDry</h1>";
  html += "<h2>" + String(config.deviceName) + "</h2>";
  
  // Status-Informationen
  html += "<div class='status-card'>";
  html += "<h3>Status</h3>";
  html += "<p>Firmware: v" + firmwareVersion + "</p>";
  html += "<p>Netzwerk: " + (wifiConnected ? "Verbunden mit " + String(config.wifiSSID) : "AP-Modus") + "</p>";
  html += "<p>IP: " + networkIP + "</p>";
  html += "<p>MQTT: " + String(mqttConnected ? "Verbunden" : "Nicht verbunden") + "</p>";
  html += "<p>SD-Karte: " + String(sdCardAvailable ? "Vorhanden" : "Nicht gefunden") + "</p>";
  html += "</div>";
  
  // Aktuelle Sensordaten
  html += "<div class='data-card'>";
  html += "<h3>Sensordaten</h3>";
  if (sensorData.dataValid) {
    html += "<p>Temperatur: " + String(sensorData.temperature, 1) + " °C</p>";
    html += "<p>Luftfeuchtigkeit: " + String(sensorData.humidity, 1) + " %</p>";
    html += "<p>Leistung: " + String(sensorData.power, 1) + " W</p>";
    html += "<p>Energieverbrauch: " + String(sensorData.energy, 2) + " kWh</p>";
    html += "<p>Laufzeit: " + formatRuntime(sensorData.totalRuntime) + "</p>";
  } else {
    html += "<p>Keine Sensordaten verfügbar</p>";
  }
  html += "</div>";
  
  // Relais-Steuerung
  html += "<div class='control-card'>";
  html += "<h3>Relais-Steuerung</h3>";
  html += "<p>Status: <span class='" + String(relayState ? "on" : "off") + "'>" + 
          String(relayState ? "Eingeschaltet" : "Ausgeschaltet") + "</span></p>";
  html += "<p>Modus: " + getRelayModeText(config.relayMode) + "</p>";
  
  if (config.relayEnabled) {
    html += "<a href='/toggle' class='button " + String(relayState ? "off" : "on") + "-button'>" + 
            String(relayState ? "Ausschalten" : "Einschalten") + "</a>";
  } else {
    html += "<p>Relais-Steuerung deaktiviert</p>";
  }
  html += "</div>";
  
  // Menü
  html += "<div class='menu'>";
  html += "<a href='/config' class='button'>Einstellungen</a>";
  html += "<a href='/data' class='button'>Datenvisualisierung</a>";
  if (sdCardAvailable) {
    html += "<a href='/logs' class='button'>Logs</a>";
  }
  html += "<a href='/update' class='button'>Update</a>";
  html += "</div>";
  
  html += "</div>";
  html += "<script src='/script.js'></script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Konfigurationsseite anzeigen
void handleConfig() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Konfiguration</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>SwissAirDry - Konfiguration</h1>";
  
  html += "<form action='/save' method='post'>";
  
  // Allgemeine Einstellungen
  html += "<div class='config-section'>";
  html += "<h3>Allgemein</h3>";
  html += "<div class='form-group'>";
  html += "<label for='deviceName'>Gerätename:</label>";
  html += "<input type='text' id='deviceName' name='deviceName' value='" + String(config.deviceName) + "' maxlength='31'>";
  html += "</div>";
  html += "</div>";
  
  // WLAN-Einstellungen
  html += "<div class='config-section'>";
  html += "<h3>WLAN</h3>";
  html += "<div class='form-group'>";
  html += "<label for='wifiSSID'>WLAN-Name (SSID):</label>";
  html += "<input type='text' id='wifiSSID' name='wifiSSID' value='" + String(config.wifiSSID) + "' maxlength='32'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='wifiPassword'>WLAN-Passwort:</label>";
  html += "<input type='password' id='wifiPassword' name='wifiPassword' value='" + String(config.wifiPassword) + "' maxlength='64'>";
  html += "</div>";
  html += "<div class='form-check'>";
  html += "<input type='checkbox' id='apMode' name='apMode' " + String(config.apMode ? "checked" : "") + ">";
  html += "<label for='apMode'>Access Point Modus aktivieren</label>";
  html += "</div>";
  html += "</div>";
  
  // MQTT-Einstellungen
  html += "<div class='config-section'>";
  html += "<h3>MQTT</h3>";
  html += "<div class='form-group'>";
  html += "<label for='mqttServer'>MQTT-Server:</label>";
  html += "<input type='text' id='mqttServer' name='mqttServer' value='" + String(config.mqttServer) + "' maxlength='64'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='mqttPort'>MQTT-Port:</label>";
  html += "<input type='number' id='mqttPort' name='mqttPort' value='" + String(config.mqttPort) + "' min='1' max='65535'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='mqttUser'>MQTT-Benutzername:</label>";
  html += "<input type='text' id='mqttUser' name='mqttUser' value='" + String(config.mqttUser) + "' maxlength='32'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='mqttPassword'>MQTT-Passwort:</label>";
  html += "<input type='password' id='mqttPassword' name='mqttPassword' value='" + String(config.mqttPassword) + "' maxlength='64'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='mqttTopic'>MQTT-Basistopic:</label>";
  html += "<input type='text' id='mqttTopic' name='mqttTopic' value='" + String(config.mqttTopic) + "' maxlength='64'>";
  html += "</div>";
  html += "</div>";
  
  // API-Einstellungen
  html += "<div class='config-section'>";
  html += "<h3>API</h3>";
  html += "<div class='form-group'>";
  html += "<label for='apiServer'>API-Server:</label>";
  html += "<input type='text' id='apiServer' name='apiServer' value='" + String(config.apiServer) + "' maxlength='64'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='apiKey'>API-Schlüssel:</label>";
  html += "<input type='text' id='apiKey' name='apiKey' value='" + String(config.apiKey) + "' maxlength='64'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='apiUpdateInterval'>Update-Intervall (Sekunden):</label>";
  html += "<input type='number' id='apiUpdateInterval' name='apiUpdateInterval' value='" + String(config.apiUpdateInterval) + "' min='10' max='3600'>";
  html += "</div>";
  html += "</div>";
  
  // Relais-Einstellungen
  html += "<div class='config-section'>";
  html += "<h3>Relais</h3>";
  html += "<div class='form-check'>";
  html += "<input type='checkbox' id='relayEnabled' name='relayEnabled' " + String(config.relayEnabled ? "checked" : "") + ">";
  html += "<label for='relayEnabled'>Relais aktivieren</label>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='relayMode'>Steuerungsmodus:</label>";
  html += "<select id='relayMode' name='relayMode'>";
  html += "<option value='0' " + String(config.relayMode == 0 ? "selected" : "") + ">Manuell</option>";
  html += "<option value='1' " + String(config.relayMode == 1 ? "selected" : "") + ">Zeitgesteuert</option>";
  html += "<option value='2' " + String(config.relayMode == 2 ? "selected" : "") + ">Sensorgesteuert</option>";
  html += "</select>";
  html += "</div>";
  
  // Zeiteinstellungen für Relais
  html += "<div id='timeSettings' " + String(config.relayMode != 1 ? "style='display:none;'" : "") + ">";
  html += "<div class='form-group'>";
  html += "<label for='relayOnHour'>Einschaltzeit:</label>";
  html += "<input type='number' id='relayOnHour' name='relayOnHour' value='" + String(config.relayOnHour) + "' min='0' max='23' style='width:50px;'> : ";
  html += "<input type='number' id='relayOnMinute' name='relayOnMinute' value='" + String(config.relayOnMinute) + "' min='0' max='59' style='width:50px;'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='relayOffHour'>Ausschaltzeit:</label>";
  html += "<input type='number' id='relayOffHour' name='relayOffHour' value='" + String(config.relayOffHour) + "' min='0' max='23' style='width:50px;'> : ";
  html += "<input type='number' id='relayOffMinute' name='relayOffMinute' value='" + String(config.relayOffMinute) + "' min='0' max='59' style='width:50px;'>";
  html += "</div>";
  html += "</div>";
  
  // Sensoreinstellungen für Relais
  html += "<div id='sensorSettings' " + String(config.relayMode != 2 ? "style='display:none;'" : "") + ">";
  html += "<div class='form-group'>";
  html += "<label for='relayThreshold'>Schwellenwert (%):</label>";
  html += "<input type='number' id='relayThreshold' name='relayThreshold' value='" + String(config.relayThreshold) + "' min='0' max='100' step='0.1'>";
  html += "</div>";
  html += "</div>";
  html += "</div>";
  
  // Sensoreinstellungen
  html += "<div class='config-section'>";
  html += "<h3>Sensoren</h3>";
  html += "<div class='form-check'>";
  html += "<input type='checkbox' id='temperatureSensor' name='temperatureSensor' " + String(config.temperatureSensor ? "checked" : "") + ">";
  html += "<label for='temperatureSensor'>Temperatursensor</label>";
  html += "</div>";
  html += "<div class='form-check'>";
  html += "<input type='checkbox' id='humiditySensor' name='humiditySensor' " + String(config.humiditySensor ? "checked" : "") + ">";
  html += "<label for='humiditySensor'>Feuchtigkeitssensor</label>";
  html += "</div>";
  html += "<div class='form-check'>";
  html += "<input type='checkbox' id='energySensor' name='energySensor' " + String(config.energySensor ? "checked" : "") + ">";
  html += "<label for='energySensor'>Energiesensor</label>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='sensorReadInterval'>Leseintervall (Sekunden):</label>";
  html += "<input type='number' id='sensorReadInterval' name='sensorReadInterval' value='" + String(config.sensorReadInterval) + "' min='1' max='600'>";
  html += "</div>";
  html += "</div>";
  
  // Logging-Einstellungen
  html += "<div class='config-section'>";
  html += "<h3>Datenprotokollierung</h3>";
  html += "<div class='form-check'>";
  html += "<input type='checkbox' id='sdLogging' name='sdLogging' " + String(config.sdLogging ? "checked" : "") + " " + String(sdCardAvailable ? "" : "disabled") + ">";
  html += "<label for='sdLogging'>SD-Kartenprotokollierung " + String(sdCardAvailable ? "" : "(SD-Karte nicht verfügbar)") + "</label>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='logInterval'>Protokollintervall (Sekunden):</label>";
  html += "<input type='number' id='logInterval' name='logInterval' value='" + String(config.logInterval) + "' min='10' max='3600'>";
  html += "</div>";
  html += "</div>";
  
  // Buttons
  html += "<div class='button-group'>";
  html += "<button type='submit' class='button'>Speichern</button>";
  html += "<a href='/' class='button secondary-button'>Abbrechen</a>";
  html += "<a href='/restart' class='button warning-button'>Neustart</a>";
  html += "<a href='/reset' class='button danger-button'>Zurücksetzen</a>";
  html += "</div>";
  
  html += "</form>";
  html += "</div>";
  
  // JavaScript für dynamische Formularelemente
  html += "<script>";
  html += "document.getElementById('relayMode').addEventListener('change', function() {";
  html += "  const timeSettings = document.getElementById('timeSettings');";
  html += "  const sensorSettings = document.getElementById('sensorSettings');";
  html += "  timeSettings.style.display = this.value == '1' ? 'block' : 'none';";
  html += "  sensorSettings.style.display = this.value == '2' ? 'block' : 'none';";
  html += "});";
  html += "</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Konfiguration speichern
void handleSave() {
  // Gerätename
  if (server.hasArg("deviceName")) {
    strncpy(config.deviceName, server.arg("deviceName").c_str(), sizeof(config.deviceName) - 1);
  }
  
  // WLAN-Einstellungen
  if (server.hasArg("wifiSSID")) {
    strncpy(config.wifiSSID, server.arg("wifiSSID").c_str(), sizeof(config.wifiSSID) - 1);
  }
  
  if (server.hasArg("wifiPassword")) {
    strncpy(config.wifiPassword, server.arg("wifiPassword").c_str(), sizeof(config.wifiPassword) - 1);
  }
  
  config.apMode = server.hasArg("apMode");
  
  // MQTT-Einstellungen
  if (server.hasArg("mqttServer")) {
    strncpy(config.mqttServer, server.arg("mqttServer").c_str(), sizeof(config.mqttServer) - 1);
  }
  
  if (server.hasArg("mqttPort")) {
    config.mqttPort = server.arg("mqttPort").toInt();
  }
  
  if (server.hasArg("mqttUser")) {
    strncpy(config.mqttUser, server.arg("mqttUser").c_str(), sizeof(config.mqttUser) - 1);
  }
  
  if (server.hasArg("mqttPassword")) {
    strncpy(config.mqttPassword, server.arg("mqttPassword").c_str(), sizeof(config.mqttPassword) - 1);
  }
  
  if (server.hasArg("mqttTopic")) {
    strncpy(config.mqttTopic, server.arg("mqttTopic").c_str(), sizeof(config.mqttTopic) - 1);
  }
  
  // API-Einstellungen
  if (server.hasArg("apiServer")) {
    strncpy(config.apiServer, server.arg("apiServer").c_str(), sizeof(config.apiServer) - 1);
  }
  
  if (server.hasArg("apiKey")) {
    strncpy(config.apiKey, server.arg("apiKey").c_str(), sizeof(config.apiKey) - 1);
  }
  
  if (server.hasArg("apiUpdateInterval")) {
    config.apiUpdateInterval = server.arg("apiUpdateInterval").toInt();
  }
  
  // Relais-Einstellungen
  config.relayEnabled = server.hasArg("relayEnabled");
  
  if (server.hasArg("relayMode")) {
    config.relayMode = server.arg("relayMode").toInt();
  }
  
  if (server.hasArg("relayOnHour")) {
    config.relayOnHour = server.arg("relayOnHour").toInt();
  }
  
  if (server.hasArg("relayOnMinute")) {
    config.relayOnMinute = server.arg("relayOnMinute").toInt();
  }
  
  if (server.hasArg("relayOffHour")) {
    config.relayOffHour = server.arg("relayOffHour").toInt();
  }
  
  if (server.hasArg("relayOffMinute")) {
    config.relayOffMinute = server.arg("relayOffMinute").toInt();
  }
  
  if (server.hasArg("relayThreshold")) {
    config.relayThreshold = server.arg("relayThreshold").toFloat();
  }
  
  // Sensoreinstellungen
  config.temperatureSensor = server.hasArg("temperatureSensor");
  config.humiditySensor = server.hasArg("humiditySensor");
  config.energySensor = server.hasArg("energySensor");
  
  if (server.hasArg("sensorReadInterval")) {
    config.sensorReadInterval = server.arg("sensorReadInterval").toInt();
  }
  
  // Logging-Einstellungen
  config.sdLogging = server.hasArg("sdLogging");
  
  if (server.hasArg("logInterval")) {
    config.logInterval = server.arg("logInterval").toInt();
  }
  
  // Konfiguration speichern
  saveConfig();
  
  // Bestätigungsseite anzeigen
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Konfiguration gespeichert</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "<meta http-equiv='refresh' content='3;url=/'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Konfiguration gespeichert</h1>";
  html += "<p>Die Einstellungen wurden erfolgreich gespeichert.</p>";
  html += "<p>Sie werden in 3 Sekunden zurück zur Startseite geleitet...</p>";
  html += "<a href='/' class='button'>Zurück zur Startseite</a>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  
  // Wenn WLAN-Einstellungen geändert wurden, neu verbinden
  if (!config.apMode && strlen(config.wifiSSID) > 0) {
    WiFi.disconnect();
    delay(1000);
    connectWiFi();
  }
  
  // MQTT neu konfigurieren, wenn erforderlich
  if (strlen(config.mqttServer) > 0) {
    setupMQTT();
  }
}

// Neustart
void handleRestart() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Neustart</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "<meta http-equiv='refresh' content='5;url=/'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Gerät wird neu gestartet</h1>";
  html += "<p>Das Gerät wird jetzt neu gestartet. Bitte warten Sie einen Moment...</p>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

// Zurücksetzen auf Werkseinstellungen
void handleReset() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Zurücksetzen</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "<meta http-equiv='refresh' content='5;url=/'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Gerät wird zurückgesetzt</h1>";
  html += "<p>Das Gerät wird auf Werkseinstellungen zurückgesetzt und neu gestartet. Bitte warten Sie einen Moment...</p>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  delay(1000);
  
  // EEPROM löschen
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  
  ESP.restart();
}

// Update-Formular anzeigen
void handleUpdateForm() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Firmware-Update</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Firmware-Update</h1>";
  html += "<p>Aktuelle Version: " + firmwareVersion + "</p>";
  html += "<p>Wählen Sie eine Firmware-Datei (BIN) zum Hochladen aus:</p>";
  html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
  html += "<div class='form-group'>";
  html += "<input type='file' name='update' accept='.bin'>";
  html += "</div>";
  html += "<button type='submit' class='button'>Update starten</button>";
  html += "<a href='/' class='button secondary-button'>Zurück</a>";
  html += "</form>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Update-Ergebnis anzeigen
void handleUpdateResult() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Update-Ergebnis</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  
  if (Update.hasError()) {
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Update fehlgeschlagen</h1>";
    html += "<p>Beim Update ist ein Fehler aufgetreten. Bitte versuchen Sie es erneut.</p>";
    html += "<a href='/update' class='button'>Erneut versuchen</a>";
    html += "<a href='/' class='button secondary-button'>Zurück zur Startseite</a>";
    html += "</div>";
    html += "</body></html>";
  } else {
    html += "<meta http-equiv='refresh' content='10;url=/'>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Update erfolgreich</h1>";
    html += "<p>Die Firmware wurde erfolgreich aktualisiert. Das Gerät wird neu gestartet...</p>";
    html += "<p>Sie werden in 10 Sekunden zur Startseite weitergeleitet.</p>";
    html += "</div>";
    html += "</body></html>";
  }
  
  server.send(200, "text/html", html);
  delay(1000);
  
  if (!Update.hasError()) {
    ESP.restart();
  }
}

// Update-Upload bearbeiten
void handleUpdateUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    
    // Anzeige aktualisieren
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Firmware-Update");
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    tft.println("Update wird gestartet...");
    
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Fortschritt anzeigen
    int progress = (upload.totalSize * 100) / (upload.contentLength);
    
    tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    tft.printf("Upload: %u%%", progress);
    
    tft.fillRect(10, 70, tft.width() - 20, 20, COLOR_BACKGROUND);
    tft.drawRect(10, 70, tft.width() - 20, 20, COLOR_TEXT);
    tft.fillRect(12, 72, (tft.width() - 24) * progress / 100, 16, COLOR_INFO);
    
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Update erfolgreich: %u Bytes\n", upload.totalSize);
      tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
      tft.setCursor(10, 40);
      tft.println("Update erfolgreich!");
      tft.setCursor(10, 100);
      tft.println("Gerät startet neu...");
    } else {
      Update.printError(Serial);
      tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
      tft.setCursor(10, 40);
      tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
      tft.println("Update fehlgeschlagen!");
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    Serial.println("Update abgebrochen");
    tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    tft.setTextColor(COLOR_WARNING, COLOR_BACKGROUND);
    tft.println("Update abgebrochen!");
  }
  
  yield();
}

// Nicht gefunden
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// Relais umschalten
void handleToggle() {
  if (config.relayEnabled && config.relayMode == 0) {  // Nur im manuellen Modus
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    
    // Status in Log speichern
    logEvent("Relais " + String(relayState ? "eingeschaltet" : "ausgeschaltet") + " (manuell)");
  }
  
  // Zurück zur Hauptseite
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Sensordaten als JSON zurückgeben
void handleData() {
  DynamicJsonDocument doc(512);
  
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["power"] = sensorData.power;
  doc["energy"] = sensorData.energy;
  doc["totalRuntime"] = sensorData.totalRuntime;
  doc["currentRuntime"] = sensorData.currentRuntime;
  doc["relayState"] = relayState;
  doc["timestamp"] = millis() / 1000;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  server.send(200, "application/json", jsonString);
}

// Log-Dateien anzeigen
void handleLogs() {
  if (!sdCardAvailable) {
    server.send(503, "text/html", "<html><body><h1>SD-Karte nicht verfügbar</h1><p><a href='/'>Zurück</a></p></body></html>");
    return;
  }
  
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>SwissAirDry - Logs</title>";
  html += "<link rel='stylesheet' href='/style.css'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Logs</h1>";
  
  // Verfügbare Log-Dateien auflisten
  File root = SD.open("/logs");
  
  if (!root) {
    html += "<p>Keine Logs gefunden</p>";
  } else {
    if (!root.isDirectory()) {
      html += "<p>Fehler: /logs ist keine Verzeichnis</p>";
    } else {
      File file = root.openNextFile();
      
      if (!file) {
        html += "<p>Keine Log-Dateien gefunden</p>";
      } else {
        html += "<table class='log-table'>";
        html += "<tr><th>Dateiname</th><th>Größe</th><th>Aktion</th></tr>";
        
        while (file) {
          if (!file.isDirectory()) {
            String fileName = file.name();
            
            html += "<tr>";
            html += "<td>" + fileName + "</td>";
            html += "<td>" + formatFileSize(file.size()) + "</td>";
            html += "<td><a href='/logs/" + fileName + "' class='button small-button'>Anzeigen</a></td>";
            html += "</tr>";
          }
          
          file = root.openNextFile();
        }
        
        html += "</table>";
      }
    }
    
    root.close();
  }
  
  html += "<a href='/' class='button'>Zurück</a>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// CSS für die Weboberfläche
void handleCSS() {
  String css = "* {box-sizing: border-box; margin: 0; padding: 0;}";
  css += "body {font-family: Arial, sans-serif; background-color: #f5f5f5; color: #333; line-height: 1.6;}";
  css += ".container {max-width: 800px; margin: 0 auto; padding: 20px;}";
  css += "h1, h2, h3 {margin-bottom: 15px; color: #0066cc;}";
  css += "p {margin-bottom: 10px;}";
  
  css += ".status-card, .data-card, .control-card, .config-section {";
  css += "background-color: white; border-radius: 5px; padding: 15px; margin-bottom: 20px;";
  css += "box-shadow: 0 2px 5px rgba(0,0,0,0.1);}";
  
  css += ".menu {display: flex; flex-wrap: wrap; gap: 10px; margin: 20px 0;}";
  
  css += ".button {";
  css += "display: inline-block; background-color: #0066cc; color: white; padding: 10px 15px;";
  css += "border-radius: 5px; text-decoration: none; border: none; cursor: pointer; font-size: 16px;";
  css += "text-align: center; transition: background-color 0.3s;}";
  
  css += ".button:hover {background-color: #0055aa;}";
  css += ".secondary-button {background-color: #666;}";
  css += ".warning-button {background-color: #f39c12;}";
  css += ".danger-button {background-color: #e74c3c;}";
  css += ".small-button {padding: 5px 10px; font-size: 14px;}";
  
  css += ".button-group {display: flex; gap: 10px; flex-wrap: wrap; margin-top: 20px;}";
  
  css += ".on {color: #4CAF50; font-weight: bold;}";
  css += ".off {color: #f44336; font-weight: bold;}";
  css += ".on-button {background-color: #4CAF50;}";
  css += ".off-button {background-color: #f44336;}";
  
  css += ".form-group {margin-bottom: 15px;}";
  css += "label {display: block; margin-bottom: 5px; font-weight: bold;}";
  css += "input[type='text'], input[type='password'], input[type='number'], select {";
  css += "width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; font-size: 16px;}";
  
  css += ".form-check {margin-bottom: 15px;}";
  css += ".form-check input {margin-right: 10px;}";
  
  css += ".log-table {width: 100%; border-collapse: collapse; margin-bottom: 20px;}";
  css += ".log-table th, .log-table td {border: 1px solid #ddd; padding: 8px; text-align: left;}";
  css += ".log-table th {background-color: #0066cc; color: white;}";
  
  css += "@media (max-width: 600px) {";
  css += "  .button-group {flex-direction: column;}";
  css += "  .button {width: 100%;}";
  css += "}";
  
  server.send(200, "text/css", css);
}

// JavaScript für die Weboberfläche
void handleJS() {
  String js = "// Automatische Aktualisierung der Sensordaten";
  js += "function updateSensorData() {";
  js += "  fetch('/data')";
  js += "    .then(response => response.json())";
  js += "    .then(data => {";
  js += "      // Hier die Daten aktualisieren";
  js += "    })";
  js += "    .catch(error => console.error('Fehler:', error));";
  js += "}";
  
  js += "// Aktuelle Seite alle 30 Sekunden neu laden";
  js += "setTimeout(function() {";
  js += "  window.location.reload();";
  js += "}, 30000);";
  
  server.send(200, "application/javascript", js);
}

// ----- MQTT -----
void setupMQTT() {
  if (strlen(config.mqttServer) == 0) {
    mqttConnected = false;
    return;
  }
  
  mqttClient.setServer(config.mqttServer, config.mqttPort);
  mqttClient.setCallback(mqttCallback);
  
  reconnectMQTT();
}

void reconnectMQTT() {
  // Nur versuchen, wenn WLAN verbunden ist
  if (!wifiConnected) {
    mqttConnected = false;
    return;
  }
  
  Serial.print("Verbinde mit MQTT-Server ");
  Serial.print(config.mqttServer);
  Serial.print(":");
  Serial.print(config.mqttPort);
  Serial.println("...");
  
  // Eindeutige Client-ID generieren
  String clientId = String(config.deviceName) + "-" + String(random(0xffff), HEX);
  
  // Verbindung herstellen
  if (mqttClient.connect(clientId.c_str(), config.mqttUser, config.mqttPassword)) {
    Serial.println("MQTT verbunden");
    mqttConnected = true;
    
    // Topics abonnieren
    String topic = String(config.mqttTopic) + config.deviceName + "/cmd/#";
    mqttClient.subscribe(topic.c_str());
    
    // Online-Status senden
    topic = String(config.mqttTopic) + config.deviceName + "/status";
    mqttClient.publish(topic.c_str(), "online", true);
    
    // Konfiguration senden
    sendMQTTConfig();
  } else {
    Serial.print("MQTT-Verbindung fehlgeschlagen, rc=");
    Serial.println(mqttClient.state());
    mqttConnected = false;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Empfangene Nachricht verarbeiten
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  String topicStr = String(topic);
  String messageStr = String(message);
  
  Serial.print("MQTT-Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageStr);
  
  // Topic parsen, um den Befehl zu extrahieren
  String baseTopic = String(config.mqttTopic) + config.deviceName + "/cmd/";
  
  if (topicStr.startsWith(baseTopic)) {
    String command = topicStr.substring(baseTopic.length());
    
    if (command == "relay") {
      // Relais steuern
      if (messageStr == "ON" || messageStr == "on" || messageStr == "1") {
        if (config.relayEnabled) {
          relayState = true;
          digitalWrite(RELAY_PIN, HIGH);
          logEvent("Relais eingeschaltet (MQTT)");
        }
      } else if (messageStr == "OFF" || messageStr == "off" || messageStr == "0") {
        if (config.relayEnabled) {
          relayState = false;
          digitalWrite(RELAY_PIN, LOW);
          logEvent("Relais ausgeschaltet (MQTT)");
        }
      }
      
      // Status zurückmelden
      String statusTopic = String(config.mqttTopic) + config.deviceName + "/state/relay";
      mqttClient.publish(statusTopic.c_str(), relayState ? "ON" : "OFF", true);
    } 
    else if (command == "restart") {
      // Gerät neu starten
      logEvent("Neustart über MQTT angefordert");
      delay(500);
      ESP.restart();
    }
    else if (command == "config") {
      // Konfiguration senden
      sendMQTTConfig();
    }
  }
}

void sendMQTTConfig() {
  if (!mqttConnected) return;
  
  // Konfiguration als JSON senden
  DynamicJsonDocument doc(1024);
  doc["deviceName"] = config.deviceName;
  doc["version"] = config.version;
  doc["relayEnabled"] = config.relayEnabled;
  doc["relayMode"] = config.relayMode;
  doc["temperatureSensor"] = config.temperatureSensor;
  doc["humiditySensor"] = config.humiditySensor;
  doc["energySensor"] = config.energySensor;
  
  String configJson;
  serializeJson(doc, configJson);
  
  String topic = String(config.mqttTopic) + config.deviceName + "/config";
  mqttClient.publish(topic.c_str(), configJson.c_str(), true);
}

void publishSensorData() {
  if (!mqttConnected || !sensorData.dataValid) return;
  
  String baseTopic = String(config.mqttTopic) + config.deviceName;
  
  // Temperatur
  if (config.temperatureSensor) {
    String topic = baseTopic + "/sensor/temperature";
    mqttClient.publish(topic.c_str(), String(sensorData.temperature, 1).c_str(), true);
  }
  
  // Luftfeuchtigkeit
  if (config.humiditySensor) {
    String topic = baseTopic + "/sensor/humidity";
    mqttClient.publish(topic.c_str(), String(sensorData.humidity, 1).c_str(), true);
  }
  
  // Energie
  if (config.energySensor) {
    String topic = baseTopic + "/sensor/power";
    mqttClient.publish(topic.c_str(), String(sensorData.power, 1).c_str(), true);
    
    topic = baseTopic + "/sensor/energy";
    mqttClient.publish(topic.c_str(), String(sensorData.energy, 2).c_str(), true);
  }
  
  // Laufzeit
  String topic = baseTopic + "/sensor/runtime";
  mqttClient.publish(topic.c_str(), String(sensorData.totalRuntime).c_str(), true);
  
  // Relais-Status
  topic = baseTopic + "/state/relay";
  mqttClient.publish(topic.c_str(), relayState ? "ON" : "OFF", true);
}

// ----- OTA -----
void setupOTA() {
  ArduinoOTA.setHostname(config.deviceName);
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else {
      type = "Dateisystem";
    }
    
    Serial.println("OTA: Start " + type);
    
    // Anzeige aktualisieren
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("OTA Update");
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    tft.println("Update wird gestartet...");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA: Ende");
    
    tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    tft.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
    tft.println("Update abgeschlossen!");
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    tft.setCursor(10, 70);
    tft.println("Gerät startet neu...");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    
    Serial.printf("OTA: %u%%\r", percent);
    
    tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    tft.printf("Fortschritt: %u%%", percent);
    
    tft.fillRect(10, 70, tft.width() - 20, 20, COLOR_BACKGROUND);
    tft.drawRect(10, 70, tft.width() - 20, 20, COLOR_TEXT);
    tft.fillRect(12, 72, (tft.width() - 24) * percent / 100, 16, COLOR_INFO);
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Fehler[%u]: ", error);
    
    tft.fillRect(0, 40, tft.width(), 20, COLOR_BACKGROUND);
    tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    tft.setCursor(10, 40);
    
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth fehlgeschlagen");
      tft.println("Auth fehlgeschlagen");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin fehlgeschlagen");
      tft.println("Begin fehlgeschlagen");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect fehlgeschlagen");
      tft.println("Connect fehlgeschlagen");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive fehlgeschlagen");
      tft.println("Receive fehlgeschlagen");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End fehlgeschlagen");
      tft.println("End fehlgeschlagen");
    }
  });
  
  ArduinoOTA.begin();
}

// ----- DISPLAY -----
void showSplashScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Logo oder Titel
  tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
  tft.setTextSize(3);
  tft.setCursor(10, 50);
  tft.println("SwissAirDry");
  
  // Version
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setTextSize(1);
  tft.setCursor(10, 90);
  tft.println("Version " + firmwareVersion);
  
  // Startet...
  tft.setCursor(10, 110);
  tft.println("Startet...");
  
  delay(2000);
}

void updateDisplay() {
  // Aktuelle Seite anzeigen
  switch (displayPage) {
    case 0:
      showMainScreen();
      break;
    case 1:
      showDetailScreen();
      break;
    case 2:
      showNetworkScreen();
      break;
    default:
      displayPage = 0;
      showMainScreen();
  }
}

void showMainScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Statusleiste
  drawStatusBar();
  
  // Titel
  tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(10, 25);
  tft.println("SwissAirDry");
  
  // Gerätename
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setTextSize(1);
  tft.setCursor(10, 45);
  tft.println(config.deviceName);
  
  // Hauptinfo
  if (sensorData.dataValid) {
    // Temperatur und Luftfeuchtigkeit
    if (config.temperatureSensor || config.humiditySensor) {
      tft.setCursor(10, 70);
      tft.setTextSize(2);
      
      if (config.temperatureSensor) {
        tft.printf("%.1f °C", sensorData.temperature);
      }
      
      if (config.temperatureSensor && config.humiditySensor) {
        tft.print(" / ");
      }
      
      if (config.humiditySensor) {
        tft.printf("%.1f %%", sensorData.humidity);
      }
    }
    
    // Energiedaten, wenn vorhanden
    if (config.energySensor) {
      tft.setTextSize(1);
      tft.setCursor(10, 100);
      tft.print("Leistung: ");
      tft.printf("%.1f W", sensorData.power);
      
      tft.setCursor(10, 115);
      tft.print("Energie: ");
      tft.printf("%.2f kWh", sensorData.energy);
    }
  } else {
    tft.setTextSize(1);
    tft.setCursor(10, 70);
    tft.println("Keine Sensordaten verfügbar");
  }
  
  // Relais-Status
  tft.setCursor(10, 140);
  tft.print("Relais: ");
  
  if (config.relayEnabled) {
    if (relayState) {
      tft.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
      tft.println("EIN");
    } else {
      tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
      tft.println("AUS");
    }
  } else {
    tft.println("Deaktiviert");
  }
  
  // Laufzeit
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setCursor(10, 155);
  tft.print("Laufzeit: ");
  tft.println(formatRuntime(sensorData.totalRuntime));
  
  // QR-Code für Webzugriff anzeigen
  if (wifiConnected) {
    String url = "http://" + networkIP + "/";
    displayQRCode(url, 100, 120, 2);
  }
  
  // Seitenwechsel-Hinweis
  tft.setTextColor(COLOR_INFO, COLOR_BACKGROUND);
  tft.setCursor(10, tft.height() - 20);
  tft.println("Drücken Sie die Taste für Details");
}

void showDetailScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Statusleiste
  drawStatusBar();
  
  // Titel
  tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(10, 25);
  tft.println("Sensordaten");
  
  // Sensordaten
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setTextSize(1);
  
  int y = 45;
  
  if (config.temperatureSensor) {
    tft.setCursor(10, y);
    tft.print("Temperatur: ");
    tft.printf("%.1f °C", sensorData.temperature);
    y += 15;
  }
  
  if (config.humiditySensor) {
    tft.setCursor(10, y);
    tft.print("Luftfeuchtigkeit: ");
    tft.printf("%.1f %%", sensorData.humidity);
    y += 15;
  }
  
  if (config.energySensor) {
    tft.setCursor(10, y);
    tft.print("Leistung: ");
    tft.printf("%.1f W", sensorData.power);
    y += 15;
    
    tft.setCursor(10, y);
    tft.print("Energieverbrauch: ");
    tft.printf("%.2f kWh", sensorData.energy);
    y += 15;
  }
  
  // Laufzeiten
  tft.setCursor(10, y += 10);
  tft.print("Gesamtlaufzeit: ");
  tft.println(formatRuntime(sensorData.totalRuntime));
  
  tft.setCursor(10, y += 15);
  tft.print("Aktuelle Laufzeit: ");
  tft.println(formatRuntime(sensorData.currentRuntime));
  
  // Relais-Information
  tft.setCursor(10, y += 25);
  tft.print("Relais-Modus: ");
  tft.println(getRelayModeText(config.relayMode));
  
  tft.setCursor(10, y += 15);
  tft.print("Relais-Status: ");
  
  if (config.relayEnabled) {
    if (relayState) {
      tft.setTextColor(COLOR_SUCCESS, COLOR_BACKGROUND);
      tft.println("EIN");
    } else {
      tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
      tft.println("AUS");
    }
  } else {
    tft.println("Deaktiviert");
  }
  
  // Seitenwechsel-Hinweis
  tft.setTextColor(COLOR_INFO, COLOR_BACKGROUND);
  tft.setCursor(10, tft.height() - 20);
  tft.println("Drücken Sie die Taste für Netzwerk");
}

void showNetworkScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Statusleiste
  drawStatusBar();
  
  // Titel
  tft.setTextColor(COLOR_TITLE, COLOR_BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(10, 25);
  tft.println("Netzwerk");
  
  // Netzwerkinformationen
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.setTextSize(1);
  
  int y = 45;
  
  tft.setCursor(10, y);
  tft.print("WLAN: ");
  if (wifiConnected) {
    tft.println(config.wifiSSID);
    
    tft.setCursor(10, y += 15);
    tft.print("IP: ");
    tft.println(networkIP);
    
    tft.setCursor(10, y += 15);
    tft.print("Signal: ");
    tft.printf("%d dBm", WiFi.RSSI());
  } else if (config.apMode) {
    tft.println("Access Point aktiv");
    
    tft.setCursor(10, y += 15);
    tft.print("AP SSID: ");
    tft.println(config.deviceName);
    
    tft.setCursor(10, y += 15);
    tft.print("IP: ");
    tft.println(networkIP);
  } else {
    tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    tft.println("Nicht verbunden");
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  }
  
  // MQTT-Status
  tft.setCursor(10, y += 25);
  tft.print("MQTT: ");
  if (mqttConnected) {
    tft.println("Verbunden");
    
    tft.setCursor(10, y += 15);
    tft.print("Server: ");
    tft.println(config.mqttServer);
    
    tft.setCursor(10, y += 15);
    tft.print("Topic: ");
    tft.println(config.mqttTopic);
  } else if (strlen(config.mqttServer) > 0) {
    tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    tft.println("Nicht verbunden");
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  } else {
    tft.println("Nicht konfiguriert");
  }
  
  // API-Status
  tft.setCursor(10, y += 25);
  tft.print("API: ");
  if (strlen(config.apiServer) > 0) {
    tft.println(config.apiServer);
    
    tft.setCursor(10, y += 15);
    tft.print("Letztes Update: ");
    if (lastApiUpdate > 0) {
      tft.printf("vor %lu s", (millis() - lastApiUpdate) / 1000);
    } else {
      tft.println("Nie");
    }
  } else {
    tft.println("Nicht konfiguriert");
  }
  
  // SD-Karten-Status
  tft.setCursor(10, y += 25);
  tft.print("SD-Karte: ");
  if (sdCardAvailable) {
    tft.println("Verfügbar");
  } else {
    tft.setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    tft.println("Nicht verfügbar");
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  }
  
  // QR-Code für Webzugriff
  if (wifiConnected) {
    String url = "http://" + networkIP + "/";
    displayQRCode(url, 100, 120, 2);
  }
  
  // Seitenwechsel-Hinweis
  tft.setTextColor(COLOR_INFO, COLOR_BACKGROUND);
  tft.setCursor(10, tft.height() - 20);
  tft.println("Drücken Sie die Taste für Hauptbildschirm");
}

void drawStatusBar() {
  // Obere Statusleiste
  tft.fillRect(0, 0, tft.width(), 20, COLOR_STATUS_BAR);
  tft.setTextColor(COLOR_TEXT, COLOR_STATUS_BAR);
  tft.setTextSize(1);
  
  // Zeit/Uptime
  long uptime = millis() / 1000;
  tft.setCursor(5, 5);
  tft.printf("%02ld:%02ld:%02ld", (uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
  
  // WLAN-Status
  tft.setCursor(tft.width() - 65, 5);
  if (wifiConnected) {
    tft.print("WLAN:");
    drawWifiSignal(tft.width() - 25, 10);
  } else if (config.apMode) {
    tft.print("AP-Modus");
  } else {
    tft.print("Offline");
  }
}

void drawWifiSignal(int x, int y) {
  int rssi = WiFi.RSSI();
  int strength;
  
  if (rssi > -50) {
    strength = 4; // Ausgezeichnet
  } else if (rssi > -65) {
    strength = 3; // Gut
  } else if (rssi > -75) {
    strength = 2; // Mittel
  } else if (rssi > -85) {
    strength = 1; // Schwach
  } else {
    strength = 0; // Sehr schwach
  }
  
  for (int i = 0; i < 4; i++) {
    if (i < strength) {
      tft.fillRect(x + (i * 5), y - (i * 2), 3, 2 + (i * 2), COLOR_TEXT);
    } else {
      tft.drawRect(x + (i * 5), y - (i * 2), 3, 2 + (i * 2), COLOR_TEXT);
    }
  }
}

void showToastMessage(String message, uint16_t color) {
  // Hintergrundspeicher für den Bereich unter dem Toast
  static uint16_t* bg = nullptr;
  static int toast_y = 0;
  static int toast_height = 0;
  static unsigned long toast_start = 0;
  
  // Wenn bereits ein Toast angezeigt wird, diesen löschen
  if (bg != nullptr && millis() - toast_start < 3000) {
    // Noch nicht ablaufen lassen
    return;
  } else if (bg != nullptr) {
    // Alten Toast entfernen
    for (int y = 0; y < toast_height; y++) {
      for (int x = 0; x < tft.width(); x++) {
        tft.drawPixel(x, toast_y + y, bg[y * tft.width() + x]);
      }
    }
    free(bg);
    bg = nullptr;
  }
  
  // Neue Nachricht anzeigen
  int y = tft.height() - 40;
  int height = 30;
  
  // Hintergrund speichern
  bg = (uint16_t*)malloc(tft.width() * height * sizeof(uint16_t));
  if (bg == nullptr) return; // Nicht genug Speicher
  
  for (int yi = 0; yi < height; yi++) {
    for (int x = 0; x < tft.width(); x++) {
      bg[yi * tft.width() + x] = tft.readPixel(x, y + yi);
    }
  }
  
  toast_y = y;
  toast_height = height;
  toast_start = millis();
  
  // Toast anzeigen
  tft.fillRoundRect(10, y, tft.width() - 20, height, 5, COLOR_BACKGROUND);
  tft.drawRoundRect(10, y, tft.width() - 20, height, 5, color);
  
  tft.setTextColor(color, COLOR_BACKGROUND);
  tft.setTextSize(1);
  tft.setCursor(15, y + (height - 8) / 2);
  tft.println(message);
}

// ----- QR-CODE -----
void displayQRCode(String text, int x, int y, int scale) {
  // QR-Code erstellen
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text.c_str());
  
  // QR-Code auf dem Display zeichnen
  for (uint8_t y_idx = 0; y_idx < qrcode.size; y_idx++) {
    for (uint8_t x_idx = 0; x_idx < qrcode.size; x_idx++) {
      if (qrcode_getModule(&qrcode, x_idx, y_idx)) {
        tft.fillRect(x + x_idx * scale, y + y_idx * scale, scale, scale, COLOR_TEXT);
      } else {
        tft.fillRect(x + x_idx * scale, y + y_idx * scale, scale, scale, COLOR_BACKGROUND);
      }
    }
  }
}

void displayWifiQR(String ssid, String password) {
  // WLAN-QR-Code erstellen
  String text = "WIFI:S:" + ssid + ";T:WPA;P:" + password + ";;";
  
  // Position berechnen
  int qrSize = 3 * 33 * QR_CODE_SIZE; // Version 3 QR hat 29x29 Module, mit 2px Rand = 33x33
  int x = (tft.width() - qrSize) / 2;
  int y = 140;
  
  displayQRCode(text, x, y, QR_CODE_SIZE);
  
  // Anweisungstext hinzufügen
  tft.setTextColor(COLOR_INFO, COLOR_BACKGROUND);
  tft.setTextSize(1);
  tft.setCursor(10, y + qrSize + 10);
  tft.println("Scannen Sie den QR-Code, um sich mit dem WLAN zu verbinden");
}

// ----- SD-KARTE -----
void initSDCard() {
  Serial.println("Initialisiere SD-Karte...");
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD-Karten-Initialisierung fehlgeschlagen!");
    sdCardAvailable = false;
    return;
  }
  
  sdCardAvailable = true;
  Serial.println("SD-Karte initialisiert");
  
  // Log-Verzeichnis erstellen, falls nicht vorhanden
  if (!SD.exists("/logs")) {
    SD.mkdir("/logs");
  }
  
  // Dateierzeugniskontrolle
  logEvent("System gestartet");
}

void logEvent(String event) {
  // Log-Eintrag im Serienmonitor
  Serial.print("Log: ");
  Serial.println(event);
  
  // Log-Eintrag auf SD-Karte speichern
  if (sdCardAvailable && config.sdLogging) {
    // Aktuelles Datum bestimmen
    unsigned long epochTime = millis() / 1000; // Sekunden seit dem Start
    
    // Dateiname mit aktuellem Datum
    char fileName[32];
    sprintf(fileName, "/logs/log_%lu.txt", epochTime / 86400); // Tage seit dem Start
    
    // Logzeile formatieren
    char logLine[256];
    sprintf(logLine, "[%02lu:%02lu:%02lu] %s\n", 
            (epochTime % 86400) / 3600, 
            (epochTime % 3600) / 60, 
            epochTime % 60, 
            event.c_str());
    
    // In Datei schreiben
    File logFile = SD.open(fileName, FILE_APPEND);
    if (logFile) {
      logFile.print(logLine);
      logFile.close();
    }
  }
}

void dataLoggingTaskFunction(void* parameter) {
  for (;;) {
    // Daten protokollieren, wenn aktiviert und SD-Karte verfügbar
    if (sdCardAvailable && config.sdLogging && sensorData.dataValid && 
        millis() - lastLog >= (config.logInterval * 1000)) {
      
      // Daten in CSV-Datei protokollieren
      char fileName[32];
      sprintf(fileName, "/logs/data_%lu.csv", millis() / 86400000); // Tage seit dem Start
      
      // Überprüfen, ob die Datei existiert, um die Kopfzeile zu schreiben
      bool fileExists = SD.exists(fileName);
      
      File dataFile = SD.open(fileName, FILE_APPEND);
      if (dataFile) {
        // Kopfzeile schreiben, wenn die Datei neu ist
        if (!fileExists) {
          dataFile.println("timestamp,temperature,humidity,power,energy,relay_state,runtime");
        }
        
        // Datensatz formatieren
        char dataLine[128];
        sprintf(dataLine, "%lu,%.1f,%.1f,%.1f,%.2f,%d,%lu", 
                millis() / 1000, 
                sensorData.temperature, 
                sensorData.humidity, 
                sensorData.power, 
                sensorData.energy, 
                relayState ? 1 : 0, 
                sensorData.totalRuntime);
        
        dataFile.println(dataLine);
        dataFile.close();
        
        lastLog = millis();
      }
    }
    
    // Pause, um CPU-Zeit zu sparen
    delay(1000);
  }
}

// ----- SENSOREN -----
void readSensors() {
  // Hier würden die tatsächlichen Sensordaten gelesen
  // In diesem Beispiel werden Daten simuliert
  
  // Temperatur simulieren
  if (config.temperatureSensor) {
    static float lastTemp = 20.0;
    sensorData.temperature = lastTemp + (random(-10, 11) / 10.0);
    if (sensorData.temperature < 5.0) sensorData.temperature = 5.0;
    if (sensorData.temperature > 40.0) sensorData.temperature = 40.0;
    lastTemp = sensorData.temperature;
  }
  
  // Luftfeuchtigkeit simulieren
  if (config.humiditySensor) {
    static float lastHumidity = 50.0;
    sensorData.humidity = lastHumidity + (random(-10, 11) / 10.0);
    if (sensorData.humidity < 10.0) sensorData.humidity = 10.0;
    if (sensorData.humidity > 90.0) sensorData.humidity = 90.0;
    lastHumidity = sensorData.humidity;
  }
  
  // Energiedaten simulieren
  if (config.energySensor) {
    if (relayState) {
      // Typischer Verbrauch eines Luftentfeuchters (200-500W)
      sensorData.power = 300.0 + (random(-50, 51) / 10.0);
      
      // Energieverbrauch akkumulieren
      float hourFraction = SENSOR_READ_INTERVAL / 3600000.0;
      sensorData.energy += sensorData.power * hourFraction / 1000.0;
    } else {
      sensorData.power = 0.0;
    }
  }
  
  // Laufzeit aktualisieren
  if (relayState) {
    sensorData.currentRuntime += SENSOR_READ_INTERVAL / 1000;
    sensorData.totalRuntime += SENSOR_READ_INTERVAL / 1000;
  } else {
    sensorData.currentRuntime = 0;
  }
  
  sensorData.dataValid = true;
  
  // MQTT-Daten senden, wenn verbunden
  if (mqttConnected) {
    publishSensorData();
  }
}

// ----- API -----
void updateAPI() {
  if (!wifiConnected || strlen(config.apiServer) == 0) {
    return;
  }
  
  // HTTP-Client für API-Anfrage
  HTTPClient http;
  
  // URL mit Gerätename erstellen
  String url = String(config.apiServer);
  if (!url.startsWith("http")) {
    url = "http://" + url;
  }
  if (!url.endsWith("/")) {
    url += "/";
  }
  url += "api/device/" + String(config.deviceName) + "/data";
  
  // Daten als JSON erstellen
  DynamicJsonDocument doc(512);
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["power"] = sensorData.power;
  doc["energy"] = sensorData.energy;
  doc["relay"] = relayState;
  doc["runtime"] = sensorData.totalRuntime;
  doc["timestamp"] = millis() / 1000;
  
  String jsonData;
  serializeJson(doc, jsonData);
  
  // Debugging
  Serial.print("Sende API-Daten: ");
  Serial.println(jsonData);
  
  // HTTP-Header setzen und Anfrage starten
  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/json");
  
  if (strlen(config.apiKey) > 0) {
    http.addHeader("Authorization", "Bearer " + String(config.apiKey));
  }
  
  // POST-Anfrage senden
  int httpCode = http.POST(jsonData);
  
  if (httpCode > 0) {
    Serial.printf("API-Antwort: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      Serial.println(response);
      
      // Antwort verarbeiten (z.B. Konfigurationsänderungen)
      DynamicJsonDocument respDoc(512);
      DeserializationError error = deserializeJson(respDoc, response);
      
      if (!error) {
        // Relais-Steuerung durch API
        if (respDoc.containsKey("relay_control") && config.relayEnabled) {
          bool apiRelayState = respDoc["relay_control"];
          
          if (apiRelayState != relayState) {
            relayState = apiRelayState;
            digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
            
            // Log-Eintrag erstellen
            logEvent("Relais " + String(relayState ? "eingeschaltet" : "ausgeschaltet") + " (API)");
          }
        }
      }
    }
  } else {
    Serial.printf("API-Fehler: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

// ----- RELAIS -----
void controlRelay() {
  if (!config.relayEnabled) {
    return;
  }
  
  bool newState = relayState;
  
  switch (config.relayMode) {
    case 0: // Manuell
      // Keine automatische Änderung
      break;
      
    case 1: // Zeitgesteuert
      {
        // Aktuelle Zeit (seit Start)
        unsigned long currentTime = millis() / 1000;
        unsigned long seconds = currentTime % 86400; // Sekunden des Tages
        int hour = (seconds / 3600);
        int minute = (seconds % 3600) / 60;
        
        // Zeit als Minuten seit Mitternacht
        int currentMinutes = hour * 60 + minute;
        int onMinutes = config.relayOnHour * 60 + config.relayOnMinute;
        int offMinutes = config.relayOffHour * 60 + config.relayOffMinute;
        
        // Überprüfen, ob Ein- oder Ausschalten erforderlich ist
        if (onMinutes < offMinutes) {
          // Normaler Fall: Ein am Morgen, Aus am Abend
          newState = (currentMinutes >= onMinutes && currentMinutes < offMinutes);
        } else {
          // Übernachtfall: Ein am Abend, Aus am Morgen
          newState = (currentMinutes >= onMinutes || currentMinutes < offMinutes);
        }
      }
      break;
      
    case 2: // Sensorgesteuert
      if (config.humiditySensor) {
        // Einschalten, wenn Luftfeuchtigkeit über Schwellenwert
        newState = (sensorData.humidity > config.relayThreshold);
      }
      break;
  }
  
  // Relais-Status aktualisieren, wenn sich etwas geändert hat
  if (newState != relayState) {
    relayState = newState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    
    // Log-Eintrag erstellen
    logEvent("Relais " + String(relayState ? "eingeschaltet" : "ausgeschaltet") + 
             " (" + getRelayModeText(config.relayMode) + ")");
    
    // MQTT-Status senden, wenn verbunden
    if (mqttConnected) {
      String topic = String(config.mqttTopic) + config.deviceName + "/state/relay";
      mqttClient.publish(topic.c_str(), relayState ? "ON" : "OFF", true);
    }
  }
}

// ----- HILFSFUNKTIONEN -----
String getRelayModeText(int mode) {
  switch (mode) {
    case 0: return "Manuell";
    case 1: return "Zeitgesteuert";
    case 2: return "Sensorgesteuert";
    default: return "Unbekannt";
  }
}

String formatRuntime(long seconds) {
  char buffer[20];
  
  if (seconds < 60) {
    sprintf(buffer, "%ld s", seconds);
  } else if (seconds < 3600) {
    sprintf(buffer, "%ld:%02ld min", seconds / 60, seconds % 60);
  } else if (seconds < 86400) {
    sprintf(buffer, "%ld:%02ld:%02ld h", seconds / 3600, (seconds % 3600) / 60, seconds % 60);
  } else {
    sprintf(buffer, "%ld T %ld:%02ld h", seconds / 86400, (seconds % 86400) / 3600, (seconds % 3600) / 60);
  }
  
  return String(buffer);
}

String formatFileSize(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0, 1) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0, 1) + " MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0, 1) + " GB";
  }
}