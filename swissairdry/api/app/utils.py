"""
SwissAirDry - Hilfsfunktionen

Enthält nützliche Hilfsfunktionen für die SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import secrets
import string
import hashlib
import logging
import requests
from typing import Dict, Any, Optional, Tuple, List, Union

# Logger konfigurieren
logger = logging.getLogger("swissairdry_api")


def generate_api_key(length: int = 32) -> str:
    """
    Generiert einen zufälligen API-Schlüssel.
    
    Args:
        length: Länge des Schlüssels in Zeichen
    
    Returns:
        str: Der generierte API-Schlüssel
    """
    alphabet = string.ascii_letters + string.digits
    api_key = ''.join(secrets.choice(alphabet) for _ in range(length))
    return api_key


def verify_api_key(api_key: str, stored_key: str) -> bool:
    """
    Überprüft, ob ein API-Schlüssel gültig ist.
    
    Args:
        api_key: Der zu überprüfende API-Schlüssel
        stored_key: Der gespeicherte API-Schlüssel zum Vergleich
    
    Returns:
        bool: True, wenn der Schlüssel gültig ist, sonst False
    """
    # Einfacher Vergleich für Beispielzwecke
    # In Produktion sollte eine zeitkonstante Vergleichsfunktion verwendet werden
    return api_key == stored_key


def hash_password(password: str) -> str:
    """
    Hasht ein Passwort sicher für die Speicherung.
    
    Args:
        password: Das zu hashende Passwort
    
    Returns:
        str: Der Passwort-Hash
    """
    # In einer realen Anwendung sollte bcrypt oder ein ähnlicher Algorithmus verwendet werden
    salt = secrets.token_hex(16)
    hash_obj = hashlib.sha256((password + salt).encode())
    return f"{salt}${hash_obj.hexdigest()}"


def verify_password(password: str, hashed_password: str) -> bool:
    """
    Überprüft, ob ein Passwort mit einem gespeicherten Hash übereinstimmt.
    
    Args:
        password: Das zu überprüfende Passwort
        hashed_password: Der gespeicherte Passwort-Hash
    
    Returns:
        bool: True, wenn das Passwort gültig ist, sonst False
    """
    if not hashed_password or '$' not in hashed_password:
        return False
    
    salt, hash_value = hashed_password.split('$')
    hash_obj = hashlib.sha256((password + salt).encode())
    return hash_obj.hexdigest() == hash_value


def calculate_energy_cost(energy_kwh: float, rate_per_kwh: float) -> float:
    """
    Berechnet die Energiekosten.
    
    Args:
        energy_kwh: Energieverbrauch in kWh
        rate_per_kwh: Preis pro kWh
    
    Returns:
        float: Berechnete Kosten
    """
    return energy_kwh * rate_per_kwh


def format_duration(seconds: int) -> str:
    """
    Formatiert eine Dauer in Sekunden in ein lesbares Format.
    
    Args:
        seconds: Anzahl der Sekunden
    
    Returns:
        str: Formatierte Dauer
    """
    if seconds < 60:
        return f"{seconds} Sekunden"
    elif seconds < 3600:
        minutes = seconds // 60
        return f"{minutes} Minuten"
    elif seconds < 86400:
        hours = seconds // 3600
        minutes = (seconds % 3600) // 60
        return f"{hours} Stunden, {minutes} Minuten"
    else:
        days = seconds // 86400
        hours = (seconds % 86400) // 3600
        return f"{days} Tage, {hours} Stunden"


def check_api_availability(url: str, timeout: int = 5) -> bool:
    """
    Prüft, ob eine API unter der angegebenen URL verfügbar ist.
    
    Args:
        url: Die zu prüfende URL
        timeout: Zeitlimit für die Anfrage in Sekunden
        
    Returns:
        bool: True, wenn die API verfügbar ist, sonst False
    """
    try:
        response = requests.get(url, timeout=timeout)
        response.raise_for_status()
        return True
    except requests.RequestException as e:
        logger.warning(f"API unter {url} nicht verfügbar: {str(e)}")
        return False


def parse_device_id_from_topic(topic: str) -> Optional[str]:
    """
    Extrahiert die Geräte-ID aus einem MQTT-Topic.
    
    Args:
        topic: Das MQTT-Topic
    
    Returns:
        Optional[str]: Die extrahierte Geräte-ID oder None, wenn keine gefunden wurde
    """
    try:
        parts = topic.split('/')
        if len(parts) >= 2 and parts[0] == "swissairdry":
            return parts[1]
        return None
    except Exception:
        return None