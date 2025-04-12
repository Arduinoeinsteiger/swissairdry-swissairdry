#!/bin/bash
#
# SwissAirDry API-Dienst-Installation
# Installiert den SwissAirDry API-Server als Docker-Container
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
API_PORT=5000
API_SECRET_KEY=$(openssl rand -hex 32)
POSTGRES_USER="swissairdry"
POSTGRES_PASSWORD=$(openssl rand -base64 12)
POSTGRES_DB="swissairdry"
NEXTCLOUD_URL=""

if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
fi

# API-Installationsverzeichnis
API_INSTALL_DIR="/opt/swissairdry-api"
DOCKER_COMPOSE_FILE="$API_INSTALL_DIR/docker-compose.yml"

# API-Konfiguration erstellen
create_api_config() {
    log "Erstelle API-Konfiguration..."
    
    mkdir -p "$API_INSTALL_DIR/config"
    
    # .env-Datei für Docker Compose erstellen
    cat > "$API_INSTALL_DIR/.env" << EOF
# SwissAirDry API Umgebungsvariablen
API_PORT=$API_PORT
SECRET_KEY=$API_SECRET_KEY
DEBUG=False
POSTGRES_USER=$POSTGRES_USER
POSTGRES_PASSWORD=$POSTGRES_PASSWORD
POSTGRES_DB=$POSTGRES_DB
NEXTCLOUD_URL=$NEXTCLOUD_URL
EOF
    
    log "API-Konfiguration erstellt."
    return 0
}

# Docker Compose-Datei erstellen
create_docker_compose() {
    log "Erstelle Docker Compose-Konfiguration..."
    
    cat > "$DOCKER_COMPOSE_FILE" << EOF
version: '3.8'

services:
  postgres:
    image: postgres:14-alpine
    container_name: swissairdry-postgres
    restart: always
    environment:
      POSTGRES_USER: \${POSTGRES_USER}
      POSTGRES_PASSWORD: \${POSTGRES_PASSWORD}
      POSTGRES_DB: \${POSTGRES_DB}
    volumes:
      - postgres_data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U \${POSTGRES_USER} -d \${POSTGRES_DB}"]
      interval: 10s
      timeout: 5s
      retries: 5

  api:
    build: ./api
    container_name: swissairdry-api
    restart: always
    depends_on:
      - postgres
    environment:
      - DATABASE_URL=postgresql://\${POSTGRES_USER}:\${POSTGRES_PASSWORD}@postgres:5432/\${POSTGRES_DB}
      - SECRET_KEY=\${SECRET_KEY}
      - DEBUG=\${DEBUG}
      - NEXTCLOUD_URL=\${NEXTCLOUD_URL}
    ports:
      - "\${API_PORT}:5000"
    volumes:
      - ./api:/app
      - ./uploads:/app/uploads
      - ./logs:/app/logs

volumes:
  postgres_data:
EOF
    
    log "Docker Compose-Konfiguration erstellt."
    return 0
}

# API-Dockerfile erstellen
create_api_dockerfile() {
    log "Erstelle API-Dockerfile..."
    
    mkdir -p "$API_INSTALL_DIR/api"
    
    cat > "$API_INSTALL_DIR/api/Dockerfile" << EOF
FROM python:3.10-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 5000

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "5000"]
EOF
    
    log "API-Dockerfile erstellt."
    return 0
}

# API-Quellcode erstellen
create_api_source() {
    log "Erstelle API-Quellcode..."
    
    mkdir -p "$API_INSTALL_DIR/api/templates"
    mkdir -p "$API_INSTALL_DIR/api/static"
    mkdir -p "$API_INSTALL_DIR/uploads"
    mkdir -p "$API_INSTALL_DIR/logs"
    
    # Requirements
    cat > "$API_INSTALL_DIR/api/requirements.txt" << EOF
fastapi==0.95.1
uvicorn==0.22.0
sqlalchemy==2.0.12
psycopg2-binary==2.9.6
pydantic==1.10.7
python-multipart==0.0.6
python-jose==3.3.0
passlib==1.7.4
bcrypt==4.0.1
pandas==2.0.1
jinja2==3.1.2
aiofiles==23.1.0
requests==2.29.0
python-dotenv==1.0.0
EOF
    
    # Hauptanwendung
    cat > "$API_INSTALL_DIR/api/main.py" << EOF
import os
from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from dotenv import load_dotenv

# Umgebungsvariablen laden
load_dotenv()

# FastAPI-Anwendung erstellen
app = FastAPI(
    title="SwissAirDry API",
    description="REST API für das SwissAirDry-System zur Bautrocknung",
    version="1.0.0"
)

# Templates und statische Dateien einrichten
templates = Jinja2Templates(directory="templates")
app.mount("/static", StaticFiles(directory="static"), name="static")

@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/health")
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    return {"status": "ok", "message": "API-Server läuft", "service": "swissairdry-api"}

@app.get("/nextcloud-connect")
async def nextcloud_connect():
    """Endpunkt zur Überprüfung der Nextcloud-Verbindung."""
    nextcloud_url = os.getenv("NEXTCLOUD_URL", "")
    
    if not nextcloud_url:
        return {"status": "error", "message": "Nextcloud-URL nicht konfiguriert"}
    
    return {
        "status": "ok", 
        "message": "Nextcloud-Verbindung konfiguriert", 
        "nextcloud_url": nextcloud_url
    }
EOF
    
    # Einfache HTML-Startseite
    cat > "$API_INSTALL_DIR/api/templates/index.html" << EOF
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SwissAirDry API</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            background-color: #f5f5f5;
        }
        .container {
            text-align: center;
            padding: 2rem;
            background-color: white;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            max-width: 600px;
        }
        h1 {
            color: #2c3e50;
        }
        p {
            color: #7f8c8d;
            margin-bottom: 1.5rem;
        }
        .links {
            margin-top: 2rem;
        }
        .links a {
            display: inline-block;
            margin: 0 10px;
            color: #3498db;
            text-decoration: none;
        }
        .links a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>SwissAirDry API Server</h1>
        <p>Der API-Server ist aktiv und bereit für die Integration mit Nextcloud.</p>
        <div class="links">
            <a href="/docs">API-Dokumentation</a>
            <a href="/health">Status-Check</a>
            <a href="/nextcloud-connect">Nextcloud-Verbindung prüfen</a>
        </div>
    </div>
