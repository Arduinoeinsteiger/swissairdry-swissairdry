// SwissAirDry Wemos D1 Mini mit Menüsystem
// OTA-Updates + Programmsteuerung für Desinfektionseinheiten

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

// Button-Pins
#define BTN_UP D6        // Up/Zurück-Taste
#define BTN_DOWN D7      // Down/Vor-Taste
#define BTN_SELECT D8    // Auswahl-Taste

// Display-Konfiguration
#define OLED_RESET -1    // Kein Reset-Pin verwendet
#define SCREEN_WIDTH 128 // OLED Display Breite in Pixeln
#define SCREEN_HEIGHT 64 // OLED Display Höhe in Pixeln
#define OLED_ADDR 0x3C   // I2C-Adresse des OLED-Displays

// Menü-Zustände
enum MenuState {
  START_SCREEN,          // Startbildschirm
  MAIN_MENU,             // Hauptmenü
  PROGRAM_MENU,          // Programmauswahl-Menü
  SETTINGS_MENU,         // Einstellungen-Menü
  PROGRAM_RUNNING,       // Laufendes Programm
  WIFI_STATUS_SCREEN     // WLAN-Status-Anzeige
};

// Programme
enum Program {
  PROGRAM_7_DAYS = 0,
  PROGRAM_14_DAYS,
  PROGRAM_28_DAYS,
  PROGRAM_CUSTOM
};

// Objekte initialisieren
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool displayAvailable = false;

// Hostname mit eindeutiger Chip-ID
String hostname = "SwissAirDry-";

// Menü-Variable
MenuState currentMenuState = START_SCREEN;
int mainMenuSelection = 0;
int programMenuSelection = 0;
int settingsMenuSelection = 0;

// Programm-Variablen
bool programRunning = false;
unsigned long programStartTime = 0;
unsigned long programDuration = 0;  // in Sekunden
unsigned long lastProgramUpdate = 0;
String titleText = "Desinfektionseinheit";
String messageText = "Bereit...";

// Scroll-Variablen für Text
int scrollPosition = 0;
int maxScrollPosition = 0;
unsigned long lastScrollTime = 0;

// Button-Variablen
bool lastBtnUpState = HIGH;
bool lastBtnDownState = HIGH;
bool lastBtnSelectState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  // Serielle Verbindung starten
  Serial.begin(115200);
  Serial.println("\n\nSwissAirDry für Wemos D1 Mini mit Menüsystem");
  
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
  
  // Buttons konfigurieren
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  
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
  
  // Mit WLAN verbinden
  connectWiFi();
  
  // OTA-Updates einrichten
  setupOTA();
  
  // Starttext für Menü vorbereiten
  updateScrollPositions();
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
    
    // Wenn Programm läuft, Relais sicherheitshalber ausschalten
    if (programRunning) {
      digitalWrite(RELAY_PIN, LOW);
      programRunning = false;
    }
    
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

void handleButtons() {
  // Button-Abfragen mit Entprellung
  bool btnUpReading = digitalRead(BTN_UP);
  bool btnDownReading = digitalRead(BTN_DOWN);
  bool btnSelectReading = digitalRead(BTN_SELECT);
  
  if (btnUpReading != lastBtnUpState || btnDownReading != lastBtnDownState || btnSelectReading != lastBtnSelectState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Up-Button
    if (btnUpReading == LOW && lastBtnUpState == HIGH) {
      handleUpButton();
    }
    
    // Down-Button
    if (btnDownReading == LOW && lastBtnDownState == HIGH) {
      handleDownButton();
    }
    
    // Select-Button
    if (btnSelectReading == LOW && lastBtnSelectState == HIGH) {
      handleSelectButton();
    }
  }
  
  lastBtnUpState = btnUpReading;
  lastBtnDownState = btnDownReading;
  lastBtnSelectState = btnSelectReading;
}

