// SwissAirDry Webkonfiguration für ESP8266/Wemos D1 Mini
// Bietet eine Weboberfläche zur Konfiguration der Pins und API-Einstellungen
// Unterstützt Verbindung mit Smartphone-Hotspot

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// ----- WLAN-KONFIGURATION -----
// Standard-WLAN-Einstellungen (können über Web-UI geändert werden)
String ssid = "SSID";
String password = "PASSWORT";
bool apMode = false;  // Access Point (AP) Modus aktivieren

// ----- AP-MODUS-KONFIGURATION -----
String apSsid = "SwissAirDry-AP";
String apPassword = "12345678";

// ----- HARDWARE-KONFIGURATION -----
// Festverdrahtete Pins für Wemos D1 Mini
#define LED_PIN 2        // GPIO2 (D4 auf Wemos D1 Mini) - Blau LED on-board
#define LED_ON LOW       // LED ist aktiv LOW (invertiert)
#define LED_OFF HIGH

// Display-Konfiguration (feste Pinbelegung)
#define OLED_RESET -1    // Kein Reset-Pin verwendet
#define SCREEN_WIDTH 128 // OLED Display Breite in Pixeln
#define SCREEN_HEIGHT 64 // OLED Display Höhe in Pixeln
#define OLED_ADDR 0x3C   // I2C-Adresse des OLED-Displays
#define SDA_PIN 4        // D2 (GPIO4)
#define SCL_PIN 5        // D1 (GPIO5)

// ----- EEPROM-KONFIGURATION -----
#define CONFIG_START 0
#define CONFIG_VERSION "v1"  // Ändern Sie diese Nummer wenn sich die Konfigstruktur ändert

// ----- WEBSERVER-KONFIGURATION -----
ESP8266WebServer server(80);
const char* www_username = "admin";
String www_password = "admin";  // Standard-Passwort, kann über Web-UI geändert werden

// Struktur für konfigurierbare Pins
struct PinConfig {
  int pinNumber;      // GPIO-Nummer
  String pinName;     // Bezeichnung im UI
  String pinFunction; // Funktion (input, output, relay, sensor)
  bool inverted;      // Invertierte Logik
  int initialState;   // Anfangszustand für Outputs
};

// Struktur für API-Einstellungen
struct ApiConfig {
  String primaryApiUrl;
  String backupApiUrl;
  String apiKey;
  int updateInterval;  // in Sekunden
};

// Konfigurationsstruktur
struct Configuration {
  char version[4];        // Konfigurationsversion
  char deviceName[32];    // Gerätename
  char wifiSsid[32];      // WLAN-SSID
  char wifiPassword[64];  // WLAN-Passwort
  bool apModeEnabled;     // Access Point Modus aktiviert
  char webPassword[32];   // Web-UI Passwort
  
  // API-Konfiguration
  char primaryApiUrl[128];
  char backupApiUrl[128];
  char apiKey[64];
  int updateInterval;
  
  // Pin-Konfiguration
  struct {
    int pinNumber;        // GPIO-Nummer
    char pinName[32];     // Bezeichnung
    char pinFunction[16]; // Funktion (input, output, relay, sensor)
    bool inverted;        // Invertierte Logik
    int initialState;     // Anfangszustand
  } pins[12];             // Bis zu 12 konfigurierbare Pins
  
  // Anzahl der konfigurierten Pins
  int numConfiguredPins;
  
  // Reserviert für zukünftige Erweiterungen
  char reserved[128];
};

// Konfigurationsvariable
Configuration config;

// Objekte initialisieren
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool displayAvailable = false;

// Aktiv konfigurierte Pins
PinConfig activePins[12];
int numActivePins = 0;

// API-Konfiguration
ApiConfig apiConfig;

// Hostname mit eindeutiger Chip-ID
String hostname = "SwissAirDry-";

// Status-Variablen
bool wifiConnected = false;
bool otaEnabled = true;

void setup() {
  // Serielle Verbindung starten
  Serial.begin(115200);
  Serial.println("\n\nSwissAirDry Web-Konfiguration");
  
  // EEPROM initialisieren
  EEPROM.begin(1024);  // 1KB für Konfigurationsdaten
  
  // GPIO2/D4 als statusLED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  
  // Eindeutigen Hostnamen erstellen
  uint16_t chipId = ESP.getChipId() & 0xFFFF;
  hostname += String(chipId, HEX);
  Serial.print("Hostname: ");
  Serial.println(hostname);
  
  // Display initialisieren
  Wire.begin(SDA_PIN, SCL_PIN);  // SDA=D2(GPIO4), SCL=D1(GPIO5)
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 Display nicht gefunden");
    displayAvailable = false;
  } else {
    Serial.println("Display initialisiert");
    displayAvailable = true;
    
    // Startbildschirm anzeigen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("SwissAirDry");
    display.println("Web-Konfiguration");
    display.println("Starte...");
    display.display();
    delay(1000);
  }
  
  // Konfiguration aus EEPROM laden
  if (loadConfiguration()) {
    Serial.println("Konfiguration geladen");
    
    // WLAN-Einstellungen aus Konfiguration übernehmen
    ssid = String(config.wifiSsid);
    password = String(config.wifiPassword);
    apMode = config.apModeEnabled;
    www_password = String(config.webPassword);
    
    // Pin-Konfiguration laden
    loadPinConfiguration();
    
    // API-Konfiguration laden
    apiConfig.primaryApiUrl = String(config.primaryApiUrl);
    apiConfig.backupApiUrl = String(config.backupApiUrl);
    apiConfig.apiKey = String(config.apiKey);
    apiConfig.updateInterval = config.updateInterval;
    
  } else {
    Serial.println("Keine gültige Konfiguration gefunden, verwende Standardwerte");
    
    // Standardkonfiguration erstellen
    strncpy(config.version, CONFIG_VERSION, 4);
    sprintf(config.deviceName, "SwissAirDry-%04X", chipId);
    strncpy(config.wifiSsid, ssid.c_str(), 32);
    strncpy(config.wifiPassword, password.c_str(), 64);
    config.apModeEnabled = false;
    strncpy(config.webPassword, www_password.c_str(), 32);
    
    // Standard-API-Konfiguration
    strncpy(config.primaryApiUrl, "https://api.swissairdry.com", 128);
    strncpy(config.backupApiUrl, "https://backup-api.swissairdry.com", 128);
    strncpy(config.apiKey, "", 64);
    config.updateInterval = 300;  // 5 Minuten
    
    // Standard-Pin-Konfiguration
    config.numConfiguredPins = 3;
    
    // Pin D5 (GPIO14) als Relais
    config.pins[0].pinNumber = 14;  // D5
    strncpy(config.pins[0].pinName, "Relais 1", 32);
    strncpy(config.pins[0].pinFunction, "relay", 16);
    config.pins[0].inverted = false;
    config.pins[0].initialState = LOW;
    
    // Pin D6 (GPIO12) als Sensor-Input
    config.pins[1].pinNumber = 12;  // D6
    strncpy(config.pins[1].pinName, "Sensor 1", 32);
    strncpy(config.pins[1].pinFunction, "input", 16);
    config.pins[1].inverted = false;
    config.pins[1].initialState = HIGH;
    
    // Pin D7 (GPIO13) als digitaler Output
    config.pins[2].pinNumber = 13;  // D7
    strncpy(config.pins[2].pinName, "LED 1", 32);
    strncpy(config.pins[2].pinFunction, "output", 16);
    config.pins[2].inverted = false;
    config.pins[2].initialState = LOW;
    
    // Konfiguration speichern
    saveConfiguration();
    
    // Pin-Konfiguration laden
    loadPinConfiguration();
  }
  
  // GPIOs basierend auf Konfiguration initialisieren
  initializePins();
  
  // Verbindungsmethode basierend auf Konfiguration wählen
  if (apMode) {
    setupAccessPoint();
  } else {
    connectWiFi();
  }
  
  // Immer den Access Point starten, wenn keine WLAN-Verbindung besteht
  if (!wifiConnected) {
    setupAccessPoint();
  }
  
  // OTA-Updates einrichten, wenn WLAN verbunden
  if (wifiConnected && otaEnabled) {
    setupOTA();
  }
  
  // Webserver-Routen definieren
  setupWebserver();
  
  // Displaystatus aktualisieren
  updateDisplay();
}

