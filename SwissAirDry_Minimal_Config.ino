// SwissAirDry Minimal Konfiguration für ESP8266
// Optimiert für minimalen Speicherverbrauch

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <EEPROM.h>

// ----- KONFIGURATION -----
// Standard-WLAN einstellungen
char ssid[32] = "SSID";          // WLAN-Name
char password[64] = "PASSWORT";  // WLAN-Passwort
bool apMode = false;             // Access Point Modus

// Display-Konfiguration (feste Pins)
#define SDA_PIN 4  // D2 (GPIO4)
#define SCL_PIN 5  // D1 (GPIO5)

// LED-Pin
#define LED_PIN 2  // D4 (GPIO2) - blaue LED auf dem Board
#define LED_ON LOW
#define LED_OFF HIGH

// Gerätename
char deviceName[32] = "SwissAirDry";

// API-Konfiguration
char apiUrl[64] = "";
char apiKey[32] = "";

// Pin-Konfiguration: Name,Pin,Funktion,Aktiv
// Funktionen: 0=input, 1=output, 2=relay
// Max. 8 Pins werden unterstützt
uint8_t pinCount = 2;
uint8_t pinNums[8] = {14, 12, 0, 0, 0, 0, 0, 0};        // D5, D6
uint8_t pinFuncs[8] = {2, 0, 0, 0, 0, 0, 0, 0};         // relay, input
char pinNames[8][16] = {"Relais", "Sensor", "", "", "", "", "", ""};

// Webserver
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* www_username = "admin";
char www_password[16] = "admin";

// Globale Variablen
unsigned long lastBlinkTime = 0;
bool wifiConnected = false;
String hostname;

void setup() {
  Serial.begin(115200);
  Serial.println("\nSwissAirDry - Minimale Konfiguration");
  
  // LED initialisieren
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  
  // EEPROM initialisieren
  EEPROM.begin(512);
  
  // Konfiguration laden
  loadConfig();
  
  // Hostname festlegen
  uint16_t chipId = ESP.getChipId() & 0xFFFF;
  hostname = String(deviceName) + "-" + String(chipId, HEX);
  Serial.print("Hostname: ");
  Serial.println(hostname);
  
  // Pins konfigurieren
  setupPins();

  // Verbindungsmethode wählen
  if (apMode) {
    setupAP();
  } else {
    connectWiFi();
  }
  
  // OTA-Updates einrichten
  setupOTA();
  
  // Webserver-Routen einrichten
  setupWebServer();
  
  Serial.println("Setup abgeschlossen");
}

// Webserver einrichten
void setupWebServer() {
  // Hauptseite
  server.on("/", handleRoot);
  
  // Konfigurationsseite
  server.on("/config", handleConfig);
  
  // Pin-Steuerung
  server.on("/pins", handlePins);
  
  // API für Konfiguration speichern
  server.on("/save", HTTP_POST, handleSave);
  
  // OTA Web-Update einrichten
  httpUpdater.setup(&server, "/update", www_username, www_password);
  
  // Server starten
  server.begin();
  
  Serial.println("Webserver gestartet");
}

// Pins konfigurieren
void setupPins() {
  for (int i = 0; i < pinCount; i++) {
    if (pinNums[i] == 0) continue;
    
    // Funktionstyp konfigurieren
    switch (pinFuncs[i]) {
      case 0: // input
        pinMode(pinNums[i], INPUT_PULLUP);
        Serial.printf("Pin %d (%s) als INPUT konfiguriert\n", pinNums[i], pinNames[i]);
        break;
      case 1: // output
      case 2: // relay
        pinMode(pinNums[i], OUTPUT);
        digitalWrite(pinNums[i], LOW); // Standardmäßig aus
        Serial.printf("Pin %d (%s) als %s konfiguriert\n", 
                 pinNums[i], pinNames[i], 
                 pinFuncs[i] == 1 ? "OUTPUT" : "RELAY");
        break;
    }
  }
}

