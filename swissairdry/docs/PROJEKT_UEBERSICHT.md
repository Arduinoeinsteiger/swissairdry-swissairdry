# SwissAirDry - Projektübersicht

## Komponenten

Das SwissAirDry-Projekt besteht aus mehreren Kernkomponenten:

### 1. API-Server

Standort: `swissairdry/api/`

Ein FastAPI-basierter Backend-Server, der folgende Funktionen bietet:
- RESTful API für alle Client-Anwendungen
- Datenbankverwaltung mit PostgreSQL
- MQTT-Broker für IoT-Kommunikation
- Authentifizierung und Autorisierung
- Automatisches Failover zwischen primärem und Backup-Server

### 2. Web-App

Standort: `swissairdry/app/`

Ein modernes Web-Frontend mit folgenden Funktionen:
- Dashboard mit Echtzeit-Statusanzeigen
- Geräte- und Sensorenverwaltung
- Auftragsverwaltung und -tracking
- Berichterstellung und Export
- Responsive Design für Desktop und Mobile

### 3. Nextcloud-Integration

Standort: `swissairdry/nextcloud/`

Eine Integration in Nextcloud als External App mit:
- Zugriff auf SwissAirDry-Funktionen direkt aus Nextcloud
- Datei- und Fotoverwaltung in Nextcloud
- Single Sign-On mit Nextcloud-Benutzerkonten
- AppAPI-basierte Integration

### 4. Mobile App

Standort: `swissairdry/mobile/`

Eine native Android-App für Feldtechniker mit:
- Offline-Funktionalität
- Kamera-Integration für Foto-Dokumentation
- QR-Code-Scanner für Geräteidentifikation
- Push-Benachrichtigungen
- Automatischem API-Failover

## Projektstruktur

```
swissairdry/
├── api/                # FastAPI Backend-Server
│   ├── app/            # API-Anwendungscode
│   ├── Dockerfile      # Docker-Konfiguration
│   ├── requirements.txt # Python-Abhängigkeiten
│   └── mosquitto.conf  # MQTT-Broker-Konfiguration
│
├── app/                # Web-Frontend
│   ├── index.html      # Hauptseite
│   ├── styles.css      # Stylesheets
│   ├── app.js          # JavaScript-Code
│   └── logo.svg        # Logo
│
├── nextcloud/          # Nextcloud-Integration
│   ├── css/            # Stylesheets
│   ├── js/             # JavaScript-Code
│   ├── lib/            # PHP-Bibliotheken
│   ├── templates/      # Vorlagen
│   ├── index.php       # Haupteinstiegspunkt
│   ├── api.php         # API-Endpunkte
│   └── Dockerfile.appapi # AppAPI-Konfiguration
│
├── mobile/             # Android-App
│   └── src/            # Quellcode der App
│
├── docs/               # Dokumentation
│   ├── INSTALLATION_ANLEITUNG.md
│   ├── FEHLERBEHEBUNG.md
│   └── PROJEKT_UEBERSICHT.md
│
└── docker-compose.yml  # Docker-Compose-Konfiguration
```

## Technologie-Stack

- **Backend**: Python, FastAPI, SQLAlchemy, PostgreSQL
- **Frontend**: HTML5, CSS3, JavaScript
- **Mobile**: Kotlin, Android Jetpack
- **Nextcloud**: PHP, AppAPI
- **IoT**: MQTT, Eclipse Mosquitto

## Deployment

Das Projekt kann auf zwei Arten bereitgestellt werden:

1. **Standalone-Modus**: Als eigenständige Docker-Container
2. **Nextcloud-Integration**: Als External App in einer bestehenden Nextcloud-Installation

Die primäre Installation befindet sich unter `api.vgnc.org` mit einer Backup-Installation auf `swissairdry.replit.app`.