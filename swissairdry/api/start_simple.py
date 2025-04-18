"""
SwissAirDry API - Starter Skript für die einfache API

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import uvicorn

if __name__ == "__main__":
    port = int(os.getenv("PORT", 5000))
    host = os.getenv("HOST", "0.0.0.0")
    
    print(f"SwissAirDry Simple API Server startet...")
    
    # Starte den API-Server
    uvicorn.run(
        "simple_app:app",
        host=host,
        port=port,
        reload=True,
        log_level="info"
    )