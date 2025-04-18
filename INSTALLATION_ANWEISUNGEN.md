# Installationsanleitung für SwissAirDry QR-Code Sketch

## Benötigte Bibliotheken

Um den `SwissAirDry_Wemos_D1_Mini_QR.ino` Sketch zu kompilieren, müssen Sie die folgenden Bibliotheken in der Arduino IDE installieren:

1. **Adafruit GFX** (für Grafikfunktionen)
2. **Adafruit SSD1306** (für OLED-Display)
3. **QRCode-Generator Bibliothek von ricmoo** 

## Installationsschritte

### 1. Adafruit Bibliotheken installieren

1. Öffnen Sie die Arduino IDE
2. Gehen Sie zu **Sketch > Bibliotheken einbinden > Bibliotheken verwalten...**
3. Suchen Sie nach "Adafruit GFX" und installieren Sie die Bibliothek
4. Suchen Sie nach "Adafruit SSD1306" und installieren Sie die Bibliothek

### 2. QRCode Bibliothek installieren

Da die benötigte QR-Code-Bibliothek nicht direkt im Library Manager verfügbar ist, müssen Sie sie manuell installieren:

1. Besuchen Sie die GitHub-Seite: https://github.com/ricmoo/QRCode
2. Klicken Sie auf den grünen "Code" Button und wählen Sie "Download ZIP"
3. In der Arduino IDE: **Sketch > Bibliotheken einbinden > .ZIP-Bibliothek hinzufügen...**
4. Wählen Sie die heruntergeladene ZIP-Datei aus

Alternativ können Sie die QR-Code-Bibliothek auch über Git klonen:
```
cd ~/Documents/Arduino/libraries/
git clone https://github.com/ricmoo/QRCode.git
```

## Verwendung des Sketches

1. Passen Sie Ihre WLAN-Daten an (Zeilen 16-17):
   ```cpp
   const char* ssid = "IHRE_SSID";
   const char* password = "IHR_PASSWORT";
   ```

2. Kompilieren und hochladen Sie den Sketch auf Ihren Wemos D1 Mini

3. Das Display zeigt einen QR-Code an, der die IP-Adresse des Geräts und ein automatisch generiertes Passwort enthält

## Hardware-Anschlüsse

- **OLED Display**: I2C Anschluss an D1 (SCL) und D2 (SDA)
- **Relais** (optional): D5

## Fehlerbehebung

Falls der QR-Code zu klein oder zu groß für Ihr Display ist, können Sie die Skalierung anpassen:
```cpp
#define QR_CODE_VERSION 3   // Verringern für kleinere QR-Codes (z.B. 1 oder 2)
#define QR_CODE_SCALE 2     // Erhöhen oder verringern Sie diesen Wert
```

Wenn die QR-Code-Bibliothek nicht korrekt funktioniert, können Sie alternativ eine Arduino-Bibliothek namens "QRCode" von Ruilov verwenden, die über den Library Manager installiert werden kann.