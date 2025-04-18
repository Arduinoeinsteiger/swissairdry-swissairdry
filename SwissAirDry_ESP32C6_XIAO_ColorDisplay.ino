/*
 * SwissAirDry ESP32-C6 XIAO Multifunktionssystem mit Farbdisplay
 * 
 * Unterstützt:
 * - 1.47" LCD-Farbdisplay (172x320)
 * - SD-Kartenspeicher
 * - QR-Code Generierung für einfache Verbindung
 * - Integrierte MQTT-Unterstützung für Sensor- und Steuerungsdaten
 * - API-Integration mit lokaler Zwischenspeicherung
 * - OTA Updates
 * 
 * Hardwarevoraussetzungen:
 * - ESP32-C6 XIAO Board mit 1.47" LCD-Display
 * - SD-Kartenslot
 * 
 * Hinweis: Diese Version verwendet die native ESP32 MQTT-Bibliothek
 * statt der PubSubClient-Bibliothek
 */

// ----- BIBLIOTHEKEN -----
#include <WiFi.h>
#include <Network.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <TFT_eSPI.h>         // Display-Treiber (muss für das spezifische Display konfiguriert sein)
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DNSServer.h>

// ----- MQTT Bibliothek für ESP32-C6 -----
#include <esp_mqtt.h>
#include <mqtt_client.h>

// ----- QR-CODE-BIBLIOTHEK -----
// Einfache QR-Code-Implementierung für ESP32
#include "qrcode.h"

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

// Pins für XIAO ESP32-C6 (anpassen je nach Hardware)
#define SD_CS_PIN SS          // SD-Karten Chip Select Pin 
#define RELAY_PIN 5           // Relais für die Steuerung
#define TFT_LED_PIN 21        // Backlight Pin (optional, falls vorhanden)

// RGB LED Pins für XIAO ESP32-C6
#define RGB_LED_PIN 8         // XIAO ESP32-C6 hat eine integrierte RGB-LED an Pin 8

// Zeitintervalle
#define API_UPDATE_INTERVAL 60000        // API-Update alle 60 Sekunden
#define DISPLAY_UPDATE_INTERVAL 1000     // Display-Update alle 1 Sekunde
#define SENSOR_READ_INTERVAL 5000        // Sensor-Leseintervall alle 5 Sekunden
#define LOG_INTERVAL 300000             // Daten-Logging alle 5 Minuten

// MQTT Topics
#define MQTT_DATA_TOPIC "swissairdry/%s/data"
#define MQTT_STATUS_TOPIC "swissairdry/%s/status"
#define MQTT_COMMAND_TOPIC "swissairdry/%s/cmd/#"
#define MQTT_RELAY_TOPIC "swissairdry/%s/cmd/relay"

// ----- GLOBALE OBJEKTE -----
TFT_eSPI tft = TFT_eSPI();              // Display-Objekt
WebServer server(80);                    // Webserver für Konfiguration
DNSServer dnsServer;                     // DNS-Server für Captive Portal

// MQTT Client
esp_mqtt_client_handle_t mqtt_client = NULL;

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

// Task Handle für Hintergrundprozesse
TaskHandle_t dataLoggingTask;            // Task-Handle für Datenprotokollierung

// QR-Code Objekt
QRCode qrcode;

// Letzte RGB-LED Aktualisierung
unsigned long lastLedUpdate = 0;
#define LED_UPDATE_INTERVAL 3000     // LED-Status alle 3 Sekunden aktualisieren

// RGB-LED Blinken für Status
bool ledBlinkState = false;
unsigned long lastLedBlinkTime = 0;
#define LED_BLINK_INTERVAL 500     // LED-Blinken alle 0,5 Sekunden

// ----- FUNKTIONEN -----