</body>
</html>
EOF
    
    log "API-Quellcode erstellt."
    return 0
}

# Hafen prüfen
check_port() {
    log "Prüfe, ob Port $API_PORT verfügbar ist..."
    
    if netstat -tuln | grep -q ":$API_PORT "; then
        error "Port $API_PORT wird bereits verwendet. Bitte ändern Sie den Port in der Konfiguration."
        return 1
    fi
    
    log "Port $API_PORT ist verfügbar."
    return 0
}

# Docker-Container bauen und starten
build_and_start() {
    log "Baue und starte Docker-Container..."
    
    cd "$API_INSTALL_DIR" || {
        error "Konnte nicht ins Verzeichnis $API_INSTALL_DIR wechseln."
        return 1
    }
    
    # Docker-Compose ausführen
    docker-compose pull || {
        error "Konnte Docker-Images nicht herunterladen."
        return 1
    }
    
    docker-compose build || {
        error "Konnte Docker-Container nicht bauen."
        return 1
    }
    
    docker-compose up -d || {
        error "Konnte Docker-Container nicht starten."
        return 1
    }
    
    log "Docker-Container erfolgreich gebaut und gestartet."
    
    # Warten, bis die Datenbank bereit ist
    echo "Warte auf Datenbank..."
    sleep 10
    
    log "API-Dienst-Installation abgeschlossen."
    return 0
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}      SwissAirDry API-Dienst-Installation          ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    # Prüfen, ob Docker installiert ist
    if ! command -v docker &> /dev/null; then
        error "Docker ist nicht installiert. Bitte führen Sie zuerst die Umgebungsprüfung durch."
        return 1
    fi
    
    # Nextcloud-URL laden
    if [ -z "$NEXTCLOUD_URL" ]; then
        if [ -f "$CONFIG_FILE" ]; then
            source "$CONFIG_FILE"
        fi
        
        if [ -z "$NEXTCLOUD_URL" ]; then
            read -p "Bitte geben Sie die Nextcloud-URL ein (z.B. https://nextcloud.example.com): " NEXTCLOUD_URL
            echo "NEXTCLOUD_URL=\"$NEXTCLOUD_URL\"" >> "$CONFIG_FILE"
        fi
    fi
    
    # API-Port abfragen oder verwenden
    read -p "API-Port [$API_PORT]: " port
    if [ ! -z "$port" ]; then
        API_PORT=$port
        echo "API_PORT=$API_PORT" >> "$CONFIG_FILE"
    fi
    
    # API-Installationsverzeichnis abfragen oder verwenden
    read -p "Installationsverzeichnis [$API_INSTALL_DIR]: " install_dir
    if [ ! -z "$install_dir" ]; then
        API_INSTALL_DIR=$install_dir
    fi
    
    # Verzeichnis erstellen
    mkdir -p "$API_INSTALL_DIR" || {
        error "Konnte Installationsverzeichnis nicht erstellen. Bitte stellen Sie sicher, dass Sie Schreibrechte haben."
        return 1
    }
    
    # Port prüfen
    check_port || return 1
    
    # Konfiguration erstellen
    create_api_config || return 1
    create_docker_compose || return 1
    create_api_dockerfile || return 1
    create_api_source || return 1
    
    # Container bauen und starten
    build_and_start || return 1
    
    echo -e "\n${GREEN}API-Dienst erfolgreich installiert!${NC}"
    echo -e "Der API-Server läuft unter http://localhost:$API_PORT"
    
    # Konfiguration in config.ini speichern
    echo "API_INSTALL_DIR=\"$API_INSTALL_DIR\"" >> "$CONFIG_FILE"
    echo "API_PORT=$API_PORT" >> "$CONFIG_FILE"
    echo "API_SECRET_KEY=\"$API_SECRET_KEY\"" >> "$CONFIG_FILE"
    echo "POSTGRES_USER=\"$POSTGRES_USER\"" >> "$CONFIG_FILE"
    echo "POSTGRES_PASSWORD=\"$POSTGRES_PASSWORD\"" >> "$CONFIG_FILE"
    echo "POSTGRES_DB=\"$POSTGRES_DB\"" >> "$CONFIG_FILE"
    
    return 0
}

# Ausführung starten
main