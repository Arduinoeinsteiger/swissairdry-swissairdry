"""
SwissAirDry - Konfigurationsdatei

Diese Datei enthält globale Konfigurationseinstellungen für die SwissAirDry-Anwendung.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import logging
import requests
from typing import List, Dict, Optional
from dotenv import load_dotenv

# Lade Umgebungsvariablen aus .env Datei, falls vorhanden
load_dotenv()

# Allgemeine Einstellungen
APP_NAME = "SwissAirDry"
APP_VERSION = "1.0.0"
APP_ENVIRONMENT = os.getenv("APP_ENVIRONMENT", "development")
APP_SECRET_KEY = os.getenv("APP_SECRET_KEY", "development_secret_key")

# Haupt-API-Server
PRIMARY_API_HOST = os.getenv("PRIMARY_API_HOST", "api.vgnc.org")
PRIMARY_API_PORT = int(os.getenv("PRIMARY_API_PORT", "443"))
PRIMARY_API_SCHEME = os.getenv("PRIMARY_API_SCHEME", "https")
PRIMARY_API_PREFIX = os.getenv("PRIMARY_API_PREFIX", "/api/v1")

# Backup-API-Server (Replit)
BACKUP_API_HOST = os.getenv("BACKUP_API_HOST", "swissairdry.replit.app")
BACKUP_API_PORT = int(os.getenv("BACKUP_API_PORT", "443"))
BACKUP_API_SCHEME = os.getenv("BACKUP_API_SCHEME", "https")
BACKUP_API_PREFIX = os.getenv("BACKUP_API_PREFIX", "/api/v1")

# Timeout für API-Health-Checks (in Sekunden)
API_HEALTH_CHECK_TIMEOUT = int(os.getenv("API_HEALTH_CHECK_TIMEOUT", "3"))

# Nextcloud-Einstellungen
NEXTCLOUD_URL = os.getenv("NEXTCLOUD_URL", "https://cloud.vgnc.org")
NEXTCLOUD_APP_KEY = os.getenv("NEXTCLOUD_APP_KEY", "")
NEXTCLOUD_TIMEOUT = int(os.getenv("NEXTCLOUD_TIMEOUT", "5"))

# JWT-Einstellungen
JWT_SECRET_KEY = os.getenv("JWT_SECRET_KEY", APP_SECRET_KEY)
JWT_ALGORITHM = os.getenv("JWT_ALGORITHM", "HS256")
JWT_ACCESS_TOKEN_EXPIRE_MINUTES = int(os.getenv("JWT_ACCESS_TOKEN_EXPIRE_MINUTES", "30"))

# Datenbank-Einstellungen
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://swissairdry:swissairdry@db:5432/swissairdry")

# MQTT-Einstellungen
MQTT_BROKER_HOST = os.getenv("MQTT_BROKER_HOST", "mosquitto")
MQTT_BROKER_PORT = int(os.getenv("MQTT_BROKER_PORT", "1883"))
MQTT_CLIENT_ID = os.getenv("MQTT_CLIENT_ID", f"swissairdry-api-{os.getpid()}")
MQTT_USERNAME = os.getenv("MQTT_USERNAME", "")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "")
MQTT_TOPIC_PREFIX = os.getenv("MQTT_TOPIC_PREFIX", "swissairdry")

# Logging-Einstellungen
LOG_LEVEL = os.getenv("LOG_LEVEL", "INFO")
LOG_FORMAT = os.getenv("LOG_FORMAT", "%(asctime)s - %(name)s - %(levelname)s - %(message)s")

# Cache für den aktiven API-Server
_active_api_host = PRIMARY_API_HOST
_active_api_port = PRIMARY_API_PORT
_active_api_scheme = PRIMARY_API_SCHEME
_active_api_prefix = PRIMARY_API_PREFIX
_is_using_backup = False

# Initialisiere Logger
logger = logging.getLogger("swissairdry")
logging.basicConfig(level=getattr(logging, LOG_LEVEL), format=LOG_FORMAT)


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
    url = f"{scheme}://{host}:{port}{prefix}/health"
    try:
        response = requests.get(url, timeout=API_HEALTH_CHECK_TIMEOUT)
        return response.status_code == 200
    except (requests.RequestException, ConnectionError):
        return False


def get_active_api_server() -> Dict[str, str]:
    """
    Gibt den aktiven API-Server zurück und wechselt bei Bedarf automatisch zwischen
    dem primären und dem Backup-Server.
    
    Returns:
        Dict: Ein Dictionary mit den Verbindungsdaten des aktiven API-Servers
    """
    global _active_api_host, _active_api_port, _active_api_scheme, _active_api_prefix, _is_using_backup
    
    # Versuche den primären Server, wenn wir aktuell den Backup-Server verwenden
    if _is_using_backup:
        if check_api_availability(PRIMARY_API_HOST, PRIMARY_API_PORT, PRIMARY_API_SCHEME, PRIMARY_API_PREFIX):
            logger.info("Primärer API-Server ist wieder erreichbar. Wechsel zurück zum Primärserver.")
            _active_api_host = PRIMARY_API_HOST
            _active_api_port = PRIMARY_API_PORT
            _active_api_scheme = PRIMARY_API_SCHEME
            _active_api_prefix = PRIMARY_API_PREFIX
            _is_using_backup = False
    
    # Versuche den Backup-Server, wenn der primäre Server nicht erreichbar ist
    else:
        if not check_api_availability(_active_api_host, _active_api_port, _active_api_scheme, _active_api_prefix):
            logger.warning("Primärer API-Server nicht erreichbar. Wechsel zum Backup-Server.")
            _active_api_host = BACKUP_API_HOST
            _active_api_port = BACKUP_API_PORT
            _active_api_scheme = BACKUP_API_SCHEME
            _active_api_prefix = BACKUP_API_PREFIX
            _is_using_backup = True
    
    return {
        "host": _active_api_host,
        "port": _active_api_port,
        "scheme": _active_api_scheme,
        "prefix": _active_api_prefix
    }


def get_full_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten API-Pfad zurück.
    
    Args:
        path: Der API-Pfad, z.B. /devices
    
    Returns:
        str: Die vollständige URL zum API-Endpunkt
    """
    api_server = get_active_api_server()
    path = path.lstrip("/")  # Entferne führende Slashes
    prefix = api_server["prefix"].rstrip("/")  # Entferne abschließende Slashes
    
    return f"{api_server['scheme']}://{api_server['host']}:{api_server['port']}{prefix}/{path}"


