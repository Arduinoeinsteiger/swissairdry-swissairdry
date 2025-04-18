// SwissAirDry ESP8266 OTA-Skript
// Speziell für ESP8266-Boards optimiert (D1 Mini, NodeMCU, etc.)

#include <ESP8266WiFi.h>        // ESP8266-spezifische WiFi-Bibliothek
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ----- KONFIGURATION -----
// WiFi-Konfiguration - HIER ANPASSEN!
const char* ssid = "SSID";        // WLAN-Name
const char* password = "PASSWORT"; // WLAN-Passwort

// GPIO-Konfiguration für ESP8266
#define LED_PIN D4       // Eingebaute LED an D4 (GPIO2) - invertiert
#define LED_ON LOW       // LED ist bei LOW eingeschaltet (invertiert)
#define LED_OFF HIGH     // LED ist bei HIGH ausgeschaltet
// SDA und SCL sind bei ESP8266 fest auf D2 (GPIO4) und D1 (GPIO5)

// Display-Konfiguration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C    // Typische I2C-Adresse für SSD1306
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool displayAvailable = false;

// Gerätekonfiguration
String deviceName = "SwissAirDry-";

void setup() {
  Serial.begin(115200);
  Serial.println("\nSwissAirDry ESP8266 OTA");
  
  // Eindeutigen Hostnamen erstellen
  uint16_t chipId = ESP.getChipId() & 0xFFFF;
  deviceName += String(chipId, HEX);
  
  Serial.print("Geräte-ID: ");
  Serial.println(deviceName);
  
  // LED konfigurieren
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  
  // I2C für Display initialisieren - bei ESP8266 feste Pins (D2=SDA, D1=SCL)
  Wire.begin();
  
  // Display initialisieren
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 Display nicht gefunden");
    displayAvailable = false;
  } else {
    Serial.println("Display gefunden und initialisiert");
    displayAvailable = true;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("SwissAirDry");
    display.println("ESP8266 OTA Boot");
    display.display();
  }
  
  // WiFi verbinden
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Verbinde mit WLAN");
  
  if (displayAvailable) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Verbinde mit WiFi:");
    display.println(ssid);
    display.display();
  }
  
  // Animation während der Verbindung
  int dots = 0;
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    
    if (displayAvailable) {
      dots = (dots + 1) % 4;
      display.fillRect(0, 24, 128, 8, BLACK);
      display.setCursor(0, 24);
      for(int i=0; i<dots; i++) {
        display.print(".");
      }
      display.display();
    }
    
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Verbunden mit IP: ");
    Serial.println(WiFi.localIP());
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WiFi verbunden!");
      display.print("IP: ");
      display.println(WiFi.localIP().toString());
      display.display();
    }
    
    // OTA konfigurieren
    ArduinoOTA.setHostname(deviceName.c_str());
    
    ArduinoOTA.onStart([]() {
      Serial.println("OTA: Start");
      digitalWrite(LED_PIN, LED_ON);
      
      if (displayAvailable) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("OTA Update");
        display.println("startet...");
        display.display();
      }
    });
    
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA: Ende");
      digitalWrite(LED_PIN, LED_OFF);
      
      if (displayAvailable) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Update OK!");
        display.println("Neustart...");
        display.display();
      }
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      int percent = (progress / (total / 100));
      Serial.printf("OTA: %u%%\r", percent);
      
      if (displayAvailable) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("OTA Update:");
        display.printf("%u%%\n", percent);
        display.drawRect(0, 20, 128, 10, WHITE);
        display.fillRect(2, 22, (percent * 124) / 100, 6, WHITE);
        display.display();
      }
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Fehler[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth fehlgeschlagen");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin fehlgeschlagen");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect fehlgeschlagen");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive fehlgeschlagen");
      else if (error == OTA_END_ERROR) Serial.println("End fehlgeschlagen");
      
      if (displayAvailable) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("OTA FEHLER!");
        char errorMsg[16];
        sprintf(errorMsg, "Code: %u", error);
        display.println(errorMsg);
        display.display();
      }
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA bereit");
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("SwissAirDry");
      display.println("OTA bereit");
      display.println();
      display.print("IP: ");
      display.println(WiFi.localIP().toString());
      display.display();
    }
  } else {
    Serial.println();
    Serial.println("WiFi-Verbindung fehlgeschlagen!");
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WiFi Fehler!");
      display.println("Neustart erforderlich");
      display.display();
    }
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  } else {
    // Bei Verbindungsabbruch alle 30 Sekunden neu verbinden
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      Serial.println("Versuche WLAN-Verbindung wiederherzustellen...");
      WiFi.reconnect();
      lastReconnectAttempt = millis();
    }
  }
  
  // Heartbeat-LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 3000) {
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlink = millis();
  }
  
  // Display aktualisieren
  static unsigned long lastDisplayUpdate = 0;
  if (displayAvailable && WiFi.status() == WL_CONNECTED && (millis() - lastDisplayUpdate > 10000)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SwissAirDry");
    display.println("Status: Bereit");
    
    // Uptime anzeigen
    long uptime = millis() / 1000;
    int uptimeMin = (uptime / 60) % 60;
    int uptimeStd = (uptime / 3600) % 24;
    
    display.print("Uptime: ");
    display.print(uptimeStd);
    display.print("h ");
    display.print(uptimeMin);
    display.println("m");
    
    // WLAN-Signalstärke
    display.print("RSSI: ");
    display.print(WiFi.RSSI());
    display.println(" dBm");
    
    display.display();
    lastDisplayUpdate = millis();
  }
  
  // Wichtig für ESP8266: Watchdog füttern
  yield();
}