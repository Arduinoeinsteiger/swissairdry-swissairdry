"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import uvicorn
from config import API_PREFIX

if __name__ == "__main__":
    print("SwissAirDry API Server startet...")
    
    # Server starten
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=int(os.environ.get("API_PORT", 5000)),
        reload=os.environ.get("DEBUG", "False").lower() == "true"
    )