// ----- MQTT Callback-Funktionen -----
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
  switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
      Serial.println("MQTT verbunden");
      mqttConnected = true;
      
      // Abonnieren der Kommandos
      char topic[128];
      sprintf(topic, MQTT_COMMAND_TOPIC, config.deviceName);
      esp_mqtt_client_subscribe(mqtt_client, topic, 0);
      
      // Online-Status senden
      sprintf(topic, MQTT_STATUS_TOPIC, config.deviceName);
      esp_mqtt_client_publish(mqtt_client, topic, "online", 0, 1, 1);
      break;
      
    case MQTT_EVENT_DISCONNECTED:
      Serial.println("MQTT getrennt");
      mqttConnected = false;
      break;
      
    case MQTT_EVENT_SUBSCRIBED:
      Serial.println("MQTT Topic abonniert");
      break;
      
    case MQTT_EVENT_UNSUBSCRIBED:
      Serial.println("MQTT Topic abbestellt");
      break;
      
    case MQTT_EVENT_PUBLISHED:
      Serial.println("MQTT Nachricht veröffentlicht");
      break;
      
    case MQTT_EVENT_DATA:
      // Empfangene Daten verarbeiten
      handleMqttMessage(event);
      break;
      
    case MQTT_EVENT_ERROR:
      Serial.println("MQTT Fehler");
      break;
      
    default:
      break;
  }
  return ESP_OK;
}

void handleMqttMessage(esp_mqtt_event_handle_t event) {
  // Null-terminieren, um sicherzustellen, dass wir einen gültigen String haben
  char topic[128];
  char payload[128];
  
  strncpy(topic, event->topic, event->topic_len);
  topic[event->topic_len] = '\0';
  
  strncpy(payload, event->data, event->data_len);
  payload[event->data_len] = '\0';
  
  Serial.print("MQTT Nachricht empfangen: ");
  Serial.print(topic);
  Serial.print(" = ");
  Serial.println(payload);
  
  // Topic parsen
  char expectedTopic[128];
  sprintf(expectedTopic, MQTT_RELAY_TOPIC, config.deviceName);
  
  // Relais-Steuerung
  if (strcmp(topic, expectedTopic) == 0) {
    if (strcmp(payload, "ON") == 0 || strcmp(payload, "on") == 0 || strcmp(payload, "1") == 0) {
      if (config.relayEnabled) {
        relayState = true;
        digitalWrite(RELAY_PIN, HIGH);
        logEvent("Relais eingeschaltet (MQTT)");
      }
    } else if (strcmp(payload, "OFF") == 0 || strcmp(payload, "off") == 0 || strcmp(payload, "0") == 0) {
      if (config.relayEnabled) {
        relayState = false;
        digitalWrite(RELAY_PIN, LOW);
        logEvent("Relais ausgeschaltet (MQTT)");
      }
    }
    
    // Status zurückmelden
    sprintf(topic, "swissairdry/%s/state/relay", config.deviceName);
    esp_mqtt_client_publish(mqtt_client, topic, relayState ? "ON" : "OFF", 0, 1, 1);
  }
}

// MQTT initialisieren
void setupMQTT() {
  if (strlen(config.mqttServer) == 0) {
    mqttConnected = false;
    return;
  }
  
  esp_mqtt_client_config_t mqtt_cfg = {
    .host = config.mqttServer,
    .port = config.mqttPort,
    .username = config.mqttUser[0] != '\0' ? config.mqttUser : NULL,
    .password = config.mqttPassword[0] != '\0' ? config.mqttPassword : NULL,
    .client_id = config.deviceName,
    .keepalive = 60,
  };
  
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, (esp_event_handler_t)mqtt_event_handler, NULL);
  esp_mqtt_client_start(mqtt_client);
}

// MQTT Nachricht senden
void publishMQTT(const char* topic, const char* payload, bool retain = false) {
  if (!mqttConnected) return;
  
  esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 0, retain ? 1 : 0);
}

// ----- RGB-LED-FUNKTIONEN -----
// RGB-LED einstellen (Werte von 0-255 für rot, grün, blau)
void setRGBLed(uint8_t red, uint8_t green, uint8_t blue) {
  // ESP32-C6 nutzt NeoPixel/WS2812 RGB LED
  // Wir simulieren PWM mit digitalWrite
  analogWrite(RGB_LED_PIN, red);   // Rot
  analogWrite(RGB_LED_PIN+1, green); // Grün
  analogWrite(RGB_LED_PIN+2, blue);  // Blau
  
  Serial.printf("RGB-LED gesetzt auf: R=%d, G=%d, B=%d\n", red, green, blue);
}

