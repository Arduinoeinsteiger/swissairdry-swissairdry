#!/bin/bash
#
# SwissAirDry Docker-Nextcloud-Integration
# Konfiguriert die Integration mit einer bestehenden Nextcloud-Docker-Instanz
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
NEXTCLOUD_DOCKER_COMPOSE_PATH=""
NEXTCLOUD_CONTAINER_NAME="nextcloud"
API_CONTAINER_NAME="swissairdry-api"
NEXTCLOUD_NETWORK_NAME=""

if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
fi

# Docker-Netzwerk konfigurieren
configure_docker_network() {
    log "Konfiguriere Docker-Netzwerk für Nextcloud-Integration..."
    
    # Nextcloud-Container-Name erfragen, falls nicht in Konfiguration
    if [ -z "$NEXTCLOUD_CONTAINER_NAME" ]; then
        read -p "Name des Nextcloud-Containers [nextcloud]: " container_name
        if [ -z "$container_name" ]; then
            NEXTCLOUD_CONTAINER_NAME="nextcloud"
        else
            NEXTCLOUD_CONTAINER_NAME="$container_name"
        fi
        echo "NEXTCLOUD_CONTAINER_NAME=\"$NEXTCLOUD_CONTAINER_NAME\"" >> "$CONFIG_FILE"
    fi
    
    # Prüfen, ob der Nextcloud-Container läuft
    if ! docker ps | grep -q "$NEXTCLOUD_CONTAINER_NAME"; then
        error "Nextcloud-Container '$NEXTCLOUD_CONTAINER_NAME' wurde nicht gefunden oder läuft nicht. Bitte prüfen Sie den Container-Namen."
        return 1
    fi
    
    # Netzwerknamen des Nextcloud-Containers ermitteln
    NEXTCLOUD_NETWORK_NAME=$(docker inspect -f '{{range $key, $value := .NetworkSettings.Networks}}{{$key}}{{"\n"}}{{end}}' "$NEXTCLOUD_CONTAINER_NAME" | head -n 1)
    
    if [ -z "$NEXTCLOUD_NETWORK_NAME" ]; then
        error "Konnte das Docker-Netzwerk des Nextcloud-Containers nicht ermitteln."
        return 1
    fi
    
    echo "NEXTCLOUD_NETWORK_NAME=\"$NEXTCLOUD_NETWORK_NAME\"" >> "$CONFIG_FILE"
    log "Nextcloud-Container verwendet Netzwerk: $NEXTCLOUD_NETWORK_NAME"
    
    # Prüfen, ob der API-Container läuft
    if ! docker ps | grep -q "$API_CONTAINER_NAME"; then
        error "API-Container '$API_CONTAINER_NAME' wurde nicht gefunden oder läuft nicht. Bitte installieren Sie zuerst den API-Dienst."
        return 1
    fi
    
    # API-Container mit dem Nextcloud-Netzwerk verbinden
    log "Verbinde API-Container mit dem Nextcloud-Netzwerk..."
    
    if ! docker network connect "$NEXTCLOUD_NETWORK_NAME" "$API_CONTAINER_NAME"; then
        error "Konnte API-Container nicht mit dem Nextcloud-Netzwerk verbinden."
        return 1
    fi
    
    log "API-Container erfolgreich mit dem Nextcloud-Netzwerk verbunden."
    return 0
}