// Mit WLAN verbinden
void connectWiFi() {
  Serial.print("Verbinde mit WLAN ");
  Serial.print(ssid);
  Serial.println("...");
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname.c_str());
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.print("Verbunden mit IP: ");
    Serial.println(WiFi.localIP());
    
    // LED bestätigt
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(100);
      digitalWrite(LED_PIN, LED_OFF);
      delay(100);
    }
  } else {
    wifiConnected = false;
    Serial.println("WLAN-Verbindung fehlgeschlagen!");
    setupAP();
  }
}

// Access Point starten
void setupAP() {
  String apSsid = hostname;
  Serial.print("Starte Access Point: ");
  Serial.println(apSsid);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid.c_str(), "12345678");
  
  Serial.print("AP IP-Adresse: ");
  Serial.println(WiFi.softAPIP());
  
  // LED-Blinkmuster für AP-Modus
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    delay(50);
  }
}

// OTA einrichten
void setupOTA() {
  ArduinoOTA.setHostname(hostname.c_str());
  
  ArduinoOTA.onStart([]() {
    Serial.println("OTA: Start");
    digitalWrite(LED_PIN, LED_ON);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA: Ende");
    digitalWrite(LED_PIN, LED_OFF);
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Fehler [%u]\n", error);
  });
  
  ArduinoOTA.begin();
}

// Konfiguration laden
void loadConfig() {
  // Magische Nummer prüfen
  if (EEPROM.read(0) == 'S' && 
      EEPROM.read(1) == 'A' && 
      EEPROM.read(2) == 'D') {
    
    Serial.println("Lade Konfiguration aus EEPROM");
    
    int addr = 3;
    
    // WLAN-Einstellungen
    for (int i = 0; i < 32; i++) {
      ssid[i] = EEPROM.read(addr++);
    }
    
    for (int i = 0; i < 64; i++) {
      password[i] = EEPROM.read(addr++);
    }
    
    apMode = EEPROM.read(addr++) == 1;
    
    // Gerätename
    for (int i = 0; i < 32; i++) {
      deviceName[i] = EEPROM.read(addr++);
    }
    
    // Web-Passwort
    for (int i = 0; i < 16; i++) {
      www_password[i] = EEPROM.read(addr++);
    }
    
    // Pin-Konfiguration
    pinCount = EEPROM.read(addr++);
    if (pinCount > 8) pinCount = 8;
    
    for (int i = 0; i < 8; i++) {
      pinNums[i] = EEPROM.read(addr++);
      pinFuncs[i] = EEPROM.read(addr++);
      
      for (int j = 0; j < 16; j++) {
        pinNames[i][j] = EEPROM.read(addr++);
      }
    }
    
    // API-Konfiguration
    for (int i = 0; i < 64; i++) {
      apiUrl[i] = EEPROM.read(addr++);
    }
    
    for (int i = 0; i < 32; i++) {
      apiKey[i] = EEPROM.read(addr++);
    }
    
    Serial.println("Konfiguration geladen");
  } else {
    Serial.println("Keine Konfiguration gefunden, verwende Standardwerte");
  }
}

// Konfiguration speichern
void saveConfig() {
  Serial.println("Speichere Konfiguration im EEPROM");
  
  int addr = 0;
  
  // Magische Nummer
  EEPROM.write(addr++, 'S');
  EEPROM.write(addr++, 'A');
  EEPROM.write(addr++, 'D');
  
  // WLAN-Einstellungen
  for (int i = 0; i < 32; i++) {
    EEPROM.write(addr++, ssid[i]);
  }
  
  for (int i = 0; i < 64; i++) {
    EEPROM.write(addr++, password[i]);
  }
  
  EEPROM.write(addr++, apMode ? 1 : 0);
  
  // Gerätename
  for (int i = 0; i < 32; i++) {
    EEPROM.write(addr++, deviceName[i]);
  }
  
  // Web-Passwort
  for (int i = 0; i < 16; i++) {
    EEPROM.write(addr++, www_password[i]);
  }
  
  // Pin-Konfiguration
  EEPROM.write(addr++, pinCount);
  
  for (int i = 0; i < 8; i++) {
    EEPROM.write(addr++, pinNums[i]);
    EEPROM.write(addr++, pinFuncs[i]);
    
    for (int j = 0; j < 16; j++) {
      EEPROM.write(addr++, pinNames[i][j]);
    }
  }
  
  // API-Konfiguration
  for (int i = 0; i < 64; i++) {
    EEPROM.write(addr++, apiUrl[i]);
  }
  
  for (int i = 0; i < 32; i++) {
    EEPROM.write(addr++, apiKey[i]);
  }
  
  EEPROM.commit();
  Serial.println("Konfiguration gespeichert");
}