// Voreingestellte Farben für die RGB-LED
void setLedConnecting() {
  setRGBLed(255, 165, 0); // Orange
}

void setLedConnected() {
  setRGBLed(0, 255, 0);   // Grün
}

void setLedError() {
  setRGBLed(255, 0, 0);   // Rot
}

void setLedWorking() {
  setRGBLed(0, 0, 255);   // Blau
}

void setLedIdle() {
  setRGBLed(0, 255, 255); // Cyan
}

void setLedOff() {
  setRGBLed(0, 0, 0);     // Aus
}

// Schnelles Blinken für kritische Meldungen
void blinkLedError(int times) {
  for (int i = 0; i < times; i++) {
    setLedError();
    delay(200);
    setLedOff();
    delay(200);
  }
}

// Aktualisieren des RGB-LED Status basierend auf dem Systemzustand
void updateRGBLedStatus() {
  // Status-LED basierend auf dem Gerätezustand einstellen
  if (errorCode > 0) {
    // Bei Fehlern: Rote LED blinken lassen
    if (millis() - lastLedBlinkTime >= LED_BLINK_INTERVAL) {
      ledBlinkState = !ledBlinkState;
      if (ledBlinkState) {
        setLedError();
      } else {
        setLedOff();
      }
      lastLedBlinkTime = millis();
    }
  } else if (!wifiConnected) {
    // Bei fehlender WLAN-Verbindung: Orange blinken lassen
    if (millis() - lastLedBlinkTime >= LED_BLINK_INTERVAL) {
      ledBlinkState = !ledBlinkState;
      if (ledBlinkState) {
        setLedConnecting();
      } else {
        setLedOff();
      }
      lastLedBlinkTime = millis();
    }
  } else if (relayState) {
    // Wenn das Relais eingeschaltet ist: Grün (Arbeitsbetrieb)
    setLedConnected();
  } else if (configMode) {
    // Im Konfigurationsmodus: Blau
    setLedWorking();
  } else {
    // Im Normalbetrieb (Standby): Cyan
    setLedIdle();
  }
}

// ----- SETUP -----
void setup() {
  // Serielle Schnittstelle initialisieren
  Serial.begin(115200);
  Serial.println("\nSwissAirDry ESP32-C6 mit Farbdisplay startet...");
  
  // Pins initialisieren
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Relais aus
  
  // Display Backlight (falls vorhanden)
  if (TFT_LED_PIN >= 0) {
    pinMode(TFT_LED_PIN, OUTPUT);
    digitalWrite(TFT_LED_PIN, HIGH);  // Backlight an
  }
  
  // RGB-LED initialisieren
  pinMode(RGB_LED_PIN, OUTPUT);
  pinMode(RGB_LED_PIN+1, OUTPUT);
  pinMode(RGB_LED_PIN+2, OUTPUT);
  setLedWorking(); // Blaue LED während Initialisierung
  
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
  
  // SPIFFS (interner Speicher) initialisieren
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS-Initialisierung fehlgeschlagen");
    blinkLedError(2); // 2x Blinken bei Fehler
  } else {
    Serial.println("SPIFFS initialisiert");
  }
  
  // RGB-LED auf Verbindungsmodus setzen
  setLedConnecting();
  
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
    8000,                      // Stack-Größe
    NULL,                      // Parameter
    1,                         // Priorität
    &dataLoggingTask,          // Task-Handle
    0                          // Core
  );
  
  // Erfolgreich initialisiert
  setLedConnected();
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
  
  // RGB-LED-Status basierend auf dem Systemzustand aktualisieren
  if (millis() - lastLedUpdate >= LED_UPDATE_INTERVAL) {
    updateRGBLedStatus();
    lastLedUpdate = millis();
  }
  
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