// Webserver-Routen einrichten
void setupWebserver() {
  // Hauptseite
  server.on("/", HTTP_GET, handleRoot);
  
  // API für Gerätestatus
  server.on("/api/status", HTTP_GET, handleStatus);
  
  // API für Pin-Steuerung
  server.on("/api/pins", HTTP_GET, handleGetPins);
  server.on("/api/pins", HTTP_POST, handleSetPins);
  
  // API für Konfigurationsverwaltung
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handleSetConfig);
  
  // API für WLAN-Einstellungen
  server.on("/api/wifi", HTTP_GET, handleGetWifi);
  server.on("/api/wifi", HTTP_POST, handleSetWifi);
  
  // API für API-Einstellungen
  server.on("/api/apiconfig", HTTP_GET, handleGetApiConfig);
  server.on("/api/apiconfig", HTTP_POST, handleSetApiConfig);
  
  // Statische Dateien (für eingebettete Web-UI)
  server.on("/styles.css", HTTP_GET, handleStyles);
  server.on("/script.js", HTTP_GET, handleScript);
  
  // Nicht gefunden
  server.onNotFound(handleNotFound);
  
  // Serverstart
  server.begin();
  Serial.println("Webserver gestartet");
}

// GPIO-Pins basierend auf Konfiguration initialisieren
void initializePins() {
  for (int i = 0; i < numActivePins; i++) {
    PinConfig& pin = activePins[i];
    
    if (pin.pinFunction == "input") {
      pinMode(pin.pinNumber, INPUT_PULLUP);
      Serial.printf("Pin %d (%s) als INPUT konfiguriert\n", pin.pinNumber, pin.pinName.c_str());
    } 
    else if (pin.pinFunction == "output" || pin.pinFunction == "relay") {
      pinMode(pin.pinNumber, OUTPUT);
      
      int state = pin.initialState;
      if (pin.inverted) state = !state;
      
      digitalWrite(pin.pinNumber, state);
      
      Serial.printf("Pin %d (%s) als %s konfiguriert, Zustand: %d\n", 
                    pin.pinNumber, pin.pinName.c_str(), 
                    pin.pinFunction.c_str(), pin.initialState);
    }
  }
}

// Aktive Pin-Konfiguration aus gespeicherter Konfiguration laden
void loadPinConfiguration() {
  numActivePins = config.numConfiguredPins;
  
  // Begrenze die maximale Anzahl an Pins
  if (numActivePins > 12) numActivePins = 12;
  
  for (int i = 0; i < numActivePins; i++) {
    activePins[i].pinNumber = config.pins[i].pinNumber;
    activePins[i].pinName = String(config.pins[i].pinName);
    activePins[i].pinFunction = String(config.pins[i].pinFunction);
    activePins[i].inverted = config.pins[i].inverted;
    activePins[i].initialState = config.pins[i].initialState;
  }
  
  Serial.printf("%d Pins aus Konfiguration geladen\n", numActivePins);
}

// Konfiguration aus EEPROM laden
bool loadConfiguration() {
  // Konfiguration aus EEPROM lesen
  uint8_t* p = (uint8_t*)&config;
  for (unsigned int i = 0; i < sizeof(config); i++) {
    *p++ = EEPROM.read(CONFIG_START + i);
  }
  
  // Überprüfen, ob die Konfiguration gültig ist
  if (String(config.version) != String(CONFIG_VERSION)) {
    // Konfiguration ist nicht gültig oder existiert noch nicht
    return false;
  }
  
  return true;
}

// Konfiguration im EEPROM speichern
void saveConfiguration() {
  // Konfigurationsversion setzen
  strncpy(config.version, CONFIG_VERSION, 4);
  
  // Konfiguration ins EEPROM schreiben
  uint8_t* p = (uint8_t*)&config;
  for (unsigned int i = 0; i < sizeof(config); i++) {
    EEPROM.write(CONFIG_START + i, *p++);
  }
  
  // EEPROM-Änderungen speichern
  EEPROM.commit();
  Serial.println("Konfiguration gespeichert");
}

// Mit WLAN verbinden
void connectWiFi() {
  Serial.println("Verbinde mit WLAN...");
  if (displayAvailable) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Verbinde mit WLAN");
    display.println(ssid);
    display.display();
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    
    if (displayAvailable) {
      display.print(".");
      if (attempts % 10 == 9) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Verbinde mit WLAN");
        display.println(ssid);
      }
      display.display();
    }
    
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("");
    Serial.print("Verbunden mit IP: ");
    Serial.println(WiFi.localIP());
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WLAN verbunden");
      display.print("IP: ");
      display.println(WiFi.localIP().toString());
      display.display();
    }
    
    // Verbindungsbestätigung mit LED
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(100);
      digitalWrite(LED_PIN, LED_OFF);
      delay(100);
    }
  } else {
    wifiConnected = false;
    Serial.println("");
    Serial.println("WLAN-Verbindung fehlgeschlagen!");
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WLAN-Fehler!");
      display.println("Starte Access Point");
      display.display();
      delay(2000);
    }
  }
}

// Access Point starten
void setupAccessPoint() {
  // AP-SSID mit Gerätename + Chip-ID
  apSsid = String(config.deviceName);
  
  Serial.print("Starte Access Point: ");
  Serial.println(apSsid);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid.c_str(), apPassword.c_str());
  
  Serial.print("AP IP-Adresse: ");
  Serial.println(WiFi.softAPIP());
  
  if (displayAvailable) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Access Point aktiv");
    display.println(apSsid);
    display.print("IP: ");
    display.println(WiFi.softAPIP().toString());
    display.println("PW: " + apPassword);
    display.display();
  }
}

// OTA-Updates einrichten
void setupOTA() {
  // OTA-Konfiguration
  ArduinoOTA.setHostname(hostname.c_str());
  
  // Falls gewünscht, OTA-Passwort setzen
  // ArduinoOTA.setPassword("admin");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else {
      type = "Dateisystem";
    }
    
    Serial.println("Start OTA: " + type);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("OTA Update");
      display.println("Starte...");
      display.display();
    }
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update beendet");
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Update beendet");
      display.println("Neustart...");
      display.display();
    }
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percentage = (progress / (total / 100));
    Serial.printf("Fortschritt: %u%%\r", percentage);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("OTA Update");
      display.printf("%u%%\n", percentage);
      display.drawRect(0, 25, 128, 10, SSD1306_WHITE);
      display.fillRect(2, 27, (percentage * 124) / 100, 6, SSD1306_WHITE);
      display.display();
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Fehler [%u]: ", error);
    
    String errorMsg = "Unbekannter Fehler";
    
    if (error == OTA_AUTH_ERROR) {
      errorMsg = "Authentifizierung";
    } else if (error == OTA_BEGIN_ERROR) {
      errorMsg = "Begin fehlgeschlagen";
    } else if (error == OTA_CONNECT_ERROR) {
      errorMsg = "Verbindungsfehler";
    } else if (error == OTA_RECEIVE_ERROR) {
      errorMsg = "Empfangsfehler";
    } else if (error == OTA_END_ERROR) {
      errorMsg = "End fehlgeschlagen";
    }
    
    Serial.println(errorMsg);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("OTA FEHLER!");
      display.println(errorMsg);
      display.printf("Code: %u\n", error);
      display.display();
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA bereit");
}

// Display mit aktuellen Informationen aktualisieren
void updateDisplay() {
  if (!displayAvailable) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SwissAirDry");
  
  // WLAN- oder AP-Modus anzeigen
  if (WiFi.getMode() == WIFI_STA) {
    display.print("WLAN: ");
    if (wifiConnected) {
      display.println("Verbunden");
      display.print("IP: ");
      display.println(WiFi.localIP().toString());
    } else {
      display.println("Getrennt");
    }
  } else {
    display.println("AP: " + apSsid);
    display.print("IP: ");
    display.println(WiFi.softAPIP().toString());
  }
  
  // Trennlinie
  display.drawLine(0, 30, 127, 30, SSD1306_WHITE);
  
  // Konfigurationsstatus
  display.setCursor(0, 32);
  display.print("Pins: ");
  display.println(numActivePins);
  
  // Web-UI Info
  display.setCursor(0, 48);
  display.println("Web-Konfig aktiv");
  display.println("http://" + WiFi.localIP().toString());
  
  display.display();
}

