# SwissAirDry ESP32 Firmware

Diese Komponente enthält die Firmware für ESP32-basierte IoT-Geräte, die mit dem SwissAirDry-System kommunizieren. Die ESP32-Geräte sammeln Sensordaten von den Trocknungsgeräten und übertragen diese über MQTT an das Backend-System.

## Überblick

Die ESP32-Geräte dienen als IoT-Gateways für die SwissAirDry-Trocknungsgeräte und bieten folgende Funktionen:

- Erfassung von Temperatur, Luftfeuchtigkeit und Druckdaten über angeschlossene Sensoren
- Überwachung des Energieverbrauchs der Trocknungsgeräte
- Steuerung der Trocknungsgeräte (Ein/Aus, Geschwindigkeit, etc.)
- MQTT-Kommunikation mit dem SwissAirDry-Backend
- OTA-Updates (Over-the-Air) für Firmware-Aktualisierungen
- Lokale Zwischenspeicherung von Daten bei Verbindungsunterbrechungen
- Bluetooth-Konfiguration über die SwissAirDry Mobile App

## Voraussetzungen

Zur Entwicklung und Bereitstellung der Firmware benötigen Sie:

- ESP32 Development Board (empfohlen: ESP32-WROOM-32)
- VSCode mit PlatformIO Extension
- USB-Kabel zur Programmierung
- DHT22 oder BME280 Sensoren für Temperatur und Luftfeuchtigkeit
- PZEM-004T Energiemessmodul (optional)
- 5V Relais für die Gerätesteuerung (optional)

## Projektstruktur

```
esp32_firmware/
├── include/                 # Header-Dateien
│   ├── config.h             # Konfigurationsdefinitionen
│   ├── mqtt_client.h        # MQTT-Client-Funktionen
│   ├── sensors.h            # Sensorfunktionen
│   └── wifi_manager.h       # WLAN-Verbindungsverwaltung
├── lib/                     # Bibliotheken
│   ├── ArduinoJson/         # JSON-Parser/Generator
│   ├── PubSubClient/        # MQTT-Client
│   └── ...                  # Weitere Bibliotheken
├── src/                     # Quellcode
│   ├── main.cpp             # Hauptprogramm
│   ├── mqtt_client.cpp      # MQTT-Client-Implementierung
│   ├── sensors.cpp          # Sensorimplementierung
│   ├── wifi_manager.cpp     # WLAN-Manager-Implementierung
│   └── ota_updater.cpp      # OTA-Update-Handler
├── data/                    # Dateien für das SPIFFS-Dateisystem
│   ├── config.json          # Konfigurationsdatei
│   ├── index.html           # Weboberfläche
│   └── styles.css           # CSS für die Weboberfläche
├── platformio.ini           # PlatformIO-Konfiguration
└── README.md                # Diese Datei
```

## Installation

1. Klonen Sie das Repository:
   ```
   git clone https://github.com/swissairdry/esp32_firmware.git
   ```

2. Öffnen Sie das Projekt in VSCode mit der PlatformIO-Erweiterung

3. Passen Sie die Konfiguration in `data/config.json` an oder erstellen Sie eine `config_local.json` mit Ihren spezifischen Einstellungen

4. Kompilieren und Hochladen der Firmware:
   ```
   pio run -t upload
   ```

5. Hochladen des Dateisystems (für die Weboberfläche und Konfigurationsdateien):
   ```
   pio run -t uploadfs
   ```

## Konfiguration

Die Basiskonfiguration erfolgt in der `data/config.json` Datei:

```json
{
  "device": {
    "name": "SwissAirDry-Gateway",
    "id": "gateway001",
    "type": "esp32-gateway"
  },
  "wifi": {
    "ssid": "",
    "password": "",
    "ap_ssid": "SwissAirDry-Setup",
    "ap_password": "setup1234"
  },
  "mqtt": {
    "broker": "mqtt.swissairdry.com",
    "port": 1883,
    "username": "",
    "password": "",
    "client_id": "gateway001",
    "base_topic": "swissairdry/devices/gateway001"
  },
  "sensors": {
    "dht_pin": 4,
    "dht_type": "DHT22",
    "use_bme280": false,
    "energy_meter_enabled": false,
    "energy_meter_rx_pin": 16,
    "energy_meter_tx_pin": 17
  },
  "control": {
    "relay_pin": 5,
    "has_speed_control": false,
    "speed_control_pin": 13
  },
  "system": {
    "read_interval": 60,
    "reporting_interval": 300,
    "ota_enabled": true,
    "debug_mode": false
  }
}
```

Bei der ersten Inbetriebnahme startet der ESP32 im Access-Point-Modus mit der SSID "SwissAirDry-Setup". Verbinden Sie sich mit diesem WLAN und navigieren Sie zu `http://192.168.4.1`, um die Konfiguration über die Weboberfläche vorzunehmen.

## Hauptfunktionen

### WiFi-Verbindung

Der WiFi-Manager versucht zunächst, eine Verbindung mit den gespeicherten Zugangsdaten herzustellen. Wenn dies nicht erfolgreich ist, wird der AP-Modus aktiviert, damit der Benutzer die Konfiguration vornehmen kann.

```cpp
// In wifi_manager.cpp
bool WifiManager::connect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < _timeout) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi-Verbindung fehlgeschlagen, AP-Modus wird gestartet");
        startAP();
        return false;
    }

    Serial.println("WiFi verbunden");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
    return true;
}
```

### MQTT-Kommunikation

