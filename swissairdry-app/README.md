# SwissAirDry Standalone Application

Ein umfassendes System zur Verwaltung von Trocknungsgeräten und Feldservice-Aufträgen.

## Features

- Verwaltung von Trocknungsgeräten, Kunden und Aufträgen
- Echtzeit-Überwachung von IoT-Geräten
- CSV-Import und Datenverarbeitung
- Admin-Dashboard und Benutzerverwaltung
- Optional: Integration mit Nextcloud für Datei- und Dokumentenverwaltung

## Systemvoraussetzungen

- Docker und Docker Compose (Version 1.29 oder höher)
- Minimale Hardwareanforderungen:
  - 2 GB RAM
  - 2 CPU-Kerne
  - 20 GB freier Festplattenspeicher

## Installation

### Standalone-Modus (ohne Nextcloud)

1. Repository klonen:
   ```bash
   git clone https://github.com/swissairdry/swissairdry-app.git
   cd swissairdry-app
   ```

2. Umgebungsvariablen konfigurieren:
   ```bash
   cp .env.example .env
   # Bearbeiten Sie die .env-Datei und passen Sie die Werte an
   ```

3. Die Anwendung starten:
   ```bash
   docker-compose up -d
   ```

4. Auf die Anwendung zugreifen:
   - API: http://localhost:5000
   - Admin-UI: http://localhost:5000/admin

### Mit Nextcloud-Integration

1. Repository klonen und in den Projektordner wechseln (wie oben beschrieben).

2. Umgebungsvariablen konfigurieren:
   ```bash
   cp .env.example .env
   # Bearbeiten Sie die .env-Datei:
   # - Setzen Sie STANDALONE_MODE=False
   # - Konfigurieren Sie NEXTCLOUD_URL, NEXTCLOUD_USER und NEXTCLOUD_PASSWORD
   ```

3. Die Anwendung starten:
   ```bash
   docker-compose up -d
   ```

4. Auf die Anwendung zugreifen:
   - API: http://localhost:5000
   - Admin-UI: http://localhost:5000/admin

## Konfiguration

### Umgebungsvariablen

Die folgenden Umgebungsvariablen können in der `.env`-Datei konfiguriert werden:

| Variable | Beschreibung | Standardwert |
|----------|--------------|--------------|
| DEBUG | Debug-Modus aktivieren | False |
| SECRET_KEY | Geheimer Schlüssel für die Verschlüsselung | supersecretkey |
| API_PORT | Port, auf dem die API läuft | 5000 |
| API_DOMAIN | Domain für die API | localhost:5000 |
| STANDALONE_MODE | Betrieb ohne Nextcloud | True |
| DATABASE_URL | PostgreSQL-Verbindungsstring | postgresql://swissairdry:swissairdry@postgres:5432/swissairdry |
| NEXTCLOUD_URL | URL der Nextcloud-Instanz (wenn STANDALONE_MODE=False) | - |
| FRONTEND_URL | URL des Frontend (wenn separat gehostet) | http://localhost:8080 |
| MQTT_PORT | Port für MQTT-Broker | 1883 |
| MQTT_WSS_PORT | WebSocket-Port für MQTT | 8083 |

## MQTT-Konfiguration für IoT-Geräte

Für die Integration von IoT-Geräten wird ein MQTT-Broker verwendet. Die Grundkonfiguration ist bereits enthalten und der MQTT-Broker ist über den in `.env` konfigurierten Port erreichbar.

### ESP32-Firmware einrichten

Die ESP32-Firmware kann für die Kommunikation mit dem MQTT-Broker konfiguriert werden:

1. Konfigurieren Sie die ESP32-Firmware mit der MQTT-Broker-Adresse.
2. Stellen Sie sicher, dass die ESP32-Geräte mit dem gleichen Netzwerk verbunden sind oder entsprechend geroutet werden können.

## Daten-Import

Der CSV-Import ist über das Admin-UI verfügbar:

1. Navigieren Sie zu http://localhost:5000/admin/csv-import
2. Laden Sie CSV-Dateien gemäß dem vorgegebenen Format hoch
3. Verfolgen Sie den Importstatus im Admin-UI

## Backup und Wiederherstellung

### Datenbank-Backup

```bash
docker-compose exec postgres pg_dump -U swissairdry swissairdry > swissairdry_backup_$(date +%Y%m%d).sql
```

### Datenbank-Wiederherstellung

```bash
cat swissairdry_backup_YYYYMMDD.sql | docker-compose exec -T postgres psql -U swissairdry swissairdry
```

## Fehlerbehebung

### API startet nicht

Überprüfen Sie die Logs:
```bash
docker-compose logs api
```

### Datenbank-Verbindungsprobleme

Überprüfen Sie, ob die Datenbank läuft:
```bash
docker-compose ps postgres
```

Testen Sie die Datenbankverbindung:
```bash
docker-compose exec postgres psql -U swissairdry -c "SELECT 1"
```

## Support

Bei Fragen oder Problemen wenden Sie sich an:
- E-Mail: info@swissairdry.com
- Support-Telefon: +41 XXX XXX XXX