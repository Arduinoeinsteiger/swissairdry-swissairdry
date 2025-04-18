/*
 * SwissAirDry_ESP32C6_MQTT
 * 
 * Firmware für ESP32-C6 XIAO, die über MQTT mit dem SwissAirDry API Server kommuniziert.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

#include <WiFi.h>                  // WiFi-Unterstützung
#include <PubSubClient.h>          // MQTT-Client für MQTT-Kommunikation
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

// MQTT-Konfiguration
const char* mqtt_server = "192.168.1.100";  // MQTT-Broker-IP oder Hostname
const int mqtt_port = 1883;                 // MQTT-Broker-Port
const char* mqtt_user = "";                 // MQTT-Benutzername (leer = kein Auth)
const char* mqtt_password = "";             // MQTT-Passwort (leer = kein Auth)
const char* device_id = "esp32c6_mqtt";     // Eindeutige Geräte-ID
const char* mqtt_topic_prefix = "swissairdry"; // MQTT-Topic-Präfix

// Intervalle (in Millisekunden)
const unsigned long sensor_read_interval = 5000;  // Sensoren alle 5 Sekunden auslesen
const unsigned long mqtt_send_interval = 30000;   // Daten alle 30 Sekunden per MQTT senden
const unsigned long display_update_interval = 1000; // Display jede Sekunde aktualisieren
const unsigned long led_update_interval = 3000;    // LED-Status alle 3 Sekunden aktualisieren
const unsigned long mqtt_reconnect_interval = 5000; // MQTT-Reconnect alle 5 Sekunden versuchen

// Zeitpunkte der letzten Aktionen
unsigned long last_sensor_read = 0;
unsigned long last_mqtt_send = 0;
unsigned long last_display_update = 0;
unsigned long last_led_update = 0;
unsigned long last_mqtt_reconnect = 0;

// Status-Variablen
bool wifi_connected = false;
bool mqtt_connected = false;
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

// WiFi-Client
WiFiClient wifi_client;

// MQTT-Client
PubSubClient mqtt_client(wifi_client);

// Display initialisieren
TFT_eSPI tft = TFT_eSPI();

// Funktionsprototypen
void setup_wifi();
void setup_mqtt();
void setup_display();
void setup_ota();
void read_sensors();
bool send_data_to_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnect_mqtt();
void update_display();
void update_led_status();

void setup() {
  // Serielle Schnittstelle initialisieren
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\nSwissAirDry ESP32-C6 XIAO mit MQTT");
  
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
  
  // MQTT einrichten
  setup_mqtt();
  
  // OTA-Updates konfigurieren
  setup_ota();
  
  // Startzeit speichern
  start_time = millis();
}

void loop() {
  // OTA-Updates behandeln
  ArduinoOTA.handle();
  
  // MQTT-Client-Loop ausführen
  if (wifi_connected) {
    mqtt_client.loop();
  }
  
  // Aktuelle Zeit
  unsigned long current_time = millis();
  
  // MQTT-Verbindung prüfen und ggf. wiederherstellen
  if (wifi_connected && !mqtt_connected && (current_time - last_mqtt_reconnect > mqtt_reconnect_interval)) {
    reconnect_mqtt();
    last_mqtt_reconnect = current_time;
  }
  
  // Sensoren periodisch auslesen
  if (current_time - last_sensor_read > sensor_read_interval) {
    read_sensors();
    last_sensor_read = current_time;
  }
  
  // Daten periodisch an MQTT senden
  if (wifi_connected && mqtt_connected && (current_time - last_mqtt_send > mqtt_send_interval)) {
    send_data_to_mqtt();
    last_mqtt_send = current_time;
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

void setup_mqtt() {
  // MQTT-Client konfigurieren
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
  
  // Verbindung zum MQTT-Broker herstellen
  if (wifi_connected) {
    reconnect_mqtt();
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
  tft.drawString("MQTT Version", tft.width() / 2, 2 * tft.height() / 3);
  
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

void reconnect_mqtt() {
  // Verbindung zum MQTT-Broker herstellen
  if (!mqtt_client.connected()) {
    Serial.print("Verbinde mit MQTT-Broker...");
    
    // Client-ID erstellen
    String clientId = "SwissAirDry-";
    clientId += device_id;
    
    // Verbindung herstellen
    bool connect_success = false;
    
    if (mqtt_user && strlen(mqtt_user) > 0) {
      // Mit Authentifizierung verbinden
      connect_success = mqtt_client.connect(clientId.c_str(), mqtt_user, mqtt_password);
    } else {
      // Ohne Authentifizierung verbinden
      connect_success = mqtt_client.connect(clientId.c_str());
    }
    
    if (connect_success) {
      Serial.println("verbunden!");
      mqtt_connected = true;
      
      // Abonniere Befehlsthemen
      String cmd_topic = String(mqtt_topic_prefix) + "/" + device_id + "/cmd/#";
      mqtt_client.subscribe(cmd_topic.c_str());
      Serial.print("Abonniert: ");
      Serial.println(cmd_topic);
      
      // Online-Status veröffentlichen
      String status_topic = String(mqtt_topic_prefix) + "/" + device_id + "/status";
      mqtt_client.publish(status_topic.c_str(), "online", true);
    } else {
      mqtt_connected = false;
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Nächster Versuch in 5 Sekunden");
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Empfangene MQTT-Nachricht verarbeiten
  Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Payload in String umwandeln
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Topic-String erstellen
  String topic_str(topic);
  
  // Prüfen, ob es sich um ein Befehlsthema handelt
  String cmd_prefix = String(mqtt_topic_prefix) + "/" + device_id + "/cmd/";
  if (topic_str.startsWith(cmd_prefix)) {
    // Befehlstyp extrahieren
    String command_type = topic_str.substring(cmd_prefix.length());
    
    // Befehle verarbeiten
    if (command_type == "relay") {
      // Relais-Steuerung
      if (message == "true" || message == "1" || message == "on") {
        relay_state = true;
        digitalWrite(relay_pin, HIGH);
        Serial.println("Relais eingeschaltet");
      } else if (message == "false" || message == "0" || message == "off") {
        relay_state = false;
        digitalWrite(relay_pin, LOW);
        Serial.println("Relais ausgeschaltet");
      }
    } else if (command_type == "reset") {
      // Gerät zurücksetzen
      Serial.println("Gerät wird zurückgesetzt...");
      ESP.restart();
    } else if (command_type == "config") {
      // Konfiguration aktualisieren
      // In einer echten Implementierung würde hier die Gerätekonfiguration aktualisiert
      Serial.println("Konfiguration aktualisieren (nicht implementiert)");
    }
  }
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

bool send_data_to_mqtt() {
  if (!mqtt_connected) {
    Serial.println("Nicht mit MQTT-Broker verbunden, Daten werden nicht gesendet");
    return false;
  }
  
  // MQTT-Thema für Sensordaten
  String data_topic = String(mqtt_topic_prefix) + "/" + device_id + "/data";
  
  // JSON-Dokument erstellen
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["power"] = power;
  doc["energy"] = energy;
  doc["relay_state"] = relay_state;
  doc["runtime"] = runtime;
  
  // JSON-Dokument in String serialisieren
  String json_data;
  serializeJson(doc, json_data);
  
  // Daten veröffentlichen
  Serial.print("Sende Daten an MQTT-Thema: ");
  Serial.println(data_topic);
  Serial.println(json_data);
  
  bool result = mqtt_client.publish(data_topic.c_str(), json_data.c_str());
  if (result) {
    Serial.println("Daten erfolgreich gesendet");
  } else {
    Serial.println("Fehler beim Senden der Daten");
  }
  
  return result;
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
  
  // MQTT-Status
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("MQTT:", 10, 40);
  if (mqtt_connected) {
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
    // Blinkendes Orange: WLAN-Verbindungsprobleme
    digitalWrite(rgb_led_pin, (millis() / 500) % 2 == 0 ? HIGH : LOW);
  } else if (!mqtt_connected) {
    // Blinkendes Rot: MQTT-Verbindungsprobleme
    digitalWrite(rgb_led_pin, (millis() / 500) % 2 == 0 ? HIGH : LOW);
  } else if (relay_state) {
    // Grün: Verbunden und aktiv
    digitalWrite(rgb_led_pin, HIGH);
  } else {
    // Cyan: Verbunden aber im Standby
    digitalWrite(rgb_led_pin, HIGH);
  }
}