// Hauptseite anzeigen
void handleRoot() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  String html = "<!DOCTYPE html>\n";
  html += "<html lang='de'>\n";
  html += "<head>\n";
  html += "  <meta charset='UTF-8'>\n";
  html += "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
  html += "  <title>SwissAirDry Konfiguration</title>\n";
  html += "  <link rel='stylesheet' href='/styles.css'>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <div class='container'>\n";
  html += "    <header>\n";
  html += "      <h1>SwissAirDry</h1>\n";
  html += "      <p>Gerätekonfiguration</p>\n";
  html += "    </header>\n";
  
  html += "    <nav>\n";
  html += "      <ul>\n";
  html += "        <li><a href='#' id='nav-dashboard'>Dashboard</a></li>\n";
  html += "        <li><a href='#' id='nav-pins'>Pin-Konfiguration</a></li>\n";
  html += "        <li><a href='#' id='nav-wifi'>WLAN-Einstellungen</a></li>\n";
  html += "        <li><a href='#' id='nav-api'>API-Einstellungen</a></li>\n";
  html += "        <li><a href='#' id='nav-system'>System</a></li>\n";
  html += "      </ul>\n";
  html += "    </nav>\n";
  
  html += "    <main id='content'>\n";
  html += "      <!-- Inhalt wird per JavaScript geladen -->\n";
  html += "      <div class='loading'>Lade...</div>\n";
  html += "    </main>\n";
  
  html += "    <footer>\n";
  html += "      <p>SwissAirDry " + hostname + " | IP: " + server.client().localIP().toString() + "</p>\n";
  html += "    </footer>\n";
  html += "  </div>\n";
  
  html += "  <script src='/script.js'></script>\n";
  html += "</body>\n";
  html += "</html>\n";
  
  server.send(200, "text/html", html);
}

// Stile für die Weboberfläche
void handleStyles() {
  String css = "* {\n";
  css += "  box-sizing: border-box;\n";
  css += "  margin: 0;\n";
  css += "  padding: 0;\n";
  css += "}\n";
  
  css += "body {\n";
  css += "  font-family: Arial, sans-serif;\n";
  css += "  background-color: #f5f5f5;\n";
  css += "  color: #333;\n";
  css += "}\n";
  
  css += ".container {\n";
  css += "  max-width: 1000px;\n";
  css += "  margin: 0 auto;\n";
  css += "  padding: 1rem;\n";
  css += "}\n";
  
  css += "header {\n";
  css += "  background-color: #0066cc;\n";
  css += "  color: white;\n";
  css += "  padding: 1rem;\n";
  css += "  margin-bottom: 1rem;\n";
  css += "  border-radius: 4px;\n";
  css += "}\n";
  
  css += "nav {\n";
  css += "  background-color: #f0f0f0;\n";
  css += "  border-radius: 4px;\n";
  css += "  margin-bottom: 1rem;\n";
  css += "}\n";
  
  css += "nav ul {\n";
  css += "  display: flex;\n";
  css += "  list-style: none;\n";
  css += "  overflow-x: auto;\n";
  css += "}\n";
  
  css += "nav li {\n";
  css += "  padding: 0.5rem 1rem;\n";
  css += "}\n";
  
  css += "nav a {\n";
  css += "  text-decoration: none;\n";
  css += "  color: #0066cc;\n";
  css += "  white-space: nowrap;\n";
  css += "}\n";
  
  css += "nav a:hover {\n";
  css += "  text-decoration: underline;\n";
  css += "}\n";
  
  css += "main {\n";
  css += "  background-color: white;\n";
  css += "  padding: 1rem;\n";
  css += "  border-radius: 4px;\n";
  css += "  box-shadow: 0 1px 3px rgba(0,0,0,0.1);\n";
  css += "  min-height: 300px;\n";
  css += "}\n";
  
  css += "footer {\n";
  css += "  text-align: center;\n";
  css += "  margin-top: 1rem;\n";
  css += "  font-size: 0.8rem;\n";
  css += "  color: #666;\n";
  css += "}\n";
  
  css += ".grid {\n";
  css += "  display: grid;\n";
  css += "  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));\n";
  css += "  gap: 1rem;\n";
  css += "}\n";
  
  css += ".card {\n";
  css += "  border: 1px solid #ddd;\n";
  css += "  border-radius: 4px;\n";
  css += "  padding: 1rem;\n";
  css += "}\n";
  
  css += ".form-group {\n";
  css += "  margin-bottom: 1rem;\n";
  css += "}\n";
  
  css += "label {\n";
  css += "  display: block;\n";
  css += "  margin-bottom: 0.5rem;\n";
  css += "  font-weight: bold;\n";
  css += "}\n";
  
  css += "input, select, textarea {\n";
  css += "  width: 100%;\n";
  css += "  padding: 0.5rem;\n";
  css += "  border: 1px solid #ddd;\n";
  css += "  border-radius: 4px;\n";
  css += "}\n";
  
  css += "button {\n";
  css += "  background-color: #0066cc;\n";
  css += "  color: white;\n";
  css += "  border: none;\n";
  css += "  padding: 0.5rem 1rem;\n";
  css += "  border-radius: 4px;\n";
  css += "  cursor: pointer;\n";
  css += "}\n";
  
  css += "button:hover {\n";
  css += "  background-color: #0055aa;\n";
  css += "}\n";
  
  css += ".pin-config {\n";
  css += "  margin-bottom: 1rem;\n";
  css += "  padding: 1rem;\n";
  css += "  border: 1px solid #ddd;\n";
  css += "  border-radius: 4px;\n";
  css += "}\n";
  
  css += ".loading {\n";
  css += "  text-align: center;\n";
  css += "  padding: 2rem;\n";
  css += "}\n";
  
  css += ".on {\n";
  css += "  background-color: #4CAF50;\n";
  css += "  color: white;\n";
  css += "}\n";
  
  css += ".off {\n";
  css += "  background-color: #f44336;\n";
  css += "  color: white;\n";
  css += "}\n";
  
  css += "@media (max-width: 600px) {\n";
  css += "  nav ul {\n";
  css += "    flex-direction: column;\n";
  css += "  }\n";
  css += "  .grid {\n";
  css += "    grid-template-columns: 1fr;\n";
  css += "  }\n";
  css += "}\n";
  
  server.send(200, "text/css", css);
}

