# SwissAirDry Mobile App - Entwicklungsanleitung

Diese Anleitung beschreibt die Einrichtung und Entwicklung der SwissAirDry Mobile App für Android.

## Überblick

Die SwissAirDry Mobile App ist eine Android-Anwendung, die Trocknungsspezialisten, Logistikmitarbeitern und Projektleitern ermöglicht, Trocknungsgeräte zu überwachen, Kundendaten zu verwalten, Aufträge zu bearbeiten und Berichte zu erstellen. Die App kommuniziert mit dem SwissAirDry Backend über eine REST-API und nutzt MQTT für die Echtzeitkommunikation mit den IoT-Geräten.

## Voraussetzungen

Für die Entwicklung der App benötigen Sie:

- [Android Studio](https://developer.android.com/studio) (empfohlen: neueste stabile Version)
- JDK 11 oder höher
- Ein Android-Gerät oder -Emulator mit Android 7.0 (API Level 24) oder höher
- Git für die Versionskontrolle

## Projektstruktur

Das Projekt folgt einer Clean Architecture mit MVVM-Pattern und nutzt folgende Hauptkomponenten:

- **app**: Hauptmodul der Anwendung
  - **src/main/java/com/swissairdry/mobile**
    - **api**: API-Schnittstellen für Netzwerkkommunikation
    - **data**: Datenmodelle und Repositories
    - **di**: Dependency Injection Module (Hilt)
    - **ui**: UI-Komponenten (Activities, Fragments, ViewModels)
    - **utils**: Utility-Klassen
  - **src/main/res**: Ressourcen (Layouts, Strings, Drawables, etc.)

## Technologien

Die App verwendet folgende Technologien und Bibliotheken:

- **Kotlin**: Hauptprogrammiersprache
- **Jetpack-Komponenten**:
  - **ViewModel**: Verwaltung der UI-Daten
  - **LiveData & Flow**: Reaktive Programmierung
  - **Navigation Component**: Navigation zwischen Screens
  - **Room**: Lokale Datenbank
  - **DataStore**: Speicherung von Einstellungen
- **Hilt**: Dependency Injection
- **Retrofit**: HTTP-Client für API-Aufrufe
- **OkHttp**: HTTP-Client und Interceptors
- **MQTT**: Protokoll für IoT-Kommunikation
- **MPAndroidChart**: Diagramme und Visualisierungen
- **ZXing**: QR/Barcode-Scanning
- **Glide**: Bildladen und -caching

## Einrichtung der Entwicklungsumgebung

1. Klonen Sie das Repository:
   ```
   git clone https://github.com/swissairdry/mobile-app.git
   ```

2. Öffnen Sie das Projekt in Android Studio.

3. Synchronisieren Sie das Projekt mit Gradle-Dateien.

4. Konfigurieren Sie die API-Basis-URL in der `app/build.gradle`-Datei:
   ```gradle
   buildTypes {
       debug {
           buildConfigField "String", "API_BASE_URL", "\"http://10.0.2.2:5000/\""
           // ...
       }
       release {
           buildConfigField "String", "API_BASE_URL", "\"https://api.swissairdry.com/\""
           // ...
       }
   }
   ```

5. Für die Entwicklung mit einem lokalen Backend stellen Sie sicher, dass der SwissAirDry API-Server auf Port 5000 läuft. Die URL `10.0.2.2` leitet auf den Localhost des Host-Computers vom Android-Emulator aus weiter.

## Bauen und Ausführen

1. Wählen Sie ein Gerät oder einen Emulator aus der Geräteliste in Android Studio.

2. Klicken Sie auf "Run" (Shift+F10) oder verwenden Sie den grünen Pfeil in der Symbolleiste.

3. Die App wird auf dem ausgewählten Gerät oder Emulator installiert und gestartet.

## Debugging

- Verwenden Sie den Logcat in Android Studio für das Debugging (Alt+6).
- Die App verwendet Timber für strukturiertes Logging im DEBUG-Modus.
- Für Netzwerk-Debugging können Sie die HTTP-Interaktionen im Logcat überwachen.

## Testen

Das Projekt enthält verschiedene Testtypen:

- **Unittests**: In `src/test/java/`
  - Testen einzelner Komponenten wie ViewModels und Repositories
  - Ausführbar mit `./gradlew test`

- **Instrumentierungstests**: In `src/androidTest/java/`
  - Testen von UI-Komponenten und Integrationen
  - Ausführbar mit `./gradlew connectedAndroidTest`

## Codekonventionen

- Befolgen Sie die [Kotlin-Coding-Conventions](https://kotlinlang.org/docs/reference/coding-conventions.html).
- Verwenden Sie aussagekräftige Variablen- und Funktionsnamen.
- Jede Klasse sollte einen Dokumentationsblock haben.
- Kommentieren Sie komplexe Logik.
- Folgen Sie dem MVVM-Pattern für UI-Komponenten.

## Deployment

### Debug-Build

```bash
./gradlew assembleDebug
```

Die APK wird im Verzeichnis `app/build/outputs/apk/debug/` erstellt.

### Release-Build

1. Konfigurieren Sie die Signierungsinformationen in `app/build.gradle`:
   ```gradle
   android {
       signingConfigs {
           release {
               storeFile file("swissairdry.keystore")
               storePassword "Ihr-Keystore-Passwort"
               keyAlias "swissairdry"
               keyPassword "Ihr-Key-Passwort"
           }
       }
       // ...
   }
   ```

2. Erstellen Sie das Release-Build:
   ```bash
   ./gradlew assembleRelease
   ```

3. Die signierte APK wird im Verzeichnis `app/build/outputs/apk/release/` erstellt.

## App-Struktur und Navigation

Die App verwendet das Navigation Component für die Navigation zwischen Screens:

- **Login**: Anmeldung mit Benutzername und Passwort
- **Dashboard**: Übersicht über Geräte, Aufträge und Kunden
- **Geräte**: Liste und Details von Trocknungsgeräten
- **Kunden**: Kundenverwaltung
- **Aufträge**: Auftragsverwaltung
- **Messungen**: Erfassung und Anzeige von Messwerten
- **Berichte**: Generierung und Verwaltung von Berichten
- **Einstellungen**: App-Konfiguration

## MQTT-Integration

Die App verwendet das MQTT-Protokoll für die Echtzeitkommunikation mit den IoT-Geräten:

1. Die Verbindung wird in `MqttClientManager` verwaltet.
2. Die App abonniert Themen für relevante Geräte.
3. Empfangene Nachrichten werden an entsprechende ViewModels weitergeleitet.

Beispiel für MQTT-Themen:
- `swissairdry/devices/{device_id}/data` - Sensordaten
- `swissairdry/devices/{device_id}/status` - Gerätestatus

## QR/Barcode-Scanning

Die App unterstützt das Scannen von QR-Codes und Barcodes für die Geräteidentifikation:

1. Die Scanfunktion ist in `ScannerFragment` implementiert.
2. Nach dem Scannen wird das Ergebnis analysiert und das entsprechende Gerät identifiziert.

## Fehlerbehebung

- **Build-Fehler**: Führen Sie "Sync Project with Gradle Files" aus und prüfen Sie die Build-Logs.
- **Verbindungsprobleme**: Überprüfen Sie die API-URL in der Build-Konfiguration.
- **Berechtigungsprobleme**: Stellen Sie sicher, dass alle erforderlichen Berechtigungen gewährt wurden.

## Support

Bei Fragen oder Problemen wenden Sie sich an das SwissAirDry-Entwicklerteam:
- E-Mail: development@swissairdry.com
- Issue-Tracker: https://github.com/swissairdry/mobile-app/issues

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.