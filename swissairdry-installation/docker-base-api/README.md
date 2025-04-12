# SwissAirDry - Docker Base API

Diese Anwendung stellt die SwissAirDry API-Backend-Komponente als Docker-Container bereit und bietet optional eine Integration mit Nextcloud.

## Komponenten

* **API-Server**: FastAPI-basierte REST-API für das SwissAirDry-System
* **PostgreSQL**: Datenbank für die Speicherung aller Daten
* **MQTT-Broker**: Für die Kommunikation mit IoT-Geräten
* **Nextcloud-Integration**: Optionale Anbindung an eine bestehende Nextcloud-Installation

## Systemanforderungen

* Docker und Docker Compose (Version 1.29+)
* 2 GB RAM
* 10 GB freier Festplattenspeicher
* Internet-Verbindung für den ersten Start (zum Herunterladen der Docker-Images)

## Installation

1. Repository klonen oder Installationspaket entpacken:
   ```
   git clone https://github.com/swissairdry/swissairdry-installation.git
   cd swissairdry-installation/docker-base-api
   ```

2. Konfigurationsdatei anpassen:
   ```
   cp .env.example .env
   # .env-Datei mit einem Editor bearbeiten und Werte anpassen
   ```

3. Container starten:
   ```
   docker-compose up -d
   ```

4. Überprüfen, ob alle Container laufen:
   ```
   docker-compose ps
   ```

## Konfiguration

### Datenbank

Die PostgreSQL-Datenbank wird automatisch beim ersten Start eingerichtet. Standardmäßig werden folgende Werte verwendet:

* **Benutzername**: swissairdry
* **Passwort**: swissairdry
* **Datenbankname**: swissairdry

Diese Werte können in der `.env`-Datei angepasst werden.

### MQTT-Broker

Der MQTT-Broker (Mosquitto) ist für die Kommunikation mit IoT-Geräten zuständig. Standardmäßig wird keine Authentifizierung verwendet.

Um die Authentifizierung zu aktivieren:

1. Passwortdatei erstellen:
   ```
   docker-compose exec mqtt mosquitto_passwd -c /mosquitto/config/passwd <benutzername>
   # Passwort eingeben, wenn gefragt
   ```

2. Konfiguration anpassen:
   ```
   # In mosquitto/config/mosquitto.conf die entsprechenden Zeilen auskommentieren
   password_file /mosquitto/config/passwd
   allow_anonymous false
   ```

3. MQTT-Broker neu starten:
   ```
   docker-compose restart mqtt
   ```

### Nextcloud-Integration

Für die Verwendung mit einer bestehenden Nextcloud-Installation folgende Werte in der `.env`-Datei anpassen:

```
NEXTCLOUD_URL=https://ihre-nextcloud-url.com
NEXTCLOUD_USERNAME=admin
NEXTCLOUD_PASSWORD=IhrPasswort
```

Alternativ kann auch die auskommentierte Nextcloud-Konfiguration in der `docker-compose.yml`-Datei aktiviert werden, um Nextcloud als Teil der Docker-Installation zu betreiben.

## Verwendung

### API-Zugriff

Die API ist über folgende URL erreichbar:

* http://localhost:5000/

Die API-Dokumentation ist hier verfügbar:

* http://localhost:5000/api/docs

### Admin-Dashboard

Ein einfaches Admin-Dashboard ist über folgende URL erreichbar:

* http://localhost:5000/admin

### MQTT-Zugriff

Der MQTT-Broker ist über folgende Ports erreichbar:

* MQTT: Port 1883
* MQTT über WebSockets: Port 9001

## Wartung und Updates

### Logs anzeigen

```
docker-compose logs -f api
```

### Updates installieren

1. Container stoppen:
   ```
   docker-compose down
   ```

2. Repository aktualisieren:
   ```
   git pull
   ```

3. Container mit neuen Images starten:
   ```
   docker-compose up -d --build
   ```

### Backup der Daten

```
# Datenbank-Backup erstellen
docker-compose exec postgres pg_dump -U swissairdry swissairdry > backup_$(date +%Y%m%d).sql

# Volumes sichern
docker run --rm -v swissairdry-docker-base-api_postgres_data:/source:ro -v $(pwd):/backup ubuntu tar -czf /backup/postgres_data_$(date +%Y%m%d).tar.gz /source
docker run --rm -v swissairdry-docker-base-api_api_data:/source:ro -v $(pwd):/backup ubuntu tar -czf /backup/api_data_$(date +%Y%m%d).tar.gz /source
```

## Fehlersuche

### Container startet nicht

Prüfen Sie die Logs:

```
docker-compose logs api
```

### Datenbank-Verbindungsprobleme

Prüfen Sie, ob der Datenbank-Container läuft:

```
docker-compose ps postgres
```

Überprüfen Sie die Verbindungseinstellungen in der `.env`-Datei.

### MQTT-Verbindungsprobleme

Prüfen Sie, ob der MQTT-Container läuft:

```
docker-compose ps mqtt
```

Testen Sie die Verbindung mit einem MQTT-Client, z.B.:

```
docker run --rm -it eclipse-mosquitto mosquitto_sub -h localhost -t test
```

## Support

Bei Fragen oder Problemen wenden Sie sich bitte an:

* E-Mail: info@swissairdry.com
* Support-Telefon: +41 123 456 789