// JavaScript für die Weboberfläche
void handleScript() {
  String js = "document.addEventListener('DOMContentLoaded', function() {\n";
  js += "  const content = document.getElementById('content');\n";
  js += "  const navDashboard = document.getElementById('nav-dashboard');\n";
  js += "  const navPins = document.getElementById('nav-pins');\n";
  js += "  const navWifi = document.getElementById('nav-wifi');\n";
  js += "  const navApi = document.getElementById('nav-api');\n";
  js += "  const navSystem = document.getElementById('nav-system');\n";
  
  js += "  // Standardansicht laden\n";
  js += "  loadDashboard();\n";
  
  js += "  // Navigationsereignisse\n";
  js += "  navDashboard.addEventListener('click', function(e) {\n";
  js += "    e.preventDefault();\n";
  js += "    loadDashboard();\n";
  js += "  });\n";
  
  js += "  navPins.addEventListener('click', function(e) {\n";
  js += "    e.preventDefault();\n";
  js += "    loadPinConfig();\n";
  js += "  });\n";
  
  js += "  navWifi.addEventListener('click', function(e) {\n";
  js += "    e.preventDefault();\n";
  js += "    loadWifiConfig();\n";
  js += "  });\n";
  
  js += "  navApi.addEventListener('click', function(e) {\n";
  js += "    e.preventDefault();\n";
  js += "    loadApiConfig();\n";
  js += "  });\n";
  
  js += "  navSystem.addEventListener('click', function(e) {\n";
  js += "    e.preventDefault();\n";
  js += "    loadSystemInfo();\n";
  js += "  });\n";
  
  js += "  // Dashboard laden\n";
  js += "  function loadDashboard() {\n";
  js += "    content.innerHTML = '<div class=\"loading\">Lade Dashboard...</div>';\n";
  js += "    fetch('/api/status')\n";
  js += "      .then(response => response.json())\n";
  js += "      .then(data => {\n";
  js += "        let html = '<h2>Dashboard</h2>';\n";
  js += "        html += '<div class=\"card\">';\n";
  js += "        html += `<h3>Gerät: ${data.deviceName}</h3>`;\n";
  js += "        html += `<p>Hostname: ${data.hostname}</p>`;\n";
  js += "        html += `<p>IP: ${data.ip}</p>`;\n";
  js += "        html += `<p>WLAN: ${data.connected ? 'Verbunden mit ' + data.ssid : 'Getrennt'}</p>`;\n";
  js += "        html += `<p>RSSI: ${data.rssi} dBm</p>`;\n";
  js += "        html += `<p>Uptime: ${formatUptime(data.uptime)}</p>`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<h3>Konfigurierte Pins</h3>';\n";
  js += "        html += '<div class=\"grid\">';\n";
  
  js += "        data.pins.forEach(pin => {\n";
  js += "          html += `<div class=\"card\">`;\n";
  js += "          html += `<h3>${pin.pinName}</h3>`;\n";
  js += "          html += `<p>GPIO ${pin.pinNumber} (${pin.pinFunction})</p>`;\n";
  
  js += "          if (pin.pinFunction === 'output' || pin.pinFunction === 'relay') {\n";
  js += "            const stateClass = pin.state ? 'on' : 'off';\n";
  js += "            html += `<button class=\"toggle-pin ${stateClass}\" data-pin=\"${pin.pinNumber}\">`;\n";
  js += "            html += pin.state ? 'AN' : 'AUS';\n";
  js += "            html += '</button>';\n";
  js += "          } else if (pin.pinFunction === 'input') {\n";
  js += "            const stateClass = pin.state ? 'on' : 'off';\n";
  js += "            html += `<div class=\"pin-state ${stateClass}\">${pin.state ? 'HOCH' : 'TIEF'}</div>`;\n";
  js += "          }\n";
  
  js += "          html += '</div>';\n";
  js += "        });\n";
  
  js += "        html += '</div>';\n";
  
  js += "        content.innerHTML = html;\n";
  
  js += "        // Pin-Schalterereignisse hinzufügen\n";
  js += "        document.querySelectorAll('.toggle-pin').forEach(button => {\n";
  js += "          button.addEventListener('click', function() {\n";
  js += "            const pinNumber = this.dataset.pin;\n";
  js += "            const newState = !this.classList.contains('on');\n";
  js += "            togglePin(pinNumber, newState, this);\n";
  js += "          });\n";
  js += "        });\n";
  
  js += "        // Regelmäßiges Update der Input-Zustände\n";
  js += "        setInterval(updateInputStates, 2000);\n";
  js += "      })\n";
  js += "      .catch(error => {\n";
  js += "        content.innerHTML = `<div class=\"error\">Fehler beim Laden des Dashboards: ${error.message}</div>`;\n";
  js += "      });\n";
  js += "  }\n";
  
  js += "  // Pin-Konfiguration laden\n";
  js += "  function loadPinConfig() {\n";
  js += "    content.innerHTML = '<div class=\"loading\">Lade Pin-Konfiguration...</div>';\n";
  js += "    fetch('/api/pins')\n";
  js += "      .then(response => response.json())\n";
  js += "      .then(data => {\n";
  js += "        let html = '<h2>Pin-Konfiguration</h2>';\n";
  js += "        html += '<form id=\"pin-config-form\">';\n";
  
  js += "        data.pins.forEach((pin, index) => {\n";
  js += "          html += `<div class=\"pin-config\">`;\n";
  js += "          html += `<h3>Pin ${index + 1}</h3>`;\n";
  
  js += "          html += `<div class=\"form-group\">`;\n";
  js += "          html += `<label for=\"pin-number-${index}\">GPIO-Nummer:</label>`;\n";
  js += "          html += `<select id=\"pin-number-${index}\" name=\"pins[${index}][pinNumber]\">`;\n";
  
  // Verfügbare GPIOs für Wemos D1 Mini / ESP8266
  js += "          const availablePins = [0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16];\n";
  js += "          availablePins.forEach(gpioNum => {\n";
  js += "            const selected = pin.pinNumber === gpioNum ? 'selected' : '';\n";
  js += "            const pinName = getPinName(gpioNum);\n";
  js += "            html += `<option value=\"${gpioNum}\" ${selected}>GPIO${gpioNum} ${pinName}</option>`;\n";
  js += "          });\n";
  js += "          html += `</select>`;\n";
  js += "          html += `</div>`;\n";
  
  js += "          html += `<div class=\"form-group\">`;\n";
  js += "          html += `<label for=\"pin-name-${index}\">Bezeichnung:</label>`;\n";
  js += "          html += `<input type=\"text\" id=\"pin-name-${index}\" name=\"pins[${index}][pinName]\" value=\"${pin.pinName}\">`;\n";
  js += "          html += `</div>`;\n";
  
  js += "          html += `<div class=\"form-group\">`;\n";
  js += "          html += `<label for=\"pin-function-${index}\">Funktion:</label>`;\n";
  js += "          html += `<select id=\"pin-function-${index}\" name=\"pins[${index}][pinFunction]\">`;\n";
  js += "          html += `<option value=\"input\" ${pin.pinFunction === 'input' ? 'selected' : ''}>Eingang</option>`;\n";
  js += "          html += `<option value=\"output\" ${pin.pinFunction === 'output' ? 'selected' : ''}>Ausgang</option>`;\n";
  js += "          html += `<option value=\"relay\" ${pin.pinFunction === 'relay' ? 'selected' : ''}>Relais</option>`;\n";
  js += "          html += `</select>`;\n";
  js += "          html += `</div>`;\n";
  
  js += "          html += `<div class=\"form-group\">`;\n";
  js += "          html += `<label>`;\n";
  js += "          html += `<input type=\"checkbox\" id=\"pin-inverted-${index}\" name=\"pins[${index}][inverted]\" ${pin.inverted ? 'checked' : ''}>`;\n";
  js += "          html += `Invertierte Logik`;\n";
  js += "          html += `</label>`;\n";
  js += "          html += `</div>`;\n";
  
  js += "          html += `<div class=\"form-group\">`;\n";
  js += "          html += `<label for=\"pin-initial-state-${index}\">Anfangszustand:</label>`;\n";
  js += "          html += `<select id=\"pin-initial-state-${index}\" name=\"pins[${index}][initialState]\">`;\n";
  js += "          html += `<option value=\"0\" ${pin.initialState === 0 ? 'selected' : ''}>LOW (0)</option>`;\n";
  js += "          html += `<option value=\"1\" ${pin.initialState === 1 ? 'selected' : ''}>HIGH (1)</option>`;\n";
  js += "          html += `</select>`;\n";
  js += "          html += `</div>`;\n";
  
  js += "          html += `</div>`;\n"; // End pin-config
  js += "        });\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<button type=\"button\" id=\"add-pin\">Pin hinzufügen</button> ';\n";
  js += "        html += '<button type=\"submit\">Speichern</button>';\n";
  js += "        html += '</div>';\n";
  js += "        html += '</form>';\n";
  
  js += "        content.innerHTML = html;\n";
  
  js += "        // Formularereignis hinzufügen\n";
  js += "        document.getElementById('pin-config-form').addEventListener('submit', function(e) {\n";
  js += "          e.preventDefault();\n";
  js += "          savePinConfig(this);\n";
  js += "        });\n";
  
  js += "        // Hinzufügen-Button-Ereignis\n";
  js += "        document.getElementById('add-pin').addEventListener('click', function() {\n";
  js += "          addNewPin();\n";
  js += "        });\n";
  js += "      })\n";
  js += "      .catch(error => {\n";
  js += "        content.innerHTML = `<div class=\"error\">Fehler beim Laden der Pin-Konfiguration: ${error.message}</div>`;\n";
  js += "      });\n";
  js += "  }\n";
  
  js += "  // WLAN-Konfiguration laden\n";
  js += "  function loadWifiConfig() {\n";
  js += "    content.innerHTML = '<div class=\"loading\">Lade WLAN-Konfiguration...</div>';\n";
  js += "    fetch('/api/wifi')\n";
  js += "      .then(response => response.json())\n";
  js += "      .then(data => {\n";
  js += "        let html = '<h2>WLAN-Konfiguration</h2>';\n";
  js += "        html += '<form id=\"wifi-config-form\">';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"device-name\">Gerätename:</label>';\n";
  js += "        html += `<input type=\"text\" id=\"device-name\" name=\"deviceName\" value=\"${data.deviceName}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"ssid\">WLAN-SSID:</label>';\n";
  js += "        html += `<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"${data.ssid}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"password\">WLAN-Passwort:</label>';\n";
  js += "        html += `<input type=\"password\" id=\"password\" name=\"password\" value=\"${data.password}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label>';\n";
  js += "        html += `<input type=\"checkbox\" id=\"ap-mode\" name=\"apMode\" ${data.apMode ? 'checked' : ''}>`;\n";
  js += "        html += 'Access Point Modus aktivieren';\n";
  js += "        html += '</label>';\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"web-password\">Web-UI Passwort:</label>';\n";
  js += "        html += `<input type=\"password\" id=\"web-password\" name=\"webPassword\" value=\"${data.webPassword}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<button type=\"submit\">Speichern</button>';\n";
  js += "        html += '</div>';\n";
  js += "        html += '</form>';\n";
  
  js += "        content.innerHTML = html;\n";
  
  js += "        // Formularereignis hinzufügen\n";
  js += "        document.getElementById('wifi-config-form').addEventListener('submit', function(e) {\n";
  js += "          e.preventDefault();\n";
  js += "          saveWifiConfig(this);\n";
  js += "        });\n";
  js += "      })\n";
  js += "      .catch(error => {\n";
  js += "        content.innerHTML = `<div class=\"error\">Fehler beim Laden der WLAN-Konfiguration: ${error.message}</div>`;\n";
  js += "      });\n";
  js += "  }\n";
  
  js += "  // API-Konfiguration laden\n";
  js += "  function loadApiConfig() {\n";
  js += "    content.innerHTML = '<div class=\"loading\">Lade API-Konfiguration...</div>';\n";
  js += "    fetch('/api/apiconfig')\n";
  js += "      .then(response => response.json())\n";
  js += "      .then(data => {\n";
  js += "        let html = '<h2>API-Konfiguration</h2>';\n";
  js += "        html += '<form id=\"api-config-form\">';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"primary-api-url\">Primäre API-URL:</label>';\n";
  js += "        html += `<input type=\"text\" id=\"primary-api-url\" name=\"primaryApiUrl\" value=\"${data.primaryApiUrl}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"backup-api-url\">Backup API-URL:</label>';\n";
  js += "        html += `<input type=\"text\" id=\"backup-api-url\" name=\"backupApiUrl\" value=\"${data.backupApiUrl}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"api-key\">API-Schlüssel:</label>';\n";
  js += "        html += `<input type=\"text\" id=\"api-key\" name=\"apiKey\" value=\"${data.apiKey}\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<label for=\"update-interval\">Update-Intervall (Sekunden):</label>';\n";
  js += "        html += `<input type=\"number\" id=\"update-interval\" name=\"updateInterval\" value=\"${data.updateInterval}\" min=\"10\" max=\"3600\">`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<button type=\"submit\">Speichern</button>';\n";
  js += "        html += '</div>';\n";
  js += "        html += '</form>';\n";
  
  js += "        content.innerHTML = html;\n";
  
  js += "        // Formularereignis hinzufügen\n";
  js += "        document.getElementById('api-config-form').addEventListener('submit', function(e) {\n";
  js += "          e.preventDefault();\n";
  js += "          saveApiConfig(this);\n";
  js += "        });\n";
  js += "      })\n";
  js += "      .catch(error => {\n";
  js += "        content.innerHTML = `<div class=\"error\">Fehler beim Laden der API-Konfiguration: ${error.message}</div>`;\n";
  js += "      });\n";
  js += "  }\n";
  
  js += "  // Systeminformationen laden\n";
  js += "  function loadSystemInfo() {\n";
  js += "    content.innerHTML = '<div class=\"loading\">Lade Systeminformationen...</div>';\n";
  js += "    fetch('/api/status')\n";
  js += "      .then(response => response.json())\n";
  js += "      .then(data => {\n";
  js += "        let html = '<h2>Systeminformationen</h2>';\n";
  
  js += "        html += '<div class=\"card\">';\n";
  js += "        html += `<h3>Hardware</h3>`;\n";
  js += "        html += `<p>Chip-ID: ${data.chipId}</p>`;\n";
  js += "        html += `<p>Flash-Größe: ${(data.flashSize / 1024 / 1024).toFixed(2)} MB</p>`;\n";
  js += "        html += `<p>Freier Speicher: ${(data.freeHeap / 1024).toFixed(2)} KB</p>`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"card\">';\n";
  js += "        html += `<h3>Software</h3>`;\n";
  js += "        html += `<p>Firmware-Version: ${data.firmwareVersion}</p>`;\n";
  js += "        html += `<p>SDK-Version: ${data.sdkVersion}</p>`;\n";
  js += "        html += `<p>Uptime: ${formatUptime(data.uptime)}</p>`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"card\">';\n";
  js += "        html += `<h3>Netzwerk</h3>`;\n";
  js += "        html += `<p>Hostname: ${data.hostname}</p>`;\n";
  js += "        html += `<p>IP-Adresse: ${data.ip}</p>`;\n";
  js += "        html += `<p>MAC-Adresse: ${data.mac}</p>`;\n";
  js += "        html += `<p>WLAN-SSID: ${data.ssid}</p>`;\n";
  js += "        html += `<p>RSSI: ${data.rssi} dBm</p>`;\n";
  js += "        html += '</div>';\n";
  
  js += "        html += '<div class=\"form-group\">';\n";
  js += "        html += '<button id=\"btn-restart\">Neustart</button> ';\n";
  js += "        html += '<button id=\"btn-factory-reset\">Werkseinstellungen</button>';\n";
  js += "        html += '</div>';\n";
  
  js += "        content.innerHTML = html;\n";
  
  js += "        // Button-Ereignisse hinzufügen\n";
  js += "        document.getElementById('btn-restart').addEventListener('click', function() {\n";
  js += "          if (confirm('Gerät wirklich neu starten?')) {\n";
  js += "            fetch('/api/restart', { method: 'POST' })\n";
  js += "              .then(() => {\n";
  js += "                alert('Gerät wird neu gestartet...');\n";
  js += "              })\n";
  js += "              .catch(error => {\n";
  js += "                alert(`Fehler: ${error.message}`);\n";
  js += "              });\n";
  js += "          }\n";
  js += "        });\n";
  
  js += "        document.getElementById('btn-factory-reset').addEventListener('click', function() {\n";
  js += "          if (confirm('Wirklich auf Werkseinstellungen zurücksetzen? Alle Einstellungen gehen verloren!')) {\n";
  js += "            fetch('/api/factory-reset', { method: 'POST' })\n";
  js += "              .then(() => {\n";
  js += "                alert('Gerät wird zurückgesetzt und neu gestartet...');\n";
  js += "              })\n";
  js += "              .catch(error => {\n";
  js += "                alert(`Fehler: ${error.message}`);\n";
  js += "              });\n";
  js += "          }\n";
  js += "        });\n";
  js += "      })\n";
  js += "      .catch(error => {\n";
  js += "        content.innerHTML = `<div class=\"error\">Fehler beim Laden der Systeminformationen: ${error.message}</div>`;\n";
  js += "      });\n";
  js += "  }\n";
  
  js += "  // Pin-Zustand umschalten\n";
  js += "  function togglePin(pinNumber, newState, button) {\n";
  js += "    fetch('/api/pins', {\n";
  js += "      method: 'POST',\n";
  js += "      headers: {\n";
  js += "        'Content-Type': 'application/json',\n";
  js += "      },\n";
  js += "      body: JSON.stringify({\n";
  js += "        action: 'toggle',\n";
  js += "        pinNumber: parseInt(pinNumber),\n";
  js += "        state: newState\n";
  js += "      })\n";
  js += "    })\n";
  js += "    .then(response => response.json())\n";
  js += "    .then(data => {\n";
  js += "      if (data.success) {\n";
  js += "        button.classList.toggle('on');\n";
  js += "        button.classList.toggle('off');\n";
  js += "        button.textContent = data.state ? 'AN' : 'AUS';\n";
  js += "      } else {\n";
  js += "        alert(`Fehler: ${data.message}`);\n";
  js += "      }\n";
  js += "    })\n";
  js += "    .catch(error => {\n";
  js += "      alert(`Fehler beim Schalten des Pins: ${error.message}`);\n";
  js += "    });\n";
  js += "  }\n";
  
  js += "  // Pin-Konfiguration speichern\n";
  js += "  function savePinConfig(form) {\n";
  js += "    const formData = new FormData(form);\n";
  js += "    const pins = [];\n";
  js += "    \n";
  js += "    // Pins sammeln\n";
  js += "    const pinConfigs = document.querySelectorAll('.pin-config');\n";
  js += "    \n";
  js += "    pinConfigs.forEach((config, index) => {\n";
  js += "      const pinNumber = parseInt(document.getElementById(`pin-number-${index}`).value);\n";
  js += "      const pinName = document.getElementById(`pin-name-${index}`).value;\n";
  js += "      const pinFunction = document.getElementById(`pin-function-${index}`).value;\n";
  js += "      const inverted = document.getElementById(`pin-inverted-${index}`).checked;\n";
  js += "      const initialState = parseInt(document.getElementById(`pin-initial-state-${index}`).value);\n";
  js += "      \n";
  js += "      pins.push({\n";
  js += "        pinNumber,\n";
  js += "        pinName,\n";
  js += "        pinFunction,\n";
  js += "        inverted,\n";
  js += "        initialState\n";
  js += "      });\n";
  js += "    });\n";
  js += "    \n";
  js += "    fetch('/api/pins', {\n";
  js += "      method: 'POST',\n";
  js += "      headers: {\n";
  js += "        'Content-Type': 'application/json',\n";
  js += "      },\n";
  js += "      body: JSON.stringify({\n";
  js += "        action: 'config',\n";
  js += "        pins\n";
  js += "      })\n";
  js += "    })\n";
  js += "    .then(response => response.json())\n";
  js += "    .then(data => {\n";
  js += "      if (data.success) {\n";
  js += "        alert('Pin-Konfiguration gespeichert. Gerät wird neu gestartet.');\n";
  js += "        // Kurze Verzögerung vor dem Neuladen, um dem Gerät Zeit zum Neustarten zu geben\n";
  js += "        setTimeout(() => {\n";
  js += "          window.location.reload();\n";
  js += "        }, 5000);\n";
  js += "      } else {\n";
  js += "        alert(`Fehler: ${data.message}`);\n";
  js += "      }\n";
  js += "    })\n";
  js += "    .catch(error => {\n";
  js += "      alert(`Fehler beim Speichern der Pin-Konfiguration: ${error.message}`);\n";
  js += "    });\n";
  js += "  }\n";
  
  js += "  // WLAN-Konfiguration speichern\n";
  js += "  function saveWifiConfig(form) {\n";
  js += "    const formData = new FormData(form);\n";
  js += "    const config = {\n";
  js += "      deviceName: formData.get('deviceName'),\n";
  js += "      ssid: formData.get('ssid'),\n";
  js += "      password: formData.get('password'),\n";
  js += "      apMode: formData.get('apMode') === 'on',\n";
  js += "      webPassword: formData.get('webPassword')\n";
  js += "    };\n";
  js += "    \n";
  js += "    fetch('/api/wifi', {\n";
  js += "      method: 'POST',\n";
  js += "      headers: {\n";
  js += "        'Content-Type': 'application/json',\n";
  js += "      },\n";
  js += "      body: JSON.stringify(config)\n";
  js += "    })\n";
  js += "    .then(response => response.json())\n";
  js += "    .then(data => {\n";
  js += "      if (data.success) {\n";
  js += "        alert('WLAN-Konfiguration gespeichert. Gerät wird neu gestartet.');\n";
  js += "        setTimeout(() => {\n";
  js += "          window.location.reload();\n";
  js += "        }, 5000);\n";
  js += "      } else {\n";
  js += "        alert(`Fehler: ${data.message}`);\n";
  js += "      }\n";
  js += "    })\n";
  js += "    .catch(error => {\n";
  js += "      alert(`Fehler beim Speichern der WLAN-Konfiguration: ${error.message}`);\n";
  js += "    });\n";
  js += "  }\n";
  
  js += "  // API-Konfiguration speichern\n";
  js += "  function saveApiConfig(form) {\n";
  js += "    const formData = new FormData(form);\n";
  js += "    const config = {\n";
  js += "      primaryApiUrl: formData.get('primaryApiUrl'),\n";
  js += "      backupApiUrl: formData.get('backupApiUrl'),\n";
  js += "      apiKey: formData.get('apiKey'),\n";
  js += "      updateInterval: parseInt(formData.get('updateInterval'))\n";
  js += "    };\n";
  js += "    \n";
  js += "    fetch('/api/apiconfig', {\n";
  js += "      method: 'POST',\n";
  js += "      headers: {\n";
  js += "        'Content-Type': 'application/json',\n";
  js += "      },\n";
  js += "      body: JSON.stringify(config)\n";
  js += "    })\n";
  js += "    .then(response => response.json())\n";
  js += "    .then(data => {\n";
  js += "      if (data.success) {\n";
  js += "        alert('API-Konfiguration gespeichert.');\n";
  js += "        loadApiConfig(); // Neu laden, um die aktualisierte Konfiguration anzuzeigen\n";
  js += "      } else {\n";
  js += "        alert(`Fehler: ${data.message}`);\n";
  js += "      }\n";
  js += "    })\n";
  js += "    .catch(error => {\n";
  js += "      alert(`Fehler beim Speichern der API-Konfiguration: ${error.message}`);\n";
  js += "    });\n";
  js += "  }\n";
  
  js += "  // Uptime formatieren\n";
  js += "  function formatUptime(seconds) {\n";
  js += "    const days = Math.floor(seconds / 86400);\n";
  js += "    const hours = Math.floor((seconds % 86400) / 3600);\n";
  js += "    const minutes = Math.floor((seconds % 3600) / 60);\n";
  js += "    const secs = seconds % 60;\n";
  js += "    \n";
  js += "    let result = '';\n";
  js += "    if (days > 0) result += `${days}d `;\n";
  js += "    if (hours > 0 || days > 0) result += `${hours}h `;\n";
  js += "    if (minutes > 0 || hours > 0 || days > 0) result += `${minutes}m `;\n";
  js += "    result += `${secs}s`;\n";
  js += "    \n";
  js += "    return result;\n";
  js += "  }\n";
  
  js += "  // GPIO-Pinnamen abrufen\n";
  js += "  function getPinName(gpioNum) {\n";
  js += "    const pinMap = {\n";
  js += "      0: '(D3)',\n";
  js += "      1: '(TX)',\n";
  js += "      2: '(D4/LED)',\n";
  js += "      3: '(RX)',\n";
  js += "      4: '(D2/SDA)',\n";
  js += "      5: '(D1/SCL)',\n";
  js += "      12: '(D6)',\n";
  js += "      13: '(D7)',\n";
  js += "      14: '(D5)',\n";
  js += "      15: '(D8)',\n";
  js += "      16: '(D0)'\n";
  js += "    };\n";
  js += "    \n";
  js += "    return pinMap[gpioNum] || '';\n";
  js += "  }\n";
  
  js += "  // Input-Zustände aktualisieren\n";
  js += "  function updateInputStates() {\n";
  js += "    const inputStates = document.querySelectorAll('.pin-state');\n";
  js += "    if (inputStates.length === 0) return;\n";
  js += "    \n";
  js += "    fetch('/api/status')\n";
  js += "      .then(response => response.json())\n";
  js += "      .then(data => {\n";
  js += "        const inputPins = data.pins.filter(pin => pin.pinFunction === 'input');\n";
  js += "        \n";
  js += "        inputStates.forEach((stateElement, index) => {\n";
  js += "          if (index < inputPins.length) {\n";
  js += "            const pin = inputPins[index];\n";
  js += "            stateElement.textContent = pin.state ? 'HOCH' : 'TIEF';\n";
  js += "            stateElement.className = `pin-state ${pin.state ? 'on' : 'off'}`;\n";
  js += "          }\n";
  js += "        });\n";
  js += "      })\n";
  js += "      .catch(error => {\n";
  js += "        console.error('Fehler beim Aktualisieren der Input-Zustände:', error);\n";
  js += "      });\n";
  js += "  }\n";
  
  js += "  // Neuen Pin zur Konfiguration hinzufügen\n";
  js += "  function addNewPin() {\n";
  js += "    const pinConfigs = document.querySelectorAll('.pin-config');\n";
  js += "    const index = pinConfigs.length;\n";
  js += "    \n";
  js += "    // Überprüfen, ob das Maximum erreicht ist\n";
  js += "    if (index >= 12) {\n";
  js += "      alert('Maximale Anzahl an konfigurierbaren Pins erreicht (12)');\n";
  js += "      return;\n";
  js += "    }\n";
  js += "    \n";
  js += "    const form = document.getElementById('pin-config-form');\n";
  js += "    const newPinConfig = document.createElement('div');\n";
  js += "    newPinConfig.className = 'pin-config';\n";
  js += "    \n";
  js += "    newPinConfig.innerHTML = `\n";
  js += "      <h3>Pin ${index + 1}</h3>\n";
  js += "      <div class=\"form-group\">\n";
  js += "        <label for=\"pin-number-${index}\">GPIO-Nummer:</label>\n";
  js += "        <select id=\"pin-number-${index}\" name=\"pins[${index}][pinNumber]\">\n";
  
  // Verfügbare GPIOs für ESP8266
  js += "          <option value=\"0\">GPIO0 (D3)</option>\n";
  js += "          <option value=\"1\">GPIO1 (TX)</option>\n";
  js += "          <option value=\"2\">GPIO2 (D4/LED)</option>\n";
  js += "          <option value=\"3\">GPIO3 (RX)</option>\n";
  js += "          <option value=\"4\">GPIO4 (D2/SDA)</option>\n";
  js += "          <option value=\"5\">GPIO5 (D1/SCL)</option>\n";
  js += "          <option value=\"12\">GPIO12 (D6)</option>\n";
  js += "          <option value=\"13\">GPIO13 (D7)</option>\n";
  js += "          <option value=\"14\">GPIO14 (D5)</option>\n";
  js += "          <option value=\"15\">GPIO15 (D8)</option>\n";
  js += "          <option value=\"16\">GPIO16 (D0)</option>\n";
  js += "        </select>\n";
  js += "      </div>\n";
  
  js += "      <div class=\"form-group\">\n";
  js += "        <label for=\"pin-name-${index}\">Bezeichnung:</label>\n";
  js += "        <input type=\"text\" id=\"pin-name-${index}\" name=\"pins[${index}][pinName]\" value=\"Pin ${index + 1}\">\n";
  js += "      </div>\n";
  
  js += "      <div class=\"form-group\">\n";
  js += "        <label for=\"pin-function-${index}\">Funktion:</label>\n";
  js += "        <select id=\"pin-function-${index}\" name=\"pins[${index}][pinFunction]\">\n";
  js += "          <option value=\"input\">Eingang</option>\n";
  js += "          <option value=\"output\">Ausgang</option>\n";
  js += "          <option value=\"relay\">Relais</option>\n";
  js += "        </select>\n";
  js += "      </div>\n";
  
  js += "      <div class=\"form-group\">\n";
  js += "        <label>\n";
  js += "          <input type=\"checkbox\" id=\"pin-inverted-${index}\" name=\"pins[${index}][inverted]\">\n";
  js += "          Invertierte Logik\n";
  js += "        </label>\n";
  js += "      </div>\n";
  
  js += "      <div class=\"form-group\">\n";
  js += "        <label for=\"pin-initial-state-${index}\">Anfangszustand:</label>\n";
  js += "        <select id=\"pin-initial-state-${index}\" name=\"pins[${index}][initialState]\">\n";
  js += "          <option value=\"0\">LOW (0)</option>\n";
  js += "          <option value=\"1\">HIGH (1)</option>\n";
  js += "        </select>\n";
  js += "      </div>\n";
  js += "    `;\n";
  
  js += "    // Vor dem Speichern-Button einfügen\n";
  js += "    const addPinButton = document.getElementById('add-pin');\n";
  js += "    form.insertBefore(newPinConfig, addPinButton.parentElement);\n";
  js += "  }\n";
  
  js += "});\n";
  
  server.send(200, "application/javascript", js);
}

