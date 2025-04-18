# SwissAirDry ESP32 Arduino Test

Diese Ordner enthält einen einfachen Test-Sketch für die SwissAirDry ESP32-Hardware, der mit der Arduino IDE geflasht werden kann.

## Voraussetzungen

1. **Arduino IDE** - Herunterladen und installieren von [arduino.cc](https://www.arduino.cc/en/software)
2. **ESP32-Boardunterstützung** - Folgen Sie diesen Schritten:
   - Öffnen Sie die Arduino IDE
   - Gehen Sie zu Datei > Voreinstellungen
   - Fügen Sie folgende URL unter "Zusätzliche Boardverwalter-URLs" ein:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Gehen Sie zu Werkzeuge > Board > Boardverwalter
   - Suchen Sie nach "esp32" und installieren Sie das Paket von Espressif Systems

3. **Benötigte Bibliotheken** - Installieren Sie diese über Sketch > Bibliothek einbinden > Bibliotheken verwalten:
   - **PubSubClient** von Nick O'Leary (für MQTT)
   - **DHT sensor library** von Adafruit (für den DHT-Sensor)
   - **Adafruit Unified Sensor** von Adafruit (wird von der DHT-Bibliothek benötigt)

## Anschlussplan

### Erforderliche Hardware
- ESP32 Development Board (ESP32-WROOM-32)
- DHT22-Sensor (oder DHT11)
- 5V Relais-Modul (optional)
- Breadboard und Jumper-Kabel
- USB-Kabel zur Programmierung

### Anschlüsse
1. **DHT22/DHT11 Sensor**:
   - VCC an 3,3V des ESP32
   - GND an GND des ESP32
   - DATA an GPIO 4 des ESP32 (kann in der Sketch geändert werden)

2. **Relais**:
   - VCC an 5V (wenn vorhanden, ansonsten 3,3V)
   - GND an GND des ESP32
   - IN an GPIO 5 des ESP32 (kann in der Sketch geändert werden)

3. **LED** (zur Statusanzeige):
   - Eingebaute LED des ESP32 an GPIO 2 wird verwendet

## Anleitung zum Flashen

1. Schließen Sie den ESP32 per USB an Ihren Computer an
2. Öffnen Sie die Arduino IDE
3. Öffnen Sie die Datei `SwissAirDry_ESP32_Test.ino` aus dem Ordner `SwissAirDry_ESP32_Test`
4. Wählen Sie das korrekte Board unter Werkzeuge > Board > ESP32 Arduino > ESP32 Dev Module
5. Wählen Sie den korrekten Port unter Werkzeuge > Port
6. Tragen Sie Ihre WLAN-Zugangsdaten in den Sketch ein (optional):
   ```c
   const char* ssid = "IhrWLANName";
   const char* password = "IhrWLANPasswort";
   ```
7. Klicken Sie auf den Upload-Button (Pfeil nach rechts)
8. Öffnen Sie den seriellen Monitor (Werkzeuge > Serieller Monitor) und stellen Sie die Baudrate auf 115200 ein, um die Ausgabe zu sehen

## Funktionen des Test-Sketches

- **LED-Blinken**: Die eingebaute LED des ESP32 blinkt, um anzuzeigen, dass das Programm läuft
- **Sensor-Auslesung**: Liest Temperatur und Luftfeuchtigkeit vom DHT-Sensor aus und zeigt sie im seriellen Monitor an
- **WLAN-Verbindung**: Verbindet sich mit dem konfigurierten WLAN-Netzwerk (wenn Zugangsdaten eingetragen wurden)
- **MQTT-Kommunikation**: Sendet Sensordaten an den MQTT-Broker und empfängt Befehle (wenn WLAN konfiguriert ist)
- **Relais-Steuerung**: Kann das angeschlossene Relais über MQTT-Befehle steuern

## Fehlerbehebung

- **ESP32 wird nicht erkannt**: Stellen Sie sicher, dass Sie den richtigen USB-Treiber installiert haben
- **Upload-Fehler**: Halten Sie den BOOT-Knopf am ESP32 gedrückt, während der Upload beginnt
- **DHT-Sensor wird nicht erkannt**: Überprüfen Sie die Verkabelung und stellen Sie sicher, dass Sie einen 4,7K-10K Pull-up-Widerstand zwischen DATA und VCC haben
- **WLAN-Verbindung fehlgeschlagen**: Überprüfen Sie die eingegebenen Zugangsdaten

## Nächste Schritte

Nachdem Sie bestätigt haben, dass der ESP32 und die angeschlossenen Komponenten korrekt funktionieren, können Sie die vollständige SwissAirDry-Firmware mit PlatformIO installieren und konfigurieren.