void handleUpButton() {
  Serial.println("UP-Button gedrückt");
  
  switch (currentMenuState) {
    case MAIN_MENU:
      mainMenuSelection = (mainMenuSelection + 2) % 3; // 0,1,2 -> Zurück hoch zählen
      break;
      
    case PROGRAM_MENU:
      programMenuSelection = (programMenuSelection + 3) % 4; // 0,1,2,3 -> Zurück hoch zählen
      break;
      
    case SETTINGS_MENU:
      settingsMenuSelection = (settingsMenuSelection + 2) % 3; // 0,1,2 -> Zurück hoch zählen
      break;
      
    case PROGRAM_RUNNING:
      // Im laufenden Programm nichts tun
      break;
      
    case WIFI_STATUS_SCREEN:
    case START_SCREEN:
      // Von anderen Screens zurück zum Hauptmenü
      currentMenuState = MAIN_MENU;
      break;
  }
  
  updateDisplay();
}

void handleDownButton() {
  Serial.println("DOWN-Button gedrückt");
  
  switch (currentMenuState) {
    case MAIN_MENU:
      mainMenuSelection = (mainMenuSelection + 1) % 3; // 0,1,2 -> Vorwärts zählen
      break;
      
    case PROGRAM_MENU:
      programMenuSelection = (programMenuSelection + 1) % 4; // 0,1,2,3 -> Vorwärts zählen
      break;
      
    case SETTINGS_MENU:
      settingsMenuSelection = (settingsMenuSelection + 1) % 3; // 0,1,2 -> Vorwärts zählen
      break;
      
    case PROGRAM_RUNNING:
      // Im laufenden Programm zeigt Down den WLAN-Status
      currentMenuState = WIFI_STATUS_SCREEN;
      break;
      
    case WIFI_STATUS_SCREEN:
    case START_SCREEN:
      // Von anderen Screens zum Hauptmenü
      currentMenuState = MAIN_MENU;
      break;
  }
  
  updateDisplay();
}

void handleSelectButton() {
  Serial.println("SELECT-Button gedrückt");
  
  switch (currentMenuState) {
    case MAIN_MENU:
      switch (mainMenuSelection) {
        case 0: // Programme
          currentMenuState = PROGRAM_MENU;
          break;
        case 1: // Einstellungen
          currentMenuState = SETTINGS_MENU;
          break;
        case 2: // WLAN-Status
          currentMenuState = WIFI_STATUS_SCREEN;
          break;
      }
      break;
      
    case PROGRAM_MENU:
      switch (programMenuSelection) {
        case 0: // 7 Tage Programm
          startProgram(7 * 24 * 60 * 60, "7 Tage Programm");
          break;
        case 1: // 14 Tage Programm
          startProgram(14 * 24 * 60 * 60, "14 Tage Programm");
          break;
        case 2: // 28 Tage Programm
          startProgram(28 * 24 * 60 * 60, "28 Tage Programm");
          break;
        case 3: // Zurück
          currentMenuState = MAIN_MENU;
          break;
      }
      break;
      
    case SETTINGS_MENU:
      switch (settingsMenuSelection) {
        case 0: // Verbindung testen
          testConnection();
          break;
        case 1: // WLAN neu verbinden
          connectWiFi();
          currentMenuState = MAIN_MENU;
          break;
        case 2: // Zurück
          currentMenuState = MAIN_MENU;
          break;
      }
      break;
      
    case PROGRAM_RUNNING:
      // Programm stoppen
      if (programRunning) {
        stopProgram();
      }
      break;
      
    case WIFI_STATUS_SCREEN:
    case START_SCREEN:
      // Zum Hauptmenü wechseln
      currentMenuState = MAIN_MENU;
      break;
  }
  
  updateDisplay();
}

void testConnection() {
  // WLAN-Verbindung testen
  if (displayAvailable) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Teste Verbindung...");
    display.display();
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Hier könnte ein Ping oder API-Aufruf stehen
    
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WLAN verbunden");
      display.print("IP: ");
      display.println(WiFi.localIP().toString());
      display.print("RSSI: ");
      display.print(WiFi.RSSI());
      display.println(" dBm");
      display.display();
      delay(3000);
    }
  } else {
    if (displayAvailable) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WLAN getrennt!");
      display.println("Bitte neu verbinden");
      display.display();
      delay(3000);
    }
  }
}

