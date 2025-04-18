// SwissAirDry Universal OTA-Skript - Verbesserte Version
// Unterstützt automatisch ESP8266 (D1 Mini, NodeMCU, etc.) und ESP32 (alle Varianten)
// Stabile Board-Erkennung ohne Abhängigkeit von speziellen Build-Flags

// ----- BIBLIOTHEKEN -----
// Bedingte Kompilierung für unterschiedliche Board-Typen
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <ESPmDNS.h>
#else
  #error "Nicht unterstütztes Board! Nur ESP8266 und ESP32 werden unterstützt."
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Display-Bibliotheken (nur wenn aktiviert)
#define USE_DISPLAY 1  // Auf 0 setzen, um Display-Unterstützung zu deaktivieren

#if USE_DISPLAY
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

// ----- KONFIGURATION -----
// WiFi-Konfiguration
const char* ssid = "SSID";        // WLAN-Name
const char* password = "PASSWORT"; // WLAN-Passwort

// Gerätepräfix (für Hostname)
const char* devicePrefix = "SwissAirDry-";

// Hardware-Konfiguration
// Diese Werte werden automatisch basierend auf der erkannten Hardware gesetzt
#if defined(ESP8266)
  // D1 Mini / NodeMCU (ESP8266) Standard-Pins
  #define DEFAULT_LED_PIN D4     // Eingebaute LED auf D4 bei D1 Mini/NodeMCU
  #define DEFAULT_LED_ON LOW     // LED ist invertiert bei ESP8266
  #define DEFAULT_LED_OFF HIGH
  #define DEFAULT_SDA_PIN D2     // Standard SDA (GPIO4) bei D1 Mini/NodeMCU
  #define DEFAULT_SCL_PIN D1     // Standard SCL (GPIO5) bei D1 Mini/NodeMCU
#elif defined(ESP32)
  // ESP32 Standard-Pins
  #define DEFAULT_LED_PIN 2      // Typischer LED-Pin bei ESP32-Boards
  #define DEFAULT_LED_ON HIGH    // LED ist normal bei ESP32
  #define DEFAULT_LED_OFF LOW
  #define DEFAULT_SDA_PIN 21     // Standard SDA bei ESP32
  #define DEFAULT_SCL_PIN 22     // Standard SCL bei ESP32
#endif

// Aktuelle Hardware-Konfiguration
int LED_PIN = DEFAULT_LED_PIN;
int LED_ON = DEFAULT_LED_ON;
int LED_OFF = DEFAULT_LED_OFF;
int SDA_PIN = DEFAULT_SDA_PIN;
int SCL_PIN = DEFAULT_SCL_PIN;

// Display-Konfiguration
#if USE_DISPLAY
  #define OLED_WIDTH 128
  #define OLED_HEIGHT 64
  #define OLED_ADDR 0x3C        // Typische I2C-Adresse für SSD1306
  Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
  bool displayAvailable = false;
#endif

// ----- GLOBALE VARIABLEN -----
String deviceName;           // Vollständiger Gerätename mit Chip-ID
bool wifiConnected = false;  // WLAN-Verbindungsstatus

// ----- BOARD-ERKENNUNG -----
void detectBoard() {
  // Basisinformationen ausgeben
  Serial.println("\n----- SwissAirDry Universal OTA -----");
  
  // ESP-Typ erkennen und anzeigen
  #if defined(ESP8266)
    Serial.println("Board: ESP8266");
    
    // Chip-ID auslesen
    uint16_t chipId = ESP.getChipId() & 0xFFFF;
    deviceName = String(devicePrefix) + String(chipId, HEX);
    
  #elif defined(ESP32)
    Serial.println("Board: ESP32");
    
    // Einfache ESP32-Varianten-Erkennung ohne komplizierte Build-Flags
    // Basiert auf verfügbaren Pins und CPU-Eigenschaften
    
    // Chip-Revision prüfen für grobe Erkennung
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    if (chip_info.model == CHIP_ESP32) {
      Serial.println("Variante: ESP32 Standard");
      LED_PIN = 2;  // Standard LED_PIN für ESP32
    }
    else if (chip_info.model == CHIP_ESP32S2) {
      Serial.println("Variante: ESP32-S2");
      LED_PIN = 2;  // Angepasst für S2
    }
    else if (chip_info.model == CHIP_ESP32S3) {
      Serial.println("Variante: ESP32-S3");
      LED_PIN = 38; // Häufiger LED_PIN für S3
      SDA_PIN = 8;  // Häufige I2C-Pins für S3
      SCL_PIN = 9;
    }
    else if (chip_info.model == CHIP_ESP32C3) {
      Serial.println("Variante: ESP32-C3");
      LED_PIN = 8;  // Typisch für C3
      SDA_PIN = 5;  // Häufige I2C-Pins für C3
      SCL_PIN = 6;
    }
    else {
      Serial.println("Variante: Unbekannte ESP32-Variante");
    }
    
    // Chip-ID für ESP32 generieren
    uint32_t chipId = 0;
    for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    deviceName = String(devicePrefix) + String(chipId, HEX);
  #endif
  
  Serial.print("Geräte-ID: ");
  Serial.println(deviceName);
  Serial.print("LED-Pin: ");
  Serial.println(LED_PIN);
  
  #if USE_DISPLAY
  Serial.print("I2C-Pins: SDA=");
  Serial.print(SDA_PIN);
  Serial.print(", SCL=");
  Serial.println(SCL_PIN);
  #endif
}

