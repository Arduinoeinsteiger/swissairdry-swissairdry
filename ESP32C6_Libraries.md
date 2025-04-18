# Bibliotheken und Konfiguration für ESP32-C6 mit Farbdisplay

## Benötigte Bibliotheken

Für das SwissAirDry ESP32-C6 Projekt mit Farbdisplay werden die folgenden Bibliotheken benötigt. Diese können über den Arduino-Bibliotheksmanager installiert werden:

1. **TFT_eSPI** - Leistungsstarke Bibliothek für TFT-Displays
   - Version: 2.5.0 oder neuer
   - Autor: Bodmer

2. **ArduinoJson** - JSON-Verarbeitung
   - Version: 6.20.0 oder neuer
   - Autor: Benoit Blanchon

3. **PubSubClient** - MQTT-Client
   - Version: 2.8.0 oder neuer
   - Autor: Nick O'Leary

4. **QRCode** - QR-Code Generator
   - Version: 0.0.1 oder neuer
   - Autor: Richard Moore

5. **Time** - Zeitfunktionen
   - Version: 1.6.1 oder neuer
   - Autor: Michael Margolis

## Konfigurationsschritte für das TFT-Display

Die TFT_eSPI-Bibliothek erfordert eine spezifische Konfiguration für Ihr Display. Nach der Installation der Bibliothek müssen Sie die Datei `User_Setup.h` in der TFT_eSPI-Bibliothek bearbeiten.

### 1. Anpassung der Datei User_Setup.h

1. Navigieren Sie zum Arduino-Bibliotheksordner
2. Öffnen Sie den Ordner `TFT_eSPI`
3. Öffnen Sie die Datei `User_Setup.h`
4. Deaktivieren Sie alle vordefinierten Display-Treiber und kommentieren Sie sie aus
5. Fügen Sie folgende Konfiguration für das 1.47" Display ein:

```cpp
// Für ESP32-C6 mit 1.47-Zoll-Display (172 x 320)
#define ST7789_DRIVER     // Stellen Sie sicher, dass der richtige Treiber aktiviert ist
#define TFT_WIDTH  172    // Displaybreite
#define TFT_HEIGHT 320    // Displayhöhe

// Definieren Sie die GPIO-Pins für das Display
#define TFT_MISO -1       // Nicht verwendet
#define TFT_MOSI 7        // SDA Pin
#define TFT_SCLK 6        // SCL Pin
#define TFT_CS   9        // CS Pin
#define TFT_DC   8        // DC Pin
#define TFT_RST  5        // Reset Pin

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

// Farbeinstellungen
#define SPI_FREQUENCY  27000000   // Standard für ESP32-C6
```

### 2. SD-Karten-Konfiguration

Wenn Sie die SD-Karte verwenden möchten, stellen Sie sicher, dass Sie den richtigen CS-Pin für die SD-Karte im Sketch konfigurieren:

```cpp
#define SD_CS_PIN 10  // Ändern Sie dies entsprechend Ihrer Hardware
```

## Anpassungen für Ihr spezifisches ESP32-C6 Board

Je nach genauem Modell und Pinout Ihres ESP32-C6 Boards mit Farbdisplay müssen Sie möglicherweise weitere Anpassungen vornehmen:

1. **Überprüfen Sie die Pins**: Kontrollieren Sie die GPIO-Nummern für Display, SD-Karte und Relais
2. **Display-Orientierung**: Passen Sie `tft.setRotation(...)` an (0-3, je nach Ausrichtung)
3. **Speicherkonfiguration**: Bei Speicherproblemen können Sie nicht benötigte Funktionen auskommentieren

## QR-Code optimieren

Für eine bessere Lesbarkeit des QR-Codes können Sie den Skalierungsfaktor anpassen:

```cpp
#define QR_CODE_SIZE 4  // Erhöhen für größere QR-Codes
```

## Hilfreiche Links

- [Dokumentation zur TFT_eSPI-Bibliothek](https://github.com/Bodmer/TFT_eSPI)
- [ESP32-C6 Dokumentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/index.html)
- [Befehlsreferenz für das ST7789-Display](https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ST7789_Rotation.h)

## Troubleshooting

- **Display bleibt leer**: Überprüfen Sie die Pins und die Stromversorgung
- **SD-Karte wird nicht erkannt**: Überprüfen Sie den CS-Pin und die SPI-Konfiguration
- **MQTT-Verbindungsprobleme**: Stellen Sie sicher, dass Ihre Netzwerkkonfiguration korrekt ist
- **Speicherfehler**: Reduzieren Sie die Komplexität (weniger Fonts oder Funktionen)