#!/bin/bash
#
# SwissAirDry-System Installationsskript
# Dieses Skript führt Sie durch die Installation des kompletten SwissAirDry-Systems
#
# Autor: SwissAirDry Team
# Version: 1.0

# Konfigurationsvariablen
INSTALL_DIR="/opt/swissairdry"
LOG_FILE="installation.log"
REQUIRED_SPACE=50000  # 50 GB in MB
POSTGRES_USER="swissairdry"
POSTGRES_DB="swissairdry"
POSTGRES_PASSWORD=$(openssl rand -base64 12)  # Zufälliges sicheres Passwort
JWT_SECRET=$(openssl rand -base64 32)  # Zufälliges JWT-Secret

# Farben für Ausgaben
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging-Funktion
log() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

# Fehler-Funktion
error() {
    echo -e "${RED}FEHLER: $1${NC}" | tee -a "$LOG_FILE"
    exit 1
}

# Prüfen, ob das Skript als Root ausgeführt wird
check_root() {
    log "Prüfe Administratorrechte..."
    if [ "$(id -u)" -ne 0 ]; then
        error "Dieses Skript muss mit Administratorrechten ausgeführt werden. Bitte mit 'sudo' ausführen."
    fi
    log "OK - Administratorrechte vorhanden."
}

# Systemvoraussetzungen prüfen
check_system_requirements() {
    log "Prüfe Systemvoraussetzungen..."
    
    # CPU-Kerne
    CPU_CORES=$(nproc)
    if [ "$CPU_CORES" -lt 4 ]; then
        echo -e "${YELLOW}WARNUNG: Nur $CPU_CORES CPU-Kerne gefunden. Mindestens 4 werden empfohlen.${NC}" | tee -a "$LOG_FILE"
    fi
    
    # RAM
    TOTAL_RAM=$(free -m | awk '/^Mem:/{print $2}')
    if [ "$TOTAL_RAM" -lt 7800 ]; then
        echo -e "${YELLOW}WARNUNG: Nur $TOTAL_RAM MB RAM gefunden. Mindestens 8 GB werden empfohlen.${NC}" | tee -a "$LOG_FILE"
    fi
    
    # Festplattenplatz
    FREE_SPACE=$(df -m "$INSTALL_DIR" | awk 'NR==2 {print $4}')
    if [ "$FREE_SPACE" -lt "$REQUIRED_SPACE" ]; then
        error "Nicht genügend freier Festplattenplatz. Mindestens ${REQUIRED_SPACE}MB benötigt, aber nur ${FREE_SPACE}MB verfügbar."
    fi
    
    log "OK - Systemanforderungen erfüllt."
}

# Erforderliche Pakete installieren
install_dependencies() {
    log "Installiere erforderliche Pakete..."
    
    apt update -y || error "Apt-Update fehlgeschlagen"
    
    # Liste der Pakete zum Installieren
    PACKAGES="docker.io docker-compose python3 python3-pip unzip curl wget git"
    
    apt install -y $PACKAGES || error "Installation der erforderlichen Pakete fehlgeschlagen"
    
    # Docker-Service aktivieren und starten
    systemctl enable docker || error "Docker-Service konnte nicht aktiviert werden"
    systemctl start docker || error "Docker-Service konnte nicht gestartet werden"
    
    log "OK - Erforderliche Pakete installiert."
}

# Verzeichnisstruktur erstellen
create_directory_structure() {
    log "Erstelle Verzeichnisstruktur..."
    
    mkdir -p "$INSTALL_DIR" || error "Konnte Installationsverzeichnis nicht erstellen"
    mkdir -p "$INSTALL_DIR/data/postgres" || error "Konnte Datenverzeichnis nicht erstellen"
    mkdir -p "$INSTALL_DIR/data/mqtt" || error "Konnte MQTT-Datenverzeichnis nicht erstellen"
    mkdir -p "$INSTALL_DIR/config" || error "Konnte Konfigurationsverzeichnis nicht erstellen"
    mkdir -p "$INSTALL_DIR/logs" || error "Konnte Logverzeichnis nicht erstellen"
    
    log "OK - Verzeichnisstruktur erstellt."
}