// Hauptseite anzeigen
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f0f0f0;}";
  html += ".container{max-width:800px;margin:0 auto;padding:15px;}";
  html += "h1{color:#0066cc;}";
  html += "table{width:100%;border-collapse:collapse;margin:15px 0;}";
  html += "th,td{border:1px solid #ddd;padding:8px;text-align:left;}";
  html += "th{background:#0066cc;color:white;}";
  html += "tr:nth-child(even){background:#f9f9f9;}";
  html += ".btn{background:#0066cc;color:white;border:none;padding:8px 16px;border-radius:4px;cursor:pointer;text-decoration:none;display:inline-block;margin:5px;}";
  html += ".btn:hover{background:#0055aa;}";
  html += ".on{background:#4CAF50;}.off{background:#f44336;}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>SwissAirDry - " + hostname + "</h1>";
  
  // Status anzeigen
  html += "<h2>Status</h2>";
  html += "<p>IP: " + (WiFi.getMode() == WIFI_STA ? WiFi.localIP().toString() : WiFi.softAPIP().toString()) + "</p>";
  html += "<p>WLAN: " + String(WiFi.getMode() == WIFI_STA ? String(ssid) + " (" + String(WiFi.RSSI()) + " dBm)" : "Access Point aktiv") + "</p>";
  
  // Pin-Steuerung
  html += "<h2>Pin-Steuerung</h2>";
  html += "<table>";
  html += "<tr><th>Name</th><th>Pin</th><th>Typ</th><th>Status</th><th>Aktion</th></tr>";
  
  for (int i = 0; i < pinCount; i++) {
    if (pinNums[i] == 0) continue;
    
    html += "<tr>";
    html += "<td>" + String(pinNames[i]) + "</td>";
    html += "<td>GPIO" + String(pinNums[i]) + "</td>";
    
    // Typ anzeigen
    String type;
    switch (pinFuncs[i]) {
      case 0: type = "Eingang"; break;
      case 1: type = "Ausgang"; break;
      case 2: type = "Relais"; break;
      default: type = "Unbekannt";
    }
    html += "<td>" + type + "</td>";
    
    // Status anzeigen
    int state = digitalRead(pinNums[i]);
    String stateText = (pinFuncs[i] == 0) ? 
                        (state == HIGH ? "HOCH" : "TIEF") : 
                        (state == HIGH ? "AN" : "AUS");
    
    html += "<td>" + stateText + "</td>";
    
    // Aktion anzeigen (nur für Ausgänge und Relais)
    if (pinFuncs[i] == 1 || pinFuncs[i] == 2) {
      String btnClass = state == HIGH ? "on" : "off";
      String newState = state == HIGH ? "0" : "1";
      html += "<td><a href='/pins?pin=" + String(i) + "&state=" + newState + "' class='btn " + btnClass + "'>" 
           + (state == HIGH ? "Ausschalten" : "Einschalten") + "</a></td>";
    } else {
      html += "<td>-</td>";
    }
    
    html += "</tr>";
  }
  
  html += "</table>";
  
  // Menü
  html += "<div>";
  html += "<a href='/config' class='btn'>Konfiguration</a> ";
  html += "<a href='/update' class='btn'>Firmware-Update</a>";
  html += "</div>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Konfigurationsseite anzeigen
