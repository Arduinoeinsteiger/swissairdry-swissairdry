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

# API-Domains und URLs
API_DOMAIN = os.getenv("API_DOMAIN", "localhost:5000")
BASE_URL = os.getenv("BASE_URL", f"http://{API_DOMAIN}")

# Datenbank-Konfiguration
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://swissairdry:swissairdry@postgres:5432/swissairdry")

# Nextcloud-Konfiguration
NEXTCLOUD_URL = os.getenv("NEXTCLOUD_URL", "")
NEXTCLOUD_USER = os.getenv("NEXTCLOUD_USER", "")
NEXTCLOUD_PASSWORD = os.getenv("NEXTCLOUD_PASSWORD", "")

# Dateipfade
UPLOAD_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "uploads")
LOG_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "logs")

# MQTT-Konfiguration
MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USERNAME = os.getenv("MQTT_USERNAME", "")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "")

# CORS-Konfiguration
CORS_ORIGINS = [
    "http://localhost",
    "http://localhost:8080",
    NEXTCLOUD_URL,
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
    """
    if not NEXTCLOUD_URL:
        return ""
        
    if path.startswith("/"):
        path = path[1:]
    return f"{NEXTCLOUD_URL}/{path}"