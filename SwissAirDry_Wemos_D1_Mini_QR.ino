// SwissAirDry Wemos D1 Mini mit QR-Code-Anzeige
// OTA-Updates + QR-Code mit IP-Adresse und Web-Passwort

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h>       // QR-Code-Bibliothek
#include <EEPROM.h>

// ----- WLAN-KONFIGURATION -----
// Bitte hier die WLAN-Daten eintragen
const char* ssid = "SSID";
const char* password = "PASSWORT";

// ----- HARDWARE-KONFIGURATION -----
// Festverdrahtete Pins für Wemos D1 Mini
#define LED_PIN 2        // GPIO2 (D4 auf Wemos D1 Mini) - Blau LED on-board
#define LED_ON LOW       // LED ist aktiv LOW (invertiert)
#define LED_OFF HIGH
#define RELAY_PIN D5     // Relais-Pin für Desinfektionsgerät

// Display-Konfiguration
#define OLED_RESET -1    // Kein Reset-Pin verwendet
#define SCREEN_WIDTH 128 // OLED Display Breite in Pixeln
#define SCREEN_HEIGHT 64 // OLED Display Höhe in Pixeln
#define OLED_ADDR 0x3C   // I2C-Adresse des OLED-Displays

// QR-Code-Konfiguration
#define QR_CODE_VERSION 3   // QR-Code-Version (1-40): 3 = 29x29 Module
#define QR_CODE_SCALE 2     // Vergrößerungsfaktor für QR-Code-Pixel

// Web-UI Konfiguration
String webPassword = "";   // Wird automatisch generiert
#define WEB_SERVER_PORT 80  // Webserver-Port

// Objekte initialisieren
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool displayAvailable = false;

// Hostname mit eindeutiger Chip-ID
String hostname = "SwissAirDry-";

// QR-Code-Objekt
QRCode qrcode;

void setup() {
  // Serielle Verbindung starten
  Serial.begin(115200);
  Serial.println("\n\nSwissAirDry für Wemos D1 Mini mit QR-Code");
  
  // EEPROM initialisieren (für Einstellungen)
  EEPROM.begin(512);
  
  // Eindeutigen Hostnamen erstellen
  uint16_t chipId = ESP.getChipId() & 0xFFFF;
  hostname += String(chipId, HEX);
  Serial.print("Hostname: ");
  Serial.println(hostname);
  
  // LED und Relais konfigurieren
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(RELAY_PIN, LOW);  // Relais aus
  
  // Display initialisieren
  Wire.begin();  // SDA=D2(GPIO4), SCL=D1(GPIO5) sind Standard bei Wemos D1 Mini
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 Display nicht gefunden");
    displayAvailable = false;
  } else {
    Serial.println("Display initialisiert");
    displayAvailable = true;
    
    // Startbildschirm anzeigen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("SwissAirDry");
    display.println("Wemos D1 Mini");
    display.println("Starte...");
    display.display();
    delay(1000);
  }
  
  // Zufälliges Passwort generieren
  generateRandomPassword();
  
  // Mit WLAN verbinden
  connectWiFi();
  
  // OTA-Updates einrichten
  setupOTA();
  
  // Webserver starten
  setupWebServer();
  
  // QR-Code anzeigen
  if (displayAvailable && WiFi.status() == WL_CONNECTED) {
    displayQRCode();
  }
}

void generateRandomPassword() {
  // Zufälliges 8-stelliges Passwort aus Zahlen und Buchstaben generieren
  const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  webPassword = "";
  
  // Verwende die Chip-ID als Teil des Seeds
  randomSeed(ESP.getChipId() + millis());
  
  // Generiere 8 zufällige Zeichen
  for (int i = 0; i < 8; i++) {
    int index = random(0, sizeof(charset) - 1);
    webPassword += charset[index];
  }
  
  Serial.print("Generiertes Web-Passwort: ");
  Serial.println(webPassword);
}

void setupWebServer() {
  // Hier können Sie später einen Webserver einrichten, falls gewünscht
  // Beispiel mit ESP8266WebServer
  
  // ESP8266WebServer server(WEB_SERVER_PORT);
  // server.on("/", handleRoot);
  // server.begin();
  
  Serial.println("Webserver bereit auf Port " + String(WEB_SERVER_PORT));
}

void connectWiFi() {
  Serial.println("Verbinde mit WLAN...");
  if (displayAvailable) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Verbinde mit WLAN");
    display.println(ssid);
    display.display();
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    
    if (displayAvailable) {
      display.print(".");
      if (attempts % 10 == 9) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Verbinde mit WLAN");
        display.println(ssid);
      }
      display.display();
    }
    
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Verbunden mit IP: ");
    Serial.println(WiFi.localIP());
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WLAN verbunden");
      display.print("IP: ");
      display.println(WiFi.localIP().toString());
      display.display();
    }
    
    // Verbindungsbestätigung mit LED
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(100);
      digitalWrite(LED_PIN, LED_OFF);
      delay(100);
    }
  } else {
    Serial.println("");
    Serial.println("WLAN-Verbindung fehlgeschlagen!");
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WLAN-Fehler!");
      display.println("Offline-Modus");
      display.display();
      delay(2000);
    }
  }
}

