"""
SwissAirDry - Router-Initialisierung

Dieses Modul initialisiert alle Router für die SwissAirDry-API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

# Importiere alle Router, die in der API verwendet werden
from .api_status import router as api_status_router

# Liste aller Router, die in der Hauptanwendung registriert werden sollen
routers = [
    api_status_router,
    # Hier weitere Router hinzufügen
]