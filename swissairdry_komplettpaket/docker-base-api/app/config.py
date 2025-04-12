"""
SwissAirDry - Konfigurationsdatei

Diese Datei enthält globale Konfigurationseinstellungen für die SwissAirDry-Anwendung.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from dotenv import load_dotenv

# Umgebungsvariablen laden
load_dotenv()

# API-Konfiguration
API_VERSION = "1.0.0"
DEBUG = os.getenv("DEBUG", "False").lower() == "true"
SECRET_KEY = os.getenv("SECRET_KEY", "supersecretkey")
API_PORT = int(os.getenv("API_PORT", "5000"))

# Port-Konfiguration
API_HTTP_PORT = int(os.getenv("API_HTTP_PORT", "80"))
API_HTTPS_PORT = int(os.getenv("API_HTTPS_PORT", "443"))
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_WSS_PORT = int(os.getenv("MQTT_WSS_PORT", "8083"))
ESP32_UPDATE_PORT = int(os.getenv("ESP32_UPDATE_PORT", "8070"))
NEXTCLOUD_HTTP_PORT = int(os.getenv("NEXTCLOUD_HTTP_PORT", "8080"))
NEXTCLOUD_HTTPS_PORT = int(os.getenv("NEXTCLOUD_HTTPS_PORT", "8443"))

# Datenbank-Konfiguration
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://swissairdry:swissairdry@postgres:5432/swissairdry")

# Domain-Konfiguration
API_DOMAIN = os.getenv("API_DOMAIN", "api.vgnc.org")
BASE_URL = os.getenv("BASE_URL", f"https://{API_DOMAIN}")

# Nextcloud-Konfiguration
NEXTCLOUD_URL = os.getenv("NEXTCLOUD_URL", "https://nextcloud.vgnc.org")
NEXTCLOUD_USER = os.getenv("NEXTCLOUD_USER", "swissairdry")
NEXTCLOUD_PASSWORD = os.getenv("NEXTCLOUD_PASSWORD", "")

# Dateipfade
UPLOAD_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "uploads")
LOG_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "logs")

# CORS-Konfiguration
CORS_ORIGINS = [
    NEXTCLOUD_URL,
    "https://vgnc.org",
    "https://*.vgnc.org",
    "http://localhost",
    "http://localhost:8080",
]

# API-Pfade
API_PREFIX = "/api"
API_V1_STR = f"{API_PREFIX}/v1"
ADMIN_PREFIX = "/admin"

# Standardwerte für Pagination
DEFAULT_PAGE_SIZE = 20
MAX_PAGE_SIZE = 100

# Erstellen der benötigten Verzeichnisse
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(LOG_FOLDER, exist_ok=True)

def get_full_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten API-Pfad zurück.
    """
    if path.startswith("/"):
        path = path[1:]
    return f"{BASE_URL}/{path}"

def get_nextcloud_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten Nextcloud-Pfad zurück.
    """
    if path.startswith("/"):
        path = path[1:]
    return f"{NEXTCLOUD_URL}/{path}"