def get_nextcloud_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten Nextcloud-Pfad zurück.
    
    Args:
        path: Der Nextcloud-Pfad, z.B. /index.php/apps/swissairdry
    
    Returns:
        str: Die vollständige URL zum Nextcloud-Endpunkt
    """
    path = path.lstrip("/")  # Entferne führende Slashes
    return f"{NEXTCLOUD_URL}/{path}"


def is_using_backup_server() -> bool:
    """
    Gibt zurück, ob aktuell der Backup-Server verwendet wird.
    
    Returns:
        bool: True, wenn der Backup-Server verwendet wird, sonst False
    """
    return _is_using_backup


def get_api_status() -> Dict[str, str]:
    """
    Gibt den Status der API-Server zurück.
    
    Returns:
        Dict: Ein Dictionary mit dem Status der API-Server
    """
    primary_available = check_api_availability(
        PRIMARY_API_HOST, PRIMARY_API_PORT, PRIMARY_API_SCHEME, PRIMARY_API_PREFIX
    )
    
    backup_available = check_api_availability(
        BACKUP_API_HOST, BACKUP_API_PORT, BACKUP_API_SCHEME, BACKUP_API_PREFIX
    )
    
    return {
        "primary_server": "online" if primary_available else "offline",
        "backup_server": "online" if backup_available else "offline",
        "active_server": "backup" if _is_using_backup else "primary"
    }