# Nextcloud Docker-Compose-Konfiguration anpassen
update_nextcloud_docker_compose() {
    log "Aktualisiere Nextcloud Docker-Compose-Konfiguration..."
    
    # Pfad zur Docker-Compose-Datei erfragen, falls nicht in Konfiguration
    if [ -z "$NEXTCLOUD_DOCKER_COMPOSE_PATH" ]; then
        read -p "Pfad zur Nextcloud docker-compose.yml: " compose_path
        if [ -z "$compose_path" ]; then
            error "Kein Pfad angegeben."
            return 1
        fi
        NEXTCLOUD_DOCKER_COMPOSE_PATH="$compose_path"
        echo "NEXTCLOUD_DOCKER_COMPOSE_PATH=\"$NEXTCLOUD_DOCKER_COMPOSE_PATH\"" >> "$CONFIG_FILE"
    fi
    
    # Prüfen, ob die Datei existiert
    if [ ! -f "$NEXTCLOUD_DOCKER_COMPOSE_PATH" ]; then
        error "Docker-Compose-Datei '$NEXTCLOUD_DOCKER_COMPOSE_PATH' nicht gefunden."
        return 1
    fi
    
    # Backup der Docker-Compose-Datei erstellen
    cp "$NEXTCLOUD_DOCKER_COMPOSE_PATH" "${NEXTCLOUD_DOCKER_COMPOSE_PATH}.backup"
    log "Backup der Docker-Compose-Datei erstellt: ${NEXTCLOUD_DOCKER_COMPOSE_PATH}.backup"
    
    # Umgebungsvariable für Nextcloud-Container hinzufügen
    # Prüfen, ob die Umgebungsvariable bereits vorhanden ist
    if grep -q "SWISSAIRDRY_API_URL" "$NEXTCLOUD_DOCKER_COMPOSE_PATH"; then
        log "SWISSAIRDRY_API_URL ist bereits in der Docker-Compose-Datei konfiguriert."
    else
        # Position für das Einfügen der Umgebungsvariable finden
        # Suche nach dem Nextcloud-Container und den environment-Block
        NEXTCLOUD_SERVICE_LINE=$(grep -n "$NEXTCLOUD_CONTAINER_NAME:" "$NEXTCLOUD_DOCKER_COMPOSE_PATH" | cut -d':' -f1)
        
        if [ -z "$NEXTCLOUD_SERVICE_LINE" ]; then
            error "Konnte den Nextcloud-Service in der Docker-Compose-Datei nicht finden."
            return 1
        fi
        
        # Suche nach dem environment-Block nach der Nextcloud-Service-Zeile
        ENV_LINE=$(tail -n +$NEXTCLOUD_SERVICE_LINE "$NEXTCLOUD_DOCKER_COMPOSE_PATH" | grep -n "environment:" | head -n 1 | cut -d':' -f1)
        
        if [ -z "$ENV_LINE" ]; then
            # Kein environment-Block gefunden, füge einen hinzu
            log "Kein environment-Block gefunden, füge einen hinzu..."
            
            # Einrückung ermitteln
            INDENT=$(grep "$NEXTCLOUD_CONTAINER_NAME:" "$NEXTCLOUD_DOCKER_COMPOSE_PATH" | sed 's/[^ ].*//')
            
            # Zeile nach dem Nextcloud-Service finden
            NEXTCLOUD_SERVICE_END=$(tail -n +$NEXTCLOUD_SERVICE_LINE "$NEXTCLOUD_DOCKER_COMPOSE_PATH" | grep -n "^$INDENT[^ ]" | head -n 1 | cut -d':' -f1)
            
            if [ -z "$NEXTCLOUD_SERVICE_END" ]; then
                # Wenn kein Ende gefunden wird, nehmen wir an, dass es der letzte Service ist
                NEXTCLOUD_SERVICE_END=$(wc -l < "$NEXTCLOUD_DOCKER_COMPOSE_PATH")
            else
                # Korrigiere den Zeilenindex relativ zum Anfang der Datei
                NEXTCLOUD_SERVICE_END=$((NEXTCLOUD_SERVICE_LINE + NEXTCLOUD_SERVICE_END - 1))
            fi
            
            # environment-Block vor dem Ende des Services einfügen
            sed -i "$NEXTCLOUD_SERVICE_END i\\${INDENT}  environment:\\${INDENT}    - SWISSAIRDRY_API_URL=http://$API_CONTAINER_NAME:5000" "$NEXTCLOUD_DOCKER_COMPOSE_PATH"
        else
            # Environment-Block gefunden, Variable hinzufügen
            ENV_LINE=$((NEXTCLOUD_SERVICE_LINE + ENV_LINE - 1))
            
            # Einrückung ermitteln
            INDENT=$(grep -A 1 "environment:" "$NEXTCLOUD_DOCKER_COMPOSE_PATH" | tail -n 1 | sed 's/[^ -].*//')
            
            # Variable einfügen
            sed -i "$ENV_LINE a\\${INDENT}- SWISSAIRDRY_API_URL=http://$API_CONTAINER_NAME:5000" "$NEXTCLOUD_DOCKER_COMPOSE_PATH"
        fi
        
        log "SWISSAIRDRY_API_URL zur Docker-Compose-Datei hinzugefügt."
    fi
    
    return 0
}