void startProgram(unsigned long durationSeconds, String programName) {
  if (programRunning) {
    stopProgram();
  }
  
  programRunning = true;
  programStartTime = millis();
  programDuration = durationSeconds;
  currentMenuState = PROGRAM_RUNNING;
  
  // Relais einschalten
  digitalWrite(RELAY_PIN, HIGH);
  
  // Titel und Nachricht setzen
  titleText = "Desinfektionseinheit";
  messageText = programName + " startet...";
  
  // Scroll-Positionen aktualisieren
  updateScrollPositions();
  
  Serial.println("Programm gestartet: " + programName);
  Serial.printf("Dauer: %lu Sekunden\n", durationSeconds);
}

void stopProgram() {
  programRunning = false;
  
  // Relais ausschalten
  digitalWrite(RELAY_PIN, LOW);
  
  // Zurück zum Hauptmenü
  currentMenuState = MAIN_MENU;
  
  Serial.println("Programm gestoppt");
}

void updateProgram() {
  if (!programRunning) return;
  
  // Aktuelle Programmdauer berechnen
  unsigned long elapsedSeconds = (millis() - programStartTime) / 1000;
  
  // Prüfen, ob Programm abgelaufen ist
  if (elapsedSeconds >= programDuration) {
    messageText = "Programm abgeschlossen!";
    updateScrollPositions();
    stopProgram();
    return;
  }
  
  // Alle 5 Sekunden Status aktualisieren
  if (millis() - lastProgramUpdate > 5000) {
    // Verbleibende Zeit berechnen
    unsigned long remainingSeconds = programDuration - elapsedSeconds;
    unsigned long remainingDays = remainingSeconds / (24 * 60 * 60);
    unsigned long remainingHours = (remainingSeconds % (24 * 60 * 60)) / 3600;
    unsigned long remainingMinutes = (remainingSeconds % 3600) / 60;
    
    // Nachricht aktualisieren
    messageText = "Verbleibend: ";
    if (remainingDays > 0) {
      messageText += String(remainingDays) + "d ";
    }
    messageText += String(remainingHours) + "h " + String(remainingMinutes) + "m";
    
    updateScrollPositions();
    lastProgramUpdate = millis();
  }
}

void updateScrollPositions() {
  // Maximale Scroll-Position berechnen
  int titleWidth = titleText.length() * 6;  // 6 Pixel pro Zeichen bei Größe 1
  int messageWidth = messageText.length() * 6;
  
  // Wenn Text breiter als Display, Scroll aktivieren
  maxScrollPosition = max(0, max(titleWidth, messageWidth) - SCREEN_WIDTH);
  
  // Scroll-Position zurücksetzen
  scrollPosition = 0;
  
  Serial.print("START_SCREEN - Titeltext: ");
  Serial.println(titleText);
  Serial.print("START_SCREEN - Nachrichtentext: ");
  Serial.println(messageText);
  Serial.print("START_SCREEN - scrollPosition (Nachricht): ");
  Serial.println(scrollPosition - maxScrollPosition);
  Serial.print("START_SCREEN - maxScrollPosition_StartScreenMessage: ");
  Serial.println(maxScrollPosition);
}

void updateScroll() {
  // Text scrollen, wenn länger als Display
  if (maxScrollPosition > 0) {
    if (millis() - lastScrollTime > 100) {  // Alle 100ms scrollen
      scrollPosition = (scrollPosition + 1) % (maxScrollPosition + SCREEN_WIDTH);
      lastScrollTime = millis();
    }
  }
}

void updateDisplay() {
  if (!displayAvailable) return;
  
  Serial.print("updateDisplay() aufgerufen - currentMenuState: ");
  Serial.println(currentMenuState);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  
  switch (currentMenuState) {
    case START_SCREEN:
      drawStartScreen();
      break;
      
    case MAIN_MENU:
      drawMainMenu();
      break;
      
    case PROGRAM_MENU:
      drawProgramMenu();
      break;
      
    case SETTINGS_MENU:
      drawSettingsMenu();
      break;
      
    case PROGRAM_RUNNING:
      drawProgramRunning();
      break;
      
    case WIFI_STATUS_SCREEN:
      drawWifiStatus();
      break;
  }
  
  display.display();
}

