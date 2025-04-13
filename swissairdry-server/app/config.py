"""
SwissAirDry - Konfigurationsdatei

Diese Datei enthält globale Konfigurationseinstellungen für die SwissAirDry-Anwendung.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from typing import Dict, Any, List, Optional
from dotenv import load_dotenv

# Umgebungsvariablen aus .env-Datei laden, falls vorhanden
load_dotenv()

# API-Einstellungen
API_VERSION = "1.0.0"
API_TITLE = "SwissAirDry API"
API_DESCRIPTION = "API für die SwissAirDry-Anwendung zur Verwaltung von Trocknungsgeräten und Aufträgen"
API_PREFIX = "/api"

# Datenbank-Einstellungen (aus Umgebungsvariablen)
DATABASE_URL = os.environ.get("DATABASE_URL", "postgresql://postgres:postgres@localhost:5432/swissairdry")

# MQTT-Einstellungen
MQTT_BROKER = os.environ.get("MQTT_BROKER", "localhost")
MQTT_PORT = int(os.environ.get("MQTT_PORT", 1883))
MQTT_USERNAME = os.environ.get("MQTT_USERNAME", None)
MQTT_PASSWORD = os.environ.get("MQTT_PASSWORD", None)
MQTT_CLIENT_ID = "swissairdry-api"
MQTT_TOPICS = [
    "swissairdry/devices/#",
    "swissairdry/sensors/#",
    "swissairdry/gateways/#"
]

# Nextcloud-Einstellungen
NEXTCLOUD_ENABLED = os.environ.get("NEXTCLOUD_URL") is not None
NEXTCLOUD_BASE_URL = os.environ.get("NEXTCLOUD_URL", "")
NEXTCLOUD_USERNAME = os.environ.get("NEXTCLOUD_USERNAME", "")
NEXTCLOUD_PASSWORD = os.environ.get("NEXTCLOUD_PASSWORD", "")

# Logging-Einstellungen
LOG_LEVEL = os.environ.get("LOG_LEVEL", "INFO")
LOG_FORMAT = "%(asctime)s - %(name)s - %(levelname)s - %(message)s"

# Redis-Cache-Einstellungen (optional)
REDIS_URL = os.environ.get("REDIS_URL", None)
REDIS_ENABLED = REDIS_URL is not None

# Security-Einstellungen
JWT_SECRET = os.environ.get("JWT_SECRET", "default-dev-secret-key-change-in-production")
JWT_ALGORITHM = "HS256"
JWT_EXPIRE_MINUTES = 60 * 8  # 8 Stunden

# CORS-Einstellungen
CORS_ORIGINS = os.environ.get("CORS_ORIGINS", "*").split(",")

# Feature-Flags
FEATURE_MQTT_ENABLED = os.environ.get("FEATURE_MQTT_ENABLED", "true").lower() == "true"
FEATURE_NEXTCLOUD_ENABLED = os.environ.get("FEATURE_NEXTCLOUD_ENABLED", "true").lower() == "true"

# Pfadeinstellungen
UPLOAD_FOLDER = os.environ.get("UPLOAD_FOLDER", "/tmp/swissairdry/uploads")
TEMPLATE_FOLDER = "templates"
STATIC_FOLDER = "static"

def get_full_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten API-Pfad zurück.
    """
    base_url = os.environ.get("API_BASE_URL", "http://localhost:5000")
    if base_url.endswith("/"):
        base_url = base_url[:-1]
    if not path.startswith("/"):
        path = "/" + path
    return f"{base_url}{path}"

def get_nextcloud_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten Nextcloud-Pfad zurück.
    """
    if not NEXTCLOUD_ENABLED:
        return None
    
    base_url = NEXTCLOUD_BASE_URL
    if base_url.endswith("/"):
        base_url = base_url[:-1]
    if not path.startswith("/"):
        path = "/" + path
    return f"{base_url}{path}"