# Migrations-Leitfaden: Von Nextcloud-Integration zu Standalone-Anwendung

Dieser Leitfaden beschreibt den Prozess der Migration vom Nextcloud-integrierten SwissAirDry-System zur eigenständigen Docker-Anwendung.

## Überblick über die Änderungen

Die Standalone-Version bietet folgende Vorteile:

- Einfachere Installation und Wartung
- Weniger Abhängigkeiten und Komplexität
- Verbesserte Performance durch direkten Zugriff
- Optionale Integration mit Nextcloud, wenn gewünscht
- Unabhängiger Betrieb ohne Nextcloud-Abhängigkeit

## Datenbank-Migration

### 1. Backup der bestehenden Datenbank erstellen

```bash
# Verbinden Sie sich mit dem Nextcloud-Docker-Container
docker exec -it <nextcloud-container-id> bash

# Erstellen Sie ein Backup der SwissAirDry-Datenbank
pg_dump -h <db-host> -U <db-user> -d swissairdry > /tmp/swissairdry_backup.sql

# Kopieren Sie das Backup auf den Host
exit
docker cp <nextcloud-container-id>:/tmp/swissairdry_backup.sql ./swissairdry_backup.sql
```

### 2. Wiederherstellen der Datenbank in der neuen Umgebung

```bash
# Starten Sie die neue Standalone-Anwendung
cd swissairdry-app
docker-compose up -d

# Importieren Sie die Datenbank
cat swissairdry_backup.sql | docker-compose exec -T postgres psql -U swissairdry swissairdry
```

## Konfiguration anpassen

### 1. Nextcloud-Zugangsdaten konfigurieren (für optionale Integration)

Bearbeiten Sie die `.env`-Datei und konfigurieren Sie die Nextcloud-Verbindung:

```
# Auf "False" setzen, um Nextcloud-Integration zu aktivieren
STANDALONE_MODE=False
NEXTCLOUD_URL=https://ihre-nextcloud-instanz.com
NEXTCLOUD_USER=swissairdry
NEXTCLOUD_PASSWORD=ihr-passwort
```

### 2. Ports und Domains anpassen

Passen Sie die Port- und Domain-Konfiguration entsprechend Ihrer Umgebung an:

```
API_PORT=5000
API_DOMAIN=swissairdry.ihre-domain.com
BASE_URL=https://swissairdry.ihre-domain.com
```

## Datenmigration

### 1. Dateien und Uploads migrieren

Wenn Sie benutzergenerierte Inhalte haben (z.B. hochgeladene CSV-Dateien oder Bilder):

```bash
# Kopieren Sie die Dateien aus der Nextcloud-Instanz
docker cp <nextcloud-container-id>:/var/www/html/data/swissairdry/uploads ./temp_uploads

# Importieren Sie die Dateien in die neue Umgebung
docker cp ./temp_uploads/. <swissairdry-api-container-id>:/app/uploads/

# Bereinigen
rm -rf ./temp_uploads
```

### 2. MQTT-Konfiguration migrieren (falls zutreffend)

Wenn Sie MQTT für IoT-Geräte verwenden:

```bash
# Kopieren Sie die MQTT-Konfiguration
docker cp <alter-mqtt-container-id>:/mosquitto/config/. ./swissairdry-app/mqtt/config/

# Starten Sie den MQTT-Service neu
docker-compose restart mqtt
```

## Anpassen von ESP32-Geräten

Falls Sie ESP32-Geräte mit dem System verbunden haben:

1. Aktualisieren Sie die Firmware mit der neuen MQTT-Broker-Adresse
2. Testen Sie die Verbindung und die Datensynchronisation

## Überprüfung der Migration

Nachdem Sie die Migration abgeschlossen haben, führen Sie folgende Überprüfungen durch:

1. Benutzer- und Gruppenkonfiguration
2. CSV-Import-Funktionalität
3. Verbindung zu IoT-Geräten
4. Nextcloud-Integration (falls konfiguriert)

## Rollback-Strategie

Falls Probleme auftreten, können Sie wie folgt zurück zur Nextcloud-Integration wechseln:

1. Sichern Sie die Datenbank der Standalone-App
2. Stellen Sie die ursprüngliche Nextcloud-Konfiguration wieder her
3. Importieren Sie das neueste Datenbank-Backup in die Nextcloud-Umgebung

## Support und Hilfe

Bei Fragen oder Problemen während der Migration wenden Sie sich an:

- E-Mail: info@swissairdry.com
- Support-Telefon: +41 XXX XXX XXX