// ----- DISPLAY-INITIALISIERUNG -----
#if USE_DISPLAY
void initDisplay() {
  #if defined(ESP8266)
    // Bei ESP8266 sind SDA/SCL fest zugewiesen, keine begin() Parameter benötigt
    Wire.begin();
  #elif defined(ESP32)
    // Bei ESP32 SDA/SCL explizit angeben
    Wire.begin(SDA_PIN, SCL_PIN);
  #endif
  
  // Display initialisieren
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 Display nicht gefunden");
    displayAvailable = false;
  } else {
    Serial.println("Display gefunden und initialisiert");
    displayAvailable = true;
    
    // Startbildschirm anzeigen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(F("SwissAirDry"));
    
    #if defined(ESP8266)
      display.println(F("Board: ESP8266"));
    #elif defined(ESP32)
      display.println(F("Board: ESP32"));
    #endif
    
    display.println(F("Boot..."));
    display.display();
  }
}

// Display aktualisieren
void updateDisplay(const char* line1, const char* line2 = "", const char* line3 = "", const char* line4 = "") {
  if (!displayAvailable) return;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(line1);
  
  if (strlen(line2) > 0) {
    display.println(line2);
  }
  
  if (strlen(line3) > 0) {
    display.println(line3);
  }
  
  if (strlen(line4) > 0) {
    display.println(line4);
  }
  
  display.display();
}

// Fortschrittsbalken anzeigen
void showProgress(int percent) {
  if (!displayAvailable) return;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("OTA Update:"));
  display.printf("%d%%\n", percent);
  display.drawRect(0, 20, 128, 10, WHITE);
  display.fillRect(2, 22, (percent * 124) / 100, 6, WHITE);
  display.display();
}
#endif

// ----- WIFI-SETUP -----
bool setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Verbinde mit WLAN");
  
  #if USE_DISPLAY
    if (displayAvailable) {
      updateDisplay("Verbinde mit WiFi:", ssid);
    }
  #endif
  
  // Animation während der Verbindung
  int dots = 0;
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    digitalWrite(LED_PIN, LED_ON);
    delay(100);
    digitalWrite(LED_PIN, LED_OFF);
    delay(400);
    Serial.print(".");
    
    #if USE_DISPLAY
      if (displayAvailable) {
        dots = (dots + 1) % 4;
        display.fillRect(0, 24, 128, 8, BLACK);
        display.setCursor(0, 24);
        for(int i=0; i<dots; i++) {
          display.print(".");
        }
        display.display();
      }
    #endif
    
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Verbunden mit IP: ");
    Serial.println(WiFi.localIP().toString());
    
    #if USE_DISPLAY
      if (displayAvailable) {
        updateDisplay("WiFi verbunden!", ("IP: " + WiFi.localIP().toString()).c_str());
      }
    #endif
    
    // Erfolgreiche Verbindung signalisieren
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(200);
      digitalWrite(LED_PIN, LED_OFF);
      delay(200);
    }
    
    return true;
  } else {
    Serial.println();
    Serial.println("WiFi-Verbindung fehlgeschlagen!");
    
    #if USE_DISPLAY
      if (displayAvailable) {
        updateDisplay("WiFi Fehler!", "Neustart erforderlich");
      }
    #endif
    
    // Fehlersignal mit schnellem Blinken
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(50);
      digitalWrite(LED_PIN, LED_OFF);
      delay(50);
    }
    
    return false;
  }
}