void handleConfig() {
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f0f0f0;}";
  html += ".container{max-width:800px;margin:0 auto;padding:15px;}";
  html += "h1,h2{color:#0066cc;}";
  html += "label{display:block;margin-top:10px;font-weight:bold;}";
  html += "input,select{width:100%;padding:8px;box-sizing:border-box;margin-bottom:10px;}";
  html += "button{background:#0066cc;color:white;border:none;padding:8px 16px;border-radius:4px;cursor:pointer;}";
  html += "button:hover{background:#0055aa;}";
  html += "a{color:#0066cc;text-decoration:none;}";
  html += "a:hover{text-decoration:underline;}";
  html += "fieldset{margin:15px 0;padding:10px;border:1px solid #ddd;}";
  html += "legend{color:#0066cc;font-weight:bold;}";
  html += ".pin-config{margin:10px 0;padding:10px;border:1px solid #ddd;background:#fff;}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>SwissAirDry - Konfiguration</h1>";
  
  html += "<form action='/save' method='post'>";
  
  // WLAN-Einstellungen
  html += "<fieldset>";
  html += "<legend>WLAN-Einstellungen</legend>";
  
  html += "<label for='deviceName'>Gerätename:</label>";
  html += "<input type='text' id='deviceName' name='deviceName' value='" + String(deviceName) + "' maxlength='31'>";
  
  html += "<label for='ssid'>WLAN-SSID:</label>";
  html += "<input type='text' id='ssid' name='ssid' value='" + String(ssid) + "' maxlength='31'>";
  
  html += "<label for='password'>WLAN-Passwort:</label>";
  html += "<input type='password' id='password' name='password' value='" + String(password) + "' maxlength='63'>";
  
  html += "<label>";
  html += "<input type='checkbox' id='apMode' name='apMode' " + String(apMode ? "checked" : "") + ">";
  html += " Access Point Modus aktivieren";
  html += "</label>";
  
  html += "<label for='www_password'>Web-UI Passwort:</label>";
  html += "<input type='password' id='www_password' name='www_password' value='" + String(www_password) + "' maxlength='15'>";
  
  html += "</fieldset>";
  
  // API-Einstellungen
  html += "<fieldset>";
  html += "<legend>API-Einstellungen</legend>";
  
  html += "<label for='apiUrl'>API-URL:</label>";
  html += "<input type='text' id='apiUrl' name='apiUrl' value='" + String(apiUrl) + "' maxlength='63'>";
  
  html += "<label for='apiKey'>API-Schlüssel:</label>";
  html += "<input type='text' id='apiKey' name='apiKey' value='" + String(apiKey) + "' maxlength='31'>";
  
  html += "</fieldset>";
  
  // Pin-Konfiguration
  html += "<fieldset>";
  html += "<legend>Pin-Konfiguration</legend>";
  
  html += "<div id='pins-container'>";
  
  for (int i = 0; i < pinCount; i++) {
    html += "<div class='pin-config'>";
    html += "<h3>Pin " + String(i+1) + "</h3>";
    
    html += "<label for='pinNum" + String(i) + "'>GPIO-Nummer:</label>";
    html += "<select id='pinNum" + String(i) + "' name='pinNum" + String(i) + "'>";
    uint8_t availablePins[] = {0, 2, 4, 5, 12, 13, 14, 15, 16};
    for (uint8_t pin : availablePins) {
      String selected = (pinNums[i] == pin) ? "selected" : "";
      String pinLabel = "GPIO" + String(pin);
      
      // Pinlabels hinzufügen
      if (pin == 0) pinLabel += " (D3)";
      else if (pin == 2) pinLabel += " (D4/LED)";
      else if (pin == 4) pinLabel += " (D2/SDA)";
      else if (pin == 5) pinLabel += " (D1/SCL)";
      else if (pin == 12) pinLabel += " (D6)";
      else if (pin == 13) pinLabel += " (D7)";
      else if (pin == 14) pinLabel += " (D5)";
      else if (pin == 15) pinLabel += " (D8)";
      else if (pin == 16) pinLabel += " (D0)";
      
      html += "<option value='" + String(pin) + "' " + selected + ">" + pinLabel + "</option>";
    }
    html += "</select>";
    
    html += "<label for='pinName" + String(i) + "'>Bezeichnung:</label>";
    html += "<input type='text' id='pinName" + String(i) + "' name='pinName" + String(i) + "' value='" + String(pinNames[i]) + "' maxlength='15'>";
    
    html += "<label for='pinFunc" + String(i) + "'>Funktion:</label>";
    html += "<select id='pinFunc" + String(i) + "' name='pinFunc" + String(i) + "'>";
    html += "<option value='0' " + String(pinFuncs[i] == 0 ? "selected" : "") + ">Eingang</option>";
    html += "<option value='1' " + String(pinFuncs[i] == 1 ? "selected" : "") + ">Ausgang</option>";
    html += "<option value='2' " + String(pinFuncs[i] == 2 ? "selected" : "") + ">Relais</option>";
    html += "</select>";
    
    html += "</div>";
  }
  
  html += "</div>";
  
  // Pins hinzufügen/entfernen
  html += "<div>";
  html += "<button type='button' id='addPin' onclick='addPin()'>Pin hinzufügen</button> ";
  html += "<button type='button' id='removePin' onclick='removePin()'>Pin entfernen</button>";
  html += "<input type='hidden' id='pinCount' name='pinCount' value='" + String(pinCount) + "'>";
  html += "</div>";
  
  html += "</fieldset>";
  
  // Aktionen
  html += "<div style='margin-top:20px;'>";
  html += "<button type='submit'>Speichern</button> ";
  html += "<a href='/'>Zurück</a>";
  html += "</div>";
  
  html += "</form>";
  
  // JavaScript für dynamische Pin-Konfiguration
  html += "<script>";
  html += "function addPin() {";
  html += "  var count = parseInt(document.getElementById('pinCount').value);";
  html += "  if (count >= 8) {";
  html += "    alert('Maximale Anzahl an Pins erreicht (8)');";
  html += "    return;";
  html += "  }";
  html += "  count++;";
  html += "  document.getElementById('pinCount').value = count;";
  html += "  var container = document.getElementById('pins-container');";
  html += "  var newPin = document.createElement('div');";
  html += "  newPin.className = 'pin-config';";
  html += "  newPin.innerHTML = `";
  html += "    <h3>Pin ${count}</h3>";
  html += "    <label for='pinNum${count-1}'>GPIO-Nummer:</label>";
  html += "    <select id='pinNum${count-1}' name='pinNum${count-1}'>";
  html += "      <option value='0'>GPIO0 (D3)</option>";
  html += "      <option value='2'>GPIO2 (D4/LED)</option>";
  html += "      <option value='4'>GPIO4 (D2/SDA)</option>";
  html += "      <option value='5'>GPIO5 (D1/SCL)</option>";
  html += "      <option value='12'>GPIO12 (D6)</option>";
  html += "      <option value='13'>GPIO13 (D7)</option>";
  html += "      <option value='14'>GPIO14 (D5)</option>";
  html += "      <option value='15'>GPIO15 (D8)</option>";
  html += "      <option value='16'>GPIO16 (D0)</option>";
  html += "    </select>";
  html += "    <label for='pinName${count-1}'>Bezeichnung:</label>";
  html += "    <input type='text' id='pinName${count-1}' name='pinName${count-1}' value='Pin ${count}' maxlength='15'>";
  html += "    <label for='pinFunc${count-1}'>Funktion:</label>";
  html += "    <select id='pinFunc${count-1}' name='pinFunc${count-1}'>";
  html += "      <option value='0'>Eingang</option>";
  html += "      <option value='1'>Ausgang</option>";
  html += "      <option value='2'>Relais</option>";
  html += "    </select>";
  html += "  `;";
  html += "  container.appendChild(newPin);";
  html += "}";
  
  html += "function removePin() {";
  html += "  var count = parseInt(document.getElementById('pinCount').value);";
  html += "  if (count <= 1) {";
  html += "    alert('Mindestens ein Pin muss konfiguriert sein');";
  html += "    return;";
  html += "  }";
  html += "  count--;";
  html += "  document.getElementById('pinCount').value = count;";
  html += "  var container = document.getElementById('pins-container');";
  html += "  container.removeChild(container.lastChild);";
  html += "}";
  html += "</script>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Pin-Steuerung
