# SwissAirDry Integration mit bestehender Nextcloud-Docker-Installation

Diese Dokumentation erläutert die Integration des SwissAirDry-Systems in eine bestehende Nextcloud-Installation, die in Docker-Containern läuft.

## Übersicht

Die Integration erfolgt durch folgende Hauptkomponenten:

1. **SwissAirDry API-Dienst** (Docker-Container):
   - Stellt die Hauptlogik und Datenbankverbindung bereit
   - Kommuniziert mit der Nextcloud-App über Docker-Netzwerk

2. **Cloud-Py-Api in Nextcloud**:
   - Ermöglicht die Ausführung von Python-Code in Nextcloud
   - Dient als Brücke zwischen Nextcloud und der SwissAirDry-API

3. **SwissAirDry-Nextcloud-App**:
   - Bietet die Benutzeroberfläche für SwissAirDry
   - Läuft innerhalb der Nextcloud-Umgebung

## Voraussetzungen

- Bestehende Nextcloud-Installation in Docker-Containern
- Docker und Docker Compose
- Admin-Zugriff auf die Nextcloud-Instanz
- Cloud-Py-Api aus dem Nextcloud App Store

## Architektur

```
┌────────────────────────────────┐     ┌────────────────────────────────┐
│                                │     │                                │
│  Nextcloud-Container           │     │  SwissAirDry API-Container     │
│                                │     │                                │
│  ┌──────────────────────────┐ │     │  ┌──────────────────────────┐  │
│  │                          │ │     │  │                          │  │
│  │  Nextcloud               │ │     │  │  FastAPI-Anwendung       │  │
│  │                          │ │     │  │                          │  │
│  │  ┌─────────────────────┐ │ │     │  │  - Geschäftslogik        │  │
│  │  │                     │ │ │     │  │  - Datenverarbeitung     │  │
│  │  │  Cloud-Py-Api       │ │ │     │  │  - CSV-Import            │  │
│  │  │                     │ │ │     │  │  - Berichtgenerierung    │  │
│  │  └─────────────────────┘ │ │     │  │                          │  │
│  │          │               │ │     │  └──────────────────────────┘  │
│  │          ▼               │ │     │              │                 │
│  │  ┌─────────────────────┐ │ │     │              │                 │
│  │  │                     │ │ │     │              │                 │
│  │  │  SwissAirDry-App    │ │ │     │              │                 │
│  │  │                     │◄┼─┼─────┼──────────────┘                 │
│  │  └─────────────────────┘ │ │     │                                │
│  │                          │ │     │                                │
│  └──────────────────────────┘ │     └────────────────────────────────┘
│                                │     │                                │
└────────────────────────────────┘     │  PostgreSQL-Container          │
                 │                     │                                │
                 │                     │  ┌──────────────────────────┐  │
                 │                     │  │                          │  │
                 │                     │  │  PostgreSQL-Datenbank    │  │
                 └─────────────────────┼─►│                          │  │
                                       │  │  - Kunden                │  │
                                       │  │  - Geräte                │  │
                                       │  │  - Aufträge              │  │
                                       │  │  - Messwerte             │  │
                                       │  │                          │  │
                                       │  └──────────────────────────┘  │
                                       │                                │
                                       └────────────────────────────────┘
```

## Integrationsschritte

### 1. Docker-Netzwerk-Konfiguration

Die Kommunikation zwischen dem SwissAirDry API-Container und dem Nextcloud-Container erfolgt über ein gemeinsames Docker-Netzwerk. Dieses kann entweder das bestehende Netzwerk des Nextcloud-Containers sein oder ein neu erstelltes Netzwerk.

```bash
# API-Container mit dem Nextcloud-Netzwerk verbinden
docker network connect nextcloud_network swissairdry-api
```

### 2. Nextcloud Docker-Compose-Konfiguration

Die Docker-Compose-Konfiguration des Nextcloud-Containers wird angepasst, um die Verbindung zum SwissAirDry API-Container zu ermöglichen. Dies geschieht durch Hinzufügen einer Umgebungsvariable:

