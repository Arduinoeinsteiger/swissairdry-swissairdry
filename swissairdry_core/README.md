# SwissAirDry

## Übersicht

SwissAirDry ist eine komplette Plattform für die Verwaltung von Trocknungsgeräten, Projektmanagement und Feldservice-Operationen. Die Anwendung bietet eine umfassende Lösung mit Echtzeitüberwachung von Trocknungsgeräten, Kundenverwaltung, Auftragsverfolgung, Rechnungsstellung und Berichterstellung inklusive Bildmaterial.

## Vereinfachte Verzeichnisstruktur

Das Projekt wurde auf eine minimale, übersichtliche Struktur reduziert:

```
swissairdry_core/
├── api/                # FastAPI Backend-Server
├── web/                # Web-Anwendung (Browser)
├── mobile/             # Android-Anwendung (Kotlin)
├── nextcloud/          # Nextcloud-Integration
└── docs/               # Dokumentation und Anleitungen
```

## Funktionen

- Echtzeitüberwachung von Trocknungsgeräten über IoT-Sensoren
- Energiekostenberechnung
- Bexio API-Integration für Buchhaltung
- Nextcloud-Integration für Dateimanagement
- Interaktives Dashboard mit Gerätestatus
- Rollenbasierte Zugriffsrechte
- Docker-basierte Bereitstellung
- Automatisches Failover-System zwischen Servern

## Installation

Die vollständige Installationsanleitung finden Sie in der Dokumentation unter `docs/SWISSAIRDRY_KOMPLETTANLEITUNG.md`.

Kurzanleitung:

```bash
# Repository klonen
git clone https://github.com/Arduinoeinsteiger/NextcloudCollaborationreplit.git

# In das Projektverzeichnis wechseln
cd swissairdry_core

# Umgebungsdatei konfigurieren
cp .env.example .env
nano .env  # Anpassen der Werte

# Docker-Container starten
docker-compose up -d
```

## Systemanforderungen

- Docker und Docker Compose
- PostgreSQL-Datenbank
- MQTT-Broker für IoT-Kommunikation
- Für Nextcloud-Integration: Nextcloud-Server mit AppAPI
- Für die mobile App: Android 8.0+

## Technischer Support

Bei Fragen oder Problemen wenden Sie sich bitte an das SwissAirDry-Team unter support@swissairdry.com oder erstellen Sie ein Issue im GitHub-Repository.

## Lizenz

Copyright 2023-2025 SwissAirDry Team