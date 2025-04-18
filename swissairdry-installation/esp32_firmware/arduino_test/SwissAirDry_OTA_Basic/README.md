# SwissAirDry ESP32 OTA-Basic

Dieses Programm ist eine minimale Version für den ESP32, die nur OTA-Updates (Over-the-Air) aktiviert. Damit kann der ESP32 über WLAN geflasht werden, ohne dass er physisch per USB angeschlossen werden muss.

## Zweck

Diese Firmware soll nur als initiale Grundinstallation über USB dienen, um danach alle weiteren Updates drahtlos durchführen zu können. Die eigentliche SwissAirDry-Funktionalität wird später über OTA nachgeladen.

## Anleitung

### 1. Vorbereitung

1. Arduino IDE installieren (von [arduino.cc](https://www.arduino.cc/en/software))
2. ESP32-Unterstützung installieren:
   - Öffnen Sie die Arduino IDE
   - Gehen Sie zu `Datei` > `Voreinstellungen`
   - Fügen Sie diese URL zu den Boardverwalter-URLs hinzu:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Gehen Sie zu `Werkzeuge` > `Board` > `Boardverwalter`
   - Suchen Sie nach "esp32" und installieren Sie das Paket von Espressif Systems

### 2. WLAN-Daten eintragen

Öffnen Sie den Sketch und tragen Sie Ihre WLAN-Daten ein:

```cpp
// ------ WLAN-KONFIGURATION ------
const char* ssid = "Ihr_WLAN_Name";          // Hier eintragen
const char* password = "Ihr_WLAN_Passwort";  // Hier eintragen
```

Optional können Sie auch den Hostnamen und das OTA-Passwort anpassen:

```cpp
// ------ OTA-KONFIGURATION ------
const char* hostname = "swissairdry-esp32";  // Name im Netzwerk
const char* ota_password = "swissairdry";    // Passwort für Updates
```

### 3. Initialen Upload per USB durchführen

1. Verbinden Sie den ESP32 per USB mit Ihrem Computer
2. Wählen Sie das richtige Board: `Werkzeuge` > `Board` > `ESP32 Arduino` > `ESP32 Dev Module`
3. Wählen Sie den richtigen COM-Port: `Werkzeuge` > `Port` > (USB-Port des ESP32)
4. Klicken Sie auf `Sketch` > `Hochladen`

### 4. Überprüfen der OTA-Funktionalität

Nach dem Upload sollte der ESP32:
1. Mit Ihrem WLAN verbinden
2. Eine IP-Adresse erhalten
3. Im seriellen Monitor die IP-Adresse und OTA-Anweisungen anzeigen
4. Die eingebaute LED sollte langsam blinken (1 Sekunde Intervall)

### 5. Weitere Updates über WLAN

Nachfolgende Updates können nun über WLAN erfolgen:

1. In der Arduino IDE: `Werkzeuge` > `Port` > `Netzwerk-Ports` > (ESP32 auswählen)
2. Dann normal auf `Sketch` > `Hochladen` klicken

## Statusanzeige (LED)

Die eingebaute LED des ESP32 zeigt den Status an:
- **Schnelles Blinken (200ms)**: Keine WLAN-Verbindung
- **Langsames Blinken (1 Sekunde)**: Verbunden mit WLAN, bereit für OTA

## Fehlerbehebung

- **ESP32 erscheint nicht in Netzwerk-Ports**: 
  - Prüfen Sie, ob die WLAN-Daten korrekt sind
  - Stellen Sie sicher, dass Ihr Computer im selben Netzwerk ist
  - Versuchen Sie, den ESP32 im Browser unter `http://swissairdry-esp32.local` aufzurufen
  - Alternativ nutzen Sie die im seriellen Monitor angezeigte IP-Adresse

- **Upload-Fehler**:
  - Bei "Connection refused": ESP32 ist möglicherweise nicht mehr erreichbar
  - Bei "Auth failed": Prüfen Sie das OTA-Passwort
  - Bei instabiler Verbindung: ESP32 näher zum WLAN-Router positionieren