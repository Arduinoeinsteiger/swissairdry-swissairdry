# SwissAirDry Nextcloud All-in-One

Eine All-in-One-Lösung für SwissAirDry mit Nextcloud, API und MQTT-Integration.

## Komponenten

- Nextcloud 26
- FastAPI Backend
- Simple Flask API
- MQTT Broker
- PostgreSQL Datenbank
- Nginx Reverse Proxy

## Voraussetzungen

- Docker
- Docker Compose
- Git

## Installation

1. Repository klonen:
```bash
git clone https://github.com/yourusername/swissairdry-nextcloud.git
cd swissairdry-nextcloud
```

2. Umgebungsvariablen setzen:
```bash
cp .env.example .env
# Bearbeiten Sie .env mit Ihren Einstellungen
```

3. Container starten:
```bash
docker-compose up -d
```

## Zugangsdaten

- Nextcloud: http://localhost:8080
  - Benutzer: admin
  - Passwort: swissairdry

- Portainer: http://localhost:9000
  - Erstellen Sie beim ersten Start einen Admin-Benutzer

- API: http://localhost:5000
- Simple API: http://localhost:5001
- MQTT: mqtt://localhost:1883

## Konfiguration

### Nginx
Die Nginx-Konfiguration befindet sich in `nginx.conf`. Sie können diese anpassen, um:
- SSL/TLS zu aktivieren
- Proxy-Einstellungen zu ändern
- Sicherheitsheader anzupassen

### MQTT
Die MQTT-Konfiguration befindet sich in `mqtt-config/mosquitto.conf`.

### Datenbank
Die PostgreSQL-Datenbank ist unter `localhost:5432` erreichbar:
- Datenbank: swissairdry
- Benutzer: swissairdry
- Passwort: swissairdry

## Entwicklung

### API Entwicklung
```bash
cd api
pip install -r requirements.api.txt
uvicorn app.main:app --reload
```

### Simple API Entwicklung
```bash
cd simple-api
pip install -r requirements.simple.txt
uvicorn start_simple:app --reload
```

## Lizenz

MIT License 