```yaml
services:
  nextcloud:
    # Bestehende Konfiguration...
    environment:
      # Bestehende Umgebungsvariablen...
      - SWISSAIRDRY_API_URL=http://swissairdry-api:5000
```

### 3. Cloud-Py-Api in Nextcloud

Die Cloud-Py-Api-App muss aus dem Nextcloud App Store installiert und aktiviert werden. Dies kann entweder über die Nextcloud-Admin-Oberfläche oder über die OCS-API erfolgen:

```bash
# Aktivieren der Cloud-Py-Api über OCS-API
curl -X POST -u "admin:password" -H "OCS-APIRequest: true" \
  "https://nextcloud.example.com/ocs/v1.php/cloud/apps/cloud_py_api"
```

### 4. SwissAirDry-App-Installation

Die SwissAirDry-App wird über die Cloud-Py-Api in Nextcloud installiert:

```bash
# Cloud-Py-Api-Token erhalten
TOKEN=$(curl -s -u "admin:password" -H "OCS-APIRequest: true" -H "Content-Type: application/json" \
  "https://nextcloud.example.com/ocs/v2.php/apps/cloud_py_api/api/v1/token" | grep -o '"token":"[^"]*"' | cut -d'"' -f4)

# App hochladen und installieren
curl -X POST -H "Authorization: Bearer $TOKEN" -H "Content-Type: multipart/form-data" \
  -F "app=@swissairdry-1.0.0.zip" \
  "https://nextcloud.example.com/index.php/apps/cloud_py_api/api/v1/upload_app"

# App aktivieren
curl -X POST -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" \
  -d '{"appId": "swissairdry"}' \
  "https://nextcloud.example.com/index.php/apps/cloud_py_api/api/v1/enable_app"
```

### 5. Konfiguration der API-URL

Die SwissAirDry-App wird so konfiguriert, dass sie die API-URL des SwissAirDry API-Containers verwendet:

```bash
# API-URL konfigurieren
curl -X POST -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" \
  -d '{"API_URL": "http://swissairdry-api:5000", "APP_VERSION": "1.0.0"}' \
  "https://nextcloud.example.com/index.php/apps/cloud_py_api/api/v1/swissairdry/config"
```

## Automatisierte Integration

Das Skript `docker_nextcloud_integration.sh` führt alle oben genannten Schritte automatisch aus:

```bash
cd modular_installation/scripts
chmod +x docker_nextcloud_integration.sh
./docker_nextcloud_integration.sh
```

Das Skript fragt interaktiv nach den erforderlichen Informationen wie dem Nextcloud-Container-Namen, dem Pfad zur Docker-Compose-Datei und den Nextcloud-Admin-Zugangsdaten.

## Fehlersuche

Bei Integrationsproblemen können Sie die folgenden Schritte durchführen:

1. **Netzwerkkonnektivität prüfen**:
   ```bash
   docker exec -it nextcloud ping swissairdry-api
   docker exec -it swissairdry-api ping nextcloud
   ```

2. **API-Zugriff testen**:
   ```bash
   docker exec -it nextcloud curl -s http://swissairdry-api:5000/health
   ```

3. **Cloud-Py-Api-Status prüfen**:
   ```bash
   curl -s -u "admin:password" -H "OCS-APIRequest: true" \
     "https://nextcloud.example.com/ocs/v1.php/cloud/apps/cloud_py_api"
   ```

4. **Protokolle der Container anzeigen**:
   ```bash
   docker logs nextcloud
   docker logs swissairdry-api
   ```

## Nächste Schritte

Nach der erfolgreichen Integration können Sie:

1. Daten über das Import-Skript in die SwissAirDry-Datenbank importieren
2. Benutzergruppen und Berechtigungen in Nextcloud konfigurieren
3. Die SwissAirDry-App unter `https://nextcloud.example.com/index.php/apps/swissairdry/` aufrufen