void setupOTA() {
  // OTA-Konfiguration
  ArduinoOTA.setHostname(hostname.c_str());
  
  // Optionaler Kennwortschutz für OTA
  // ArduinoOTA.setPassword("admin");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else {
      type = "Dateisystem";
    }
    
    Serial.println("Start OTA: " + type);
    
    // Alle Systeme für Update vorbereiten
    digitalWrite(LED_PIN, LED_ON);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("OTA Update");
      display.println("Starte...");
      display.display();
    }
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update beendet");
    digitalWrite(LED_PIN, LED_OFF);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Update beendet");
      display.println("Neustart...");
      display.display();
    }
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percentage = (progress / (total / 100));
    Serial.printf("Fortschritt: %u%%\r", percentage);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("OTA Update");
      display.printf("%u%%\n", percentage);
      display.drawRect(0, 25, 128, 10, SSD1306_WHITE);
      display.fillRect(2, 27, (percentage * 124) / 100, 6, SSD1306_WHITE);
      display.display();
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Fehler [%u]: ", error);
    
    String errorMsg = "Unbekannter Fehler";
    
    if (error == OTA_AUTH_ERROR) {
      errorMsg = "Authentifizierung";
    } else if (error == OTA_BEGIN_ERROR) {
      errorMsg = "Begin fehlgeschlagen";
    } else if (error == OTA_CONNECT_ERROR) {
      errorMsg = "Verbindungsfehler";
    } else if (error == OTA_RECEIVE_ERROR) {
      errorMsg = "Empfangsfehler";
    } else if (error == OTA_END_ERROR) {
      errorMsg = "End fehlgeschlagen";
    }
    
    Serial.println(errorMsg);
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("OTA FEHLER!");
      display.println(errorMsg);
      display.printf("Code: %u\n", error);
      display.display();
    }
    
    // Fehler mit LED signalisieren
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(100);
      digitalWrite(LED_PIN, LED_OFF);
      delay(100);
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA bereit");
}

void displayQRCode() {
  if (!displayAvailable || WiFi.status() != WL_CONNECTED) return;
  
  // QR-Code-Inhalt: IP-Adresse und Passwort
  String qrContent = "http://" + WiFi.localIP().toString() + "/ Passwort: " + webPassword;
  Serial.println("QR-Code Inhalt: " + qrContent);
  
  // QR-Code generieren und auf Display anzeigen
  display.clearDisplay();
  
  // Kleinen Titeltext oben anzeigen
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SwissAirDry");
  
  // QR-Code zeichnen (Mitte-unten)
  int qrSize = QR_CODE_VERSION * 4 + 17; // Größe des QR-Codes in Modulen
  int scaledSize = qrSize * QR_CODE_SCALE; // Skalierte Größe in Pixeln
  
  // Berechne die Position für den QR-Code, um ihn zu zentrieren
  int qrX = (SCREEN_WIDTH - scaledSize) / 2;
  int qrY = 10; // Etwas unter dem Titel
  
  // QR-Code initialisieren und generieren
  uint8_t qrcodeData[qrcode_getBufferSize(QR_CODE_VERSION)];
  qrcode_initText(&qrcode, qrcodeData, QR_CODE_VERSION, 0, qrContent.c_str());
  
  // QR-Code auf das Display zeichnen
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(
          qrX + x * QR_CODE_SCALE, 
          qrY + y * QR_CODE_SCALE, 
          QR_CODE_SCALE, 
          QR_CODE_SCALE, 
          SSD1306_WHITE
        );
      }
    }
  }
  
  display.display();
  
  Serial.println("QR-Code angezeigt");
}

void loop() {
  // OTA-Anfragen bearbeiten
  ArduinoOTA.handle();
  
  // Webserver verarbeiten (falls implementiert)
  // server.handleClient();
  
  // Alle 30 Sekunden die angezeigten Daten aktualisieren
  static unsigned long lastDisplayUpdate = 0;
  if (displayAvailable && WiFi.status() == WL_CONNECTED && 
      (millis() - lastDisplayUpdate > 30000)) {
    displayQRCode();
    lastDisplayUpdate = millis();
  }
  
  // WLAN-Verbindung prüfen und ggf. erneut verbinden
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastReconnectAttempt > 60000) {  // Alle 60 Sekunden versuchen
      lastReconnectAttempt = currentMillis;
      Serial.println("WLAN-Verbindung verloren. Versuche neu zu verbinden...");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
    }
  }
  
  // Heartbeat-LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 3000) {  // Alle 3 Sekunden
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlink = millis();
  }
  
  // Watchdog füttern (wichtig für ESP8266)
  yield();
}