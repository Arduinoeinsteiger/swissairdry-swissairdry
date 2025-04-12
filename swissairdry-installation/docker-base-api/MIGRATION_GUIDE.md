# SwissAirDry - Migrations-Leitfaden

Dieser Leitfaden hilft Ihnen bei der Migration von einer bestehenden SwissAirDry-Installation auf das neue Docker-basierte System.

## Voraussetzungen

Bevor Sie mit der Migration beginnen, stellen Sie sicher, dass Sie über Folgendes verfügen:

* Backup aller Daten Ihrer bestehenden Installation
* Zugriff auf die bestehende PostgreSQL-Datenbank
* Docker und Docker Compose auf dem Zielsystem installiert
* Ausreichend Speicherplatz (mindestens das Doppelte Ihrer aktuellen Installation)

## Migration in 5 Schritten

### Schritt 1: Vorbereitung

1. Backup der bestehenden Datenbank erstellen:
   ```
   pg_dump -U <benutzername> <datenbankname> > swissairdry_backup.sql
   ```

2. Backup der Nextcloud-Daten erstellen (falls vorhanden):
   ```
   # Anpassen an Ihre Nextcloud-Installation
   tar -czf nextcloud_data_backup.tar.gz /pfad/zu/nextcloud/data
   ```

3. Installationspaket entpacken oder Repository klonen:
   ```
   git clone https://github.com/swissairdry/swissairdry-installation.git
   cd swissairdry-installation/docker-base-api
   ```

4. Konfiguration vorbereiten:
   ```
   cp .env.example .env
   # .env-Datei mit einem Editor bearbeiten und an Ihre Umgebung anpassen
   ```

### Schritt 2: Migration der Datenbank

1. Container starten:
   ```
   docker-compose up -d postgres
   ```

2. Warten, bis der Datenbank-Container bereit ist:
   ```
   docker-compose logs -f postgres
   # Warten bis "database system is ready to accept connections" erscheint
   ```

3. Datenbank-Dump importieren:
   ```
   cat swissairdry_backup.sql | docker-compose exec -T postgres psql -U swissairdry swissairdry
   ```

### Schritt 3: Anpassen der Konfiguration

1. Umgebungsvariablen in der `.env`-Datei überprüfen und anpassen:
   * `POSTGRES_USER`, `POSTGRES_PASSWORD`: Diese sollten mit Ihrer bestehenden Datenbank übereinstimmen
   * `NEXTCLOUD_URL`: URL Ihrer bestehenden Nextcloud-Installation
   * `NEXTCLOUD_USERNAME`, `NEXTCLOUD_PASSWORD`: Zugangsdaten für Nextcloud
   * `JWT_SECRET`: Ein sicherer Schlüssel für die Token-Generierung

2. MQTT-Konfiguration in `mosquitto/config/mosquitto.conf` überprüfen und anpassen.

### Schritt 4: Starten des neuen Systems

1. Alle Container starten:
   ```
   docker-compose up -d
   ```

2. Überprüfen, ob alle Container laufen:
   ```
   docker-compose ps
   ```

3. Die Logs überwachen, um sicherzustellen, dass keine Fehler auftreten:
   ```
   docker-compose logs -f
   ```

### Schritt 5: Überprüfung und Feinabstimmung

1. API-Zugriff testen:
   ```
   curl http://localhost:5000/health
   ```

2. Admin-Dashboard aufrufen:
   * http://localhost:5000/admin

3. API-Dokumentation überprüfen:
   * http://localhost:5000/api/docs

4. Falls nötig, Systemressourcen für die Container in der `docker-compose.yml`-Datei anpassen:
   ```yaml
   services:
     api:
       # ...
       deploy:
         resources:
           limits:
             cpus: '1'
             memory: 1G
   ```

## Unterschiede zur vorherigen Version

Die neue Docker-basierte Version unterscheidet sich in einigen Aspekten von der vorherigen Version:

1. **Architektur**:
   * Containerisierte Umgebung statt einer direkten Installation
   * Klare Trennung der Komponenten (API, Datenbank, MQTT)

2. **Verzeichnisstruktur**:
   * Alle Daten werden in Docker-Volumes gespeichert
   * Konfiguration erfolgt über Umgebungsvariablen oder Konfigurationsdateien

3. **API-Endpunkte**:
   * Neue API-Basis-URL: `http://localhost:5000/api/`
   * Verbesserte API-Dokumentation unter `/api/docs`

4. **Authentifizierung**:
   * JWT-basierte Authentifizierung statt Cookie-basierter Authentifizierung
   * Separate Endpunkte für Anmeldung und Token-Erneuerung

## Fehlerbehebung bei der Migration

### Probleme mit der Datenbank-Migration

Falls beim Import der Datenbank Fehler auftreten:

1. Prüfen Sie, ob die Versionen kompatibel sind:
   ```
   # In der alten Umgebung
   psql --version
   
   # In der neuen Umgebung
   docker-compose exec postgres psql --version
   ```

2. Bei Versionskonflikten können Sie eine spezifische PostgreSQL-Version in der `docker-compose.yml`-Datei festlegen:
   ```yaml
   postgres:
     image: postgres:12.7-alpine  # Version an Ihre alte Installation anpassen
   ```

### Fehlende Tabellen oder Daten

Falls nach der Migration Tabellen oder Daten fehlen:

1. Überprüfen Sie, ob alle Schema-Definitionen im Datenbank-Dump enthalten waren:
   ```
   cat swissairdry_backup.sql | grep -i "create table"
   ```

2. Fehlende Tabellen können bei Bedarf durch Starten der API-Anwendung automatisch erstellt werden:
   ```
   docker-compose up -d api
   ```

### Nextcloud-Verbindungsprobleme

Falls Probleme mit der Nextcloud-Verbindung auftreten:

1. Überprüfen Sie die Nextcloud-URL und Zugangsdaten in der `.env`-Datei
2. Stellen Sie sicher, dass Ihre Nextcloud-Installation von der Docker-Umgebung aus erreichbar ist
3. Bei CORS-Problemen die entsprechenden Header in Ihrer Nextcloud-Installation aktivieren

## Nach der Migration

Nach erfolgreicher Migration sollten Sie:

1. Die Funktionalität aller Module testen
2. Die Leistung und Ressourcennutzung überwachen
3. Regelmäßige Backups der Docker-Volumes einrichten
4. Die alte Installation erst nach erfolgreicher Überprüfung der neuen Installation außer Betrieb nehmen

Bei Fragen oder Problemen während der Migration wenden Sie sich bitte an unseren Support:

* E-Mail: info@swissairdry.com
* Support-Telefon: +41 123 456 789