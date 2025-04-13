# SwissAirDry Mobile App

Eine Android-Anwendung für das SwissAirDry System zur Überwachung und Steuerung von Trocknungsgeräten.

## Entwicklungsumgebung

- Java 17 (GraalVM CE)
- Gradle 7.5
- Android Gradle Plugin 7.4.2
- Kotlin 1.8.0

## Setup in Replit

1. Fork das Projekt
2. Replit wird automatisch die notwendigen Abhängigkeiten installieren
3. Warte bis der Language Server vollständig geladen ist
4. Führe `./gradlew build` aus, um das Projekt zu bauen

## Projektstruktur

- `/app` - Hauptmodul der Android-Anwendung
- `/gradle` - Gradle-Wrapper und Konfigurationsdateien

## Funktionen

- Überwachung von Trocknungsgeräten in Echtzeit
- Verwaltung von Kundendaten und Aufträgen
- Erfassung von Messungen und Erstellung von Berichten
- QR-Code/Barcode-Scanning für die Geräteidentifikation
- MQTT-Integration für IoT-Kommunikation
- Offline-Funktionalität mit lokaler Datenspeicherung

## Build

```bash
./gradlew build
```

## APK generieren

```bash
./gradlew assembleDebug
```

## Tests

```bash
./gradlew test
```

## Weitere Informationen

Detaillierte Informationen zur Entwicklung finden Sie in der [Entwicklungsanleitung](ENTWICKLUNG.md).

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.