// Pfad nicht gefunden
void handleNotFound() {
  String message = "404 Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}

// API: Gerätestatus abrufen
void handleStatus() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  DynamicJsonDocument doc(4096);
  
  // Geräteinformationen
  doc["hostname"] = hostname;
  doc["deviceName"] = config.deviceName;
  doc["chipId"] = String(ESP.getChipId(), HEX);
  doc["firmwareVersion"] = "1.0.0";
  doc["sdkVersion"] = ESP.getSdkVersion();
  doc["flashSize"] = ESP.getFlashChipRealSize();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  
  // Netzwerkinformationen
  if (WiFi.getMode() == WIFI_STA) {
    doc["ip"] = WiFi.localIP().toString();
    doc["connected"] = WiFi.status() == WL_CONNECTED;
    doc["ssid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["mac"] = WiFi.macAddress();
  } else {
    doc["ip"] = WiFi.softAPIP().toString();
    doc["connected"] = false;
    doc["ssid"] = apSsid;
    doc["rssi"] = 0;
    doc["mac"] = WiFi.softAPmacAddress();
  }
  
  // Pin-Informationen
  JsonArray pins = doc.createNestedArray("pins");
  
  for (int i = 0; i < numActivePins; i++) {
    JsonObject pin = pins.createNestedObject();
    
    pin["pinNumber"] = activePins[i].pinNumber;
    pin["pinName"] = activePins[i].pinName;
    pin["pinFunction"] = activePins[i].pinFunction;
    pin["inverted"] = activePins[i].inverted;
    
    // Aktuellen Zustand lesen
    if (activePins[i].pinFunction == "input") {
      int state = digitalRead(activePins[i].pinNumber);
      if (activePins[i].inverted) state = !state;
      pin["state"] = state;
    } else if (activePins[i].pinFunction == "output" || activePins[i].pinFunction == "relay") {
      int state = digitalRead(activePins[i].pinNumber);
      if (activePins[i].inverted) state = !state;
      pin["state"] = state;
    }
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// API: Pin-Konfiguration abrufen
void handleGetPins() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  DynamicJsonDocument doc(2048);
  JsonArray pins = doc.createNestedArray("pins");
  
  for (int i = 0; i < numActivePins; i++) {
    JsonObject pin = pins.createNestedObject();
    
    pin["pinNumber"] = activePins[i].pinNumber;
    pin["pinName"] = activePins[i].pinName;
    pin["pinFunction"] = activePins[i].pinFunction;
    pin["inverted"] = activePins[i].inverted;
    pin["initialState"] = activePins[i].initialState;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// API: Pin-Konfiguration setzen oder Pin-Zustand ändern
void handleSetPins() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.send(401, "application/json", "{\"success\":false,\"message\":\"Nicht authentifiziert\"}");
    return;
  }
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      String errorMsg = String("JSON-Fehler: ") + error.c_str();
      server.send(400, "application/json", "{\"success\":false,\"message\":\"" + errorMsg + "\"}");
      return;
    }
    
    // Aktion überprüfen
    if (doc.containsKey("action")) {
      String action = doc["action"].as<String>();
      
      if (action == "toggle") {
        // Pin-Zustand umschalten
        if (doc.containsKey("pinNumber") && doc.containsKey("state")) {
          int pinNumber = doc["pinNumber"];
          bool state = doc["state"];
          
          // Pin-Konfiguration suchen
          bool found = false;
          for (int i = 0; i < numActivePins; i++) {
            if (activePins[i].pinNumber == pinNumber && 
                (activePins[i].pinFunction == "output" || activePins[i].pinFunction == "relay")) {
              
              // Zustand setzen (unter Berücksichtigung der invertierten Logik)
              int writeState = state;
              if (activePins[i].inverted) writeState = !writeState;
              
              digitalWrite(pinNumber, writeState);
              
              // Erfolg zurückmelden
              String response = "{\"success\":true,\"pinNumber\":" + String(pinNumber) + ",\"state\":" + String(state) + "}";
              server.send(200, "application/json", response);
              found = true;
              break;
            }
          }
          
          if (!found) {
            server.send(404, "application/json", "{\"success\":false,\"message\":\"Pin nicht gefunden oder kein Output\"}");
          }
        } else {
          server.send(400, "application/json", "{\"success\":false,\"message\":\"Fehlende Parameter\"}");
        }
      } 
      else if (action == "config") {
        // Pin-Konfiguration aktualisieren
        if (doc.containsKey("pins")) {
          JsonArray pins = doc["pins"];
          
          // Anzahl der Pins überprüfen
          if (pins.size() == 0 || pins.size() > 12) {
            server.send(400, "application/json", "{\"success\":false,\"message\":\"Ungültige Anzahl an Pins (max. 12)\"}");
            return;
          }
          
          // Neue Pin-Konfiguration speichern
          config.numConfiguredPins = pins.size();
          
          for (size_t i = 0; i < pins.size(); i++) {
            JsonObject pin = pins[i];
            
            config.pins[i].pinNumber = pin["pinNumber"];
            strncpy(config.pins[i].pinName, pin["pinName"].as<String>().c_str(), 32);
            strncpy(config.pins[i].pinFunction, pin["pinFunction"].as<String>().c_str(), 16);
            config.pins[i].inverted = pin["inverted"];
            config.pins[i].initialState = pin["initialState"];
          }
          
          // Konfiguration speichern
          saveConfiguration();
          
          // Erfolg zurückmelden
          server.send(200, "application/json", "{\"success\":true,\"message\":\"Konfiguration gespeichert\"}");
          
          // Gerät neu starten, um neue Konfiguration anzuwenden
          delay(1000);
          ESP.restart();
        } else {
          server.send(400, "application/json", "{\"success\":false,\"message\":\"Fehlende Pin-Konfiguration\"}");
        }
      } else {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Ungültige Aktion\"}");
      }
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"Keine Aktion angegeben\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Keine JSON-Daten\"}");
  }
}