// ----- OTA-SETUP -----
void setupOTA() {
  // Hostname für OTA setzen
  ArduinoOTA.setHostname(deviceName.c_str());
  
  // Optional: Passwortschutz für OTA
  // ArduinoOTA.setPassword("admin");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    Serial.println("OTA: Start " + type);
    digitalWrite(LED_PIN, LED_ON);
    
    #if USE_DISPLAY
      if (displayAvailable) {
        updateDisplay("OTA Update", "startet...");
      }
    #endif
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA: Ende");
    digitalWrite(LED_PIN, LED_OFF);
    
    #if USE_DISPLAY
      if (displayAvailable) {
        updateDisplay("Update OK!", "Neustart...");
      }
    #endif
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    Serial.printf("OTA: %u%%\r", percent);
    
    #if USE_DISPLAY
      if (displayAvailable) {
        showProgress(percent);
      }
    #endif
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Fehler[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth fehlgeschlagen");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin fehlgeschlagen");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect fehlgeschlagen");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive fehlgeschlagen");
    else if (error == OTA_END_ERROR) Serial.println("End fehlgeschlagen");
    
    #if USE_DISPLAY
      if (displayAvailable) {
        char errorMsg[32];
        sprintf(errorMsg, "Code: %u", error);
        updateDisplay("OTA FEHLER!", errorMsg);
      }
    #endif
    
    // Fehlersignal mit LED
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, LED_ON);
      delay(100);
      digitalWrite(LED_PIN, LED_OFF);
      delay(100);
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA bereit");
  
  #if USE_DISPLAY
    if (displayAvailable) {
      updateDisplay("SwissAirDry", "OTA bereit", 
                   ("IP: " + WiFi.localIP().toString()).c_str(),
                   deviceName.c_str());
    }
  #endif
}

// ----- SETUP -----
void setup() {
  Serial.begin(115200);
  
  // Board erkennen und konfigurieren
  detectBoard();
  
  // LED konfigurieren
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  
  #if USE_DISPLAY
    // Display initialisieren
    initDisplay();
  #endif
  
  // WiFi verbinden
  wifiConnected = setupWifi();
  
  // OTA einrichten, wenn WiFi verbunden
  if (wifiConnected) {
    setupOTA();
  }
}

// ----- LOOP -----
void loop() {
  // OTA-Handler aufrufen, wenn WiFi verbunden
  if (wifiConnected) {
    ArduinoOTA.handle();
  } else {
    // Bei Verbindungsverlust alle 30 Sekunden neu verbinden
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      Serial.println("Versuche WLAN-Verbindung wiederherzustellen...");
      
      #if USE_DISPLAY
        if (displayAvailable) {
          updateDisplay("WiFi getrennt!", "Verbinde neu...");
        }
      #endif
      
      #if defined(ESP8266)
        WiFi.reconnect();
      #elif defined(ESP32)
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(ssid, password);
      #endif
      
      lastReconnectAttempt = millis();
      
      // Überprüfen, ob die Verbindung hergestellt wurde
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(500);
        attempts++;
      }
      
      wifiConnected = (WiFi.status() == WL_CONNECTED);
      
      if (wifiConnected) {
        Serial.println("WiFi wieder verbunden!");
        
        #if USE_DISPLAY
          if (displayAvailable) {
            updateDisplay("WiFi verbunden!", ("IP: " + WiFi.localIP().toString()).c_str());
          }
        #endif
      }
    }
  }
  
  // Heartbeat-LED - ein kurzer Blink alle 3 Sekunden
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 3000) {
    digitalWrite(LED_PIN, LED_ON);
    delay(50);
    digitalWrite(LED_PIN, LED_OFF);
    lastBlink = millis();
  }
  
  #if USE_DISPLAY
    // Display regelmäßig aktualisieren (Uptime, WLAN-Signalstärke, etc.)
    static unsigned long lastDisplayUpdate = 0;
    if (displayAvailable && wifiConnected && (millis() - lastDisplayUpdate > 10000)) {  // Alle 10 Sekunden
      // Uptime berechnen
      long uptime = millis() / 1000;
      int uptimeMin = (uptime / 60) % 60;
      int uptimeStd = (uptime / 3600) % 24;
      int uptimeTage = uptime / 86400;
      
      char uptimeStr[32];
      if (uptimeTage > 0) {
        sprintf(uptimeStr, "Uptime: %dT %dh %dm", uptimeTage, uptimeStd, uptimeMin);
      } else {
        sprintf(uptimeStr, "Uptime: %dh %dm", uptimeStd, uptimeMin);
      }
      
      char rssiStr[16];
      sprintf(rssiStr, "RSSI: %d dBm", WiFi.RSSI());
      
      updateDisplay("SwissAirDry", "Status: Bereit", uptimeStr, rssiStr);
      
      lastDisplayUpdate = millis();
    }
  #endif
  
  // Watchdog füttern
  #if defined(ESP8266)
    yield();
  #endif
}