# Cloud-Py-Api-App in Nextcloud installieren
install_cloud_py_api() {
    log "Überprüfe, ob Cloud-Py-Api in Nextcloud installiert ist..."
    
    # Nextcloud-URL prüfen
    if [ -z "$NEXTCLOUD_URL" ]; then
        if [ -f "$CONFIG_FILE" ]; then
            source "$CONFIG_FILE"
        fi
        
        if [ -z "$NEXTCLOUD_URL" ]; then
            read -p "Bitte geben Sie die Nextcloud-URL ein (z.B. https://nextcloud.example.com): " NEXTCLOUD_URL
            echo "NEXTCLOUD_URL=\"$NEXTCLOUD_URL\"" >> "$CONFIG_FILE"
        fi
    fi
    
    # Nextcloud-Admin-Zugangsdaten abfragen, falls nicht in Konfiguration vorhanden
    if [ -z "$NEXTCLOUD_ADMIN" ] || [ -z "$NEXTCLOUD_ADMIN_PASS" ]; then
        read -p "Bitte geben Sie den Nextcloud-Administrator-Benutzernamen ein: " NEXTCLOUD_ADMIN
        read -s -p "Bitte geben Sie das Passwort ein: " NEXTCLOUD_ADMIN_PASS
        echo
        
        echo "NEXTCLOUD_ADMIN=\"$NEXTCLOUD_ADMIN\"" >> "$CONFIG_FILE"
        echo "NEXTCLOUD_ADMIN_PASS=\"$NEXTCLOUD_ADMIN_PASS\"" >> "$CONFIG_FILE"
    fi
    
    # Prüfen, ob Cloud-Py-Api installiert ist
    log "Prüfe, ob Cloud-Py-Api installiert ist..."
    
    APPS_RESPONSE=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$NEXTCLOUD_URL/ocs/v1.php/cloud/apps")
    
    if echo "$APPS_RESPONSE" | grep -q "<id>cloud_py_api</id>"; then
        log "Cloud-Py-Api ist bereits installiert."
        
        # Prüfen, ob die App aktiviert ist
        if echo "$APPS_RESPONSE" | grep -q "<id>cloud_py_api</id>.*<active>true</active>"; then
            log "Cloud-Py-Api ist aktiviert."
        else
            log "Cloud-Py-Api ist installiert, aber nicht aktiviert. Aktiviere App..."
            
            ACTIVATE_RESPONSE=$(curl -s -X POST -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$NEXTCLOUD_URL/ocs/v1.php/cloud/apps/cloud_py_api")
            
            if echo "$ACTIVATE_RESPONSE" | grep -q "<status>ok</status>"; then
                log "Cloud-Py-Api erfolgreich aktiviert."
            else
                error "Konnte Cloud-Py-Api nicht aktivieren. Bitte aktivieren Sie die App manuell über die Nextcloud-Admin-Oberfläche."
                return 1
            fi
        fi
    else
        log "Cloud-Py-Api ist nicht installiert. Installiere App..."
        
        # Dies setzt voraus, dass der Nextcloud-Admin-Benutzer die App über die Web-UI installieren kann
        # In einer Docker-Umgebung ist eine direkte Installation über API nicht immer möglich
        
        echo -e "${YELLOW}Cloud-Py-Api muss über die Nextcloud-Admin-Oberfläche installiert werden:${NC}"
        echo -e "1. Melden Sie sich als Administrator bei Nextcloud an: $NEXTCLOUD_URL"
        echo -e "2. Navigieren Sie zu: Einstellungen > Apps > App Store"
        echo -e "3. Suchen Sie nach 'Cloud Py Api' und installieren Sie die App"
        echo -e "4. Stellen Sie sicher, dass die App aktiviert ist"
        
        read -p "Drücken Sie eine Taste, wenn Sie die App installiert haben..."
        
        # Erneut prüfen, ob die App jetzt installiert ist
        APPS_RESPONSE=$(curl -s -u "$NEXTCLOUD_ADMIN:$NEXTCLOUD_ADMIN_PASS" -H "OCS-APIRequest: true" "$NEXTCLOUD_URL/ocs/v1.php/cloud/apps")
        
        if echo "$APPS_RESPONSE" | grep -q "<id>cloud_py_api</id>"; then
            log "Cloud-Py-Api wurde erfolgreich installiert."
        else
            error "Cloud-Py-Api ist immer noch nicht installiert. Bitte stellen Sie sicher, dass die App korrekt installiert wurde."
            return 1
        fi
    fi
    
    return 0
}

# SwissAirDry-App über Cloud-Py-Api installieren
install_swissairdry_via_cloud_py_api() {
    log "Installiere SwissAirDry-App über Cloud-Py-Api..."
    
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
    
    # App-Paket erstellen oder verwenden, wenn vorhanden
    APP_DIR="$BASE_DIR/nextcloud_integration/swissairdry-app"
    APP_VERSION="1.0.0"
    
    if [ ! -d "$APP_DIR" ]; then
        error "App-Verzeichnis nicht gefunden. Bitte führen Sie zuerst 03_install_nextcloud_app.sh aus."
        return 1
    fi
    
    APP_ZIP="$BASE_DIR/nextcloud_integration/swissairdry-$APP_VERSION.zip"
    
    if [ ! -f "$APP_ZIP" ]; then
        log "App-Paket nicht gefunden. Erstelle neues Paket..."
        
        # Ins App-Verzeichnis wechseln
        cd "$APP_DIR" || {
            error "Konnte nicht ins App-Verzeichnis wechseln."
            return 1
        }
        
        # ZIP-Datei erstellen
        zip -r "$APP_ZIP" * || {
            error "Konnte App-ZIP-Datei nicht erstellen."
            return 1
        }
        
        log "App-Paket erstellt: $APP_ZIP"
    fi
    
    # App über Cloud-Py-Api hochladen und installieren
    log "Lade App-Paket in Nextcloud hoch..."
    
    CLOUD_PY_API_URL="$NEXTCLOUD_URL/index.php/apps/cloud_py_api/api/v1"
    
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
    
    # Konfiguration aktualisieren
    log "Aktualisiere App-Konfiguration..."
    
    # API-URL für die App
    API_URL="http://$API_CONTAINER_NAME:5000"
    
    # Konfiguration für Cloud-Py-Api erstellen
    CLOUD_PY_CONFIG="{
        \"API_URL\": \"$API_URL\",
        \"APP_VERSION\": \"$APP_VERSION\"
    }"
    
    # Konfiguration über Cloud-Py-Api aktualisieren
    CONFIG_RESPONSE=$(curl -s -X POST \
        -H "Authorization: Bearer $TOKEN" \
        -H "Content-Type: application/json" \
        -d "$CLOUD_PY_CONFIG" \
        "$CLOUD_PY_API_URL/swissairdry/config")
    
    # Prüfen, ob die Konfiguration erfolgreich aktualisiert wurde
    if ! echo "$CONFIG_RESPONSE" | grep -q '"success":true'; then
        error "Konfigurationsaktualisierung fehlgeschlagen: $CONFIG_RESPONSE"
        return 1
    fi
    
    log "App-Konfiguration erfolgreich aktualisiert."
    return 0
}

