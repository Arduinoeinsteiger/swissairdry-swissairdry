/*
 * SwissAirDry_ESP32C6_RestAPI
 * 
 * Firmware für ESP32-C6 XIAO, die direkt mit dem SwissAirDry REST API Server kommuniziert.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

#include <WiFi.h>                  // WiFi-Unterstützung
#include <HTTPClient.h>            // HTTP-Client für REST API
#include <ArduinoJson.h>           // JSON-Verarbeitung
#include <TFT_eSPI.h>              // TFT-Display-Bibliothek
#include <SPI.h>                   // SPI für Display
#include <FS.h>                    // Dateisystem
#include <SPIFFS.h>                // SPIFFS Dateisystem
#include <Update.h>                // OTA-Update
#include <ArduinoOTA.h>            // OTA-Bibliothek

// WLAN-Konfiguration - ändern Sie diese Werte entsprechend Ihrem Netzwerk
const char* ssid = "WLAN-SSID";           // WLAN-SSID
const char* password = "WLAN-PASSWORD";   // WLAN-Passwort

// API-Konfiguration
const char* api_host = "192.168.1.100";   // IP-Adresse oder Hostname des API-Servers
const int api_port = 5000;                // Port des API-Servers
const char* device_id = "esp32c6_test";   // Eindeutige Geräte-ID

// Intervalle (in Millisekunden)
const unsigned long sensor_read_interval = 5000;  // Sensoren alle 5 Sekunden auslesen
const unsigned long api_send_interval = 30000;    // Daten alle 30 Sekunden an API senden
const unsigned long display_update_interval = 1000; // Display jede Sekunde aktualisieren
const unsigned long led_update_interval = 3000;    // LED-Status alle 3 Sekunden aktualisieren

// Zeitpunkte der letzten Aktionen
unsigned long last_sensor_read = 0;
unsigned long last_api_send = 0;
unsigned long last_display_update = 0;
unsigned long last_led_update = 0;

// Status-Variablen
bool wifi_connected = false;
bool api_connected = false;
bool relay_state = false;
float temperature = 0.0;
float humidity = 0.0;
float power = 0.0;
float energy = 0.0;
unsigned long runtime = 0;
unsigned long start_time = 0;

// RGB-LED-Pin (auf dem XIAO ESP32C6)
const int rgb_led_pin = D8;  // WS2812 RGB-LED

// Relais-Pin
const int relay_pin = D1;  // Relais auf Pin D1

// Display initialisieren
TFT_eSPI tft = TFT_eSPI();

// Funktionsprototypen
void setup_wifi();
void setup_display();
void setup_ota();
void read_sensors();
bool send_data_to_api();
void update_display();
void update_led_status();
void check_api_commands();

void setup() {
  // Serielle Schnittstelle initialisieren
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\nSwissAirDry ESP32-C6 XIAO mit REST API");
  
  // Relais-Pin konfigurieren
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, LOW);  // Relais standardmäßig ausschalten
  
  // RGB-LED konfigurieren
  pinMode(rgb_led_pin, OUTPUT);
  digitalWrite(rgb_led_pin, LOW);
  
  // Starte Dateisystem
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS-Initialisierung fehlgeschlagen!");
  }
  
  // Display initialisieren
  setup_display();
  
  // WLAN verbinden
  setup_wifi();
  
  // OTA-Updates konfigurieren
  setup_ota();
  
  // Startzeit speichern
  start_time = millis();
}

void loop() {
  // OTA-Updates behandeln
  ArduinoOTA.handle();
  
  // Aktuelle Zeit
  unsigned long current_time = millis();
  
  // Sensoren periodisch auslesen
  if (current_time - last_sensor_read > sensor_read_interval) {
    read_sensors();
    last_sensor_read = current_time;
  }
  
  // Daten periodisch an API senden
  if (wifi_connected && (current_time - last_api_send > api_send_interval)) {
    api_connected = send_data_to_api();
    last_api_send = current_time;
    
    // Nach dem Senden nach Befehlen prüfen
    if (api_connected) {
      check_api_commands();
    }
  }
  
  // Display aktualisieren
  if (current_time - last_display_update > display_update_interval) {
    update_display();
    last_display_update = current_time;
  }
  
  // LED-Status aktualisieren
  if (current_time - last_led_update > led_update_interval) {
    update_led_status();
    last_led_update = current_time;
  }
  
  // Laufzeit aktualisieren
  runtime = (millis() - start_time) / 1000;  // Laufzeit in Sekunden
}

void setup_wifi() {
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);
  
  // Verbindungsversuch anzeigen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Verbinde mit WLAN...", tft.width() / 2, tft.height() / 2);
  
  // Mit WLAN verbinden
  WiFi.begin(ssid, password);
  
  // Warten auf Verbindung (max. 20 Sekunden)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Blinkenden Punkt auf Display anzeigen
    if (attempts % 2 == 0) {
      tft.drawString(".", tft.width() / 2, tft.height() / 2 + 20);
    } else {
      tft.drawString(" ", tft.width() / 2, tft.height() / 2 + 20);
    }
  }
  
  // Prüfen, ob verbunden
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println("\nWLAN verbunden!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
    
    // Erfolgsmeldung auf Display
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("WLAN verbunden!", tft.width() / 2, tft.height() / 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(WiFi.localIP().toString(), tft.width() / 2, tft.height() / 2 + 20);
    delay(2000);
  } else {
    wifi_connected = false;
    Serial.println("\nVerbindung zum WLAN fehlgeschlagen!");
    
    // Fehlermeldung auf Display
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("WLAN-Verbindung", tft.width() / 2, tft.height() / 2);
    tft.drawString("fehlgeschlagen!", tft.width() / 2, tft.height() / 2 + 20);
    delay(2000);
  }
}

void setup_display() {
  // Display initialisieren
  tft.init();
  tft.setRotation(3);  // Ausrichtung für XIAO ESP32C6 1.47" Display
  tft.fillScreen(TFT_BLACK);
  
  // Startbildschirm anzeigen
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLUE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SwissAirDry", tft.width() / 2, tft.height() / 3);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("ESP32-C6 XIAO", tft.width() / 2, tft.height() / 2);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("REST API Version", tft.width() / 2, 2 * tft.height() / 3);
  
  delay(2000);
}

void setup_ota() {
  if (!wifi_connected) return;
  
  // OTA-Hostname setzen
  String hostname = "SwissAirDry-" + String(device_id);
  ArduinoOTA.setHostname(hostname.c_str());
  
  // OTA-Passwort setzen (optional, aber empfohlen)
  // ArduinoOTA.setPassword("admin");
  
  // OTA-Ereignishandler
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else {
      type = "Dateisystem";
    }
    
    Serial.println("Starte OTA-Update: " + type);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("OTA-Update läuft...", tft.width() / 2, tft.height() / 2);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA-Update abgeschlossen");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Update abgeschlossen!", tft.width() / 2, tft.height() / 2);
    tft.drawString("Neustart...", tft.width() / 2, tft.height() / 2 + 20);
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percent = progress / (total / 100);
    Serial.printf("Fortschritt: %u%%\r", percent);
    
    // Fortschrittsbalken auf Display
    static int last_percent = 0;
    if (percent != last_percent) {
      last_percent = percent;
      
      tft.fillRect(10, tft.height() / 2 + 20, tft.width() - 20, 10, TFT_BLACK);
      tft.drawRect(10, tft.height() / 2 + 20, tft.width() - 20, 10, TFT_WHITE);
      tft.fillRect(10, tft.height() / 2 + 20, (tft.width() - 20) * percent / 100, 10, TFT_GREEN);
      
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(String(percent) + "%", tft.width() / 2, tft.height() / 2 + 40);
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA-Fehler[%u]: ", error);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Authentifizierungsfehler");
      tft.drawString("Auth-Fehler!", tft.width() / 2, tft.height() / 2);
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin-Fehler");
      tft.drawString("Begin-Fehler!", tft.width() / 2, tft.height() / 2);
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect-Fehler");
      tft.drawString("Connect-Fehler!", tft.width() / 2, tft.height() / 2);
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive-Fehler");
      tft.drawString("Receive-Fehler!", tft.width() / 2, tft.height() / 2);
    } else if (error == OTA_END_ERROR) {
      Serial.println("End-Fehler");
      tft.drawString("End-Fehler!", tft.width() / 2, tft.height() / 2);
    }
  });
  
  // OTA starten
  ArduinoOTA.begin();
  Serial.println("OTA bereit");
}

void read_sensors() {
  // Simulierte Sensorwerte für den Test
  temperature = 20.0 + random(0, 100) / 10.0;  // 20.0 - 30.0 °C
  humidity = 50.0 + random(0, 200) / 10.0;     // 50.0 - 70.0 %
  
  // Wenn Relais eingeschaltet, simuliere Leistung
  if (relay_state) {
    power = 400.0 + random(0, 100) / 10.0;     // 400.0 - 410.0 Watt
    energy += power / 3600.0 * (sensor_read_interval / 1000.0);  // kWh berechnen
  } else {
    power = 0.0;
  }
  
  Serial.println("Sensordaten ausgelesen:");
  Serial.print("Temperatur: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Luftfeuchtigkeit: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Leistung: ");
  Serial.print(power);
  Serial.println(" W");
  Serial.print("Energie: ");
  Serial.print(energy);
  Serial.println(" kWh");
  Serial.print("Relais: ");
  Serial.println(relay_state ? "EIN" : "AUS");
  Serial.print("Laufzeit: ");
  Serial.print(runtime);
  Serial.println(" s");
}

bool send_data_to_api() {
  if (!wifi_connected) {
    Serial.println("Keine WLAN-Verbindung, Daten werden nicht gesendet");
    return false;
  }
  
  HTTPClient http;
  
  // URL zusammensetzen
  String url = "http://" + String(api_host) + ":" + String(api_port) + "/api/device/" + String(device_id) + "/data";
  
  Serial.print("Sende Daten an API: ");
  Serial.println(url);
  
  // Verbindung herstellen
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  // JSON-Daten erstellen
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["power"] = power;
  doc["energy"] = energy;
  doc["relay_state"] = relay_state;
  doc["runtime"] = runtime;
  
  String json;
  serializeJson(doc, json);
  
  // Daten senden
  int httpResponseCode = http.POST(json);
  
  // Antwort auswerten
  bool success = false;
  if (httpResponseCode > 0) {
    Serial.print("HTTP Antwort-Code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println("Antwort: " + response);
    
    // API-Antwort verarbeiten
    StaticJsonDocument<256> response_doc;
    DeserializationError error = deserializeJson(response_doc, response);
    
    if (!error) {
      // Prüfen, ob die Antwort einen Steuerbefehl enthält
      if (response_doc.containsKey("relay_control")) {
        bool new_relay_state = response_doc["relay_control"];
        if (relay_state != new_relay_state) {
          Serial.print("Relais-Steuerung von API: ");
          Serial.println(new_relay_state ? "EIN" : "AUS");
          relay_state = new_relay_state;
          digitalWrite(relay_pin, relay_state ? HIGH : LOW);
        }
      }
      
      success = true;
    }
  } else {
    Serial.print("Fehler beim Senden der Daten. Fehlercode: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
  return success;
}

void update_display() {
  // Display aktualisieren
  tft.fillScreen(TFT_BLACK);
  
  // Titel
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("SwissAirDry", tft.width() / 2, 5);
  
  // Status
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  
  // Netzwerkstatus
  tft.drawString("Netzwerk:", 10, 25);
  if (wifi_connected) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Verbunden", 90, 25);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Getrennt", 90, 25);
  }
  
  // API-Status
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("API:", 10, 40);
  if (api_connected) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Verbunden", 90, 40);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Getrennt", 90, 40);
  }
  
  // Sensordaten
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Temperatur:", 10, 60);
  tft.drawString(String(temperature, 1) + " C", 100, 60);
  
  tft.drawString("Feuchtigkeit:", 10, 75);
  tft.drawString(String(humidity, 1) + " %", 100, 75);
  
  tft.drawString("Leistung:", 10, 90);
  tft.drawString(String(power, 1) + " W", 100, 90);
  
  tft.drawString("Energie:", 10, 105);
  tft.drawString(String(energy, 3) + " kWh", 100, 105);
  
  // Relais-Status
  tft.drawString("Relais:", 10, 120);
  if (relay_state) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("EIN", 100, 120);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("AUS", 100, 120);
  }
  
  // Laufzeit formatieren
  unsigned long days = runtime / 86400;
  unsigned long hours = (runtime % 86400) / 3600;
  unsigned long minutes = (runtime % 3600) / 60;
  unsigned long seconds = runtime % 60;
  
  char runtime_str[20];
  sprintf(runtime_str, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Laufzeit:", 10, 135);
  if (days > 0) {
    tft.drawString(String(days) + "d " + String(runtime_str), 100, 135);
  } else {
    tft.drawString(String(runtime_str), 100, 135);
  }
}

void update_led_status() {
  // RGB-LED-Farbe basierend auf dem Gerätestatus setzen
  if (!wifi_connected) {
    // Blinkendes Orange: Verbindungsprobleme
    digitalWrite(rgb_led_pin, (millis() / 500) % 2 == 0 ? HIGH : LOW);
  } else if (!api_connected) {
    // Blinkendes Rot: API-Verbindungsprobleme
    digitalWrite(rgb_led_pin, (millis() / 500) % 2 == 0 ? HIGH : LOW);
  } else if (relay_state) {
    // Grün: Verbunden und aktiv
    digitalWrite(rgb_led_pin, HIGH);
  } else {
    // Cyan: Verbunden aber im Standby
    digitalWrite(rgb_led_pin, HIGH);
  }
}

void check_api_commands() {
  // Diese Funktion würde normalerweise separat Befehle von der API abrufen
  // In unserer Implementierung werden die Befehle bereits in der Antwort auf send_data_to_api() verarbeitet
}