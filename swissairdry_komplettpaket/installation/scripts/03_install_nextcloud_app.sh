#!/bin/bash
#
# SwissAirDry Nextcloud-App-Installation
# Installiert die SwissAirDry-App in Nextcloud unter Verwendung von cloud-py-api
#
# Version: 1.0

# Farben für Ausgaben
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Skriptverzeichnis ermitteln
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
BASE_DIR=$(dirname "$SCRIPT_DIR")
LOG_FILE="$BASE_DIR/installation.log"
CONFIG_FILE="$BASE_DIR/config.ini"
APP_DIR="$BASE_DIR/nextcloud_integration/swissairdry-app"

# Logging-Funktion
log() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

# Fehler-Funktion
error() {
    echo -e "${RED}FEHLER: $1${NC}" | tee -a "$LOG_FILE"
    return 1
}

# Einstellungen aus Konfiguration laden
NEXTCLOUD_URL=""
NEXTCLOUD_ADMIN=""
NEXTCLOUD_ADMIN_PASS=""
APP_VERSION="1.0.0"

if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
fi

# App-Dateien vorbereiten
prepare_app_files() {
    log "Bereite App-Dateien vor..."
    
    # App-Verzeichnis erstellen
    mkdir -p "$APP_DIR" || {
        error "Konnte App-Verzeichnis nicht erstellen."
        return 1
    }
    
    # Basis-App-Dateien erstellen
    cat > "$APP_DIR/appinfo.json" << EOF
{
    "id": "swissairdry",
    "name": "SwissAirDry",
    "description": "SwissAirDry-System zur Bautrocknung und Geräteverwaltung",
    "version": "$APP_VERSION",
    "author": "SwissAirDry Team",
    "website": "https://swissairdry.com",
    "category": "tools",
    "bugs": "https://github.com/swissairdry/swissairdry/issues",
    "repository": "https://github.com/swissairdry/swissairdry",
    "dependencies": {
        "cloud_py_api": ">=1.0.0"
    },
    "python_versions": ["3.10"]
}
EOF
    
    # Python-Requirements für die Cloud-Py-Api-App
    cat > "$APP_DIR/requirements.txt" << EOF
fastapi==0.95.1
uvicorn==0.22.0
requests==2.29.0
python-jose==3.3.0
python-dotenv==1.0.0
EOF
    
    # Python-Hauptdatei für die App
    cat > "$APP_DIR/main.py" << EOF
from fastapi import FastAPI, Request, Depends, HTTPException
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
import os
import json
import requests
from datetime import datetime, timedelta

# FastAPI-App erstellen
app = FastAPI()

# Templates und statische Dateien einrichten
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")

# Konfiguration laden
API_URL = os.getenv("API_URL", "http://localhost:5000")
APP_VERSION = os.getenv("APP_VERSION", "1.0.0")

@app.get("/")
async def root(request: Request):
    """Dashboard-Startseite"""
    return templates.TemplateResponse("index.html", {
        "request": request,
        "api_url": API_URL,
        "version": APP_VERSION
    })

@app.get("/health")
async def health():
    """Gesundheitscheck für die App"""
    return {"status": "ok", "version": APP_VERSION}

@app.get("/api_status")
async def api_status():
    """Status des API-Servers prüfen"""
    try:
        response = requests.get(f"{API_URL}/health", timeout=5)
        if response.status_code == 200:
            return response.json()
        else:
            return {"status": "error", "message": f"API-Server antwortet mit Status {response.status_code}"}
    except Exception as e:
        return {"status": "error", "message": f"Verbindung zum API-Server fehlgeschlagen: {str(e)}"}
EOF
    
    # Erstellen der Templatesordner
    mkdir -p "$APP_DIR/templates" || {
        error "Konnte Templates-Verzeichnis nicht erstellen."
        return 1
    }
    
    # Einfache Indexseite
    cat > "$APP_DIR/templates/index.html" << EOF
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SwissAirDry Dashboard</title>
    <link rel="stylesheet" href="{{ url_for('static', path='/css/style.css') }}">
</head>
<body>
    <div class="container">
        <header>
            <h1>SwissAirDry Dashboard</h1>
            <p>Version: {{ version }}</p>
        </header>
        
        <main>
            <div class="card">
                <h2>Willkommen bei SwissAirDry</h2>
                <p>Das integrierte System zur Verwaltung von Bautrocknungsgeräten und -aufträgen.</p>
                
                <div class="api-status">
                    <h3>API-Verbindungsstatus</h3>
                    <div id="status-indicator" class="status-pending">
                        Prüfe Verbindung...
                    </div>
                </div>
            </div>
            
            <div class="card">
                <h2>Module</h2>
                <div class="modules">
                    <a href="#" class="module">
                        <span class="icon">📊</span>
                        Dashboard
                    </a>
                    <a href="#" class="module">
                        <span class="icon">🏢</span>
                        Kunden
                    </a>
                    <a href="#" class="module">
                        <span class="icon">📝</span>
                        Aufträge
                    </a>
                    <a href="#" class="module">
                        <span class="icon">🔧</span>
                        Geräte
                    </a>
                    <a href="#" class="module">
                        <span class="icon">📈</span>
                        Berichte
                    </a>
                </div>
            </div>
        </main>
        
        <footer>
            &copy; 2023-2025 SwissAirDry Team
        </footer>
    </div>
    
    <script>
        // API-Status prüfen
        async function checkApiStatus() {
            const statusIndicator = document.getElementById('status-indicator');
            
            try {
                const response = await fetch('/api_status');
                const data = await response.json();
                
                if (data.status === 'ok') {
                    statusIndicator.className = 'status-ok';
                    statusIndicator.innerHTML = 'Verbunden';
                } else {
                    statusIndicator.className = 'status-error';
                    statusIndicator.innerHTML = 'Fehler: ' + data.message;
                }
            } catch (err) {
                statusIndicator.className = 'status-error';
                statusIndicator.innerHTML = 'Verbindungsfehler';
            }
        }
        
        // Bei Seitenladung prüfen
        document.addEventListener('DOMContentLoaded', checkApiStatus);
    </script>
</body>
</html>
EOF
    
    # CSS-Datei erstellen
    mkdir -p "$APP_DIR/static/css" || {
        error "Konnte static/css-Verzeichnis nicht erstellen."
        return 1
    }
    
    cat > "$APP_DIR/static/css/style.css" << EOF
:root {
    --primary-color: #1a73e8;
    --secondary-color: #34a853;
    --accent-color: #ea4335;
    --background-color: #f8f9fa;
    --text-color: #202124;
}

* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background-color: var(--background-color);
    color: var(--text-color);
    line-height: 1.6;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 20px;
}

