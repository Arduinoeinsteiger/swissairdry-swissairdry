"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import uvicorn
from main import app
from config import API_PORT, DEBUG

if __name__ == "__main__":
    print("SwissAirDry API Server startet...")
    uvicorn.run(
        "main:app", 
        host="0.0.0.0", 
        port=API_PORT, 
        reload=DEBUG
    )