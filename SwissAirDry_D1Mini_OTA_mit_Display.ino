// SwissAirDry OTA-Skript für D1 Mini (ESP8266) mit OLED-Display
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// D1 Mini spezifische Konfiguration
#define LED_PIN D4     // Eingebaute LED auf D4 (GPIO2)
#define LED_ON LOW     // LED ist invertiert: LOW = ein, HIGH = aus
#define LED_OFF HIGH

// OLED-Display Konfiguration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C    // Typische I2C-Adresse für SSD1306
// SDA = D2 (GPIO4) und SCL = D1 (GPIO5) sind beim D1 Mini fest zugewiesen

// Display-Objekt erstellen
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// Gerätekonfiguration
String deviceName = "SwissAirDry-";   // Wird mit Chip-ID ergänzt

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);  // LED initial aus
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("SwissAirDry OTA mit Display");
  
  // Eindeutigen Hostnamen erstellen
  uint16_t chipId = ESP.getChipId() & 0xFFFF;
  deviceName += String(chipId, HEX);
  
  Serial.print("Geräte-ID: ");
  Serial.println(deviceName);
  
  // Display initialisieren
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 Display nicht gefunden"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(F("SwissAirDry"));
    display.println(F("Boot..."));
    display.display();
  }
  
  // WiFi-Verbindung herstellen
  WiFi.mode(WIFI_STA);
  
  // HIER BEIM ERSTEN FLASH DIE WLAN-DATEN EINTRAGEN
  WiFi.begin("SSID", "PASSWORT");
  
  // Warten auf WiFi-Verbindung mit LED-Blinken
  Serial.print("Verbinde mit WLAN");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Verbinde mit WiFi:"));
  display.println(F("SSID"));
  display.display();
  
  int dots = 0;
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    
    // Animierte Punkte auf dem Display
    dots = (dots + 1) % 4;
    display.fillRect(0, 24, 128, 8, BLACK);
    display.setCursor(0, 24);
    for(int i=0; i<dots; i++) {
      display.print(".");
    }
    display.display();
    
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Erfolgreiches Verbinden anzeigen
    Serial.println();
    Serial.print("Verbunden mit IP-Adresse: ");
    Serial.println(WiFi.localIP());
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi verbunden!"));
    display.print(F("IP: "));
    display.println(WiFi.localIP().toString());
    display.display();
    
    // Erfolgreiche Verbindung signalisieren
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(200);
      digitalWrite(LED_PIN, LED_OFF);
      delay(200);
    }
    
    // OTA-Konfiguration
    ArduinoOTA.setHostname(deviceName.c_str());
    
    ArduinoOTA.onStart([]() {
      Serial.println("OTA: Start");
      digitalWrite(LED_PIN, LED_ON);
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("OTA Update"));
      display.println(F("startet..."));
      display.display();
    });
    
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA: Ende");
      digitalWrite(LED_PIN, LED_OFF);
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("Update OK!"));
      display.println(F("Neustart..."));
      display.display();
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      int percent = (progress / (total / 100));
      Serial.printf("OTA: %u%%\r", percent);
      
      // Fortschrittsbalken auf dem Display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("OTA Update:"));
      display.printf("%u%%\n", percent);
      display.drawRect(0, 20, 128, 10, WHITE);
      display.fillRect(2, 22, (percent * 124) / 100, 6, WHITE);
      display.display();
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Fehler[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth fehlgeschlagen");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin fehlgeschlagen");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect fehlgeschlagen");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive fehlgeschlagen");
      else if (error == OTA_END_ERROR) Serial.println("End fehlgeschlagen");
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("OTA FEHLER!"));
      display.printf("Code: %u\n", error);
      display.display();
      
      // Fehlersignal mit LED
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, LED_ON);
        delay(100);
        digitalWrite(LED_PIN, LED_OFF);
        delay(100);
      }
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA bereit");
    
    // OTA-Status auf Display anzeigen
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("SwissAirDry"));
    display.println(F("OTA bereit"));
    display.println();
    display.print(F("IP: "));
    display.println(WiFi.localIP().toString());
    display.print(F("Host: "));
    display.println(deviceName);
    display.display();
  } else {
    Serial.println();
    Serial.println("WiFi-Verbindung fehlgeschlagen!");
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi Fehler!"));
    display.println(F("Neustart erforderlich"));
    display.display();
    
    // Fehlersignal mit schnellem Blinken
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(50);
      digitalWrite(LED_PIN, LED_OFF);
      delay(50);
    }
  }
}

void loop() {
  // OTA-Handler regelmäßig aufrufen
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  } else {
    // Bei Verbindungsverlust alle 30 Sekunden neu verbinden
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      Serial.println("Versuche WLAN-Verbindung wiederherzustellen...");
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("WiFi getrennt!"));
      display.println(F("Verbinde neu..."));
      display.display();
      
      WiFi.reconnect();
      lastReconnectAttempt = millis();
    }
  }
  
  // Heartbeat-LED - ein kurzer Blink alle 3 Sekunden
  static unsigned long lastBlink = 0;
  static unsigned long lastDisplayUpdate = 0;
  
  if (millis() - lastBlink > 3000) {
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlink = millis();
  }
  
  // Display regelmäßig aktualisieren (Uptime, WLAN-Signalstärke, etc.)
  if (millis() - lastDisplayUpdate > 10000) {  // Alle 10 Sekunden
    if (WiFi.status() == WL_CONNECTED) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("SwissAirDry"));
      display.println(F("Status: Bereit"));
      display.println();
      
      // Uptime anzeigen
      long uptime = millis() / 1000;
      int uptimeMin = (uptime / 60) % 60;
      int uptimeStd = (uptime / 3600) % 24;
      int uptimeTage = uptime / 86400;
      
      display.print(F("Uptime: "));
      if (uptimeTage > 0) {
        display.print(uptimeTage);
        display.print(F("T "));
      }
      display.print(uptimeStd);
      display.print(F("h "));
      display.print(uptimeMin);
      display.println(F("m"));
      
      // WLAN-Signalstärke
      display.print(F("RSSI: "));
      display.print(WiFi.RSSI());
      display.println(F(" dBm"));
      
      display.display();
    }
    lastDisplayUpdate = millis();
  }
  
  yield();  // Wichtig für ESP8266, um den Watchdog zu füttern
}