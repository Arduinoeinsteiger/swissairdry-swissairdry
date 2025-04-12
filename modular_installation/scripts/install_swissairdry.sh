#!/bin/bash
#
# SwissAirDry Hauptinstallationsskript
# Installiert alle Komponenten des SwissAirDry-Systems in eine bestehende Nextcloud-Umgebung
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
    exit 1
}

# Hauptmenü anzeigen
show_menu() {
    clear
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}        SwissAirDry Modulare Installation        ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    echo -e "Installationsschritte:"
    echo -e "  ${YELLOW}1.${NC} Umgebung prüfen"
    echo -e "  ${YELLOW}2.${NC} API-Dienst installieren"
    echo -e "  ${YELLOW}3.${NC} Nextcloud-App installieren"
    echo -e "  ${YELLOW}4.${NC} Verbindung konfigurieren"
    echo -e "  ${YELLOW}5.${NC} Daten importieren (optional)"
    echo -e "  ${YELLOW}6.${NC} Alles installieren (1-5)"
    echo -e "  ${YELLOW}7.${NC} Beenden\n"
    
    echo -e "Zusätzliche Optionen:"
    echo -e "  ${YELLOW}d.${NC} Deinstallieren"
    echo -e "  ${YELLOW}r.${NC} Neu starten"
    echo -e "  ${YELLOW}h.${NC} Hilfe anzeigen\n"
    
    read -p "Bitte wählen Sie eine Option [1-7]: " option
    
    case $option in
        1) run_environment_check ;;
        2) install_api_service ;;
        3) install_nextcloud_app ;;
        4) configure_connection ;;
        5) import_data ;;
        6) run_all ;;
        7) exit 0 ;;
        d) uninstall ;;
        r) restart_services ;;
        h) show_help ;;
        *) echo -e "${RED}Ungültige Option.${NC}" ; sleep 2 ; show_menu ;;
    esac
}