# Dateien extrahieren
extract_files() {
    log "Extrahiere Anwendungsdateien..."
    
    # Pfad zum Installations-Paket
    PACKAGE_DIR=$(dirname "$(readlink -f "$0")")/..
    FILES_DIR="$PACKAGE_DIR/files"
    
    # Prüfen, ob die Dateien existieren
    if [ ! -d "$FILES_DIR" ]; then
        error "Verzeichnis mit Anwendungsdateien nicht gefunden: $FILES_DIR"
    fi
    
    # Kopieren und Entpacken der ZIP-Dateien
    cp "$FILES_DIR"/*.zip "$INSTALL_DIR/" || error "Konnte ZIP-Dateien nicht kopieren"
    
    cd "$INSTALL_DIR" || error "Konnte nicht ins Installationsverzeichnis wechseln"
    
    # Prüfen, ob die ZIP-Dateien vorhanden sind
    if [ ! -f "swissairdry_admin_dashboard.zip" ]; then
        error "Admin-Dashboard-ZIP-Datei nicht gefunden"
    fi
    
    # Entpacken
    unzip -o swissairdry_admin_dashboard.zip || error "Konnte Admin-Dashboard nicht entpacken"
    
    # Andere ZIP-Dateien entpacken, falls vorhanden
    for zip_file in docker-base-api.zip mobile-api.zip esp32-firmware.zip nextcloud-frontend.zip; do
        if [ -f "$zip_file" ]; then
            unzip -o "$zip_file" || echo -e "${YELLOW}WARNUNG: Konnte $zip_file nicht entpacken${NC}" | tee -a "$LOG_FILE"
        fi
    done
    
    log "OK - Anwendungsdateien extrahiert."
}

# Docker-Compose-Datei erstellen
create_docker_compose() {
    log "Erstelle Docker-Compose-Konfiguration..."
    
    cat > "$INSTALL_DIR/docker-compose.yml" << EOF
version: '3.8'

services:
  postgres:
    image: postgres:14-alpine
    container_name: swissairdry-postgres
    restart: always
    environment:
      POSTGRES_USER: ${POSTGRES_USER}
      POSTGRES_PASSWORD: ${POSTGRES_PASSWORD}
      POSTGRES_DB: ${POSTGRES_DB}
    volumes:
      - ./data/postgres:/var/lib/postgresql/data
    ports:
      - "127.0.0.1:5432:5432"
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${POSTGRES_USER} -d ${POSTGRES_DB}"]
      interval: 10s
      timeout: 5s
      retries: 5

  docker-base-api:
    build: ./docker-base-api
    container_name: swissairdry-api
    restart: always
    depends_on:
      - postgres
    environment:
      - DATABASE_URL=postgresql://${POSTGRES_USER}:${POSTGRES_PASSWORD}@postgres:5432/${POSTGRES_DB}
      - JWT_SECRET_KEY=${JWT_SECRET}
      - DEBUG=False
      - API_PORT=5000
      - API_BASE_URL=http://localhost:5000
      - NEXTCLOUD_FULL_URL=http://localhost:8080/apps/swissairdry
    volumes:
      - ./docker-base-api:/app
      - ./logs/api:/app/logs
    ports:
      - "5000:5000"

  mosquitto:
    image: eclipse-mosquitto:2
    container_name: swissairdry-mqtt
    restart: always
    volumes:
      - ./config/mosquitto.conf:/mosquitto/config/mosquitto.conf
      - ./data/mqtt:/mosquitto/data
      - ./logs/mqtt:/mosquitto/log
    ports:
      - "1883:1883"
      - "9001:9001"

  nextcloud:
    image: nextcloud:25-apache
    container_name: swissairdry-nextcloud
    restart: always
    depends_on:
      - postgres
    environment:
      - POSTGRES_HOST=postgres
      - POSTGRES_DB=nextcloud
      - POSTGRES_USER=${POSTGRES_USER}
      - POSTGRES_PASSWORD=${POSTGRES_PASSWORD}
    volumes:
      - ./nextcloud-frontend:/var/www/html/apps/swissairdry
      - ./data/nextcloud:/var/www/html/data
    ports:
      - "8080:80"

volumes:
  postgres_data:
  nextcloud_data:
EOF

    # Mosquitto-Konfiguration erstellen
    cat > "$INSTALL_DIR/config/mosquitto.conf" << EOF
listener 1883
allow_anonymous false
password_file /mosquitto/config/passwd

persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
EOF

    log "OK - Docker-Compose-Konfiguration erstellt."
}

# Umgebungsvariablen erstellen
create_env_file() {
    log "Erstelle .env-Datei..."
    
    cat > "$INSTALL_DIR/.env" << EOF
# SwissAirDry Umgebungskonfiguration
# Generiert durch Installationsskript am $(date '+%Y-%m-%d %H:%M:%S')

# API-Konfiguration
API_BASE_URL=http://localhost:5000
PORT=5000
DEBUG=False

# Datenbank-Verbindung
DATABASE_URL=postgresql://${POSTGRES_USER}:${POSTGRES_PASSWORD}@postgres:5432/${POSTGRES_DB}

# Nextcloud-Integration
NEXTCLOUD_FULL_URL=http://localhost:8080/apps/swissairdry
NEXTCLOUD_USERNAME=admin
NEXTCLOUD_PASSWORD=nextcloud_password

# MQTT-Konfiguration
MQTT_BROKER_HOST=mosquitto
MQTT_BROKER_PORT=1883
MQTT_USERNAME=swissairdry
MQTT_PASSWORD=mqtt_password

# JWT-Secrets
JWT_SECRET_KEY=${JWT_SECRET}
JWT_ALGORITHM=HS256
JWT_EXPIRATION_MINUTES=1440
EOF

    log "OK - .env-Datei erstellt."
    
    echo -e "${YELLOW}WICHTIG: Bitte bearbeiten Sie die Datei '$INSTALL_DIR/.env' und passen Sie die Passwörter an!${NC}" | tee -a "$LOG_FILE"
}

# Datenbank initialisieren
initialize_database() {
    log "Initialisiere Datenbank und starte Dienste..."
    
    # In das Installationsverzeichnis wechseln
    cd "$INSTALL_DIR" || error "Konnte nicht ins Installationsverzeichnis wechseln"
    
    # Docker Compose-Dienste starten
    docker-compose up -d postgres || error "Konnte PostgreSQL-Container nicht starten"
    
    # Warten, bis die Datenbank bereit ist
    echo "Warte auf Datenbank..." | tee -a "$LOG_FILE"
    sleep 10
    
    # Testen, ob die Datenbank erreichbar ist
    docker-compose exec -T postgres pg_isready -U "$POSTGRES_USER" -d "$POSTGRES_DB" || error "Datenbank ist nicht erreichbar"
    
    log "OK - Datenbank initialisiert und erreichbar."
    
    # Weitere Dienste starten
    docker-compose up -d || error "Konnte nicht alle Dienste starten"
    
    log "OK - Alle Dienste gestartet."
}

# Abschluss-Informationen anzeigen
show_completion_info() {
    echo -e "\n${GREEN}=========================================================${NC}"
    echo -e "${GREEN}           SwissAirDry-System wurde installiert!           ${NC}"
    echo -e "${GREEN}=========================================================${NC}\n"
    
    echo -e "Web-Schnittstellen:"
    echo -e "  - Admin-Dashboard:   ${BLUE}http://localhost:5000/admin${NC}"
    echo -e "  - API-Dokumentation: ${BLUE}http://localhost:5000/docs${NC}"
    echo -e "  - Nextcloud-App:     ${BLUE}http://localhost:8080/apps/swissairdry${NC}\n"
    
    echo -e "Standard-Anmeldeinformationen:"
    echo -e "  - Benutzername: ${BLUE}admin${NC}"
    echo -e "  - Passwort:     ${BLUE}admin123${NC} (Bitte nach der ersten Anmeldung ändern!)\n"
    
    echo -e "Datenbank-Informationen:"
    echo -e "  - Host:     ${BLUE}localhost:5432${NC}"
    echo -e "  - Benutzer: ${BLUE}${POSTGRES_USER}${NC}"
    echo -e "  - Passwort: ${BLUE}${POSTGRES_PASSWORD}${NC}"
    echo -e "  - DB-Name:  ${BLUE}${POSTGRES_DB}${NC}\n"
    
    echo -e "Weitere Informationen finden Sie in der Installationsanleitung:"
    echo -e "  ${BLUE}$INSTALL_DIR/docs/INSTALLATIONSANLEITUNG.md${NC}\n"
    
    echo -e "${YELLOW}WICHTIG: Bitte ändern Sie alle Standard-Passwörter nach der Installation!${NC}\n"
    
    # Kopieren der Installationsanleitung
    mkdir -p "$INSTALL_DIR/docs" || error "Konnte Dokumentationsverzeichnis nicht erstellen"
    cp "$(dirname "$(readlink -f "$0")")/../docs/INSTALLATIONSANLEITUNG.md" "$INSTALL_DIR/docs/" || echo -e "${YELLOW}WARNUNG: Konnte Installationsanleitung nicht kopieren${NC}" | tee -a "$LOG_FILE"
}

# Hauptfunktion
main() {
    echo -e "${BLUE}=========================================================${NC}"
    echo -e "${BLUE}           SwissAirDry-System Installation           ${NC}"
    echo -e "${BLUE}=========================================================${NC}\n"
    
    check_root
    check_system_requirements
    install_dependencies
    create_directory_structure
    extract_files
    create_docker_compose
    create_env_file
    initialize_database
    show_completion_info
    
    log "Installation abgeschlossen."
}

# Ausführung starten
main