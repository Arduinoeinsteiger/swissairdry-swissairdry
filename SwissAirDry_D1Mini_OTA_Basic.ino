// Minimales SwissAirDry OTA-Basisskript für D1 Mini (ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// D1 Mini spezifische Konfiguration
#define LED_PIN D4     // Eingebaute LED auf D4 (GPIO2)
#define LED_ON LOW     // LED ist invertiert: LOW = ein, HIGH = aus
#define LED_OFF HIGH

// Gerätekonfiguration
String deviceName = "SwissAirDry-";   // Wird mit Chip-ID ergänzt

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("SwissAirDry OTA Basic");
  
  // Eindeutigen Hostnamen erstellen
  uint16_t chipId = ESP.getChipId() & 0xFFFF;
  deviceName += String(chipId, HEX);
  
  Serial.print("Geräte-ID: ");
  Serial.println(deviceName);
  
  // WiFi-Manager starten (WifiManager-Bibliothek wird nicht verwendet)
  // Stattdessen zunächst mit festen Zugangsdaten arbeiten
  WiFi.mode(WIFI_STA);
  
  // HIER BEIM ERSTEN FLASH DIE WLAN-DATEN EINTRAGEN
  WiFi.begin("SSID", "PASSWORT");
  
  // Auf WiFi-Verbindung warten
  Serial.print("Verbinde mit WLAN");
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Verbunden mit IP: ");
    Serial.println(WiFi.localIP());
    
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
    });
    
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA: Ende");
      digitalWrite(LED_PIN, LED_OFF);
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Fehler[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth fehlgeschlagen");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin fehlgeschlagen");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect fehlgeschlagen");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive fehlgeschlagen");
      else if (error == OTA_END_ERROR) Serial.println("End fehlgeschlagen");
      
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
  } else {
    Serial.println();
    Serial.println("WiFi-Verbindung fehlgeschlagen!");
    
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
      WiFi.reconnect();
      lastReconnectAttempt = millis();
    }
  }
  
  // Heartbeat-LED - ein kurzer Blink alle 3 Sekunden
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 3000) {
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlink = millis();
  }
  
  yield();  // Watchdog füttern
}