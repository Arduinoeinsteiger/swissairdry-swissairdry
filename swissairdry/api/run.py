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
app_dir = os.path.join(current_dir, 'app')
sys.path.insert(0, current_dir)
sys.path.insert(0, app_dir)

if __name__ == "__main__":
    port = int(os.getenv("PORT", 5000))
    host = os.getenv("HOST", "0.0.0.0")
    
    print(f"SwissAirDry API Server startet...")
    
    # Starte den API-Server
    uvicorn.run(
        "app.run:app",
        host=host,
        port=port,
        reload=True,
        log_level="info"
    )