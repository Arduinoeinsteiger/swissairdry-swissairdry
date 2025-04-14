# SwissAirDry Nextcloud External App

Diese Anleitung beschreibt die Installation und Konfiguration der SwissAirDry External App für Nextcloud.

## Voraussetzungen

- Nextcloud 25 oder höher mit aktivierter AppAPI
- Docker und Docker Compose auf dem Server
- PHP 8.0 oder höher mit Mosquitto-Erweiterung
- Zugriff auf die SwissAirDry-API

## Installation

### 1. AppAPI aktivieren

Falls die AppAPI noch nicht aktiviert ist, installieren Sie sie in Nextcloud:

```bash
sudo -u www-data php /path/to/nextcloud/occ app:install app_api
sudo -u www-data php /path/to/nextcloud/occ app:enable app_api
```

### 2. App registrieren

Registrieren Sie die SwissAirDry-App in der AppAPI:

```bash
sudo -u www-data php /path/to/nextcloud/occ app_api:app:register swissairdry docker \
    --json-info /path/to/swissairdry-nextcloud/appapi.json \
    --env "MQTT_BROKER=swissairdry-mqtt" \
    --env "MQTT_PORT=1883" \
    --env "API_URL=http://swissairdry-api:5000"
```

Ersetzen Sie die Umgebungsvariablen nach Bedarf mit Ihren tatsächlichen Werten.

### 3. App installieren

Installieren Sie die App über die AppAPI:

```bash
sudo -u www-data php /path/to/nextcloud/occ app_api:app:install swissairdry
```

### 4. App einrichten

Nachdem die App installiert wurde, können Sie sie in der Nextcloud-Oberfläche konfigurieren:

1. Melden Sie sich als Administrator an
2. Gehen Sie zu "Einstellungen" > "SwissAirDry"
3. Geben Sie die Zugangsdaten für die SwissAirDry-API ein

## Verbindung mit der API

Die App verbindet sich automatisch mit der SwissAirDry-API. Falls die Verbindung nicht funktioniert, überprüfen Sie folgende Punkte:

1. Stellen Sie sicher, dass die API-URL korrekt ist
2. Überprüfen Sie, ob der MQTT-Broker erreichbar ist
3. Prüfen Sie, ob die Zugangsdaten korrekt sind

## Fehlerbehebung

### App erscheint nicht in Nextcloud

Überprüfen Sie, ob die AppAPI korrekt installiert ist:

```bash
sudo -u www-data php /path/to/nextcloud/occ app:list | grep app_api
```

Überprüfen Sie, ob die SwissAirDry-App registriert ist:

```bash
sudo -u www-data php /path/to/nextcloud/occ app_api:app:list
```

### Verbindungsprobleme mit der API

Prüfen Sie, ob die API erreichbar ist:

```bash
curl -v http://swissairdry-api:5000/api/v1/health
```

### MQTT-Verbindungsprobleme

Prüfen Sie, ob der MQTT-Broker läuft:

```bash
mosquitto_sub -h swissairdry-mqtt -p 1883 -t test
```

## Support

Bei Problemen oder Fragen wenden Sie sich an support@swissairdry.com.