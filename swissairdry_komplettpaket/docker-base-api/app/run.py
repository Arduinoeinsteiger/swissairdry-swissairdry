"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import uvicorn
import os
from main import app

if __name__ == "__main__":
    print("SwissAirDry API Server startet...")
    port = int(os.environ.get("API_PORT", "5000"))
    uvicorn.run(app, host="0.0.0.0", port=port)