// API: WLAN-Konfiguration abrufen
void handleGetWifi() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  DynamicJsonDocument doc(1024);
  
  doc["deviceName"] = config.deviceName;
  doc["ssid"] = config.wifiSsid;
  doc["password"] = config.wifiPassword;
  doc["apMode"] = config.apModeEnabled;
  doc["webPassword"] = config.webPassword;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// API: WLAN-Konfiguration setzen
void handleSetWifi() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.send(401, "application/json", "{\"success\":false,\"message\":\"Nicht authentifiziert\"}");
    return;
  }
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      String errorMsg = String("JSON-Fehler: ") + error.c_str();
      server.send(400, "application/json", "{\"success\":false,\"message\":\"" + errorMsg + "\"}");
      return;
    }
    
    // Konfiguration aktualisieren
    if (doc.containsKey("deviceName")) {
      strncpy(config.deviceName, doc["deviceName"].as<String>().c_str(), 32);
    }
    
    if (doc.containsKey("ssid")) {
      strncpy(config.wifiSsid, doc["ssid"].as<String>().c_str(), 32);
    }
    
    if (doc.containsKey("password")) {
      strncpy(config.wifiPassword, doc["password"].as<String>().c_str(), 64);
    }
    
    if (doc.containsKey("apMode")) {
      config.apModeEnabled = doc["apMode"];
    }
    
    if (doc.containsKey("webPassword")) {
      strncpy(config.webPassword, doc["webPassword"].as<String>().c_str(), 32);
    }
    
    // Konfiguration speichern
    saveConfiguration();
    
    // Erfolg zurückmelden
    server.send(200, "application/json", "{\"success\":true,\"message\":\"WLAN-Konfiguration gespeichert\"}");
    
    // Gerät neu starten, um neue Konfiguration anzuwenden
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Keine JSON-Daten\"}");
  }
}

