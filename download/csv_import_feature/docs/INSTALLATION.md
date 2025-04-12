# SwissAirDry CSV-Import-Funktionalität - Installationsanleitung

Diese Anleitung beschreibt, wie Sie die CSV-Import-Funktionalität in Ihren bestehenden SwissAirDry Docker-Container integrieren können.

## Übersicht der Funktionalität

Die CSV-Import-Funktionalität ermöglicht das Hochladen und Verarbeiten von SwissAirDry-CSV-Dateien aus dem alten System in die neue Datenbank. Die folgenden Dateitypen werden unterstützt:

- `SwissAirDry_AUFTRAGSPROTOKOLL.csv`: Enthält Auftrags-/Einsatzdaten
- `SwissAirDry_GERAETESTAMMVERZEICHNISS.csv`: Enthält Geräteinformationen
- `SwissAirDry_KUNDENSTAMM.csv`: Enthält Kundendaten
- `SwissAirDry_GERAETESTANDORTWECHSELPROTOKOLL.csv`: Enthält Gerätezuweisungen
- `SwissAirDry_MESSPROTOKOLLWERTERFASSUNG.csv`: Enthält Messwerte und Arbeitszeiten

## Voraussetzungen

- Laufende SwissAirDry Docker-Installation
- PostgreSQL-Datenbank (über DATABASE_URL konfiguriert)
- Python-Abhängigkeiten: fastapi, uvicorn, sqlalchemy, psycopg2-binary, pandas, jinja2

## Installationsschritte

Führen Sie die folgenden Schritte auf Ihrem Server aus, auf dem der SwissAirDry Docker-Container läuft:

### 1. Python-Abhängigkeiten installieren

Stellen Sie sicher, dass alle erforderlichen Python-Pakete im Container installiert sind:

```bash
pip install fastapi uvicorn sqlalchemy psycopg2-binary pandas jinja2 python-multipart aiofiles python-dotenv bcrypt
```

### 2. Dateien kopieren

Kopieren Sie die folgenden Dateien in die entsprechenden Verzeichnisse Ihrer Docker-Installation:

1. `code/admin_ui.py` → `docker-base-api/app/routers/admin_ui.py` (überschreiben oder ergänzen)
2. `code/data_processing.py` → `docker-base-api/app/routers/data_processing.py` (neu oder überschreiben)
3. `code/csv_import.html` → `docker-base-api/app/templates/csv_import.html` (neu)
4. `code/utils/csv_processor.py` → `docker-base-api/app/utils/csv_processor.py` (neu oder überschreiben)

Wenn bestimmte Verzeichnisse nicht existieren, erstellen Sie diese vorher:

```bash
mkdir -p docker-base-api/app/utils
mkdir -p docker-base-api/app/templates
```

### 3. Router-Import prüfen

Stellen Sie sicher, dass in Ihrer `main.py`-Datei die Router korrekt importiert werden:

```python
# Import router
from routers.api import router as api_router
from routers.admin_ui import router as admin_ui_router
from routers.data_processing import router as data_processing_router

# Include routers
app.include_router(api_router)
app.include_router(admin_ui_router)
app.include_router(data_processing_router, prefix="/api/data")
```

### 4. Anpassung der Konfiguration

Stellen Sie sicher, dass die Konfiguration in `config.py` die richtigen URLs für Produktionsumgebung enthält:

```python
API_BASE_URL = {
    "development": "http://localhost:5000",
    "production": "https://api.vgnc.org"  # Ihre API-URL hier anpassen
}.get(ENVIRONMENT, "http://localhost:5000")

# Nextcloud-Konfiguration
NEXTCLOUD_BASE_URL = "https://vgnc.org"  # Ihre Nextcloud-URL hier anpassen
NEXTCLOUD_APP_PATH = "/apps/swissairdry"
NEXTCLOUD_FULL_URL = f"{NEXTCLOUD_BASE_URL}{NEXTCLOUD_APP_PATH}"
```

### 5. Container neu starten

Nach dem Kopieren aller Dateien starten Sie den Docker-Container neu:

```bash
docker-compose restart
```

## Überprüfung der Installation

Nach erfolgreichem Neustart sollten Sie auf folgende URLs zugreifen können:

- API-Root: `https://api.vgnc.org/` (oder Ihre konfigurierte URL)
- Admin-UI: `https://api.vgnc.org/admin/`
- CSV-Import-Seite: `https://api.vgnc.org/admin/csv-import`

## CSV-Dateien importieren

1. Navigieren Sie zur CSV-Import-Seite im Admin-UI
2. Wählen Sie eine oder mehrere CSV-Dateien aus
3. Klicken Sie auf "Dateien hochladen"
4. Der Import wird im Hintergrund durchgeführt und die Ergebnisse angezeigt

## Fehlerbehebung

### Problem: Die Seite wird nicht geladen
- Überprüfen Sie, ob der Server läuft
- Prüfen Sie die Server-Logs auf Fehler
- Stellen Sie sicher, dass die Router korrekt eingebunden sind

### Problem: Dateien können nicht hochgeladen werden
- Überprüfen Sie die Berechtigungen des Upload-Verzeichnisses
- Stellen Sie sicher, dass der API-Endpunkt `/api/data/csv/upload-batch` korrekt konfiguriert ist
- Prüfen Sie die Server-Logs auf Fehler während des Uploads

### Problem: Die Dateien werden nicht verarbeitet
- Überprüfen Sie die CSV-Dateien auf korrekte Formatierung
- Stellen Sie sicher, dass der CSV-Prozessor die richtige Spaltenzuordnung verwendet
- Prüfen Sie die Datenbankverbindung

## Unterstützte Datenbankmodelle

Die CSV-Import-Funktionalität unterstützt folgende Datenbankmodelle:

- `Customer`: Kundeninformationen
- `Device`: Geräte und Sensoren
- `Job`: Aufträge/Einsätze
- `JobDevice`: Verknüpfung zwischen Jobs und Geräten
- `Measurement`: Gerätesmessungen
- `SystemLog`: Systemlogs für Nachverfolgung

## Kontakt und Support

Bei Fragen oder Problemen wenden Sie sich an das SwissAirDry-Team:
- E-Mail: info@swissairdry.com