// Datenprotokollierung im Hintergrund
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
  html += "<style>";
  // Hier CSS-Stile einfügen
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f5f9ff; color: #333; }";
  html += ".container { max-width: 800px; margin: 0 auto; padding: 20px; }";
  html += "h1, h2, h3 { color: #0066cc; }";
  html += ".card { background-color: white; border-radius: 8px; padding: 15px; margin-bottom: 20px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
  html += ".button { display: inline-block; background-color: #0066cc; color: white; padding: 10px 15px; border-radius: 5px; text-decoration: none; margin: 5px; }";
  html += ".on { color: #4CAF50; font-weight: bold; }";
  html += ".off { color: #f44336; font-weight: bold; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>SwissAirDry</h1>";
  html += "<h2>" + String(config.deviceName) + "</h2>";
  
  // Status-Informationen
  html += "<div class='card'>";
  html += "<h3>Status</h3>";
  html += "<p>Firmware: v" + firmwareVersion + "</p>";
  html += "<p>Netzwerk: " + (wifiConnected ? "Verbunden mit " + String(config.wifiSSID) : "AP-Modus") + "</p>";
  html += "<p>IP: " + networkIP + "</p>";
  html += "<p>MQTT: " + String(mqttConnected ? "Verbunden" : "Nicht verbunden") + "</p>";
  html += "<p>SD-Karte: " + String(sdCardAvailable ? "Vorhanden" : "Nicht gefunden") + "</p>";
  html += "</div>";
  
  // Aktuelle Sensordaten
  html += "<div class='card'>";
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
  html += "<div class='card'>";
  html += "<h3>Relais-Steuerung</h3>";
  html += "<p>Status: <span class='" + String(relayState ? "on" : "off") + "'>" + 
          String(relayState ? "Eingeschaltet" : "Ausgeschaltet") + "</span></p>";
  html += "<p>Modus: " + getRelayModeText(config.relayMode) + "</p>";
  
  if (config.relayEnabled) {
    html += "<a href='/toggle' class='button'>" + 
            String(relayState ? "Ausschalten" : "Einschalten") + "</a>";
  } else {
    html += "<p>Relais-Steuerung deaktiviert</p>";
  }
  html += "</div>";
  
  // Menü
  html += "<div class='card'>";
  html += "<a href='/config' class='button'>Einstellungen</a>";
  html += "<a href='/data' class='button'>Datenvisualisierung</a>";
  if (sdCardAvailable) {
    html += "<a href='/logs' class='button'>Logs</a>";
  }
  html += "<a href='/update' class='button'>Update</a>";
  html += "</div>";
  
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Weitere Handler-Funktionen (handleConfig, handleSave, usw.) implementieren...
// Diese sind im ursprünglichen Sketch detailliert ausgeführt

// Einfacher Platzhalter für fehlende Handler
void handleConfig() {
  server.send(200, "text/html", "<html><body><h1>Konfiguration</h1><p>Konfigurationsschnittstelle wird implementiert...</p><a href='/'>Zurück</a></body></html>");
}

void handleSave() {
  server.send(200, "text/html", "<html><body><h1>Gespeichert</h1><p>Konfiguration gespeichert.</p><a href='/'>Zurück</a></body></html>");
}

void handleRestart() {
  server.send(200, "text/html", "<html><body><h1>Neustart</h1><p>Gerät wird neu gestartet...</p></body></html>");
  delay(1000);
  ESP.restart();
}