# Umgebung prüfen
run_environment_check() {
    log "Prüfe Umgebung..."
    
    # Prüfskript ausführen
    bash "$SCRIPT_DIR/01_check_environment.sh" || {
        echo -e "${RED}Umgebungsprüfung fehlgeschlagen. Bitte beheben Sie die Probleme.${NC}"
        read -p "Drücken Sie eine Taste, um fortzufahren..."
        show_menu
        return
    }
    
    echo -e "${GREEN}Umgebungsprüfung erfolgreich!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# API-Dienst installieren
install_api_service() {
    log "Installiere API-Dienst..."
    
    bash "$SCRIPT_DIR/02_install_api_service.sh" || {
        echo -e "${RED}API-Dienst-Installation fehlgeschlagen.${NC}"
        read -p "Drücken Sie eine Taste, um fortzufahren..."
        show_menu
        return
    }
    
    echo -e "${GREEN}API-Dienst erfolgreich installiert!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Nextcloud-App installieren
install_nextcloud_app() {
    log "Installiere Nextcloud-App..."
    
    bash "$SCRIPT_DIR/03_install_nextcloud_app.sh" || {
        echo -e "${RED}Nextcloud-App-Installation fehlgeschlagen.${NC}"
        read -p "Drücken Sie eine Taste, um fortzufahren..."
        show_menu
        return
    }
    
    echo -e "${GREEN}Nextcloud-App erfolgreich installiert!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Verbindung konfigurieren
configure_connection() {
    log "Konfiguriere Verbindung..."
    
    bash "$SCRIPT_DIR/04_configure_connection.sh" || {
        echo -e "${RED}Verbindungskonfiguration fehlgeschlagen.${NC}"
        read -p "Drücken Sie eine Taste, um fortzufahren..."
        show_menu
        return
    }
    
    echo -e "${GREEN}Verbindung erfolgreich konfiguriert!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Daten importieren
import_data() {
    log "Importiere Daten..."
    
    bash "$SCRIPT_DIR/05_import_data.sh" || {
        echo -e "${RED}Datenimport fehlgeschlagen.${NC}"
        read -p "Drücken Sie eine Taste, um fortzufahren..."
        show_menu
        return
    }
    
    echo -e "${GREEN}Daten erfolgreich importiert!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Alles installieren
run_all() {
    log "Führe vollständige Installation durch..."
    
    # Alle Skripte nacheinander ausführen
    run_environment_check || return
    install_api_service || return
    install_nextcloud_app || return
    configure_connection || return
    import_data || return
    
    echo -e "${GREEN}SwissAirDry wurde erfolgreich installiert!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Deinstallation
uninstall() {
    log "Deinstalliere SwissAirDry..."
    
    echo -e "${YELLOW}Warnung: Dies wird alle SwissAirDry-Komponenten entfernen.${NC}"
    read -p "Sind Sie sicher? (j/n): " confirm
    
    if [[ $confirm == "j" || $confirm == "J" ]]; then
        # Deinstallationsskript ausführen, falls vorhanden
        if [ -f "$SCRIPT_DIR/uninstall.sh" ]; then
            bash "$SCRIPT_DIR/uninstall.sh" || {
                echo -e "${RED}Deinstallation fehlgeschlagen.${NC}"
                read -p "Drücken Sie eine Taste, um fortzufahren..."
                show_menu
                return
            }
        else
            echo -e "${RED}Deinstallationsskript nicht gefunden.${NC}"
            read -p "Drücken Sie eine Taste, um fortzufahren..."
            show_menu
            return
        fi
        
        echo -e "${GREEN}SwissAirDry wurde erfolgreich deinstalliert!${NC}"
    else
        echo -e "Deinstallation abgebrochen."
    fi
    
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Dienste neu starten
restart_services() {
    log "Starte Dienste neu..."
    
    # Neustart-Skript ausführen, falls vorhanden
    if [ -f "$SCRIPT_DIR/restart_services.sh" ]; then
        bash "$SCRIPT_DIR/restart_services.sh" || {
            echo -e "${RED}Neustart fehlgeschlagen.${NC}"
            read -p "Drücken Sie eine Taste, um fortzufahren..."
            show_menu
            return
        }
    else
        echo -e "${RED}Neustart-Skript nicht gefunden.${NC}"
        read -p "Drücken Sie eine Taste, um fortzufahren..."
        show_menu
        return
    fi
    
    echo -e "${GREEN}Dienste wurden erfolgreich neu gestartet!${NC}"
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    show_menu
}

# Hilfe anzeigen
show_help() {
    clear
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}             SwissAirDry Installation            ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    echo -e "Dieses Skript führt Sie durch die modulare Installation der SwissAirDry-Komponenten."
    echo -e "Die Installation ist in einzelne Schritte unterteilt, die Sie separat oder zusammen ausführen können.\n"
    
    echo -e "${YELLOW}Installationsschritte im Detail:${NC}"
    echo -e "1. ${BLUE}Umgebung prüfen${NC}"
    echo -e "   Prüft, ob alle Voraussetzungen für die Installation erfüllt sind,\n   einschließlich Docker, Nextcloud und Cloud-Py-Api.\n"
    
    echo -e "2. ${BLUE}API-Dienst installieren${NC}"
    echo -e "   Richtet den SwissAirDry API-Server als Docker-Container ein,\n   der die Hauptlogik und Datenbankverbindung bereitstellt.\n"
    
    echo -e "3. ${BLUE}Nextcloud-App installieren${NC}"
    echo -e "   Installiert die SwissAirDry-App in Ihrer Nextcloud-Instanz,\n   die als Benutzeroberfläche und Dashboard dient.\n"
    
    echo -e "4. ${BLUE}Verbindung konfigurieren${NC}"
    echo -e "   Richtet die Kommunikation zwischen Nextcloud und dem API-Dienst\n   über Cloud-Py-Api ein und konfiguriert die Authentifizierung.\n"
    
    echo -e "5. ${BLUE}Daten importieren${NC}"
    echo -e "   Importiert optional vorhandene Daten aus CSV-Dateien\n   in die SwissAirDry-Datenbank.\n"
    
    echo -e "${YELLOW}Weitere Informationen:${NC}"
    echo -e "  - Die vollständige Dokumentation finden Sie im Verzeichnis 'docs'."
    echo -e "  - Konfigurationsdateien werden im Verzeichnis 'config' gespeichert."
    echo -e "  - Protokolle der Installation werden in 'installation.log' geschrieben.\n"
    
    read -p "Drücken Sie eine Taste, um zum Menü zurückzukehren..."
    show_menu
}

# Hauptfunktion
main() {
    # Prüfen, ob die Skripte existieren
    for script in "01_check_environment.sh" "02_install_api_service.sh" "03_install_nextcloud_app.sh" "04_configure_connection.sh" "05_import_data.sh"; do
        if [ ! -f "$SCRIPT_DIR/$script" ]; then
            error "Skript '$script' nicht gefunden. Bitte stellen Sie sicher, dass alle Installationsdateien vorhanden sind."
        fi
    done
    
    # Willkommensnachricht
    clear
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}      Willkommen zur SwissAirDry Installation     ${NC}"
    echo -e "${BLUE}==================================================${NC}\n"
    
    echo -e "Dieses Skript führt Sie durch die Installation des SwissAirDry-Systems."
    echo -e "Die Installation ist in Module aufgeteilt, die in Ihre bestehende Nextcloud-Umgebung integriert werden.\n"
    
    echo -e "${YELLOW}Hinweis:${NC} Stellen Sie sicher, dass Sie bereits eine funktionierende Nextcloud-Installation haben"
    echo -e "und dass die Cloud-Py-Api App aus dem Nextcloud App Store installiert ist.\n"
    
    read -p "Drücken Sie eine Taste, um fortzufahren..."
    
    # Menü anzeigen
    show_menu
}

# Skript starten
main