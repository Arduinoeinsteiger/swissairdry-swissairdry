# SwissAirDry Installation und Integration mit Nextcloud

Diese Dokumentation erklärt die Installation und Integration der SwissAirDry-Anwendung mit einer bestehenden Nextcloud-Instanz.

## Voraussetzungen

Bevor Sie mit der Installation beginnen, stellen Sie sicher, dass die folgenden Voraussetzungen erfüllt sind:

1. **Bestehende Nextcloud-Installation** (Version 25 oder höher)
   - Der Administrator muss vollen Zugriff auf die Nextcloud-Instanz haben
   - Die Installation muss über HTTPS erreichbar sein (für sichere Verbindungen)

2. **Cloud-Py-Api installiert**
   - Die App "Cloud Py Api" muss aus dem Nextcloud App Store installiert und aktiviert sein
   - [Cloud-Py-Api im App Store](https://apps.nextcloud.com/apps/cloud_py_api)

3. **Docker und Docker Compose**
   - Docker muss auf dem Server installiert sein
   - Docker Compose muss verfügbar sein (entweder als eigenständiges Tool oder als Docker-Plugin)

4. **Systemanforderungen**
   - Mindestens 2 GB RAM
   - Mindestens 20 GB freier Festplattenspeicher
   - PostgreSQL-Datenbank oder Docker-Container mit PostgreSQL

## Installation

Die Installation der SwissAirDry-Anwendung ist in mehrere Schritte unterteilt, die über das Installationsskript ausgeführt werden können:

### 1. Vorbereitung

Entpacken Sie das Installationspaket in ein Verzeichnis Ihrer Wahl:

```bash
unzip swissairdry_modular_installation.zip -d /opt/swissairdry
cd /opt/swissairdry/scripts
chmod +x *.sh
```

### 2. Ausführen des Installationsskripts

Starten Sie das Hauptinstallationsskript:

```bash
./install_swissairdry.sh
```

Das Skript führt Sie durch die folgenden Schritte:

1. **Umgebungsprüfung** - Prüft, ob alle Voraussetzungen erfüllt sind
2. **API-Dienst-Installation** - Installiert den API-Server als Docker-Container
3. **Nextcloud-App-Installation** - Installiert die SwissAirDry-App in Nextcloud
4. **Verbindungskonfiguration** - Konfiguriert die Kommunikation zwischen den Komponenten
5. **Datenimport** (optional) - Importiert vorhandene Daten aus CSV-Dateien

Sie können auch jeden Schritt einzeln ausführen:

```bash
./01_check_environment.sh
./02_install_api_service.sh
./03_install_nextcloud_app.sh
./04_configure_connection.sh
./05_import_data.sh
```

## Integration mit Nextcloud

Nach der Installation ist die SwissAirDry-App in Ihrer Nextcloud-Instanz verfügbar. Sie können die App über das Anwendungsmenü in Nextcloud aufrufen.

### Zugriff auf die App

Die SwissAirDry-App ist unter der folgenden URL erreichbar:

```
https://ihre-nextcloud.domain/index.php/apps/swissairdry/
```

### Benutzergruppen und Berechtigungen

Die App unterstützt die folgenden vordefinierten Benutzergruppen:

- **Trocknungsspezialist** - Zugriff auf Aufträge, Messungen, Geräte, Berichte, Fotos
- **Logistikmitarbeiter** - Zugriff auf Inventar, Scanner, Logistik, Transporte
- **Projektleiter** - Voller Zugriff auf alle Funktionen einschließlich Administration
- **ESP-Host** - Zugriff auf ESP, Firmware, Gateways, MQTT
- **Schlusskontrolleur** - Zugriff auf Aufträge, Checklisten, Messungen, Abschlussberichte

## Konfiguration

Nach der Installation können Sie verschiedene Konfigurationsoptionen anpassen:

### API-Dienst

Die API-Dienstkonfiguration befindet sich in der Datei `.env` im API-Installationsverzeichnis (standardmäßig `/opt/swissairdry-api/.env`).

Wichtige Konfigurationsoptionen:

- `API_PORT` - Der Port, auf dem der API-Server läuft (Standard: 5000)
- `DEBUG` - Debug-Modus aktivieren/deaktivieren (Standard: False)
- `NEXTCLOUD_URL` - Die URL Ihrer Nextcloud-Instanz

### Nextcloud-App

Die Konfiguration der Nextcloud-App erfolgt über die Cloud-Py-Api-Schnittstelle. Die Konfiguration wird in der Nextcloud-Datenbank gespeichert.

## Datenimport

Wenn Sie bereits Daten aus einer bestehenden Installation haben, können Sie diese über das Importskript importieren:

```bash
./05_import_data.sh
```

Unterstützte CSV-Formate:

- Kundenstamm (KUNDENSTAMM.csv)
- Gerätestamm (GERAETESTAMMVERZEICHNISS.csv)
- Auftragsprotokolle (AUFTRAGSPROTOKOLL.csv)
- Gerätestandortwechsel (GERAETESTANDORTWECHSELPROTOKOLL.csv)
- Leistungserfassung (LEISTUNGSERFASSUNG.csv)
- Messprotokollwerterfassung (MESSPROTOKOLLWERTERFASSUNG.csv)

## Fehlersuche

Sollten bei der Installation oder Verwendung Probleme auftreten, finden Sie in den folgenden Protokolldateien weitere Informationen:

- `installation.log` - Protokoll der Installation
- `/opt/swissairdry-api/logs/api.log` - API-Server-Protokoll
- Nextcloud-Protokolle in der Nextcloud-Admin-Oberfläche

Häufige Probleme und Lösungen:

1. **API-Server nicht erreichbar**
   - Prüfen Sie, ob der Docker-Container läuft: `docker ps | grep swissairdry-api`
   - Prüfen Sie die Protokolle: `docker logs swissairdry-api`
   - Prüfen Sie die Firewall-Einstellungen

2. **Cloud-Py-Api-Fehler**
   - Stellen Sie sicher, dass die App aktiviert ist
   - Prüfen Sie die Python-Version in Nextcloud

3. **Datenbank-Verbindungsprobleme**
   - Prüfen Sie die Datenbankverbindungsdaten in der `.env`-Datei
   - Prüfen Sie, ob der Datenbankserver läuft und erreichbar ist

## Update der Anwendung

Für Updates der SwissAirDry-Anwendung können Sie das folgende Verfahren verwenden:

1. API-Server aktualisieren:
   ```bash
   cd /opt/swissairdry-api
   docker-compose pull
   docker-compose build
   docker-compose up -d
   ```

2. Nextcloud-App aktualisieren:
   - Laden Sie die neue Version der App herunter
   - Führen Sie das Skript zur Nextcloud-App-Installation erneut aus

## Support

Bei Fragen oder Problemen wenden Sie sich bitte an:

- Support-E-Mail: info@swissairdry.com
- Support-Website: https://swissairdry.com/support