header {
    text-align: center;
    margin-bottom: 40px;
}

header h1 {
    color: var(--primary-color);
    margin-bottom: 10px;
}

.card {
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    padding: 20px;
    margin-bottom: 20px;
}

.card h2 {
    color: var(--primary-color);
    margin-bottom: 15px;
    border-bottom: 1px solid #eee;
    padding-bottom: 10px;
}

.api-status {
    margin-top: 20px;
    padding: 15px;
    background-color: #f5f5f5;
    border-radius: 4px;
}

.api-status h3 {
    margin-bottom: 10px;
    font-size: 16px;
}

.status-pending {
    color: #f39c12;
    font-weight: bold;
}

.status-ok {
    color: var(--secondary-color);
    font-weight: bold;
}

.status-error {
    color: var(--accent-color);
    font-weight: bold;
}

.modules {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 15px;
    margin-top: 20px;
}

.module {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 15px;
    background-color: #f5f5f5;
    border-radius: 4px;
    text-decoration: none;
    color: var(--text-color);
    transition: transform 0.2s, box-shadow 0.2s;
}

.module:hover {
    transform: translateY(-5px);
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
}

.module .icon {
    font-size: 24px;
    margin-bottom: 10px;
}

footer {
    text-align: center;
    margin-top: 40px;
    padding-top: 20px;
    border-top: 1px solid #eee;
    color: #666;
}
EOF
    
    log "App-Dateien vorbereitet."
    return 0
}

# App-Paket erstellen
create_app_package() {
    log "Erstelle App-Paket..."
    
    # Ins App-Verzeichnis wechseln
    cd "$APP_DIR" || {
        error "Konnte nicht ins App-Verzeichnis wechseln."
        return 1
    }
    
    # ZIP-Datei erstellen
    APP_ZIP="$BASE_DIR/nextcloud_integration/swissairdry-$APP_VERSION.zip"
    zip -r "$APP_ZIP" * || {
        error "Konnte App-ZIP-Datei nicht erstellen."
        return 1
    }
    
    log "App-Paket erstellt: $APP_ZIP"
    return 0
}