void handleReset() {
  server.send(200, "text/html", "<html><body><h1>Zurücksetzen</h1><p>Einstellungen werden zurückgesetzt...</p></body></html>");
  delay(1000);
  // EEPROM löschen
  for (int i = 0; i < 10; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  ESP.restart();
}

void handleUpdateForm() {
  server.send(200, "text/html", "<html><body><h1>Update</h1><p>Update-Formular wird implementiert...</p><a href='/'>Zurück</a></body></html>");
}

void handleUpdateResult() {
  server.send(200, "text/html", "<html><body><h1>Update-Ergebnis</h1><p>Update abgeschlossen.</p><a href='/'>Zurück</a></body></html>");
}

void handleUpdateUpload() {
  // Wird für Dateiuploads benötigt, hier nur Platzhalter
  server.send(200, "text/plain", "Upload wird verarbeitet");
}

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

void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(sensorData.temperature, 1) + ",";
  json += "\"humidity\":" + String(sensorData.humidity, 1) + ",";
  json += "\"power\":" + String(sensorData.power, 1) + ",";
  json += "\"energy\":" + String(sensorData.energy, 2) + ",";
  json += "\"relay_state\":" + String(relayState ? "true" : "false") + ",";
  json += "\"runtime\":" + String(sensorData.totalRuntime) + ",";
  json += "\"timestamp\":" + String(millis() / 1000);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleLogs() {
  if (!sdCardAvailable) {
    server.send(503, "text/html", "<html><body><h1>SD-Karte nicht verfügbar</h1><p><a href='/'>Zurück</a></p></body></html>");
    return;
  }
  
  server.send(200, "text/html", "<html><body><h1>Logs</h1><p>Log-Anzeige wird implementiert...</p><a href='/'>Zurück</a></body></html>");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
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

// ----- QR-CODE -----
// Zeigt einen QR-Code auf dem Display an
void displayQRCode(String text, int x, int y, int scale) {
  uint8_t qrcode_data[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcode_data, 3, 0, text.c_str());
  
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
  int qrSize = 3 * 33 * QR_CODE_SIZE;
  int x = (tft.width() - qrSize) / 2;
  int y = 140;
  
  displayQRCode(text, x, y, QR_CODE_SIZE);
  
  // Anweisungstext hinzufügen
  tft.setTextColor(COLOR_INFO, COLOR_BACKGROUND);
  tft.setTextSize(1);
  tft.setCursor(10, y + qrSize + 10);
  tft.println("Scannen Sie den QR-Code, um sich mit dem WLAN zu verbinden");
}

// Zeichnet eine Statusleiste am oberen Displayrand
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

// Zeichnet die WLAN-Signalstärke
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

// Zeigt Detailbildschirm mit Sensordaten
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

// Zeigt Netzwerkinformationsbildschirm
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

// Zeigt eine Toast-Nachricht am unteren Bildschirmrand
void showToastMessage(String message, uint16_t color) {
  int y = tft.height() - 40;
  int height = 30;
  
  // Toast anzeigen
  tft.fillRoundRect(10, y, tft.width() - 20, height, 5, COLOR_BACKGROUND);
  tft.drawRoundRect(10, y, tft.width() - 20, height, 5, color);
  
  tft.setTextColor(color, COLOR_BACKGROUND);
  tft.setTextSize(1);
  tft.setCursor(15, y + (height - 8) / 2);
  tft.println(message);
  
  // Kurz anzeigen lassen
  delay(2000);
  
  // Bildschirm aktualisieren
  updateDisplay();
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
    // Sensordaten als JSON formatieren
    String jsonPayload = "{";
    jsonPayload += "\"temperature\":" + String(sensorData.temperature, 1) + ",";
    jsonPayload += "\"humidity\":" + String(sensorData.humidity, 1) + ",";
    jsonPayload += "\"power\":" + String(sensorData.power, 1) + ",";
    jsonPayload += "\"energy\":" + String(sensorData.energy, 2) + ",";
    jsonPayload += "\"relay_state\":" + String(relayState ? "true" : "false") + ",";
    jsonPayload += "\"runtime\":" + String(sensorData.totalRuntime);
    jsonPayload += "}";
    
    // Topic formatieren
    char topic[128];
    sprintf(topic, MQTT_DATA_TOPIC, config.deviceName);
    
    // Veröffentlichen
    publishMQTT(topic, jsonPayload.c_str());
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
  String jsonData = "{";
  jsonData += "\"temperature\":" + String(sensorData.temperature, 1) + ",";
  jsonData += "\"humidity\":" + String(sensorData.humidity, 1) + ",";
  jsonData += "\"power\":" + String(sensorData.power, 1) + ",";
  jsonData += "\"energy\":" + String(sensorData.energy, 2) + ",";
  jsonData += "\"relay_state\":" + String(relayState ? "true" : "false") + ",";
  jsonData += "\"runtime\":" + String(sensorData.totalRuntime) + ",";
  jsonData += "\"timestamp\":" + String(millis() / 1000);
  jsonData += "}";
  
  // Debugging
  Serial.print("Sende API-Daten: ");
  Serial.println(jsonData);
  
  // HTTP-Header setzen und Anfrage starten
  http.begin(url);
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
      
      // Antwort verarbeiten
      // Hier könnte JSON-Parsing stattfinden, um Befehle vom Server zu verarbeiten
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
      char topic[128];
      sprintf(topic, "swissairdry/%s/state/relay", config.deviceName);
      publishMQTT(topic, relayState ? "ON" : "OFF", true);
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