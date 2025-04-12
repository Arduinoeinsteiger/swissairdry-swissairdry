#!/bin/bash
#
# SwissAirDry Datenimport
# Importiert Daten aus CSV-Dateien in die SwissAirDry-Datenbank
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
DATA_DIR="$BASE_DIR/api_service/data"

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
API_URL=""
API_PORT=""
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

# CSV-Dateien auswählen
select_csv_files() {
    log "Wähle CSV-Dateien für den Import aus..."
    
    # Datenverzeichnis erstellen, falls es nicht existiert
    mkdir -p "$DATA_DIR"
    
    # Bestehende CSV-Dateien im aktuellen Verzeichnis finden
    CSV_FILES=($(find . -maxdepth 1 -name "*.csv"))
    
    if [ ${#CSV_FILES[@]} -eq 0 ]; then
        echo -e "${YELLOW}Keine CSV-Dateien im aktuellen Verzeichnis gefunden.${NC}"
        read -p "Möchten Sie CSV-Dateien manuell angeben? (j/n): " manual_input
        
        if [[ $manual_input == "j" || $manual_input == "J" ]]; then
            read -p "Bitte geben Sie die Pfade zu den CSV-Dateien an (durch Leerzeichen getrennt): " -a manual_csv_files
            CSV_FILES=("${manual_csv_files[@]}")
        else
            error "Keine CSV-Dateien für den Import angegeben."
            return 1
        fi
    else
        echo -e "Gefundene CSV-Dateien:"
        for i in "${!CSV_FILES[@]}"; do
            echo -e "  $((i+1)). ${CSV_FILES[$i]}"
        done
        
        read -p "Möchten Sie alle gefundenen Dateien importieren? (j/n): " import_all
        
        if [[ $import_all != "j" && $import_all != "J" ]]; then
            read -p "Bitte geben Sie die Nummern der zu importierenden Dateien an (durch Leerzeichen getrennt): " -a selected_indices
            
            selected_files=()
            for index in "${selected_indices[@]}"; do
                selected_files+=("${CSV_FILES[$((index-1))]}")
            done
            CSV_FILES=("${selected_files[@]}")
        fi
    fi
    
    # Kopiere ausgewählte CSV-Dateien ins Datenverzeichnis
    echo -e "Folgende Dateien werden importiert:"
    for csv_file in "${CSV_FILES[@]}"; do
        echo -e "  - $csv_file"
        cp "$csv_file" "$DATA_DIR/" || {
            error "Konnte $csv_file nicht ins Datenverzeichnis kopieren."
            return 1
        }
    done
    
    log "CSV-Dateien ausgewählt und ins Datenverzeichnis kopiert."
    return 0
}

# CSV-Daten importieren
import_data() {
    log "Importiere CSV-Daten in die SwissAirDry-Datenbank..."
    
    # Prüfen, ob Docker-Container läuft
    if [ ! -z "$API_INSTALL_DIR" ] && [ -f "$API_INSTALL_DIR/docker-compose.yml" ]; then
        # Kopiere CSV-Dateien in den Container
        log "Kopiere CSV-Dateien in den API-Container..."
        
        # Zielverzeichnis im Container
        CONTAINER_DATA_DIR="/app/data"
        
        # Erstelle Verzeichnis im Container
        docker-compose -f "$API_INSTALL_DIR/docker-compose.yml" exec -T api mkdir -p "$CONTAINER_DATA_DIR" || {
            error "Konnte Datenverzeichnis im Container nicht erstellen."
            return 1
        }
        
        # Kopiere jede CSV-Datei
        for csv_file in "$DATA_DIR"/*.csv; do
            filename=$(basename "$csv_file")
            cat "$csv_file" | docker-compose -f "$API_INSTALL_DIR/docker-compose.yml" exec -T api sh -c "cat > $CONTAINER_DATA_DIR/$filename" || {
                error "Konnte $filename nicht in den Container kopieren."
                return 1
            }
            log "Datei $filename in Container kopiert."
        done
        
        # Import-Befehl im Container ausführen
        log "Führe Import im Container aus..."
        
        docker-compose -f "$API_INSTALL_DIR/docker-compose.yml" exec -T api python -c "
import os
import glob
import pandas as pd
from sqlalchemy import create_engine

# Datenbankverbindung herstellen
db_url = os.environ.get('DATABASE_URL')
if not db_url:
    print('ERROR: DATABASE_URL nicht konfiguriert')
    exit(1)

engine = create_engine(db_url)

# CSV-Dateien im Datenverzeichnis finden
csv_files = glob.glob('/app/data/*.csv')
print(f'Gefunden: {len(csv_files)} CSV-Dateien')

for csv_file in csv_files:
    filename = os.path.basename(csv_file)
    print(f'Importiere {filename}...')
    
    try:
        # Datei lesen
        df = pd.read_csv(csv_file, encoding='utf-8')
        
        # Tabellennamen aus Dateinamen ableiten
        table_name = filename.split('.')[0].lower()
        if table_name.startswith('swissairdry_'):
            table_name = table_name[12:]
        
        # In Datenbank schreiben
        df.to_sql(table_name, engine, if_exists='append', index=False)
        print(f'  {len(df)} Datensätze erfolgreich importiert')
        
    except Exception as e:
        print(f'FEHLER beim Import von {filename}: {str(e)}')
        continue

print('Import abgeschlossen!')
" || {
            error "Datenbankimport fehlgeschlagen."
            return 1
        }
        
        log "Daten erfolgreich importiert."
    else
        error "API-Container nicht gefunden. Bitte installieren Sie zuerst den API-Dienst."
        return 1
    fi
    
    return 0
}

# Import über API (Alternative, falls Container-Methode nicht funktioniert)
import_via_api() {
    log "Importiere CSV-Daten über API..."
    
    # Prüfen, ob die API erreichbar ist
    if ! curl -s "$API_URL/health" | grep -q '"status":"ok"'; then
        error "API-Dienst unter $API_URL ist nicht erreichbar. Bitte stellen Sie sicher, dass der API-Dienst läuft."
        return 1
    fi
    
    # Für jede CSV-Datei einen Import durchführen
    for csv_file in "$DATA_DIR"/*.csv; do
        filename=$(basename "$csv_file")
        log "Importiere $filename über API..."
        
        # Upload der CSV-Datei
        UPLOAD_RESPONSE=$(curl -s -X POST \
            -F "file=@$csv_file" \
            "$API_URL/api/import/csv")
        
        # Prüfen, ob der Upload erfolgreich war
        if ! echo "$UPLOAD_RESPONSE" | grep -q '"success":true'; then
            error "Upload von $filename fehlgeschlagen: $UPLOAD_RESPONSE"
            continue
        fi
        
        log "Datei $filename erfolgreich importiert."
    done
    
    log "Datenimport über API abgeschlossen."
    return 0
}

# Hauptfunktion
main() {
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}           SwissAirDry Datenimport                 ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    echo -e "Dieser Assistent führt Sie durch den Import von CSV-Daten in die SwissAirDry-Datenbank."
    echo -e "Unterstützte CSV-Formate:"
    echo -e "  - Kundenstamm (KUNDENSTAMM.csv)"
    echo -e "  - Gerätestamm (GERAETESTAMMVERZEICHNISS.csv)"
    echo -e "  - Auftragsprotokolle (AUFTRAGSPROTOKOLL.csv)"
    echo -e "  - Gerätestandortwechsel (GERAETESTANDORTWECHSELPROTOKOLL.csv)"
    echo -e "  - Leistungserfassung (LEISTUNGSERFASSUNG.csv)"
    echo -e "  - Messprotokollwerterfassung (MESSPROTOKOLLWERTERFASSUNG.csv)\n"
    
    # CSV-Dateien auswählen
    select_csv_files || return 1
    
    # Import-Methode wählen
    echo -e "\nWählen Sie die Import-Methode:"
    echo -e "  1. Direkter Import in die Datenbank (erfordert Docker-Container)"
    echo -e "  2. Import über API-Endpunkt"
    
    read -p "Import-Methode [1]: " import_method
    
    case $import_method in
        2) import_via_api || return 1 ;;
        *) import_data || return 1 ;;
    esac
    
    echo -e "\n${GREEN}Datenimport erfolgreich abgeschlossen!${NC}"
    echo -e "Die Daten wurden in die SwissAirDry-Datenbank importiert und stehen jetzt zur Verfügung."
    
    return 0
}

# Ausführung starten
main