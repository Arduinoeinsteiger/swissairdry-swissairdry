"""
SwissAirDry API Server - Hauptskript

Dieses Skript startet den FastAPI-Server für die SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import sys
import uvicorn

# Füge das aktuelle Verzeichnis zum Python-Pfad hinzu,
# damit Python die Module finden kann
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(current_dir))

if __name__ == "__main__":
    port = int(os.getenv("PORT", 5000))
    host = os.getenv("HOST", "0.0.0.0")
    
    print(f"SwissAirDry API Server startet...")
    
    uvicorn.run(
        "swissairdry.api.app.run:app",
        host=host,
        port=port,
        reload=True,
        log_level="info"
    )