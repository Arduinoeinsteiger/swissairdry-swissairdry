# SwissAirDry Mobile App

Eine Android-Anwendung für das SwissAirDry System zur Überwachung und Steuerung von Trocknungsgeräten.

## Projektbeschreibung

Die SwissAirDry-App ist eine Android-Anwendung, die mit Kotlin entwickelt wurde. Die App bietet eine Lösung für die Überwachung und Verwaltung von Trocknungsgeräten, Kundendaten und Aufträgen.

## Aktueller Entwicklungsstand

Die App verfügt bereits über folgende implementierte Funktionen:

- **Grundgerüst**:
  - MVVM-Architektur vollständig implementiert
  - Dependency Injection mit Hilt eingerichtet
  - API-Anbindung mit Retrofit konfiguriert
  
- **Navigation**:
  - MainActivity mit Navigation-Drawer und Bottom-Navigation
  - NavController für Fragment-Navigation
  - Routing zwischen allen Hauptbereichen

- **Dashboard/Home**:
  - Übersicht mit Geräte-, Auftrags- und Kundenzahlen
  - Gerätemonitor mit Status-Anzeige (77%)
  - Temperaturanzeige
  - Zeitstempel der letzten Aktualisierung
  - Aktualisieren-Button

- **UI/UX**:
  - Dunkles Thema implementiert (Tag/Nacht-Design)
  - Responsive Layouts für verschiedene Bildschirmgrößen
  - SwipeRefreshLayout für Datenaktualisierung

- **Datenschicht**:
  - Repository-Pattern für Datenzugriff
  - API-Service mit definierten Endpunkten
  - Modelle für Benutzer, Geräte, Aufträge, Kunden etc.

## Entwicklungsumgebung

- Android Studio (neueste Version empfohlen)
- Java 17 (GraalVM CE)
- Gradle 7.5
- Android Gradle Plugin 7.4.2
- Kotlin 1.8.0
- JDK 11 oder höher

## Projektstruktur

- `/app` - Hauptmodul der Android-Anwendung
  - `/src/main/java/com/swissairdry/mobile` - Kotlin-Quellcode
    - `/api` - API-Schnittstellen für Netzwerkkommunikation
    - `/data` - Datenmodelle und Repositories
    - `/di` - Dependency Injection Module
    - `/ui` - UI-Komponenten (Activities, Fragments, ViewModels)
    - `/utils` - Utility-Klassen
  - `/src/main/res` - Android-Ressourcen (Layouts, Strings, Drawables, etc.)
  - `/src/test` - Unit Tests
  - `/src/androidTest` - Instrumentierte Tests
- `/gradle` - Gradle-Wrapper und Konfigurationsdateien

## Verwendete Technologien

- **Kotlin**: Hauptprogrammiersprache
- **Jetpack-Komponenten**:
  - ViewModel und LiveData
  - Navigation Component
  - Room (für lokale Datenpersistenz)
- **Hilt**: Dependency Injection
- **Retrofit & OkHttp**: Netzwerkkommunikation
- **MQTT**: Protokoll für IoT-Kommunikation
- **Timber**: Logging

## Installation und Setup

1. Klonen Sie das Repository
2. Öffnen Sie das Projekt in Android Studio
3. Warten Sie, bis die Gradle-Synchronisation abgeschlossen ist
4. Verbinden Sie ein Android-Gerät oder starten Sie einen Emulator

## Build & Run

```bash
# Debug-Build erstellen
./gradlew assembleDebug

# Release-Build erstellen
./gradlew assembleRelease

# Tests ausführen
./gradlew test
```

## Weitere Informationen

Detaillierte Informationen zur Entwicklung und Erweiterung der App finden Sie in der [ausführlichen Entwicklungsanleitung](APP_ENTWICKLUNGSANLEITUNG.md).

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.