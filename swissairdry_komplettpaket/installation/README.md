# SwissAirDry Modulare Installation

Dieses Installationspaket ist speziell für die Integration in eine bestehende Nextcloud-Umgebung konzipiert.

## Übersicht der Komponenten

1. **Nextcloud-Integration**
   - Installiert die SwissAirDry-App in Ihre bestehende Nextcloud-Instanz
   - Konfiguriert die Verbindung über Cloud-Py-Api

2. **API-Dienst**
   - Richtet den SwissAirDry API-Server ein
   - Konfiguriert die Kommunikation mit Nextcloud

3. **IoT-Komponenten** (optional)
   - MQTT-Broker für die Kommunikation mit Sensorgeräten
   - ESP32-Firmware für Gateways

## Installationsschritte

Führen Sie die Skripte in der folgenden Reihenfolge aus:

1. `scripts/01_check_environment.sh` - Prüft Voraussetzungen
2. `scripts/02_install_api_service.sh` - Installiert den API-Dienst
3. `scripts/03_install_nextcloud_app.sh` - Integriert die App in Nextcloud
4. `scripts/04_configure_connection.sh` - Richtet die Verbindung ein
5. `scripts/05_import_data.sh` (optional) - Importiert CSV-Daten

Alternativ können Sie das Hauptskript verwenden:
```bash
cd scripts
chmod +x install_swissairdry.sh
./install_swissairdry.sh
```

## Voraussetzungen

- Docker und Docker Compose
- Funktionierende Nextcloud-Installation (Version 25+)
- cloud-py-api App aus dem Nextcloud App Store installiert
- PostgreSQL-Datenbank

## Unterstützte Konfigurationen

- Eigenständige Nextcloud-Installation
- Nextcloud mit Docker-Compose
- Nextcloud in Docker Swarm