"""
SwissAirDry - Konfigurationsdatei

Diese Datei enthält globale Konfigurationseinstellungen für die SwissAirDry-Anwendung.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from typing import Dict, List, Optional

# Umgebungsvariablen
ENVIRONMENT = os.getenv("ENVIRONMENT", "development")  # development, testing, production

# API-Konfiguration
API_BASE_URL = {
    "development": "http://localhost:5000",
    "production": "https://api.vgnc.org"
}.get(ENVIRONMENT, "http://localhost:5000")

# Nextcloud-Konfiguration
NEXTCLOUD_BASE_URL = "https://vgnc.org"
NEXTCLOUD_APP_PATH = "/apps/swissairdry"
NEXTCLOUD_FULL_URL = f"{NEXTCLOUD_BASE_URL}{NEXTCLOUD_APP_PATH}"

# Datenbank-Konfiguration wird aus der Umgebungsvariable DATABASE_URL gelesen
DATABASE_URL = os.getenv("DATABASE_URL")

# MQTT-Broker-Konfiguration
MQTT_BROKER_HOST = os.getenv("MQTT_BROKER_HOST", "localhost")
MQTT_BROKER_PORT = int(os.getenv("MQTT_BROKER_PORT", "1883"))
MQTT_CLIENT_ID = os.getenv("MQTT_CLIENT_ID", "swissairdry_api")
MQTT_TOPIC_PREFIX = "swissairdry/"

# Security-Konfiguration
JWT_SECRET_KEY = os.getenv("JWT_SECRET_KEY", "dev_secret_key_do_not_use_in_production")
JWT_ALGORITHM = "HS256"
JWT_EXPIRATION_MINUTES = 60 * 24  # 24 Stunden

# Allowed domains for CORS
CORS_ALLOWED_ORIGINS = [
    "https://vgnc.org",
    "https://*.vgnc.org",
    "http://localhost:5000",
]

# Cloudflare-Konfiguration (falls verwendet)
CLOUDFLARE_ENABLED = os.getenv("CLOUDFLARE_ENABLED", "false").lower() == "true"
CLOUDFLARE_ZONE_ID = os.getenv("CLOUDFLARE_ZONE_ID", "")
CLOUDFLARE_API_TOKEN = os.getenv("CLOUDFLARE_API_TOKEN", "")

# Feature flags
FEATURES = {
    "bexio_integration": False,
    "live_device_monitoring": True,
    "report_generation": True,
    "barcode_scanning": True,
}

# WebSocket-Konfiguration
WEBSOCKET_PATH = "/ws"

# Logging-Konfiguration
LOG_LEVEL = os.getenv("LOG_LEVEL", "INFO").upper()

def get_full_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten API-Pfad zurück.
    """
    # Entfernt führende '/' vom Pfad, falls vorhanden
    if path.startswith('/'):
        path = path[1:]
    
    return f"{API_BASE_URL}/{path}"

def get_nextcloud_url(path: str) -> str:
    """
    Gibt die vollständige URL für einen bestimmten Nextcloud-Pfad zurück.
    """
    # Entfernt führende '/' vom Pfad, falls vorhanden
    if path.startswith('/'):
        path = path[1:]
    
    return f"{NEXTCLOUD_FULL_URL}/{path}"