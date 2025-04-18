# SwissAirDry ESP32-C6 XIAO Firmware

Diese Firmware ermöglicht die Kommunikation eines ESP32-C6 XIAO mit dem SwissAirDry API Server über REST API oder MQTT.

## Funktionen

- Kommunikation mit dem SwissAirDry API-Server über HTTP (REST API) oder MQTT
- Anzeige von Sensordaten auf dem 1.47" Display
- Fernsteuerung von Geräten über die API
- OTA-Updates (Over-the-Air) für einfache Firmware-Aktualisierungen
- RGB-LED-Statusanzeige für schnelle visuelle Diagnose
- Unterstützung für SPIFFS-Dateisystem
- Automatische Reconnect-Funktion für WLAN und MQTT

## Hardware-Anforderungen

- Seeed Studio XIAO ESP32C6 Board
- 1.47" RGB-Display (172x320 Pixel) für XIAO
- (Optional) Relais-Modul für Gerätesteuerung
- (Optional) Sensoren für Temperatur, Luftfeuchtigkeit usw.

## Benötigte Bibliotheken

- WiFi.h (in ESP32-Arduino-Core enthalten)
- HTTPClient.h (in ESP32-Arduino-Core enthalten)
- ArduinoJson (Version 7.x oder höher)
- TFT_eSPI (angepasst für XIAO ESP32C6 1.47" Display)
- SPI.h (in ESP32-Arduino-Core enthalten)
- FS.h (in ESP32-Arduino-Core enthalten)
- SPIFFS.h (in ESP32-Arduino-Core enthalten)
- Update.h (in ESP32-Arduino-Core enthalten)
- ArduinoOTA.h (in ESP32-Arduino-Core enthalten)

## PubSubClient für MQTT installieren

Die PubSubClient-Bibliothek wird für die MQTT-Kommunikation benötigt. Sie kann über den Arduino Library Manager installiert werden:

1. Öffnen Sie die Arduino IDE
2. Gehen Sie zu Sketch -> Bibliothek einbinden -> Bibliotheken verwalten
3. Suchen Sie nach "PubSubClient"
4. Installieren Sie die Bibliothek von Nick O'Leary

Alternativ können Sie die Bibliothek auch manuell von GitHub herunterladen und installieren:
[https://github.com/knolleary/pubsubclient](https://github.com/knolleary/pubsubclient)

## TFT_eSPI konfigurieren

Die TFT_eSPI-Bibliothek muss für das XIAO ESP32C6 1.47" Display konfiguriert werden:

1. Navigieren Sie zum Installationsverzeichnis der TFT_eSPI-Bibliothek (normalerweise unter Arduino/libraries/TFT_eSPI)
2. Öffnen Sie die Datei User_Setup_Select.h und kommentieren Sie alle #include-Zeilen aus
3. Fügen Sie folgende Zeile hinzu: `#include <User_Setups/Setup_XIAO_ESP32C6.h>`
4. Erstellen Sie eine neue Datei Setup_XIAO_ESP32C6.h im Verzeichnis User_Setups mit folgendem Inhalt:

```c
// Konfiguration für XIAO ESP32C6 mit 1.47" Display

#define ST7789_DRIVER
#define TFT_WIDTH 172
#define TFT_HEIGHT 320
#define TFT_MOSI 10  // SDA
#define TFT_SCLK 8   // SCL
#define TFT_CS 9     // CS
#define TFT_DC 7     // DC
#define TFT_RST 6    // RESET
#define TFT_BL 5     // Hintergrundbeleuchtung
#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY 40000000
```

## Installation und Konfiguration

1. Stellen Sie sicher, dass alle oben genannten Bibliotheken installiert sind
2. Öffnen Sie die Sketch-Datei in der Arduino IDE
3. Passen Sie die WLAN-Konfiguration an (ssid und password)
4. Passen Sie die API-Konfiguration an (api_host, api_port und device_id)
5. Überprüfen Sie die Pin-Konfiguration für Relais und RGB-LED
6. Kompilieren und hochladen Sie die Firmware auf Ihr ESP32-C6 XIAO Board

## Verwendung mit dem SwissAirDry API-Server

1. Starten Sie den SwissAirDry API-Server
2. Stellen Sie sicher, dass der ESP32-C6 und der API-Server im selben Netzwerk sind
3. Geben Sie die IP-Adresse des API-Servers in der Variable `api_host` an
4. Der Standard-Port des API-Servers ist 5000 (kann in `api_port` geändert werden)
5. Vergeben Sie eine eindeutige Geräte-ID in der Variable `device_id`

## LED-Statusanzeigen

- **Blinkendes Orange**: WLAN-Verbindungsprobleme
- **Blinkendes Rot**: API-Verbindungsprobleme
- **Grün**: Verbunden und aktiv (Relais eingeschaltet)
- **Cyan**: Verbunden und im Standby (Relais ausgeschaltet)

## Fehlerbehebung

- **Keine WLAN-Verbindung**: Überprüfen Sie SSID und Passwort
- **Keine API-Verbindung**: Überprüfen Sie die IP-Adresse des API-Servers und den Port
- **Display zeigt nichts an**: Überprüfen Sie die TFT_eSPI-Konfiguration
- **OTA funktioniert nicht**: Stellen Sie sicher, dass ESP32 und Computer im selben Netzwerk sind

## Vergleich der REST API und MQTT Versionen

### SwissAirDry_ESP32C6_RestAPI.ino
Diese Firmware verwendet HTTP-Requests zum Senden und Empfangen von Daten. Vorteile:
- Einfache Implementierung ohne zusätzliche Server-Komponenten
- Direkter Zugriff auf API-Endpunkte
- Keine dauerhafte Verbindung nötig

### SwissAirDry_ESP32C6_MQTT.ino
Diese Firmware verwendet MQTT für die Kommunikation. Vorteile:
- Effizienteres Protokoll für IoT-Anwendungen
- Echtzeit-Kommunikation in beide Richtungen
- Geringerer Overhead bei Datenübertragung
- Bessere Unterstützung für Netzwerke mit instabiler Verbindung
- Ermöglicht Gruppierung von Geräten über Topics

Für Produktionsumgebungen wird die MQTT-Version empfohlen, da sie effizienter und zuverlässiger ist, besonders bei vielen Geräten.

## Lizenz

Copyright © 2023-2025 Swiss Air Dry Team
Alle Rechte vorbehalten.