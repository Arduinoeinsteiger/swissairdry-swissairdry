"""
SwissAirDry - Konfigurationsdatei

Diese Datei enthält globale Konfigurationseinstellungen für die SwissAirDry-Anwendung.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import requests
import logging
from typing import Dict, Optional, Union

# Logger-Konfiguration
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler("logs/swissairdry.log", encoding="utf-8")
    ] if os.path.exists("logs") else [logging.StreamHandler()]
)
logger = logging.getLogger("swissairdry")

# API-Server-Konfiguration
PRIMARY_API_HOST = os.environ.get("PRIMARY_API_HOST", "api.vgnc.org")
PRIMARY_API_PORT = int(os.environ.get("PRIMARY_API_PORT", "443"))
PRIMARY_API_SCHEME = os.environ.get("PRIMARY_API_SCHEME", "https")
PRIMARY_API_PREFIX = "/api/v1"

BACKUP_API_HOST = os.environ.get("BACKUP_API_HOST", "swissairdry.replit.app")
BACKUP_API_PORT = int(os.environ.get("BACKUP_API_PORT", "443"))
BACKUP_API_SCHEME = os.environ.get("BACKUP_API_SCHEME", "https")
BACKUP_API_PREFIX = "/api/v1"

# Globale Variablen für den Serverstatus
_using_backup_server = False
_primary_server_available = True
_backup_server_available = True

# Nextcloud-Konfiguration
NEXTCLOUD_URL = os.environ.get("NEXTCLOUD_URL", "https://cloud.vgnc.org")
NEXTCLOUD_APP_KEY = os.environ.get("NEXTCLOUD_APP_KEY", "")


def check_api_availability(host: str, port: int, scheme: str, prefix: str) -> bool:
    """
    Überprüft, ob der angegebene API-Server erreichbar ist.
    
    Args:
        host: Hostname des API-Servers
        port: Port des API-Servers
        scheme: HTTP-Schema (http oder https)
        prefix: API-Prefix (z.B. /api/v1)
    
    Returns:
        bool: True, wenn der Server erreichbar ist, sonst False
    """
    url = f"{scheme}://{host}"
    if port != 80 and port != 443:
        url += f":{port}"
    url += f"{prefix}/health"
    
    try:
        response = requests.get(url, timeout=5)
        return response.status_code == 200
    except requests.RequestException:
        return False


def get_active_api_server() -> Dict[str, str]:
    """
    Gibt den aktiven API-Server zurück und wechselt bei Bedarf automatisch zwischen
    dem primären und dem Backup-Server.
    
    Returns:
        Dict: Ein Dictionary mit den Verbindungsdaten des aktiven API-Servers
    """
    global _using_backup_server, _primary_server_available, _backup_server_available
    
    # Überprüfen, ob der primäre Server erreichbar ist
    primary_available = check_api_availability(
        PRIMARY_API_HOST, PRIMARY_API_PORT, PRIMARY_API_SCHEME, PRIMARY_API_PREFIX
    )
    
    # Status des primären Servers aktualisieren
    if primary_available != _primary_server_available:
        _primary_server_available = primary_available
        if primary_available:
            logger.info("Primärer Server ist wieder erreichbar: %s", PRIMARY_API_HOST)
        else:
            logger.warning("Primärer Server ist nicht erreichbar: %s", PRIMARY_API_HOST)
    
    # Wenn wir den Backup-Server verwenden und der primäre Server wieder verfügbar ist,
    # wechseln wir zurück zum primären Server
    if _using_backup_server and primary_available:
        _using_backup_server = False
        logger.info("Wechsel zurück zum primären Server: %s", PRIMARY_API_HOST)
    
    # Wenn wir den primären Server verwenden und er nicht verfügbar ist,
    # prüfen wir, ob der Backup-Server verfügbar ist
    if not _using_backup_server and not primary_available:
        backup_available = check_api_availability(
            BACKUP_API_HOST, BACKUP_API_PORT, BACKUP_API_SCHEME, BACKUP_API_PREFIX
        )
        
        # Status des Backup-Servers aktualisieren
        if backup_available != _backup_server_available:
            _backup_server_available = backup_available
            if backup_available:
                logger.info("Backup-Server ist erreichbar: %s", BACKUP_API_HOST)
            else:
                logger.warning("Backup-Server ist nicht erreichbar: %s", BACKUP_API_HOST)
        
        # Wenn der Backup-Server verfügbar ist, wechseln wir zu diesem
        if backup_available:
            _using_backup_server = True
            logger.info("Verwende Backup-Server: %s", BACKUP_API_HOST)
    
    # Rückgabe der Verbindungsdaten des aktiven Servers
    if _using_backup_server:
        return {
            "host": BACKUP_API_HOST,
            "port": BACKUP_API_PORT,
            "scheme": BACKUP_API_SCHEME,
            "prefix": BACKUP_API_PREFIX
        }
    else:
        return {
            "host": PRIMARY_API_HOST,
            "port": PRIMARY_API_PORT,
            "scheme": PRIMARY_API_SCHEME,
            "prefix": PRIMARY_API_PREFIX
        }


def get_full_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten API-Pfad zurück.
    
    Args:
        path: Der API-Pfad, z.B. /devices
    
    Returns:
        str: Die vollständige URL zum API-Endpunkt
    """
    server = get_active_api_server()
    url = f"{server['scheme']}://{server['host']}"
    if server['port'] != 80 and server['port'] != 443:
        url += f":{server['port']}"
    url += f"{server['prefix']}{path}"
    return url


def get_nextcloud_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten Nextcloud-Pfad zurück.
    
    Args:
        path: Der Nextcloud-Pfad, z.B. /index.php/apps/swissairdry
    
    Returns:
        str: Die vollständige URL zum Nextcloud-Endpunkt
    """
    return f"{NEXTCLOUD_URL}{path}"


def is_using_backup_server() -> bool:
    """
    Gibt zurück, ob aktuell der Backup-Server verwendet wird.
    
    Returns:
        bool: True, wenn der Backup-Server verwendet wird, sonst False
    """
    return _using_backup_server


def get_api_status() -> Dict[str, str]:
    """
    Gibt den Status der API-Server zurück.
    
    Returns:
        Dict: Ein Dictionary mit dem Status der API-Server
    """
    return {
        "primary_server": "online" if _primary_server_available else "offline",
        "backup_server": "online" if _backup_server_available else "offline",
        "active_server": BACKUP_API_HOST if _using_backup_server else PRIMARY_API_HOST,
        "using_backup": _using_backup_server
    }


def switch_to_backup_server() -> None:
    """
    Wechselt manuell zum Backup-Server.
    """
    global _using_backup_server
    if not _using_backup_server:
        _using_backup_server = True
        logger.info("Manueller Wechsel zum Backup-Server: %s", BACKUP_API_HOST)


def switch_to_primary_server() -> None:
    """
    Wechselt manuell zum primären Server.
    """
    global _using_backup_server
    if _using_backup_server:
        _using_backup_server = False
        logger.info("Manueller Wechsel zum primären Server: %s", PRIMARY_API_HOST)