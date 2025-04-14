# SwissAirDry System

Das SwissAirDry System ist eine umfassende Lösung für die Verwaltung von Trocknungsgeräten, Kundendaten, Aufträgen und Messwerten für Trocknungsspezialisten und Projektleiter im Bereich der Bautrocknung.

## Projektstruktur

Das Projekt ist in vier Hauptkomponenten unterteilt:

1. **SwissAirDry APK (`swissairdry-apk/`)**: 
   Die mobile Android-Anwendung für Feldtechniker und Projektleiter.

2. **SwissAirDry Docker (`swissairdry-docker/`)**: 
   Docker-Konfigurationen für die einfache Bereitstellung des gesamten Systems.

3. **SwissAirDry Nextcloud (`swissairdry-nextcloud/`)**: 
   Integration mit Nextcloud für die Dateiverwaltung und Benutzerauthentifizierung.

4. **SwissAirDry Server (`swissairdry-server/`)**: 
   Der Backend-Server mit API-Endpunkten, Datenbankmodellen und Geschäftslogik.

## Installations- und Entwicklungsanleitungen

Detaillierte Anleitungen finden Sie in den jeweiligen README-Dateien der Teilkomponenten:

- [Android-App Entwicklungsanleitung](swissairdry-apk/ENTWICKLUNG.md)
- [Server-Konfiguration](swissairdry-server/README.md)
- [Nextcloud-Integration](swissairdry-nextcloud/README.md)
- [Docker-Deployment](swissairdry-docker/docker-compose.yml)

## Technologie-Stack

- **Backend**: FastAPI (Python), SQLAlchemy, PostgreSQL
- **Frontend**: Android (Kotlin), Vue.js (für Weboberflächen)
- **IoT**: MQTT, ESP32-Firmware
- **Integration**: Nextcloud, Bexio API
- **Deployment**: Docker, Docker Compose

## ESP32 Firmware

Die Firmware für ESP32-basierte IoT-Geräte befindet sich im Verzeichnis `swissairdry-server/esp32/`. Sie ermöglicht die direkte Kommunikation zwischen den Trocknungsgeräten und dem Backend-Server über MQTT.

## Funktionsübersicht

- Echtzeit-Überwachung von Trocknungsgeräten
- Verwaltung von Kundendaten und Aufträgen
- Erfassung von Messungen und Erstellung von Berichten mit Fotos
- Energiekostenberechnung
- Bexio API-Integration für die Abrechnung
- Nextcloud-Integration für Dateiverwaltung
- IoT-Gerätekommunikation über MQTT

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.