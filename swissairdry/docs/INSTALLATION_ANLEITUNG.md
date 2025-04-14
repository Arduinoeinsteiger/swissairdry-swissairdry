# SwissAirDry Installationsanleitung

Diese Anleitung beschreibt, wie Sie das SwissAirDry-System auf Ihrem Server installieren.

## Voraussetzungen

- Docker und Docker Compose müssen auf dem Server installiert sein
- Ports 5000, 1883, 8080 und 5432 (optional) müssen freigegeben sein
- SSH-Zugang zum Server

## Installation

### 1. Dateien auf den Server übertragen

Laden Sie alle Projektdateien auf Ihren Server hoch:

```bash
# Beispiel mit scp (ersetzen Sie user@server mit Ihren Daten)
scp -r swissairdry_komplettpaket.zip user@server:/pfad/zum/ziel
```

Entpacken Sie das Archiv:

```bash
unzip swissairdry_komplettpaket.zip
cd swissairdry
```

### 2. Umgebungsvariablen konfigurieren

Kopieren Sie die Beispielkonfiguration und passen Sie sie an:

```bash
cp .env.example .env
```

Bearbeiten Sie die .env-Datei mit einem Texteditor:

```bash
nano .env
```

Passen Sie mindestens die folgenden Werte an:
- APP_SECRET_KEY (ein zufälliger, sicherer Schlüssel)
- POSTGRES_PASSWORD (ein sicheres Passwort für die Datenbank)
- JWT_SECRET_KEY (ein zufälliger, sicherer Schlüssel)
- PRIMARY_API_HOST=api.vgnc.org
- BACKUP_API_HOST=swissairdry.replit.app
- MQTT_PASSWORD (ein sicheres Passwort für den MQTT-Broker)

### 3. MQTT-Passwort einrichten

Erstellen Sie eine Passwortdatei für den MQTT-Broker:

```bash
mkdir -p data/mosquitto/{data,log}
docker run --rm -it eclipse-mosquitto:2 mosquitto_passwd -c /mosquitto/config/mosquitto.passwd swissairdry
```

Geben Sie das Passwort ein (dasselbe wie MQTT_PASSWORD in der .env-Datei).

Kopieren Sie die generierte Passwortdatei:

```bash
mkdir -p swissairdry-docker/mosquitto/config
docker cp <container_id>:/mosquitto/config/mosquitto.passwd swissairdry-docker/mosquitto/config/
```

### 4. Docker-Compose starten

Starten Sie die Container:

```bash
docker-compose up -d
```

Die Container werden gebaut und gestartet. Dies kann einige Minuten dauern.

### 5. Zugriff auf die Anwendung

- API-Server: http://ihre-domain:5000/
- API-Dokumentation: http://ihre-domain:5000/docs
- MQTT-Broker: ihre-domain:1883
- Nextcloud (falls konfiguriert): http://ihre-domain:8080/

## Failover-System

Das Failover-System ist so konfiguriert, dass es automatisch zwischen dem primären Server (api.vgnc.org) und dem Backup-Server (swissairdry.replit.app) wechselt. Der aktive Server wird im Admin-Bereich angezeigt.

Sie können den Status der Server über http://ihre-domain:5000/api-status/ überprüfen. Eine Authentifizierung ist für diesen Endpunkt erforderlich.

## Tipps zur Fehlerbehebung

### Logs einsehen

Die Logs der Container können mit dem folgenden Befehl eingesehen werden:

```bash
docker-compose logs -f api
```

Für andere Container den Namen entsprechend ändern (z.B. db, mqtt, nextcloud).

### API-Server neu starten

Falls der API-Server Probleme verursacht, kann er wie folgt neu gestartet werden:

```bash
docker-compose restart api
```

### Datenbank-Verbindung überprüfen

Falls Probleme mit der Datenbank auftreten, können Sie die Verbindung wie folgt überprüfen:

```bash
docker-compose exec db psql -U swissairdry -d swissairdry -c "SELECT NOW();"
```

## Support

Bei Fragen oder Problemen wenden Sie sich bitte an support@swissairdry.com.