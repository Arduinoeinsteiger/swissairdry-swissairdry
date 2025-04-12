#!/bin/bash
#
# SwissAirDry Docker-Compose-Update
# Aktualisiert eine bestehende docker-compose.yml für die Integration von SwissAirDry
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

# docker-compose.yml aktualisieren
update_docker_compose() {
    log "Aktualisiere docker-compose.yml..."
    
    # Eingabeparameter
    local DOCKER_COMPOSE_FILE="$1"
    local NEXTCLOUD_SERVICE_NAME="$2"
    local API_SERVICE_NAME="swissairdry-api"
    local DB_SERVICE_NAME="swissairdry-db"
    
    # Prüfen, ob die Datei existiert
    if [ ! -f "$DOCKER_COMPOSE_FILE" ]; then
        error "Docker-Compose-Datei '$DOCKER_COMPOSE_FILE' nicht gefunden."
        return 1
    fi
    
    # Backup erstellen
    cp "$DOCKER_COMPOSE_FILE" "${DOCKER_COMPOSE_FILE}.backup"
    log "Backup erstellt: ${DOCKER_COMPOSE_FILE}.backup"
    
    # Prüfen, ob der Nextcloud-Service in der Datei vorhanden ist
    if ! grep -q "$NEXTCLOUD_SERVICE_NAME:" "$DOCKER_COMPOSE_FILE"; then
        error "Nextcloud-Service '$NEXTCLOUD_SERVICE_NAME' nicht in der Docker-Compose-Datei gefunden."
        return 1
    fi
    
    # Prüfen, ob die SwissAirDry-Services bereits vorhanden sind
    if grep -q "$API_SERVICE_NAME:" "$DOCKER_COMPOSE_FILE"; then
        log "SwissAirDry-API-Service bereits in der Docker-Compose-Datei vorhanden."
        return 0
    fi
    
    # Ermitteln der Version des Compose-Files
    COMPOSE_VERSION=$(grep -m 1 "^version:" "$DOCKER_COMPOSE_FILE" | cut -d "'" -f 2 | cut -d '"' -f 2)
    if [ -z "$COMPOSE_VERSION" ]; then
        COMPOSE_VERSION="3.8"  # Standardversion, falls keine gefunden wurde
        log "Keine Docker-Compose-Version gefunden, verwende Standard: $COMPOSE_VERSION"
    else
        log "Gefundene Docker-Compose-Version: $COMPOSE_VERSION"
    fi
    
    # Netzwerkname ermitteln oder Standardwert verwenden
    NETWORK_NAME="default"
    if grep -q "networks:" "$DOCKER_COMPOSE_FILE"; then
        # Erste Definition eines Netzwerks nehmen
        NETWORK_NAME=$(grep -A 10 "networks:" "$DOCKER_COMPOSE_FILE" | grep -m 1 "^\s*[a-zA-Z0-9_-]\+:" | awk '{print $1}' | tr -d ':')
        if [ -z "$NETWORK_NAME" ]; then
            NETWORK_NAME="default"
        fi
        log "Gefundenes Netzwerk: $NETWORK_NAME"
    else
        log "Kein Netzwerk gefunden, verwende Standardnetzwerk."
    fi
    
    # SwissAirDry-Services zur docker-compose.yml hinzufügen
    log "Füge SwissAirDry-Services hinzu..."
    
    # Indentation ermitteln
    INDENT=$(grep "$NEXTCLOUD_SERVICE_NAME:" "$DOCKER_COMPOSE_FILE" | sed 's/[^ ].*//')
    
    # Erstelle temporäre Datei mit den neuen Services
    TEMP_FILE=$(mktemp)
    
    cat > "$TEMP_FILE" << EOF

${INDENT}$API_SERVICE_NAME:
${INDENT}  image: swissairdry/api:latest
${INDENT}  container_name: $API_SERVICE_NAME
${INDENT}  restart: always
${INDENT}  depends_on:
${INDENT}    - $DB_SERVICE_NAME
${INDENT}  environment:
${INDENT}    - DATABASE_URL=postgresql://swissairdry:swissairdry@$DB_SERVICE_NAME:5432/swissairdry
${INDENT}    - SECRET_KEY=\${SWISSAIRDRY_SECRET_KEY:-$(openssl rand -hex 32)}
${INDENT}    - DEBUG=False
${INDENT}    - NEXTCLOUD_URL=http://$NEXTCLOUD_SERVICE_NAME:80
${INDENT}  volumes:
${INDENT}    - swissairdry_data:/app/data
${INDENT}    - swissairdry_uploads:/app/uploads
${INDENT}    - swissairdry_logs:/app/logs
${INDENT}  networks:
${INDENT}    - $NETWORK_NAME

${INDENT}$DB_SERVICE_NAME:
${INDENT}  image: postgres:14-alpine
${INDENT}  container_name: $DB_SERVICE_NAME
${INDENT}  restart: always
${INDENT}  environment:
${INDENT}    - POSTGRES_USER=swissairdry
${INDENT}    - POSTGRES_PASSWORD=swissairdry
${INDENT}    - POSTGRES_DB=swissairdry
${INDENT}  volumes:
${INDENT}    - swissairdry_db:/var/lib/postgresql/data
${INDENT}  networks:
${INDENT}    - $NETWORK_NAME
EOF
    
    # Füge die neuen Services nach dem letzten Service ein
    # Suche die Zeile, an der der Volumes-Block beginnt, oder das Ende der Datei
    LAST_SERVICE_LINE=$(grep -n "^volumes:" "$DOCKER_COMPOSE_FILE" | head -n 1 | cut -d ':' -f 1)
    
    if [ -z "$LAST_SERVICE_LINE" ]; then
        # Kein Volumes-Block gefunden, füge am Ende der Datei hinzu
        cat "$TEMP_FILE" >> "$DOCKER_COMPOSE_FILE"
        
        # Füge Volumes-Block hinzu, falls keiner vorhanden ist
        if ! grep -q "^volumes:" "$DOCKER_COMPOSE_FILE"; then
            echo -e "\nvolumes:" >> "$DOCKER_COMPOSE_FILE"
        fi
        
        # Füge SwissAirDry-Volumes hinzu
        cat >> "$DOCKER_COMPOSE_FILE" << EOF
  swissairdry_data:
  swissairdry_uploads:
  swissairdry_logs:
  swissairdry_db:
EOF
    else
        # Volumes-Block gefunden, füge Services davor ein
        head -n $((LAST_SERVICE_LINE - 1)) "$DOCKER_COMPOSE_FILE" > "${DOCKER_COMPOSE_FILE}.tmp"
        cat "$TEMP_FILE" >> "${DOCKER_COMPOSE_FILE}.tmp"
        tail -n +$LAST_SERVICE_LINE "$DOCKER_COMPOSE_FILE" >> "${DOCKER_COMPOSE_FILE}.tmp"
        
        # SwissAirDry-Volumes zum Volumes-Block hinzufügen
        if ! grep -q "swissairdry_" "${DOCKER_COMPOSE_FILE}.tmp"; then
            # Finde die erste Zeile nach "volumes:", um die Indentation zu ermitteln
            VOLUMES_INDENT=$(grep -A 1 "^volumes:" "${DOCKER_COMPOSE_FILE}.tmp" | tail -n 1 | grep -o "^ *")
            
            # Füge Volumes ein
            sed -i "/^volumes:/a\\
${VOLUMES_INDENT}swissairdry_data:\\
${VOLUMES_INDENT}swissairdry_uploads:\\
${VOLUMES_INDENT}swissairdry_logs:\\
${VOLUMES_INDENT}swissairdry_db:" "${DOCKER_COMPOSE_FILE}.tmp"
        fi
        
        # Ersetze die ursprüngliche Datei
        mv "${DOCKER_COMPOSE_FILE}.tmp" "$DOCKER_COMPOSE_FILE"
    fi
    
    # Temporäre Datei löschen
    rm -f "$TEMP_FILE"
    
    # Umgebungsvariable zum Nextcloud-Service hinzufügen
    log "Füge Umgebungsvariable zum Nextcloud-Service hinzu..."
    
    # Suche nach dem environment-Block im Nextcloud-Service
    NEXTCLOUD_SERVICE_START=$(grep -n "$NEXTCLOUD_SERVICE_NAME:" "$DOCKER_COMPOSE_FILE" | head -n 1 | cut -d ':' -f 1)
    
    if [ -z "$NEXTCLOUD_SERVICE_START" ]; then
        error "Konnte den Beginn des Nextcloud-Services nicht finden."
        return 1
    fi
    
    # Suche das Ende des Nextcloud-Services
    NEXTCLOUD_SERVICE_END=$(tail -n +$NEXTCLOUD_SERVICE_START "$DOCKER_COMPOSE_FILE" | grep -n "^$INDENT[a-zA-Z0-9_-]\+:" | head -n 1 | cut -d ':' -f 1)
    
    if [ -z "$NEXTCLOUD_SERVICE_END" ]; then
        # Wenn kein weiterer Service folgt, nehme den Rest der Datei
        NEXTCLOUD_SERVICE_END=$(wc -l < "$DOCKER_COMPOSE_FILE")
    else
        # Korrigiere den Index relativ zum Dateianfang
        NEXTCLOUD_SERVICE_END=$((NEXTCLOUD_SERVICE_START + NEXTCLOUD_SERVICE_END - 1))
    fi
    
    # Extrahiere den Nextcloud-Service-Block
    NEXTCLOUD_BLOCK=$(sed -n "${NEXTCLOUD_SERVICE_START},${NEXTCLOUD_SERVICE_END}p" "$DOCKER_COMPOSE_FILE")
    
    # Prüfe, ob ein environment-Block existiert
    if echo "$NEXTCLOUD_BLOCK" | grep -q "environment:"; then
        # Finde die Zeile mit "environment:"
        ENV_LINE=$(echo "$NEXTCLOUD_BLOCK" | grep -n "environment:" | head -n 1 | cut -d ':' -f 1)
        ENV_LINE=$((NEXTCLOUD_SERVICE_START + ENV_LINE - 1))
        
        # Finde die Einrückung der environment-Variablen
        ENV_INDENT=$(echo "$NEXTCLOUD_BLOCK" | grep "environment:" | sed 's/[^ ].*//')
        ENV_INDENT="${ENV_INDENT}  "
        
        # Prüfe, ob die Variable bereits existiert
        if ! echo "$NEXTCLOUD_BLOCK" | grep -q "SWISSAIRDRY_API_URL"; then
            # Füge die Variable zum environment-Block hinzu
            sed -i "${ENV_LINE}a\\
${ENV_INDENT}- SWISSAIRDRY_API_URL=http://$API_SERVICE_NAME:5000" "$DOCKER_COMPOSE_FILE"
        fi
    else
        # Kein environment-Block gefunden, füge einen hinzu
        # Finde eine gute Stelle zum Einfügen (vor volumes, ports, etc.)
        INSERT_POINT=$NEXTCLOUD_SERVICE_START
        
        for keyword in "volumes:" "ports:" "networks:" "depends_on:"; do
            KEYWORD_LINE=$(echo "$NEXTCLOUD_BLOCK" | grep -n "$keyword" | head -n 1 | cut -d ':' -f 1)
            if [ ! -z "$KEYWORD_LINE" ]; then
                KEYWORD_LINE=$((NEXTCLOUD_SERVICE_START + KEYWORD_LINE - 1))
                if [ $KEYWORD_LINE -gt $INSERT_POINT ]; then
                    INSERT_POINT=$KEYWORD_LINE
                    break
                fi
            fi
        done
        
        # Ermittle die Einrückung für den environment-Block
        ENV_BLOCK_INDENT=$(echo "$NEXTCLOUD_BLOCK" | grep -m 1 "  [a-zA-Z]" | sed 's/[^ ].*//')
        
        # Füge den environment-Block hinzu
        sed -i "${INSERT_POINT}i\\
${ENV_BLOCK_INDENT}environment:\\
${ENV_BLOCK_INDENT}  - SWISSAIRDRY_API_URL=http://$API_SERVICE_NAME:5000" "$DOCKER_COMPOSE_FILE"
    fi
    
    log "Docker-Compose-Datei erfolgreich aktualisiert."
    return 0
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}       SwissAirDry Docker-Compose-Update           ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    echo -e "Dieses Skript aktualisiert eine bestehende docker-compose.yml für die Integration von SwissAirDry."
    echo -e "Es fügt die notwendigen Services, Volumes und Umgebungsvariablen hinzu.\n"
    
    # Eingabeparameter abfragen
    read -p "Pfad zur docker-compose.yml: " DOCKER_COMPOSE_FILE
    
    if [ -z "$DOCKER_COMPOSE_FILE" ]; then
        error "Kein Pfad angegeben."
        return 1
    fi
    
    if [ ! -f "$DOCKER_COMPOSE_FILE" ]; then
        error "Datei nicht gefunden: $DOCKER_COMPOSE_FILE"
        return 1
    fi
    
    read -p "Name des Nextcloud-Services [nextcloud]: " NEXTCLOUD_SERVICE_NAME
    
    if [ -z "$NEXTCLOUD_SERVICE_NAME" ]; then
        NEXTCLOUD_SERVICE_NAME="nextcloud"
    fi
    
    # Docker-Compose-Datei aktualisieren
    update_docker_compose "$DOCKER_COMPOSE_FILE" "$NEXTCLOUD_SERVICE_NAME" || return 1
    
    echo -e "\n${GREEN}Docker-Compose-Datei erfolgreich aktualisiert!${NC}"
    echo -e "Die SwissAirDry-Services wurden zur Docker-Compose-Konfiguration hinzugefügt."
    echo -e "Sie können die Dienste jetzt starten mit:"
    echo -e "${BLUE}cd $(dirname "$DOCKER_COMPOSE_FILE") && docker-compose up -d${NC}"
    
    return 0
}

# Ausführung starten
main