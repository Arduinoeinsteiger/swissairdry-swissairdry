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

# Standalone-Modus (ohne Nextcloud-Integration)
STANDALONE_MODE = os.getenv("STANDALONE_MODE", "False").lower() == "true"

# Port-Konfiguration
API_HTTP_PORT = int(os.getenv("API_HTTP_PORT", "80"))
API_HTTPS_PORT = int(os.getenv("API_HTTPS_PORT", "443"))
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_WSS_PORT = int(os.getenv("MQTT_WSS_PORT", "8083"))
ESP32_UPDATE_PORT = int(os.getenv("ESP32_UPDATE_PORT", "8070"))

# Datenbank-Konfiguration
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://swissairdry:swissairdry@postgres:5432/swissairdry")

# Domain-Konfiguration
API_DOMAIN = os.getenv("API_DOMAIN", "localhost:5000")
BASE_URL = os.getenv("BASE_URL", f"http://{API_DOMAIN}")

# Nextcloud-Konfiguration (optional, wird nur verwendet, wenn nicht im Standalone-Modus)
NEXTCLOUD_URL = os.getenv("NEXTCLOUD_URL", "https://nextcloud.vgnc.org")
NEXTCLOUD_USER = os.getenv("NEXTCLOUD_USER", "swissairdry")
NEXTCLOUD_PASSWORD = os.getenv("NEXTCLOUD_PASSWORD", "")

# Frontend-Konfiguration
FRONTEND_URL = os.getenv("FRONTEND_URL", "http://localhost:8080")

# Dateipfade
UPLOAD_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "uploads")
LOG_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "logs")

# CORS-Konfiguration
CORS_ORIGINS = [
    FRONTEND_URL,
    NEXTCLOUD_URL,
    "http://localhost",
    "http://localhost:8080",
    "*"  # Im Entwicklungsmodus erlauben wir alle Ursprünge
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
    Wird nur verwendet, wenn nicht im Standalone-Modus.
    """
    if STANDALONE_MODE:
        return None
        
    if path.startswith("/"):
        path = path[1:]
    return f"{NEXTCLOUD_URL}/{path}"