void handlePins() {
  if (server.hasArg("pin") && server.hasArg("state")) {
    int pin = server.arg("pin").toInt();
    int state = server.arg("state").toInt();
    
    if (pin >= 0 && pin < pinCount) {
      if (pinFuncs[pin] == 1 || pinFuncs[pin] == 2) { // output oder relay
        digitalWrite(pinNums[pin], state ? HIGH : LOW);
        Serial.printf("Pin %d auf %s gesetzt\n", pinNums[pin], state ? "HIGH" : "LOW");
      }
    }
  }
  
  // Zurück zur Hauptseite
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Konfiguration speichern
void handleSave() {
  if (!authenticate()) {
    server.requestAuthentication();
    return;
  }
  
  if (server.hasArg("deviceName")) {
    strncpy(deviceName, server.arg("deviceName").c_str(), 31);
    deviceName[31] = '\0';
  }
  
  if (server.hasArg("ssid")) {
    strncpy(ssid, server.arg("ssid").c_str(), 31);
    ssid[31] = '\0';
  }
  
  if (server.hasArg("password")) {
    strncpy(password, server.arg("password").c_str(), 63);
    password[63] = '\0';
  }
  
  apMode = server.hasArg("apMode");
  
  if (server.hasArg("www_password")) {
    strncpy(www_password, server.arg("www_password").c_str(), 15);
    www_password[15] = '\0';
  }
  
  if (server.hasArg("apiUrl")) {
    strncpy(apiUrl, server.arg("apiUrl").c_str(), 63);
    apiUrl[63] = '\0';
  }
  
  if (server.hasArg("apiKey")) {
    strncpy(apiKey, server.arg("apiKey").c_str(), 31);
    apiKey[31] = '\0';
  }
  
  // Pin-Konfiguration speichern
  if (server.hasArg("pinCount")) {
    pinCount = server.arg("pinCount").toInt();
    if (pinCount > 8) pinCount = 8;
    
    for (int i = 0; i < pinCount; i++) {
      if (server.hasArg("pinNum" + String(i))) {
        pinNums[i] = server.arg("pinNum" + String(i)).toInt();
      }
      
      if (server.hasArg("pinFunc" + String(i))) {
        pinFuncs[i] = server.arg("pinFunc" + String(i)).toInt();
      }
      
      if (server.hasArg("pinName" + String(i))) {
        strncpy(pinNames[i], server.arg("pinName" + String(i)).c_str(), 15);
        pinNames[i][15] = '\0';
      }
    }
  }
  
  // Konfiguration speichern
  saveConfig();
  
  // Erfolg-Seite anzeigen
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f0f0f0;}";
  html += ".container{max-width:600px;margin:0 auto;padding:20px;text-align:center;}";
  html += "h1{color:#0066cc;}";
  html += "p{margin:20px 0;}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>Konfiguration gespeichert</h1>";
  html += "<p>Die Konfiguration wurde erfolgreich gespeichert. Das Gerät wird nun neu gestartet.</p>";
  html += "<p>Bitte warten Sie...</p>";
  html += "<p><a href='javascript:window.close();'>Fenster schließen</a></p>";
  html += "</div>";
  html += "<script>setTimeout(function(){window.location.href='/';}, 5000);</script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  
  // Kurze Verzögerung und Neustart
  delay(1000);
  ESP.restart();
}

// Authentifizierung prüfen
bool authenticate() {
  if (strlen(www_password) > 0) {
    return server.authenticate(www_username, www_password);
  }
  return true;
}

void loop() {
  // OTA-Updates verarbeiten
  ArduinoOTA.handle();
  
  // Webserver-Anfragen verarbeiten
  server.handleClient();
  
  // LED blinken im WLAN-Modus, schnell blinken im AP-Modus
  unsigned long now = millis();
  if (now - lastBlinkTime > (WiFi.getMode() == WIFI_AP ? 500 : 3000)) {
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlinkTime = now;
  }
  
  // Watchdog füttern
  yield();
}