void drawStartScreen() {
  Serial.println("Zeichne START_SCREEN");
  
  // Titel groß oben
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(titleText);
  
  // Trennlinie
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Nachricht in der Mitte
  display.setCursor(0, 20);
  display.println(messageText);
  
  // Hinweis unten
  display.setCursor(0, 50);
  display.println("Taste drucken fur Menu");
}

void drawMainMenu() {
  display.setTextSize(1);
  display.println("HAUPTMENU:");
  display.println();
  
  // Menüpunkte
  String menuItems[] = {"Programme", "Einstellungen", "WLAN-Status"};
  
  for (int i = 0; i < 3; i++) {
    if (i == mainMenuSelection) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.println(menuItems[i]);
  }
}

void drawProgramMenu() {
  display.setTextSize(1);
  display.println("PROGRAMME:");
  display.println();
  
  // Menüpunkte
  String menuItems[] = {"7 Tage Programm", "14 Tage Programm", "28 Tage Programm", "Zuruck"};
  
  for (int i = 0; i < 4; i++) {
    if (i == programMenuSelection) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.println(menuItems[i]);
  }
}

void drawSettingsMenu() {
  display.setTextSize(1);
  display.println("EINSTELLUNGEN:");
  display.println();
  
  // Menüpunkte
  String menuItems[] = {"Verbindung testen", "WLAN neu verbinden", "Zuruck"};
  
  for (int i = 0; i < 3; i++) {
    if (i == settingsMenuSelection) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.println(menuItems[i]);
  }
}

void drawProgramRunning() {
  // Titel
  display.setTextSize(1);
  display.println(titleText);
  
  // Trennlinie
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Programm-Status
  display.setCursor(0, 15);
  display.print("Status: ");
  display.println("AKTIV");
  
  // Nachricht
  display.setCursor(0, 30);
  display.println(messageText);
  
  // Fortschrittsbalken
  if (programDuration > 0) {
    unsigned long elapsedSeconds = (millis() - programStartTime) / 1000;
    int percent = min(100, (int)((elapsedSeconds * 100) / programDuration));
    
    display.drawRect(0, 45, 128, 8, SSD1306_WHITE);
    display.fillRect(2, 47, (percent * 124) / 100, 4, SSD1306_WHITE);
    
    display.setCursor(0, 55);
    display.print(percent);
    display.println("% abgeschlossen");
  }
}

void drawWifiStatus() {
  display.setTextSize(1);
  display.println("WLAN-STATUS:");
  display.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    display.println("Verbunden");
    display.print("SSID: ");
    display.println(ssid);
    display.print("IP: ");
    display.println(WiFi.localIP().toString());
    display.print("RSSI: ");
    display.print(WiFi.RSSI());
    display.println(" dBm");
  } else {
    display.println("Nicht verbunden");
    display.println("SSID: " + String(ssid));
    display.println("Taste drucken zum");
    display.println("neu verbinden");
  }
}

void loop() {
  // OTA-Anfragen bearbeiten
  ArduinoOTA.handle();
  
  // Tasten abfragen
  handleButtons();
  
  // Programm aktualisieren, falls aktiv
  updateProgram();
  
  // Text Scroll-Effekt aktualisieren
  updateScroll();
  
  // Display nur aktualisieren, wenn wir im Startbildschirm oder Programmlauf sind und gescrollt wird
  if ((currentMenuState == START_SCREEN || currentMenuState == PROGRAM_RUNNING) && 
      (millis() - lastScrollTime < 20)) {
    updateDisplay();
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
  
  // Heartbeat-LED (nur wenn kein Programm läuft)
  if (!programRunning) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 3000) {  // Alle 3 Sekunden
      digitalWrite(LED_PIN, LED_ON);
      delay(50);
      digitalWrite(LED_PIN, LED_OFF);
      lastBlink = millis();
    }
  } else {
    // Bei laufendem Programm LED schneller blinken lassen
    static unsigned long lastProgramBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastProgramBlink > 500) {  // Alle 0,5 Sekunden
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? LED_ON : LED_OFF);
      lastProgramBlink = millis();
    }
  }
  
  // Watchdog füttern (wichtig für ESP8266)
  yield();
}