// API: API-Konfiguration abrufen
void handleGetApiConfig() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  DynamicJsonDocument doc(1024);
  
  doc["primaryApiUrl"] = config.primaryApiUrl;
  doc["backupApiUrl"] = config.backupApiUrl;
  doc["apiKey"] = config.apiKey;
  doc["updateInterval"] = config.updateInterval;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// API: API-Konfiguration setzen
void handleSetApiConfig() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.send(401, "application/json", "{\"success\":false,\"message\":\"Nicht authentifiziert\"}");
    return;
  }
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      String errorMsg = String("JSON-Fehler: ") + error.c_str();
      server.send(400, "application/json", "{\"success\":false,\"message\":\"" + errorMsg + "\"}");
      return;
    }
    
    // Konfiguration aktualisieren
    if (doc.containsKey("primaryApiUrl")) {
      strncpy(config.primaryApiUrl, doc["primaryApiUrl"].as<String>().c_str(), 128);
    }
    
    if (doc.containsKey("backupApiUrl")) {
      strncpy(config.backupApiUrl, doc["backupApiUrl"].as<String>().c_str(), 128);
    }
    
    if (doc.containsKey("apiKey")) {
      strncpy(config.apiKey, doc["apiKey"].as<String>().c_str(), 64);
    }
    
    if (doc.containsKey("updateInterval")) {
      config.updateInterval = doc["updateInterval"];
    }
    
    // Konfiguration speichern
    saveConfiguration();
    
    // Erfolg zurückmelden
    server.send(200, "application/json", "{\"success\":true,\"message\":\"API-Konfiguration gespeichert\"}");
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Keine JSON-Daten\"}");
  }
}

