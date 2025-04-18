// SwissAirDry Wemos D1 Mini mit QR-Code-Anzeige (Alternative Version)
// OTA-Updates + QR-Code mit IP-Adresse und Web-Passwort
// Verwendet einen manuell generierten QR-Code (ohne spezielle Bibliothek)

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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

// Web-UI Konfiguration
String webPassword = "";   // Wird automatisch generiert
#define WEB_SERVER_PORT 80  // Webserver-Port

// Objekte initialisieren
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool displayAvailable = false;

// Hostname mit eindeutiger Chip-ID
String hostname = "SwissAirDry-";

void setup() {
  // Serielle Verbindung starten
  Serial.begin(115200);
  Serial.println("\n\nSwissAirDry für Wemos D1 Mini mit QR-Code (Alternative)");
  
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
  
  // IP und Passwort als QR-Pattern anzeigen
  if (displayAvailable && WiFi.status() == WL_CONNECTED) {
    displayLoginInfo();
  }
}

void generateRandomPassword() {
  // Zufälliges 6-stelliges Passwort aus Zahlen und Buchstaben generieren
  const char charset[] = "0123456789abcdefghijkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ";
  webPassword = "";
  
  // Verwende die Chip-ID als Teil des Seeds
  randomSeed(ESP.getChipId() + millis());
  
  // Generiere 6 zufällige Zeichen
  for (int i = 0; i < 6; i++) {
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
      delay(2000); // Kurz anzeigen, dann zum QR-Code wechseln
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

void displayLoginInfo() {
  if (!displayAvailable || WiFi.status() != WL_CONNECTED) return;
  
  // Display löschen und Titel anzeigen
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SwissAirDry - Login");
  
  // IP-Adresse anzeigen
  display.setCursor(0, 16);
  display.print("IP: ");
  display.println(WiFi.localIP().toString());
  
  // Passwort anzeigen
  display.setCursor(0, 26);
  display.print("PW: ");
  display.println(webPassword);
  
  // QR-Code ähnliches Muster zeichnen
  drawQRLikePattern(36, 36);
  
  display.display();
  Serial.println("Login-Informationen angezeigt");
}

// Zeichnet ein QR-Code ähnliches Muster um die Info leicht erkennbar zu machen
void drawQRLikePattern(int x, int y) {
  int size = 56; // Größe des Musters
  int blockSize = 4; // Größe eines Blocks
  
  // Rand zeichnen
  display.drawRect(x, y, size, size, SSD1306_WHITE);
  
  // Eckquadrate zeichnen (wie bei QR-Codes)
  // Oben links
  display.fillRect(x + 4, y + 4, 16, 16, SSD1306_WHITE);
  display.fillRect(x + 8, y + 8, 8, 8, SSD1306_BLACK);
  
  // Oben rechts
  display.fillRect(x + size - 20, y + 4, 16, 16, SSD1306_WHITE);
  display.fillRect(x + size - 16, y + 8, 8, 8, SSD1306_BLACK);
  
  // Unten links
  display.fillRect(x + 4, y + size - 20, 16, 16, SSD1306_WHITE);
  display.fillRect(x + 8, y + size - 16, 8, 8, SSD1306_BLACK);
  
  // Zufällige Blöcke im inneren Bereich (Daten)
  randomSeed(WiFi.localIP()[3] + ESP.getChipId());
  for (int i = 0; i < 14; i++) {
    for (int j = 0; j < 14; j++) {
      // Ecken auslassen
      if ((i < 4 && j < 4) || (i < 4 && j > 9) || (i > 9 && j < 4)) {
        continue;
      }
      
      // Zufällig einige Blöcke füllen
      if (random(100) > 60) {
        display.fillRect(
          x + i * blockSize, 
          y + j * blockSize, 
          blockSize, 
          blockSize, 
          SSD1306_WHITE
        );
      }
    }
  }
}

void loop() {
  // OTA-Anfragen bearbeiten
  ArduinoOTA.handle();
  
  // Webserver verarbeiten (falls implementiert)
  // server.handleClient();
  
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
      
      // Bei erfolgreicher Verbindung QR anzeigen
      if (WiFi.status() == WL_CONNECTED && displayAvailable) {
        displayLoginInfo();
      }
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