Die MQTT-Klasse verwaltet die Verbindung zum MQTT-Broker und die Veröffentlichung von Sensordaten sowie den Empfang von Befehlen.

```cpp
// In mqtt_client.cpp
void MqttClient::publishData(const JsonDocument& data) {
    if (!_client.connected()) {
        if (!connect()) {
            return;
        }
    }

    String topic = _baseTopic + "/data";
    String payload;
    serializeJson(data, payload);

    Serial.print("Veröffentliche Daten auf Topic: ");
    Serial.println(topic);
    Serial.println(payload);

    if (_client.publish(topic.c_str(), payload.c_str(), true)) {
        Serial.println("Daten erfolgreich veröffentlicht");
    } else {
        Serial.println("Veröffentlichung fehlgeschlagen");
    }
}
```

### Sensor-Datenerfassung

Die Sensorklasse liest die angeschlossenen Sensoren aus und gibt die Daten als JSON-Dokument zurück.

```cpp
// In sensors.cpp
JsonDocument SensorManager::readSensorData() {
    JsonDocument data;
    data["timestamp"] = getTime();
    
    JsonObject environment = data.createNestedObject("environment");
    
    if (_useDHT) {
        float h = _dht.readHumidity();
        float t = _dht.readTemperature();
        
        if (!isnan(h) && !isnan(t)) {
            environment["humidity"] = h;
            environment["temperature"] = t;
        }
    } else if (_useBME280) {
        environment["temperature"] = _bme.readTemperature();
        environment["humidity"] = _bme.readHumidity();
        environment["pressure"] = _bme.readPressure() / 100.0F;
    }
    
    if (_useEnergyMeter) {
        JsonObject energy = data.createNestedObject("energy");
        energy["voltage"] = _pzem.voltage();
        energy["current"] = _pzem.current();
        energy["power"] = _pzem.power();
        energy["energy"] = _pzem.energy();
        energy["frequency"] = _pzem.frequency();
        energy["pf"] = _pzem.pf();
    }
    
    return data;
}
```

### OTA-Updates

Die Firmware unterstützt Over-the-Air-Updates, um die Geräte aus der Ferne zu aktualisieren.

```cpp
// In ota_updater.cpp
void OtaUpdater::setup() {
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
        Serial.println("\nOTA-Update abgeschlossen");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Fortschritt: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Fehler[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Authentifizierungsfehler");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin-Fehler");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Verbindungsfehler");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Empfangsfehler");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End-Fehler");
        }
    });
    
    ArduinoOTA.begin();
}
```

## Konfiguration des MQTT-Brokers

Für die Kommunikation mit dem SwissAirDry-Backend muss der MQTT-Broker entsprechend konfiguriert sein. Der ESP32 kommuniziert mit dem Broker über folgende Topics:

### Veröffentlichte Topics (ESP32 → Backend):

- `swissairdry/devices/{device_id}/data` - Sensordaten im JSON-Format
- `swissairdry/devices/{device_id}/status` - Statusupdates (online, offline, etc.)
- `swissairdry/devices/{device_id}/debug` - Debug-Informationen (nur im Debug-Modus)

### Abonnierte Topics (Backend → ESP32):

- `swissairdry/devices/{device_id}/commands` - Steuerungsbefehle für das Gerät
- `swissairdry/devices/{device_id}/config` - Konfigurationsänderungen
- `swissairdry/devices/{device_id}/ota` - OTA-Update-Befehle

## Integration mit dem SwissAirDry-Backend

Die Integration mit dem Backend erfolgt über den MQTT-Broker. Stellen Sie sicher, dass der MQTT-Broker im `docker-compose.yml` der Docker Base API korrekt konfiguriert ist.

## Fehlerbehebung

1. **ESP32 verbindet sich nicht mit WLAN**
   - Überprüfen Sie SSID und Passwort in der Konfiguration
   - Prüfen Sie die Signalstärke des WLANs
   - Starten Sie den ESP32 im AP-Modus und konfigurieren Sie das WLAN neu

2. **Keine MQTT-Verbindung**
   - Überprüfen Sie die MQTT-Broker-Konfiguration
   - Stellen Sie sicher, dass der MQTT-Broker läuft und erreichbar ist
   - Überprüfen Sie Benutzername und Passwort für den MQTT-Broker
   - Prüfen Sie, ob die Firewall die Verbindung blockiert

3. **Sensoren liefern keine Daten**
   - Überprüfen Sie die Verkabelung der Sensoren
   - Prüfen Sie, ob die Sensoren in der Konfiguration richtig aktiviert sind
   - Prüfen Sie, ob die richtigen Pins in der Konfiguration angegeben sind

4. **OTA-Updates funktionieren nicht**
   - Stellen Sie sicher, dass das Gerät mit dem WLAN verbunden ist
   - Prüfen Sie, ob OTA in der Konfiguration aktiviert ist
   - Vergewissern Sie sich, dass genügend Speicherplatz für das Update verfügbar ist

## Weiterentwicklung

Diese Firmware kann je nach Ihren spezifischen Anforderungen erweitert werden. Mögliche Erweiterungen sind:

- Integration weiterer Sensoren
- Erweiterung der Weboberfläche
- Implementierung von Bluetooth Low Energy für die Konfiguration über die Mobile App
- Hinzufügen von Energiesparfunktionen für batteriebetriebene Geräte
- Implementierung von Verschlüsselung für die MQTT-Kommunikation

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.

Diese Firmware ist ausschließlich für die Verwendung mit SwissAirDry-Produkten bestimmt.