# Neustart der Docker-Container
restart_containers() {
    log "Starte Docker-Container neu..."
    
    # Prüfen, ob Docker Compose-Pfad konfiguriert ist
    if [ -z "$NEXTCLOUD_DOCKER_COMPOSE_PATH" ]; then
        error "Nextcloud Docker-Compose-Pfad nicht konfiguriert. Neustart nicht möglich."
        return 1
    fi
    
    # In das Verzeichnis mit der Docker-Compose-Datei wechseln
    cd "$(dirname "$NEXTCLOUD_DOCKER_COMPOSE_PATH")" || {
        error "Konnte nicht ins Verzeichnis mit der Docker-Compose-Datei wechseln."
        return 1
    }
    
    log "Starte Nextcloud-Container neu..."
    docker-compose up -d "$NEXTCLOUD_CONTAINER_NAME" || {
        error "Konnte Nextcloud-Container nicht neu starten."
        return 1
    }
    
    log "Starte API-Container neu..."
    if [ ! -z "$API_INSTALL_DIR" ]; then
        cd "$API_INSTALL_DIR" || {
            error "Konnte nicht ins API-Installationsverzeichnis wechseln."
            return 1
        }
        
        docker-compose restart api || {
            error "Konnte API-Container nicht neu starten."
            return 1
        }
    fi
    
    log "Docker-Container erfolgreich neu gestartet."
    return 0
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}      SwissAirDry Docker-Nextcloud-Integration      ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    echo -e "Dieses Skript konfiguriert die Integration von SwissAirDry mit einer bestehenden Nextcloud-Docker-Installation."
    echo -e "Es wird die Cloud-Py-Api verwendet, um die Kommunikation zwischen den Komponenten zu ermöglichen.\n"
    
    # Schritte anzeigen
    echo -e "Durchzuführende Schritte:"
    echo -e "  1. Docker-Netzwerk-Konfiguration"
    echo -e "  2. Anpassung der Nextcloud Docker-Compose-Konfiguration"
    echo -e "  3. Installation/Aktivierung von Cloud-Py-Api in Nextcloud"
    echo -e "  4. Installation der SwissAirDry-App über Cloud-Py-Api"
    echo -e "  5. Neustart der Docker-Container\n"
    
    read -p "Möchten Sie fortfahren? (j/n): " continue_setup
    
    if [[ $continue_setup != "j" && $continue_setup != "J" ]]; then
        echo -e "Setup abgebrochen."
        return 1
    fi
    
    # Docker-Netzwerk konfigurieren
    configure_docker_network || return 1
    
    # Nextcloud Docker-Compose-Konfiguration anpassen
    update_nextcloud_docker_compose || return 1
    
    # Cloud-Py-Api in Nextcloud installieren
    install_cloud_py_api || return 1
    
    # SwissAirDry-App über Cloud-Py-Api installieren
    install_swissairdry_via_cloud_py_api || return 1
    
    # Docker-Container neu starten
    restart_containers || return 1
    
    echo -e "\n${GREEN}Integration erfolgreich abgeschlossen!${NC}"
    echo -e "SwissAirDry ist jetzt mit Ihrer Nextcloud-Docker-Installation verbunden."
    echo -e "Sie können auf die App unter folgender URL zugreifen:"
    echo -e "${BLUE}$NEXTCLOUD_URL/index.php/apps/swissairdry/${NC}"
    
    return 0
}

# Ausführung starten
main