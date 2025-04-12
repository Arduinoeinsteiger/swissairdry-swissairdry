# SwissAirDry - Komplettanleitung

## Einleitung

SwissAirDry ist ein umfassendes System zur Verwaltung von Bautrocknungsgeräten und -aufträgen. Es besteht aus mehreren Komponenten, die zusammenarbeiten, um eine vollständige Lösung für die Verwaltung von Trocknungsprojekten zu bieten.

## Systemübersicht

Das System besteht aus folgenden Hauptkomponenten:

1. **API-Dienst (Backend)**
   - Stellt die Hauptlogik und Datenbankverbindung bereit
   - Implementiert als FastAPI-Anwendung in Python
   - Läuft in einem Docker-Container

2. **Nextcloud-App (Frontend)**
   - Bietet die Benutzeroberfläche für das System
   - Läuft innerhalb einer bestehenden Nextcloud-Installation
   - Kommuniziert mit dem API-Dienst über HTTP

3. **Datenbank**
   - PostgreSQL-Datenbank zur Speicherung aller Daten
   - Läuft in einem Docker-Container

4. **Cloud-Py-Api**
   - Ermöglicht die Integration der Python-basierten SwissAirDry-App in Nextcloud
   - Muss aus dem Nextcloud App Store installiert werden

## Installation

### Voraussetzungen

- Docker und Docker Compose
- Bestehende Nextcloud-Installation (Version 25+)
- Cloud-Py-Api App aus dem Nextcloud App Store
- 2 GB RAM und 20 GB Festplattenspeicher

### Installationsoptionen

Es gibt verschiedene Möglichkeiten, SwissAirDry zu installieren:

1. **Integration in bestehende Nextcloud-Docker-Installation**
   ```bash
   cd installation/scripts
   chmod +x *.sh
   ./docker_nextcloud_integration.sh
   ```

2. **Modulare Installation mit einzelnen Schritten**
   ```bash
   cd installation/scripts
   chmod +x *.sh
   ./install_swissairdry.sh
   ```

3. **Manuelle Installation der einzelnen Komponenten**
   ```bash
   # Umgebung prüfen
   ./01_check_environment.sh
   
   # API-Dienst installieren
   ./02_install_api_service.sh
   
   # Nextcloud-App installieren
   ./03_install_nextcloud_app.sh
   
   # Verbindung konfigurieren
   ./04_configure_connection.sh
   
   # Daten importieren (optional)
   ./05_import_data.sh
   ```

## Konfiguration

### API-Dienst

Die Konfiguration des API-Dienstes erfolgt über die Datei `.env` im API-Installationsverzeichnis:

```
API_PORT=5000
SECRET_KEY=geheimer_schluessel
DEBUG=False
POSTGRES_USER=swissairdry
POSTGRES_PASSWORD=sicheres_passwort
POSTGRES_DB=swissairdry
NEXTCLOUD_URL=https://ihre-nextcloud.example.com
```

### Nextcloud-App

Die Konfiguration der Nextcloud-App erfolgt automatisch während der Installation. Bei Bedarf kann sie über die Cloud-Py-Api angepasst werden.

## Verwendung

Nach der Installation können Sie auf die SwissAirDry-App über Ihre Nextcloud-Installation zugreifen:

```
https://ihre-nextcloud.example.com/index.php/apps/swissairdry/
```

### Benutzerrollen

Das System unterstützt folgende vordefinierte Benutzerrollen:

- **Trocknungsspezialist**: Zugriff auf Aufträge, Messungen, Geräte, Berichte, Fotos
- **Logistikmitarbeiter**: Zugriff auf Inventar, Scanner, Logistik, Transporte
- **Projektleiter**: Voller Zugriff auf alle Funktionen einschließlich Administration
- **ESP-Host**: Zugriff auf ESP, Firmware, Gateways, MQTT
- **Schlusskontrolleur**: Zugriff auf Aufträge, Checklisten, Messungen, Abschlussberichte

## Datenimport

Sie können bestehende Daten über das Import-Skript importieren:

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

## Wartung

### Backups

Führen Sie regelmäßige Backups der Datenbank und Konfigurationsdateien durch:

```bash
# Datenbank sichern
docker exec -it swissairdry-db pg_dump -U swissairdry swissairdry > backup_$(date +%Y%m%d).sql

# Uploads sichern
docker cp swissairdry-api:/app/uploads ./backup_uploads
```

### Updates

Um das System zu aktualisieren:

1. Erstellen Sie ein Backup der Datenbank und Konfigurationsdateien
2. Laden Sie die neueste Version herunter
3. Führen Sie das Update-Skript aus: `./update_swissairdry.sh`

### Fehlerbehebung

Bei Problemen konsultieren Sie bitte die Dokumentation unter [FAQ und Fehlerbehebung](../installation/docs/FAQ_UND_FEHLERBEHEBUNG.md).

## Support

Bei Fragen oder Problemen wenden Sie sich bitte an:

- Support-E-Mail: info@swissairdry.com
