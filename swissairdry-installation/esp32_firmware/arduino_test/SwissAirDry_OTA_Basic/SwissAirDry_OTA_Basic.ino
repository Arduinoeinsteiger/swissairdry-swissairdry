/**
 * SwissAirDry ESP32 OTA-Basic Sketch
 * 
 * Minimales Programm für ESP32, das nur OTA-Updates (Over-the-Air) aktiviert,
 * damit der ESP32 über WLAN geflasht werden kann.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 * @version 1.0.0
 */

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// ------ WLAN-KONFIGURATION ------
const char* ssid = ""; // Hier WLAN-Namen eintragen
const char* password = ""; // Hier WLAN-Passwort eintragen

// ------ OTA-KONFIGURATION ------
const char* hostname = "swissairdry-esp32"; // Hostname für OTA
const char* ota_password = "swissairdry";   // Passwort für OTA (optional)

// ------ STATUS-LED ------
#define LED_PIN 2  // Eingebaute LED des ESP32 (Pin 2 bei den meisten Modellen)
bool ledState = false;
unsigned long lastBlink = 0;

void setup() {
  // Serielle Verbindung starten
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("====================================================");
  Serial.println("SwissAirDry ESP32 OTA-Basic");
  Serial.println("Version 1.0.0 - Minimaler Sketch für OTA-Updates");
  Serial.println("====================================================");
  
  // LED-Pin als Ausgang konfigurieren
  pinMode(LED_PIN, OUTPUT);
  
  // WLAN-Verbindung starten
  setupWifi();
  
  // OTA-Updates einrichten
  setupOTA();
  
  Serial.println("Setup abgeschlossen. Warte auf OTA-Updates...");
}

void loop() {
  // OTA-Updates verarbeiten
  ArduinoOTA.handle();
  
  // WLAN-Verbindung prüfen und bei Bedarf wiederherstellen
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
  
  // Status-LED blinken lassen - unterschiedliche Muster je nach WLAN-Status
  unsigned long currentMillis = millis();
  if (WiFi.status() == WL_CONNECTED) {
    // Bei WLAN-Verbindung: Langsames Blinken (1 Sekunde Intervall)
    if (currentMillis - lastBlink > 1000) {
      lastBlink = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } else {
    // Ohne WLAN-Verbindung: Schnelles Blinken (200ms Intervall)
    if (currentMillis - lastBlink > 200) {
      lastBlink = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
  
  // Kurze Pause für Stabilität
  delay(10);
}

// WLAN-Verbindung herstellen
void setupWifi() {
  if (strlen(ssid) == 0) {
    Serial.println("FEHLER: WLAN-Zugangsdaten nicht konfiguriert!");
    Serial.println("Bitte tragen Sie Ihre WLAN-SSID und das Passwort im Sketch ein.");
    return;
  }
  
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Auf Verbindung warten (mit Timeout)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWLAN-Verbindung hergestellt!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWLAN-Verbindung fehlgeschlagen!");
  }
}

// OTA-Updates konfigurieren
void setupOTA() {
  // mDNS-Hostnamen setzen
  ArduinoOTA.setHostname(hostname);
  
  // Passwort für Updates festlegen
  if (strlen(ota_password) > 0) {
    ArduinoOTA.setPassword(ota_password);
  }
  
  // Event-Handler für Update-Prozess
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Programm";
    } else {
      type = "Dateisystem";
    }
    Serial.println("OTA-Update gestartet: " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nUpdate abgeschlossen");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Fortschritt: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Fehler[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Authentifizierung fehlgeschlagen");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin fehlgeschlagen");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Verbindung fehlgeschlagen");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Empfangen fehlgeschlagen");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End fehlgeschlagen");
    }
  });
  
  // OTA-Service starten
  ArduinoOTA.begin();
  
  // Hinweise zur Verwendung
  Serial.println("OTA-Updates aktiviert");
  Serial.println("Hostname: " + String(hostname) + ".local");
  Serial.println("IP: " + WiFi.localIP().toString());
  Serial.println("Port: 3232");
  Serial.println("\nZum Flashen über WLAN in der Arduino IDE:");
  Serial.println("1. Werkzeuge > Port > Netzwerk-Ports > " + String(hostname) + ".local");
  Serial.println("2. Sketch > Hochladen");
  
  // mDNS-Responder starten (für .local Adressierung)
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS-Responder gestartet");
  } else {
    Serial.println("Fehler beim Starten des mDNS-Responders");
  }
}