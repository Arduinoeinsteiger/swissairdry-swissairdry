#!/bin/bash
#
# SwissAirDry Verbindungskonfiguration
# Konfiguriert die Verbindung zwischen Nextcloud-App und API-Dienst
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
NEXTCLOUD_URL=""
NEXTCLOUD_ADMIN=""
NEXTCLOUD_ADMIN_PASS=""
API_URL=""
API_PORT=""
API_SECRET_KEY=""
API_INSTALL_DIR=""

if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
fi

# API-URL ermitteln
if [ -z "$API_URL" ]; then
    if [ ! -z "$API_PORT" ]; then
        API_URL="http://localhost:$API_PORT"
    else
        API_URL="http://localhost:5000"
    fi
fi

# Verbindungsüberprüfung
check_connection() {
    log "Prüfe Verbindung zwischen Nextcloud und API-Dienst..."
    
    # Prüfen, ob die API erreichbar ist
    log "Prüfe, ob API-Dienst erreichbar ist..."
    
    if ! curl -s "$API_URL/health" | grep -q '"status":"ok"'; then
        error "API-Dienst unter $API_URL ist nicht erreichbar. Bitte stellen Sie sicher, dass der API-Dienst läuft."
        return 1
    fi
    
    log "API-Dienst ist erreichbar."
    
    # Prüfen, ob Nextcloud erreichbar ist
    log "Prüfe, ob Nextcloud erreichbar ist..."
    
    if ! curl -s "$NEXTCLOUD_URL/status.php" | grep -q '"installed":true'; then
        error "Nextcloud unter $NEXTCLOUD_URL ist nicht erreichbar. Bitte überprüfen Sie die URL."
        return 1
    fi
    
    log "Nextcloud ist erreichbar."
    
    # Prüfen, ob Cloud-Py-Api aktiviert ist
    log "Prüfe, ob Cloud-Py-Api aktiviert ist..."
    
    if ! curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$NEXTCLOUD_URL/ocs/v1.php/cloud/apps/cloud_py_api" | grep -q "<enabled>1</enabled>"; then
        error "Cloud-Py-Api ist nicht aktiviert. Bitte aktivieren Sie die App in Nextcloud."
        return 1
    fi
    
    log "Cloud-Py-Api ist aktiviert."
    
    # Prüfen, ob SwissAirDry-App installiert ist
    log "Prüfe, ob SwissAirDry-App installiert ist..."
    
    if ! curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$NEXTCLOUD_URL/ocs/v1.php/cloud/apps/swissairdry" | grep -q "<id>swissairdry</id>"; then
        error "SwissAirDry-App ist nicht installiert. Bitte installieren Sie zuerst die Nextcloud-App."
        return 1
    fi
    
    log "SwissAirDry-App ist installiert."
    
    return 0
}

# API-Konfiguration für Nextcloud aktualisieren
update_api_config() {
    log "Aktualisiere API-Konfiguration für Nextcloud..."
    
    # Nextcloud-URL in API-Konfiguration aktualisieren
    if [ ! -z "$API_INSTALL_DIR" ] && [ -f "$API_INSTALL_DIR/.env" ]; then
        # Prüfen, ob die Nextcloud-URL bereits in der Konfiguration steht
        if grep -q "NEXTCLOUD_URL=" "$API_INSTALL_DIR/.env"; then
            # Bestehenden Eintrag aktualisieren
            sed -i "s|NEXTCLOUD_URL=.*|NEXTCLOUD_URL=$NEXTCLOUD_URL|" "$API_INSTALL_DIR/.env"
        else
            # Neuen Eintrag hinzufügen
            echo "NEXTCLOUD_URL=$NEXTCLOUD_URL" >> "$API_INSTALL_DIR/.env"
        fi
        
        log "API-Konfiguration aktualisiert."
    else
        error "API-Konfigurationsdatei nicht gefunden. Bitte installieren Sie zuerst den API-Dienst."
        return 1
    fi
    
    # Docker-Container neu starten, falls vorhanden
    if [ ! -z "$API_INSTALL_DIR" ] && [ -f "$API_INSTALL_DIR/docker-compose.yml" ]; then
        log "Starte API-Dienst neu..."
        
        cd "$API_INSTALL_DIR" && docker-compose restart api || {
            error "Konnte API-Dienst nicht neu starten."
            return 1
        }
        
        log "API-Dienst neu gestartet."
    fi
    
    return 0
}

