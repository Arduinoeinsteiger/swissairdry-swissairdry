/**
 * SwissAirDry ESP32 Test-Sketch für Arduino IDE
 * 
 * Dieses einfache Testprogramm hilft zu überprüfen, ob Ihr ESP32 korrekt funktioniert,
 * bevor Sie die vollständige SwissAirDry-Firmware flashen.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 * @version 1.0.0
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ------ KONFIGURATION ------
// WLAN-Einstellungen
const char* ssid = ""; // Hier Ihren WLAN-Namen eintragen
const char* password = ""; // Hier Ihr WLAN-Passwort eintragen

// MQTT-Einstellungen (Optional)
const char* mqtt_server = "mqtt.vgnc.org";
const int mqtt_port = 1883;
const char* mqtt_client_id = "swissairdry-esp32-test";
const char* mqtt_topic = "swissairdry/test";

// Sensor-Einstellungen
#define DHT_PIN 4     // GPIO-Pin für den DHT-Sensor
#define DHT_TYPE DHT22 // DHT-Sensortyp (DHT11 oder DHT22)

// Relais-Pin
#define RELAY_PIN 5   // GPIO-Pin für das Relais

// Statusanzeige
#define LED_PIN 2     // Eingebaute LED des ESP32 (Pin 2 bei den meisten Modellen)

// ------ GLOBALE VARIABLEN ------
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastMsgTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastBlinkTime = 0;
bool ledState = false;
float temperature = 0;
float humidity = 0;
bool relayState = false;

// ------ SETUP ------
void setup() {
  // Serielle Schnittstelle initialisieren
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("=============================================");
  Serial.println("SwissAirDry ESP32 Test-Programm");
  Serial.println("Version: 1.0.0");
  Serial.println("=============================================");
  
  // GPIO-Pins initialisieren
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Relais initial ausschalten
  
  // DHT-Sensor initialisieren
  dht.begin();
  Serial.println("DHT-Sensor initialisiert");
  
  // WLAN-Verbindung herstellen, wenn Zugangsdaten eingetragen wurden
  if (strlen(ssid) > 0) {
    setupWifi();
    
    // MQTT-Client konfigurieren
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);
  } else {
    Serial.println("WLAN-Zugangsdaten nicht konfiguriert. Arbeite im Offline-Modus.");
  }
  
  Serial.println("Setup abgeschlossen. Test-Programm läuft...");
}

// ------ LOOP ------
void loop() {
  // Aktuelle Zeit
  unsigned long currentMillis = millis();
  
  // WLAN und MQTT-Verbindung prüfen
  if (strlen(ssid) > 0) {
    if (WiFi.status() != WL_CONNECTED) {
      setupWifi();
    }
    
    if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
      reconnectMqtt();
    }
    
    if (mqttClient.connected()) {
      mqttClient.loop();
    }
  }
  
  // Sensordaten alle 2 Sekunden auslesen
  if (currentMillis - lastReadTime > 2000) {
    lastReadTime = currentMillis;
    readSensorData();
  }
  
  // MQTT-Nachricht alle 10 Sekunden senden, wenn verbunden
  if (mqttClient.connected() && currentMillis - lastMsgTime > 10000) {
    lastMsgTime = currentMillis;
    publishSensorData();
  }
  
  // LED alle 500ms blinken lassen
  if (currentMillis - lastBlinkTime > 500) {
    lastBlinkTime = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
  
  // Kurze Pause
  delay(10);
}

// ------ WLAN-VERBINDUNG HERSTELLEN ------
void setupWifi() {
  Serial.print("Verbinde mit WLAN SSID: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Mit WLAN verbunden");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Verbindung mit WLAN fehlgeschlagen");
  }
}

// ------ MQTT-VERBINDUNG WIEDERHERSTELLEN ------
void reconnectMqtt() {
  int attempts = 0;
  
  // Versuche, eine Verbindung herzustellen
  while (!mqttClient.connected() && attempts < 3) {
    Serial.print("Versuche, eine MQTT-Verbindung herzustellen...");
    
    // Verbindungsversuch
    if (mqttClient.connect(mqtt_client_id)) {
      Serial.println("verbunden");
      
      // Nach erfolgreicher Verbindung Topic abonnieren
      mqttClient.subscribe(mqtt_topic);
      
      // Online-Status senden
      mqttClient.publish((String(mqtt_topic) + "/status").c_str(), "online", true);
    } else {
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Versuche es in 5 Sekunden erneut...");
      delay(5000);
    }
    
    attempts++;
  }
}

// ------ MQTT-CALLBACK-FUNKTION ------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Empfangene Nachricht ausgeben
  Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println(message);
  
  // Relais steuern, wenn Befehle empfangen werden
  if (message == "relay_on") {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    Serial.println("Relais eingeschaltet");
    mqttClient.publish((String(mqtt_topic) + "/relay").c_str(), "on", true);
  } else if (message == "relay_off") {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    Serial.println("Relais ausgeschaltet");
    mqttClient.publish((String(mqtt_topic) + "/relay").c_str(), "off", true);
  }
}

// ------ SENSORDATEN AUSLESEN ------
void readSensorData() {
  // Temperatur und Luftfeuchtigkeit vom DHT-Sensor auslesen
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();
  
  // Prüfen, ob Auslesen erfolgreich war
  if (isnan(newT) || isnan(newH)) {
    Serial.println("Fehler beim Auslesen des DHT-Sensors!");
  } else {
    // Sensordaten aktualisieren
    temperature = newT;
    humidity = newH;
    
    // Auf der seriellen Schnittstelle ausgeben
    Serial.print("Temperatur: ");
    Serial.print(temperature);
    Serial.print(" °C, Luftfeuchtigkeit: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}

// ------ SENSORDATEN ÜBER MQTT SENDEN ------
void publishSensorData() {
  // JSON-String für Sensordaten erstellen
  String json = "{";
  json += "\"temperature\":" + String(temperature);
  json += ",\"humidity\":" + String(humidity);
  json += ",\"relay\":" + String(relayState ? "true" : "false");
  json += "}";
  
  // Daten senden
  mqttClient.publish(mqtt_topic, json.c_str(), true);
  Serial.println("Sensordaten per MQTT gesendet");
}