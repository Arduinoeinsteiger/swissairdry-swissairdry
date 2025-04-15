# SwissAirDry Komplettanleitung

**Version:** 1.0.0  
**Datum:** 15. April 2025  
**Autor:** SwissAirDry Team  

## Inhaltsverzeichnis

1. [Einführung](#1-einführung)
2. [Systemarchitektur](#2-systemarchitektur)
3. [Installation](#3-installation)
   - [3.1 Voraussetzungen](#31-voraussetzungen)
   - [3.2 Docker-Installation](#32-docker-installation)
   - [3.3 Nextcloud-Integration](#33-nextcloud-integration)
   - [3.4 Mobile App Setup](#34-mobile-app-setup)
4. [Konfiguration](#4-konfiguration)
   - [4.1 Umgebungsvariablen](#41-umgebungsvariablen)
   - [4.2 Port-Konfiguration](#42-port-konfiguration)
   - [4.3 API-Server-Konfiguration](#43-api-server-konfiguration)
   - [4.4 MQTT-Broker-Konfiguration](#44-mqtt-broker-konfiguration)
5. [Fehlerbehebung](#5-fehlerbehebung)
   - [5.1 Docker-Probleme](#51-docker-probleme)
   - [5.2 Datenbank-Probleme](#52-datenbank-probleme)
   - [5.3 API-Server-Probleme](#53-api-server-probleme)
   - [5.4 Nextcloud-Integrationsprobleme](#54-nextcloud-integrationsprobleme)
   - [5.5 Mobile App Probleme](#55-mobile-app-probleme)
6. [Wartung und Backup](#6-wartung-und-backup)
7. [Entwicklungshinweise](#7-entwicklungshinweise)

## 1. Einführung

SwissAirDry ist eine umfassende Anwendung für die Verwaltung von Trocknungsgeräten und Feldservice-Operationen, mit erweiterter Authentifizierung und IoT-Integrationsmöglichkeiten. Das System besteht aus:

- API-Server (FastAPI)
- Datenbank (PostgreSQL)
- MQTT-Broker für IoT-Geräte
- Web-Anwendung für den Browser
- Mobile App für Android-Geräte
- Nextcloud-Integration als externe App

## 2. Systemarchitektur

Die SwissAirDry-Plattform verwendet eine modulare Architektur:

```
SwissAirDry/
├── api/                # FastAPI Backend
├── mobile/             # Android-App (Kotlin)
├── ExApp/              # Web-App (Browser)
├── nextcloud/          # Nextcloud-Integration
└── docs/               # Dokumentation
```

### Produktionskonfiguration

- **Hauptdomain:** vgnc.org
- **API Domain:** api.vgnc.org
- **MQTT Domain:** mqtt.vgnc.org

#### Port-Konfiguration

| Port | Protokoll | Service | Intern | Extern |
|------|-----------|----------|---------|---------|
| 5000 | TCP/UDP | API | Debian12:5000 | api.vgnc.org:5000 |
| 1883 | TCP/UDP | MQTT | Debian12:1883 | mqtt.vgnc.org:1883 |
| 5432 | TCP/UDP | PostgreSQL | Debian12:5432 | (intern) |
| 8080 | TCP | Nextcloud | Debian12:8080 | vgnc.org:8080 |
| 9001 | TCP | MQTT-WS | Debian12:9001 | mqtt.vgnc.org:9001 |

## 3. Installation

### 3.1 Voraussetzungen

- Docker und Docker Compose
- Git
- Für Nextcloud-Integration: Nextcloud-Server (Version 25+) mit aktivierter AppAPI
- Für Mobile App: Android Studio

### 3.2 Docker-Installation

1. Repository klonen:
   ```bash
   git clone https://github.com/Arduinoeinsteiger/NextcloudCollaborationreplit.git
   cd NextcloudCollaborationreplit
   ```

2. Umgebungsvariablen einrichten:
   ```bash
   cp .env.example .env
   # Bearbeiten Sie die .env-Datei mit den richtigen Werten
   ```

3. Docker-Container starten:
   ```bash
   docker-compose up -d
   ```

4. Überprüfen, ob die Container laufen:
   ```bash
   docker-compose ps
   ```

5. API-Server testen:
   ```bash
   curl http://localhost:5000/health
   ```

### 3.3 Nextcloud-Integration

1. Nextcloud-Apps-Verzeichnis vorbereiten:
   ```bash
   sudo mkdir -p /var/www/nextcloud/apps/swissairdry
   sudo chown www-data:www-data /var/www/nextcloud/apps/swissairdry
   ```

2. App-Dateien kopieren:
   ```bash
   sudo cp -r nextcloud/* /var/www/nextcloud/apps/swissairdry/
   ```

3. AppAPI installieren und aktivieren:
   ```bash
   sudo -u www-data php /var/www/nextcloud/occ app:install app_api
   sudo -u www-data php /var/www/nextcloud/occ app:enable app_api
   ```

4. SwissAirDry-App registrieren:
   ```bash
   sudo -u www-data php /var/www/nextcloud/occ app_api:app:register swissairdry docker \
       --json-info /var/www/nextcloud/apps/swissairdry/appapi.json \
       --env "MQTT_BROKER=mqtt.vgnc.org" \
       --env "MQTT_PORT=1883" \
       --env "API_URL=https://api.vgnc.org"
   ```

5. App installieren und aktivieren:
   ```bash
   sudo -u www-data php /var/www/nextcloud/occ app_api:app:install swissairdry
   sudo -u www-data php /var/www/nextcloud/occ app:enable swissairdry
   ```

### 3.4 Mobile App Setup

1. Android Studio installieren
2. Projekt öffnen:
   ```bash
   cd mobile
   ```
3. Gradle-Build ausführen
4. API-Endpunkt in der Konfiguration anpassen (`ApiConfig.kt`)
5. App auf dem Gerät oder Emulator installieren

## 4. Konfiguration

### 4.1 Umgebungsvariablen

Erstellen Sie eine `.env`-Datei mit den folgenden Werten:

```
# API Konfiguration
API_PORT=5000
API_PREFIX=/api/v1
API_HOST=0.0.0.0
API_URL=https://api.vgnc.org

# Datenbank
POSTGRES_USER=swissairdry
POSTGRES_PASSWORD=swissairdry
POSTGRES_DB=swissairdry
POSTGRES_HOST=postgres
POSTGRES_PORT=5432

# MQTT
MQTT_BROKER=mqtt
MQTT_PORT=1883
MQTT_USERNAME=swissairdry
MQTT_PASSWORD=swissairdry
MQTT_TOPIC_PREFIX=swissairdry

# Nextcloud
NEXTCLOUD_URL=https://vgnc.org
NEXTCLOUD_USERNAME=admin
NEXTCLOUD_PASSWORD=admin

# Features
FEATURE_NEXTCLOUD_ENABLED=true
```

### 4.2 Port-Konfiguration

Die Ports werden in der `docker-compose.yml` konfiguriert:

```yaml
services:
  api:
    ports:
      - "5000:5000"    # API Server

  postgres:
    ports:
      - "5432:5432"    # PostgreSQL (optional für externen Zugriff)

  mqtt:
    ports:
      - "1883:1883"    # MQTT Broker
      - "9001:9001"    # MQTT WebSocket
```

Für die Produktion sollten Sie eine entsprechende Firewall-Konfiguration und ggf. einen Reverse-Proxy einrichten.

### 4.3 API-Server-Konfiguration

Die API-Server-Konfiguration befindet sich in `api/app/config.py`:

```python
# Primärer API-Server
PRIMARY_API_HOST = "api.vgnc.org"
PRIMARY_API_PORT = 5000
PRIMARY_API_SCHEME = "https"
PRIMARY_API_PREFIX = "/api/v1"

# Backup API-Server
BACKUP_API_HOST = "swissairdry.replit.app"
BACKUP_API_PORT = 443
BACKUP_API_SCHEME = "https"
BACKUP_API_PREFIX = "/api/v1"
```

### 4.4 MQTT-Broker-Konfiguration

Die MQTT-Broker-Konfiguration befindet sich in `mqtt/config/mosquitto.conf`:

```
listener 1883
allow_anonymous true
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
```

Für Produktionsumgebungen sollten Sie die Authentifizierung aktivieren:

```
listener 1883
allow_anonymous false
password_file /mosquitto/config/passwd
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
```

Erstellen Sie die Passwortdatei:
```bash
mosquitto_passwd -c /mosquitto/config/passwd swissairdry
```

## 5. Fehlerbehebung

### 5.1 Docker-Probleme

**Container startet nicht:**
```bash
# Container-Logs anzeigen
docker-compose logs [service_name]

# Container-Status überprüfen
docker-compose ps

# Container neu starten
docker-compose restart [service_name]
```

**Docker-Netzwerkprobleme:**
```bash
# Docker-Netzwerke anzeigen
docker network ls

# Docker-Netzwerk prüfen
docker network inspect swissairdry_network
```

### 5.2 Datenbank-Probleme

**Verbindungsprobleme:**
```bash
# Datenbank-Container-Logs anzeigen
docker-compose logs postgres

# Verbindung zur Datenbank testen
docker-compose exec postgres psql -U swissairdry -d swissairdry -c "SELECT 1;"
```

**Datenbankschema aktualisieren:**
```bash
# Datenbankschema manuell aktualisieren
docker-compose exec api python -c "from app.models import Base; from app.database import engine; Base.metadata.create_all(bind=engine)"
```

### 5.3 API-Server-Probleme

**API-Server reagiert nicht:**
```bash
# API-Server-Logs anzeigen
docker-compose logs api

# API-Server neu starten
docker-compose restart api

# API-Server-Status überprüfen
curl http://localhost:5000/health
```

**Fehler in der API-Konfiguration:**
```bash
# API-Konfiguration überprüfen
cat api/app/config.py

# Variable im Container überprüfen
docker-compose exec api python -c "import os; print(os.environ.get('API_URL'))"
```

### 5.4 Nextcloud-Integrationsprobleme

**App erscheint nicht in Nextcloud:**
```bash
# Nextcloud-Logs überprüfen
tail -f /var/www/nextcloud/data/nextcloud.log

# AppAPI-Status überprüfen
sudo -u www-data php /var/www/nextcloud/occ app:list | grep app_api

# SwissAirDry-App-Status überprüfen
sudo -u www-data php /var/www/nextcloud/occ app_api:app:list
```

**Versionskonflikt:**
```bash
# Nextcloud-Version überprüfen
sudo -u www-data php /var/www/nextcloud/occ status

# Container-Version aktualisieren
# Ändern Sie in docker-compose.yml:
# image: nextcloud:latest
```

### 5.5 Mobile App Probleme

**Verbindungsprobleme:**
- Überprüfen Sie die API-URL in der App-Konfiguration
- Stellen Sie sicher, dass die App über das Internet auf die API zugreifen kann
- Überprüfen Sie die Firewall-Regeln und den Netzwerkzugriff

**App-Absturz:**
- Logcat-Ausgabe überprüfen
- App neu installieren
- Cache leeren

## 6. Wartung und Backup

### Datenbank-Backup

```bash
# Datenbank sichern
docker-compose exec postgres pg_dump -U swissairdry swissairdry > backup_$(date +%Y%m%d).sql

# Datenbank wiederherstellen
cat backup_YYYYMMDD.sql | docker-compose exec -T postgres psql -U swissairdry -d swissairdry
```

### Systemupdate

```bash
# Repository aktualisieren
git pull

# Container neu erstellen und starten
docker-compose down
docker-compose up -d
```

### Logs überprüfen

```bash
# Logs aller Container anzeigen
docker-compose logs

# Logs eines bestimmten Services anzeigen
docker-compose logs [service_name]

# Fortlaufende Logs anzeigen
docker-compose logs -f
```

## 7. Entwicklungshinweise

### API-Entwicklung

- Die API ist in FastAPI implementiert
- Quellcode befindet sich im `api/app/`-Verzeichnis
- OpenAPI-Dokumentation unter `http://localhost:5000/docs`

### Mobile App Entwicklung

- Android-App in Kotlin
- MVVM-Architektur
- Repository-Pattern für Datenquellen
- Retrofit für API-Kommunikation
- AndroidX-Komponenten

### Web-App Entwicklung

- Frontend in JavaScript, HTML, CSS
- API-Kommunikation über Fetch API
- Dark Mode Toggle
- Responsive Design

### Nextcloud-Integration

- AppAPI für externe Apps
- OAuth2-Authentifizierung
- Ereignis-basierte Kommunikation