# App über Cloud-Py-Api installieren
install_app_via_api() {
    log "Installiere App über Cloud-Py-Api..."
    
    # Nextcloud-URL und Adminzugang prüfen
    if [ -z "$NEXTCLOUD_URL" ] || [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
        error "Nextcloud-URL oder Admin-Zugangsdaten nicht konfiguriert. Bitte führen Sie zuerst die Umgebungsprüfung durch."
        return 1
    fi
    
    # API-URL für Cloud-Py-Api
    CLOUD_PY_API_URL="$NEXTCLOUD_URL/index.php/apps/cloud_py_api/api/v1"
    
    # Cloud-Py-Api-Token erhalten
    log "Fordere Cloud-Py-Api-Token an..."
    
    TOKEN_RESPONSE=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" \
        -H "OCS-APIRequest: true" \
        -H "Content-Type: application/json" \
        "$NEXTCLOUD_URL/ocs/v2.php/apps/cloud_py_api/api/v1/token")
    
    # Token aus Antwort extrahieren
    TOKEN=$(echo "$TOKEN_RESPONSE" | grep -o '"token":"[^"]*"' | cut -d '"' -f 4)
    
    if [ -z "$TOKEN" ]; then
        error "Konnte kein Cloud-Py-Api-Token erhalten. Bitte prüfen Sie die Nextcloud-Konfiguration."
        return 1
    fi
    
    log "Cloud-Py-Api-Token erhalten."
    
    # App über Cloud-Py-Api hochladen und installieren
    APP_ZIP="$BASE_DIR/nextcloud_integration/swissairdry-$APP_VERSION.zip"
    
    log "Lade App-Paket in Nextcloud hoch..."
    
    UPLOAD_RESPONSE=$(curl -s -X POST \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: multipart/form-data" \
        -F "app=@$APP_ZIP" \
        "$CLOUD_PY_API_URL/upload_app")
    
    # Prüfen, ob der Upload erfolgreich war
    if ! echo "$UPLOAD_RESPONSE" | grep -q '"success":true'; then
        error "App-Upload fehlgeschlagen: $UPLOAD_RESPONSE"
        return 1
    fi
    
    log "App erfolgreich hochgeladen. Aktiviere App..."
    
    # App aktivieren
    ENABLE_RESPONSE=$(curl -s -X POST \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d '{"appId": "swissairdry"}' \
        "$CLOUD_PY_API_URL/enable_app")
    
    # Prüfen, ob die Aktivierung erfolgreich war
    if ! echo "$ENABLE_RESPONSE" | grep -q '"success":true'; then
        error "App-Aktivierung fehlgeschlagen: $ENABLE_RESPONSE"
        return 1
    fi
    
    log "App erfolgreich in Nextcloud installiert und aktiviert."
    return 0
}

# Konfiguration aktualisieren
update_config() {
    log "Aktualisiere Konfiguration..."
    
    # API-URL für die App
    if [ -f "$CONFIG_FILE" ]; then
        source "$CONFIG_FILE"
    fi
    
    # API-URL aus Konfiguration lesen oder Standard verwenden
    API_URL="http://localhost:$API_PORT"
    if [ ! -z "$API_PORT" ]; then
        API_URL="http://localhost:$API_PORT"
    fi
    
    # Konfiguration für Cloud-Py-Api erstellen
    CLOUD_PY_CONFIG="{
        \"API_URL\": \"$API_URL\",
        \"APP_VERSION\": \"$APP_VERSION\"
    }"
    
    # Cloud-Py-Api-Token erhalten
    TOKEN_RESPONSE=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" \
        -H "OCS-APIRequest: true" \
        -H "Content-Type: application/json" \
        "$NEXTCLOUD_URL/ocs/v2.php/apps/cloud_py_api/api/v1/token")
    
    # Token aus Antwort extrahieren
    TOKEN=$(echo "$TOKEN_RESPONSE" | grep -o '"token":"[^"]*"' | cut -d '"' -f 4)
    
    if [ -z "$TOKEN" ]; then
        error "Konnte kein Cloud-Py-Api-Token erhalten. Konfiguration konnte nicht aktualisiert werden."
        return 1
    fi
    
    # Konfiguration über Cloud-Py-Api aktualisieren
    CONFIG_RESPONSE=$(curl -s -X POST \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$CLOUD_PY_CONFIG" \
        "$NEXTCLOUD_URL/index.php/apps/cloud_py_api/api/v1/swissairdry/config")
    
    # Prüfen, ob die Konfiguration erfolgreich aktualisiert wurde
    if ! echo "$CONFIG_RESPONSE" | grep -q '"success":true'; then
        error "Konfigurationsaktualisierung fehlgeschlagen: $CONFIG_RESPONSE"
        return 1
    fi
    
    log "Konfiguration erfolgreich aktualisiert."
    return 0
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}     SwissAirDry Nextcloud-App-Installation        ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    # Nextcloud-Konfiguration prüfen
    if [ -z "$NEXTCLOUD_URL" ] || [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
        if [ -f "$CONFIG_FILE" ]; then
            source "$CONFIG_FILE"
        fi
        
        if [ -z "$NEXTCLOUD_URL" ] || [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
            error "Nextcloud-Konfiguration fehlt. Bitte führen Sie zuerst die Umgebungsprüfung durch."
            return 1
        fi
    fi
    
    # App-Version festlegen
    read -p "App-Version [$APP_VERSION]: " version
    if [ ! -z "$version" ]; then
        APP_VERSION=$version
    fi
    
    # App-Dateien vorbereiten
    prepare_app_files || return 1
    
    # App-Paket erstellen
    create_app_package || return 1
    
    # App in Nextcloud installieren
    install_app_via_api || return 1
    
    # Konfiguration aktualisieren
    update_config || return 1
    
    echo -e "\n${GREEN}Nextcloud-App erfolgreich installiert!${NC}"
    echo -e "Die SwissAirDry-App ist jetzt in Ihrer Nextcloud-Instanz unter der folgenden URL verfügbar:"
    echo -e "${BLUE}$NEXTCLOUD_URL/index.php/apps/swissairdry/${NC}"
    
    return 0
}

# Ausführung starten
main