// API: Allgemeine Konfiguration abrufen
void handleGetConfig() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  DynamicJsonDocument doc(2048);
  
  doc["deviceName"] = config.deviceName;
  doc["hostname"] = hostname;
  
  doc["wifi"]["ssid"] = config.wifiSsid;
  doc["wifi"]["password"] = config.wifiPassword;
  doc["wifi"]["apMode"] = config.apModeEnabled;
  
  doc["api"]["primaryApiUrl"] = config.primaryApiUrl;
  doc["api"]["backupApiUrl"] = config.backupApiUrl;
  doc["api"]["apiKey"] = config.apiKey;
  doc["api"]["updateInterval"] = config.updateInterval;
  
  doc["security"]["webPassword"] = config.webPassword;
  
  JsonArray pins = doc.createNestedArray("pins");
  for (int i = 0; i < config.numConfiguredPins; i++) {
    JsonObject pin = pins.createNestedObject();
    
    pin["pinNumber"] = config.pins[i].pinNumber;
    pin["pinName"] = config.pins[i].pinName;
    pin["pinFunction"] = config.pins[i].pinFunction;
    pin["inverted"] = config.pins[i].inverted;
    pin["initialState"] = config.pins[i].initialState;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// API: Allgemeine Konfiguration setzen
void handleSetConfig() {
  // Prüfe Authentifizierung
  if (!authenticate()) {
    server.send(401, "application/json", "{\"success\":false,\"message\":\"Nicht authentifiziert\"}");
    return;
  }
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      String errorMsg = String("JSON-Fehler: ") + error.c_str();
      server.send(400, "application/json", "{\"success\":false,\"message\":\"" + errorMsg + "\"}");
      return;
    }
    
    // Konfiguration vollständig aktualisieren
    bool needsRestart = false;
    
    // Basiseinstellungen
    if (doc.containsKey("deviceName")) {
      strncpy(config.deviceName, doc["deviceName"].as<String>().c_str(), 32);
    }
    
    // WLAN-Einstellungen
    if (doc.containsKey("wifi")) {
      if (doc["wifi"].containsKey("ssid")) {
        strncpy(config.wifiSsid, doc["wifi"]["ssid"].as<String>().c_str(), 32);
        needsRestart = true;
      }
      
      if (doc["wifi"].containsKey("password")) {
        strncpy(config.wifiPassword, doc["wifi"]["password"].as<String>().c_str(), 64);
        needsRestart = true;
      }
      
      if (doc["wifi"].containsKey("apMode")) {
        config.apModeEnabled = doc["wifi"]["apMode"];
        needsRestart = true;
      }
    }
    
    // API-Einstellungen
    if (doc.containsKey("api")) {
      if (doc["api"].containsKey("primaryApiUrl")) {
        strncpy(config.primaryApiUrl, doc["api"]["primaryApiUrl"].as<String>().c_str(), 128);
      }
      
      if (doc["api"].containsKey("backupApiUrl")) {
        strncpy(config.backupApiUrl, doc["api"]["backupApiUrl"].as<String>().c_str(), 128);
      }
      
      if (doc["api"].containsKey("apiKey")) {
        strncpy(config.apiKey, doc["api"]["apiKey"].as<String>().c_str(), 64);
      }
      
      if (doc["api"].containsKey("updateInterval")) {
        config.updateInterval = doc["api"]["updateInterval"];
      }
    }
    
    // Sicherheitseinstellungen
    if (doc.containsKey("security")) {
      if (doc["security"].containsKey("webPassword")) {
        strncpy(config.webPassword, doc["security"]["webPassword"].as<String>().c_str(), 32);
      }
    }
    
    // Pin-Konfiguration
    if (doc.containsKey("pins")) {
      JsonArray pins = doc["pins"];
      
      if (pins.size() > 0 && pins.size() <= 12) {
        config.numConfiguredPins = pins.size();
        
        for (size_t i = 0; i < pins.size(); i++) {
          JsonObject pin = pins[i];
          
          config.pins[i].pinNumber = pin["pinNumber"];
          strncpy(config.pins[i].pinName, pin["pinName"].as<String>().c_str(), 32);
          strncpy(config.pins[i].pinFunction, pin["pinFunction"].as<String>().c_str(), 16);
          config.pins[i].inverted = pin["inverted"];
          config.pins[i].initialState = pin["initialState"];
        }
        
        needsRestart = true;
      }
    }
    
    // Konfiguration speichern
    saveConfiguration();
    
    // Erfolg zurückmelden
    if (needsRestart) {
      server.send(200, "application/json", "{\"success\":true,\"message\":\"Konfiguration gespeichert. Gerät wird neu gestartet.\"}");
      delay(1000);
      ESP.restart();
    } else {
      server.send(200, "application/json", "{\"success\":true,\"message\":\"Konfiguration gespeichert.\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Keine JSON-Daten\"}");
  }
}

// Authentifizierung prüfen
bool authenticate() {
  if (www_password.length() > 0) {
    return server.authenticate(www_username, www_password.c_str());
  }
  return true;
}

void loop() {
  // OTA-Updates verarbeiten, wenn aktiviert
  if (otaEnabled) {
    ArduinoOTA.handle();
  }
  
  // Webserver-Anfragen verarbeiten
  server.handleClient();
  
  // Regelmäßig den WiFi-Status prüfen
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 30000) {  // Alle 30 Sekunden
    lastWifiCheck = millis();
    
    if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED) {
      Serial.println("WLAN-Verbindung verloren, versuche neu zu verbinden...");
      WiFi.reconnect();
    }
  }
  
  // Heartbeat-LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 3000) {  // Alle 3 Sekunden
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlink = millis();
  }
  
  // Regelmäßig das Display aktualisieren
  static unsigned long lastDisplayUpdate = 0;
  if (displayAvailable && millis() - lastDisplayUpdate > 60000) {  // Jede Minute
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Wichtig für ESP8266: Watchdog füttern
  yield();
}