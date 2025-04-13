# SwissAirDry Docker-Konfiguration

Diese Verzeichnis enthält alle Docker-Konfigurationen für die einfache Bereitstellung des SwissAirDry-Systems.

## Komponenten

Die Docker-Konfiguration umfasst folgende Komponenten:

1. **API-Server**: FastAPI-basierter Backend-Server, der die Geschäftslogik und API-Endpunkte bereitstellt.
2. **PostgreSQL-Datenbank**: Datenbank für die Speicherung aller Daten des Systems.
3. **MQTT-Broker**: Mosquitto MQTT-Broker für die Kommunikation mit IoT-Geräten.

## Schnellstart

Um das SwissAirDry-System zu starten, führen Sie folgenden Befehl aus:

```bash
docker-compose up -d
```

Dadurch werden alle benötigten Container erstellt und gestartet.

## Konfiguration

Sie können die Konfiguration des Systems über Umgebungsvariablen anpassen:

### Allgemeine Einstellungen

```
APP_ENVIRONMENT=production  # 'development' oder 'production'
APP_PORT=5000               # Port für den API-Server
```

### Datenbank-Einstellungen

```
POSTGRES_USER=swissairdry
POSTGRES_PASSWORD=swissairdry
POSTGRES_DB=swissairdry
```

### MQTT-Einstellungen

```
MQTT_BROKER_HOST=mosquitto
MQTT_BROKER_PORT=1883
```

### Nextcloud-Integration (Optional)

```
NEXTCLOUD_URL=https://nextcloud.example.com
NEXTCLOUD_APP_KEY=your_app_key
```

## Zugriff auf die Anwendung

Nach dem Start können Sie auf die Anwendung über folgende URLs zugreifen:

- **API-Server**: http://localhost:5000/
- **API-Dokumentation**: http://localhost:5000/docs
- **Admin-Dashboard**: http://localhost:5000/admin

## Datenbank-Verwaltung

Sie können auf die PostgreSQL-Datenbank mit folgenden Verbindungsdaten zugreifen:

- **Host**: localhost
- **Port**: 5432
- **Datenbank**: swissairdry
- **Benutzer**: swissairdry
- **Passwort**: swissairdry

## Protokollierung

Die Protokolle der Container können mit folgendem Befehl angezeigt werden:

```bash
docker-compose logs -f
```

## Backup und Wiederherstellung

### Backup erstellen

```bash
docker-compose exec postgres pg_dump -U swissairdry swissairdry > backup.sql
```

### Backup wiederherstellen

```bash
docker-compose exec -T postgres psql -U swissairdry swissairdry < backup.sql
```

## Fehlersuche

Falls Probleme auftreten, prüfen Sie:

1. Ob alle Container laufen: `docker-compose ps`
2. Die Protokolle des API-Servers: `docker-compose logs api`
3. Die Verbindung zur Datenbank: `docker-compose exec postgres psql -U swissairdry -d swissairdry -c "SELECT 1;"`

## Produktionsbereitstellung

Für den Einsatz in einer Produktionsumgebung empfehlen wir:

1. Sichere Passwörter für Datenbank und MQTT-Broker
2. Einen Reverse-Proxy wie Nginx oder Traefik für TLS-Verschlüsselung
3. Regelmäßige Backups der Datenbank
4. Monitoring der Container-Gesundheit

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.