# Nextcloud-App-Konfiguration aktualisieren
update_nextcloud_config() {
    log "Aktualisiere Nextcloud-App-Konfiguration..."
    
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
    
    # Konfiguration für Cloud-Py-Api erstellen
    log "Sende API-Konfiguration an SwissAirDry-App..."
    
    CLOUD_PY_CONFIG="{
        \"API_URL\": \"$API_URL\",
        \"API_SECRET_KEY\": \"$API_SECRET_KEY\"
    }"
    
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
    
    log "Nextcloud-App-Konfiguration erfolgreich aktualisiert."
    return 0
}

# Test der Verbindung
test_connection() {
    log "Teste Verbindung zwischen Nextcloud und API-Dienst..."
    
    # Token für Cloud-Py-Api erhalten
    TOKEN_RESPONSE=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" \
        -H "OCS-APIRequest: true" \
        -H "Content-Type: application/json" \
        "$NEXTCLOUD_URL/ocs/v2.php/apps/cloud_py_api/api/v1/token")
    
    TOKEN=$(echo "$TOKEN_RESPONSE" | grep -o '"token":"[^"]*"' | cut -d '"' -f 4)
    
    if [ -z "$TOKEN" ]; then
        error "Konnte kein Cloud-Py-Api-Token erhalten. Verbindungstest nicht möglich."
        return 1
    fi
    
    # Testanfrage über Cloud-Py-Api an SwissAirDry-App senden
    TEST_RESPONSE=$(curl -s -X GET \
        -H "Authorization: Bearer $TOKEN" \
        "$NEXTCLOUD_URL/index.php/apps/cloud_py_api/api/v1/swissairdry/api_status")
    
    # Prüfen, ob die Anfrage erfolgreich war
    if echo "$TEST_RESPONSE" | grep -q '"status":"ok"'; then
        log "Verbindungstest erfolgreich! Die Nextcloud-App kann mit dem API-Dienst kommunizieren."
        return 0
    else
        error "Verbindungstest fehlgeschlagen: $TEST_RESPONSE"
        return 1
    fi
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}      SwissAirDry Verbindungskonfiguration         ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    # Konfiguration prüfen
    if [ -z "$NEXTCLOUD_URL" ] || [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
        if [ -f "$CONFIG_FILE" ]; then
            source "$CONFIG_FILE"
        fi
        
        if [ -z "$NEXTCLOUD_URL" ] || [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
            error "Nextcloud-Konfiguration fehlt. Bitte führen Sie zuerst die Umgebungsprüfung durch."
            return 1
        fi
    fi
    
    # API-URL abfragen, falls nicht in Konfiguration
    if [ -z "$API_URL" ]; then
        read -p "API-URL [http://localhost:5000]: " url
        if [ -z "$url" ]; then
            API_URL="http://localhost:5000"
        else
            API_URL="$url"
        fi
        echo "API_URL=\"$API_URL\"" >> "$CONFIG_FILE"
    fi
    
    # Verbindung prüfen
    check_connection || return 1
    
    # API-Konfiguration aktualisieren
    update_api_config || return 1
    
    # Nextcloud-App-Konfiguration aktualisieren
    update_nextcloud_config || return 1
    
    # Verbindung testen
    test_connection || return 1
    
    echo -e "\n${GREEN}Verbindung erfolgreich konfiguriert!${NC}"
    echo -e "Die SwissAirDry-App in Nextcloud ist jetzt mit dem API-Dienst verbunden."
    echo -e "Sie können auf die App unter folgender URL zugreifen:"
    echo -e "${BLUE}$NEXTCLOUD_URL/index.php/apps/swissairdry/${NC}"
    
    return 0
}

# Ausführung starten
main