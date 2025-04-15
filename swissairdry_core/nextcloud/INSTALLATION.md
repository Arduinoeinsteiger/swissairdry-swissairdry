# SwissAirDry Nextcloud External App

Diese Anleitung beschreibt die Installation und Konfiguration der SwissAirDry External App für Nextcloud.

## Voraussetzungen

- Nextcloud 25 oder höher mit aktivierter AppAPI
- Docker und Docker Compose auf dem Server
- PHP 8.2 oder höher mit den folgenden Erweiterungen:
  - Mosquitto (für MQTT-Kommunikation)
  - PDO und PDO_PGSQL (für Datenbankverbindung)
  - GD (für Bildbearbeitung)
  - ZIP (für Dateioperationen)
- Zugriff auf die SwissAirDry-API (https://api.vgnc.org)

## Installation

### 1. AppAPI aktivieren

Falls die AppAPI noch nicht aktiviert ist, installieren Sie sie in Nextcloud:

```bash
# Nextcloud-Installationsverzeichnis
NC_PATH=/var/www/nextcloud

# AppAPI installieren und aktivieren
sudo -u www-data php $NC_PATH/occ app:install app_api
sudo -u www-data php $NC_PATH/occ app:enable app_api
```

### 2. App registrieren

Kopieren Sie zuerst die SwissAirDry-App-Dateien in das Nextcloud-Apps-Verzeichnis:

```bash
# SwissAirDry-Verzeichnis erstellen
sudo mkdir -p $NC_PATH/apps/swissairdry
sudo chown -R www-data:www-data $NC_PATH/apps/swissairdry

# App-Dateien kopieren (passen Sie den Pfad zu Ihrem Repository an)
sudo cp -r /path/to/swissairdry_core/nextcloud/* $NC_PATH/apps/swissairdry/
```

Registrieren Sie die SwissAirDry-App in der AppAPI:

```bash
sudo -u www-data php $NC_PATH/occ app_api:app:register swissairdry docker \
    --json-info $NC_PATH/apps/swissairdry/appapi.json \
    --env "MQTT_BROKER=mqtt.vgnc.org" \
    --env "MQTT_PORT=1883" \
    --env "API_URL=https://api.vgnc.org"
```

### 3. App installieren und aktivieren

Installieren Sie die App über die AppAPI und aktivieren Sie sie:

```bash
# App installieren
sudo -u www-data php $NC_PATH/occ app_api:app:install swissairdry

# App aktivieren
sudo -u www-data php $NC_PATH/occ app:enable swissairdry
```

### 4. App konfigurieren

Nachdem die App installiert wurde, können Sie sie in der Nextcloud-Oberfläche konfigurieren:

1. Melden Sie sich als Administrator an
2. Gehen Sie zu "Apps" und prüfen Sie, ob SwissAirDry aktiviert ist
3. Gehen Sie zu "Einstellungen" > "SwissAirDry"
4. Konfigurieren Sie folgende Einstellungen:
   - API-URL: https://api.vgnc.org
   - MQTT-Broker: mqtt.vgnc.org
   - MQTT-Port: 1883
   - Zugangsdaten für die API

## Versionsupgrade

Falls Sie von einer älteren Version aktualisieren, befolgen Sie diese Schritte:

```bash
# Sichern Sie Ihre Konfiguration
sudo cp $NC_PATH/apps/swissairdry/config/config.php $NC_PATH/apps/swissairdry/config/config.php.bak

# Aktualisieren Sie die App-Dateien
sudo cp -r /path/to/swissairdry_core/nextcloud/* $NC_PATH/apps/swissairdry/

# Setzen Sie die Berechtigungen
sudo chown -R www-data:www-data $NC_PATH/apps/swissairdry

# Aktualisieren Sie die App über die Nextcloud-Konsole
sudo -u www-data php $NC_PATH/occ app:update swissairdry

# Führen Sie die Datenbank-Migrationen aus
sudo -u www-data php $NC_PATH/occ maintenance:repair
```

## Verbindung mit der API

Die App verbindet sich automatisch mit der SwissAirDry-API. Falls die Verbindung nicht funktioniert, überprüfen Sie folgende Punkte:

1. Stellen Sie sicher, dass die API-URL korrekt ist (https://api.vgnc.org)
2. Überprüfen Sie, ob der MQTT-Broker erreichbar ist (mqtt.vgnc.org:1883)
3. Prüfen Sie, ob die Zugangsdaten korrekt sind
4. Überprüfen Sie die Firewall-Regeln für ausgehende Verbindungen

## Fehlerbehebung

### App erscheint nicht in Nextcloud

Überprüfen Sie, ob die AppAPI korrekt installiert ist:

```bash
sudo -u www-data php $NC_PATH/occ app:list | grep app_api
```

Überprüfen Sie, ob die SwissAirDry-App registriert und aktiviert ist:

```bash
# Registrierte AppAPI-Apps anzeigen
sudo -u www-data php $NC_PATH/occ app_api:app:list

# Aktivierte Apps anzeigen
sudo -u www-data php $NC_PATH/occ app:list | grep swissairdry
```

### Verbindungsprobleme mit der API

Prüfen Sie, ob die API erreichbar ist:

```bash
# Gesundheitsendpunkt der API aufrufen
curl -v https://api.vgnc.org/health

# Überprüfen Sie die Netzwerkverbindung
ping api.vgnc.org

# Prüfen Sie auf SSL/TLS-Probleme
curl -v --insecure https://api.vgnc.org/health
```

### MQTT-Verbindungsprobleme

Prüfen Sie, ob der MQTT-Broker läuft und erreichbar ist:

```bash
# Mosquitto-Client installieren falls nicht vorhanden
sudo apt install mosquitto-clients

# Verbindung zum MQTT-Broker testen
mosquitto_sub -h mqtt.vgnc.org -p 1883 -t swissairdry/test -C 1
```

### App-Update-Probleme

Bei Problemen nach einem Update:

```bash
# Cache leeren
sudo -u www-data php $NC_PATH/occ maintenance:repair
sudo -u www-data php $NC_PATH/occ maintenance:mode --off

# Fehler in den Logs überprüfen
sudo tail -f $NC_PATH/data/nextcloud.log
```

## Produktionsumgebung

Für den Einsatz in einer Produktionsumgebung empfehlen wir folgende zusätzliche Maßnahmen:

1. Verwenden Sie HTTPS für alle Verbindungen
2. Konfigurieren Sie eine sichere MQTT-Verbindung mit Benutzername und Passwort
3. Setzen Sie die Nextcloud- und API-Sicherheitseinstellungen entsprechend Ihrer Unternehmensrichtlinien
4. Konfigurieren Sie regelmäßige Backups der Datenbank und Dateien

## Support

Bei Problemen oder Fragen wenden Sie sich an:

- E-Mail: support@swissairdry.com
- Website: https://vgnc.org
- GitHub: https://github.com/Arduinoeinsteiger/NextcloudCollaborationreplit/issues