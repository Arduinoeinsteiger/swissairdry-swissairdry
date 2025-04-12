#!/bin/bash
#
# SwissAirDry Umgebungsprüfung
# Prüft, ob alle Voraussetzungen für die Installation erfüllt sind
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

# Logging-Funktion
log() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

# Fehler-Funktion
error() {
    echo -e "${RED}FEHLER: $1${NC}" | tee -a "$LOG_FILE"
    return 1
}

# Einstellungen aus Konfiguration laden, falls vorhanden
NEXTCLOUD_URL=""
NEXTCLOUD_ADMIN=""
NEXTCLOUD_ADMIN_PASS=""

CONFIG_FILE="$BASE_DIR/config.ini"
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
fi

# Docker prüfen
check_docker() {
    log "Prüfe Docker-Installation..."
    
    if ! command -v docker &> /dev/null; then
        error "Docker ist nicht installiert. Bitte installieren Sie Docker: https://docs.docker.com/get-docker/"
        return 1
    fi
    
    if ! docker info &> /dev/null; then
        error "Docker-Daemon läuft nicht oder Sie haben keine Berechtigungen. Bitte starten Sie Docker oder fügen Sie Ihren Benutzer zur Docker-Gruppe hinzu."
        return 1
    fi
    
    log "Docker ist korrekt installiert und läuft."
    return 0
}

# Docker Compose prüfen
check_docker_compose() {
    log "Prüfe Docker Compose-Installation..."
    
    if ! command -v docker-compose &> /dev/null; then
        if ! command -v docker compose &> /dev/null; then
            error "Docker Compose ist nicht installiert. Bitte installieren Sie Docker Compose: https://docs.docker.com/compose/install/"
            return 1
        else
            log "Docker Compose (Plugin-Version) ist installiert."
            return 0
        fi
    fi
    
    log "Docker Compose ist korrekt installiert."
    return 0
}

# Nextcloud-Verbindung prüfen
check_nextcloud() {
    log "Prüfe Nextcloud-Verbindung..."
    
    # Nextcloud-URL abfragen, falls nicht in Konfiguration vorhanden
    if [ -z "$NEXTCLOUD_URL" ]; then
        read -p "Bitte geben Sie die Nextcloud-URL ein (z.B. https://nextcloud.example.com): " NEXTCLOUD_URL
        echo "NEXTCLOUD_URL=\"$NEXTCLOUD_URL\"" >> "$CONFIG_FILE"
    fi
    
    # Prüfen, ob die URL erreichbar ist
    if ! curl -s -o /dev/null -w "%{http_code}" "$NEXTCLOUD_URL" | grep -q "200\|301\|302"; then
        error "Nextcloud-URL '$NEXTCLOUD_URL' ist nicht erreichbar. Bitte überprüfen Sie die URL und stellen Sie sicher, dass Nextcloud läuft."
        return 1
    fi
    
    log "Nextcloud-URL ist erreichbar."
    
    # Nextcloud-Version prüfen
    NEXTCLOUD_VERSION=$(curl -s -L "$NEXTCLOUD_URL/status.php" | grep -o '"version":"[0-9.]*"' | cut -d '"' -f 4)
    
    if [ -z "$NEXTCLOUD_VERSION" ]; then
        error "Konnte Nextcloud-Version nicht ermitteln. Ist die URL korrekt?"
        return 1
    fi
    
    MAJOR_VERSION=$(echo "$NEXTCLOUD_VERSION" | cut -d '.' -f 1)
    if [ "$MAJOR_VERSION" -lt 25 ]; then
        error "Nextcloud-Version ist zu alt ($NEXTCLOUD_VERSION). Mindestens Version 25 wird benötigt."
        return 1
    fi
    
    log "Nextcloud-Version $NEXTCLOUD_VERSION ist kompatibel."
    return 0
}

# Cloud-Py-Api prüfen
check_cloud_py_api() {
    log "Prüfe Cloud-Py-Api-Installation..."
    
    # Nextcloud-Admin-Zugangsdaten abfragen, falls nicht in Konfiguration vorhanden
    if [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
        read -p "Bitte geben Sie den Nextcloud-Administrator-Benutzernamen ein: " NEXTCLOUD_ADMIN
        read -s -p "Bitte geben Sie das Passwort ein: " NEXTCLOUD_ADMIN_PASS
        echo
        
        echo "NEXTCLOUD_ADMIN=\"$NEXTCLOUD_ADMIN\"" >> "$CONFIG_FILE"
        echo "NEXTCLOUD_ADMIN_PASS=\"$NEXTCLOUD_ADMIN_PASS\"" >> "$CONFIG_FILE"
    fi
    
    # Mit Nextcloud-OCS-API prüfen, ob Cloud-Py-Api installiert ist
    APPS_URL="$NEXTCLOUD_URL/ocs/v1.php/cloud/apps"
    APPS_JSON=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$APPS_URL")
    
    if ! echo "$APPS_JSON" | grep -q "cloud_py_api"; then
        error "Cloud-Py-Api ist nicht in Nextcloud installiert. Bitte installieren Sie die App aus dem Nextcloud App Store."
        return 1
    fi
    
    # Prüfen, ob die App aktiviert ist
    APP_STATE=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$APPS_URL/cloud_py_api")
    
    if ! echo "$APP_STATE" | grep -q '"enabled":true'; then
        error "Cloud-Py-Api ist installiert, aber nicht aktiviert. Bitte aktivieren Sie die App in Nextcloud."
        return 1
    fi
    
    log "Cloud-Py-Api ist installiert und aktiviert."
    return 0
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}       SwissAirDry Umgebungsprüfung               ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    # Konfigurationsverzeichnis erstellen, falls es nicht existiert
    mkdir -p "$(dirname "$CONFIG_FILE")"
    
    # Prüfungen durchführen
    check_docker || exit 1
    check_docker_compose || exit 1
    check_nextcloud || exit 1
    check_cloud_py_api || exit 1
    
    echo -e "\n${GREEN}Alle Umgebungsprüfungen erfolgreich bestanden!${NC}"
    echo -e "Das System ist bereit für die Installation der SwissAirDry-Komponenten."
    
    return 0
}

# Ausführung starten
main