#!/usr/bin/env python3
"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import uvicorn
import asyncio
import logging
from fastapi import FastAPI, Request, Depends
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from dotenv import load_dotenv

# Import der Router
try:
    from routers import routers
except ImportError:
    routers = []

# Import der Konfiguration
try:
    import config
except ImportError:
    config = None

# Umgebungsvariablen laden
load_dotenv()

# Logger einrichten
logger = logging.getLogger("swissairdry")

# FastAPI-Anwendung erstellen
app = FastAPI(
    title="SwissAirDry API",
    description="REST API für das SwissAirDry-System zur Bautrocknung",
    version="1.0.0"
)

# CORS-Middleware hinzufügen
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In Produktion einschränken
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Verzeichnisse für Templates und statische Dateien
templates_dir = os.path.join(os.path.dirname(__file__), "templates")
static_dir = os.path.join(os.path.dirname(__file__), "static")

# Existenz der Verzeichnisse prüfen und erstellen, falls nötig
os.makedirs(templates_dir, exist_ok=True)
os.makedirs(static_dir, exist_ok=True)

# Templates und statische Dateien einrichten
try:
    templates = Jinja2Templates(directory=templates_dir)
    app.mount("/static", StaticFiles(directory=static_dir), name="static")
except Exception as e:
    logger.warning(f"Konnte Templates oder statische Dateien nicht einrichten: {e}")

# Router registrieren
for router in routers:
    app.include_router(router)

# Hintergrundaufgabe für API Server Health Checks
async def check_primary_server_availability():
    """
    Hintergrundaufgabe, die regelmäßig prüft, ob der primäre API-Server 
    verfügbar ist, und bei Bedarf automatisch umschaltet.
    """
    if not config:
        logger.warning("Konfiguration nicht verfügbar, API Server Health Checks deaktiviert")
        return
        
    while True:
        try:
            # Aktuellen API-Server prüfen und ggf. umschalten
            config.get_active_api_server()
            # Status protokollieren
            status = config.get_api_status()
            if config.is_using_backup_server():
                logger.info(f"Verwende Backup-Server: {config.BACKUP_API_HOST}")
            await asyncio.sleep(60)  # Alle 60 Sekunden prüfen
        except Exception as e:
            logger.error(f"Fehler bei der Überprüfung der API-Server: {e}")
            await asyncio.sleep(60)  # Bei Fehler trotzdem warten

@app.on_event("startup")
async def startup_event():
    """Wird beim Start der Anwendung aufgerufen."""
    # Hintergrundaufgabe für API Server Health Checks starten
    if config:
        asyncio.create_task(check_primary_server_availability())
        logger.info("API Server Health Checks gestartet")

@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    return """
    <html>
        <head>
            <title>SwissAirDry API</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 0;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    background-color: #f5f5f5;
                }
                .container {
                    text-align: center;
                    padding: 2rem;
                    background-color: white;
                    border-radius: 10px;
                    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    max-width: 600px;
                }
                h1 {
                    color: #2c3e50;
                }
                p {
                    color: #7f8c8d;
                    margin-bottom: 1.5rem;
                }
                .links {
                    margin-top: 2rem;
                }
                .links a {
                    display: inline-block;
                    margin: 0 10px;
                    color: #3498db;
                    text-decoration: none;
                }
                .links a:hover {
                    text-decoration: underline;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>SwissAirDry API Server</h1>
                <p>Der API-Server ist aktiv und bereit für Anfragen.</p>
                <div class="links">
                    <a href="/docs">API-Dokumentation</a>
                    <a href="/admin">Admin-Bereich</a>
                </div>
            </div>
        </body>
    </html>
    """

@app.get("/health")
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    return {"status": "ok", "message": "API-Server läuft"}

@app.get("/admin")
async def admin_placeholder():
    """Platzhalter für den Admin-Bereich."""
    return HTMLResponse("""
    <html>
        <head>
            <title>SwissAirDry Admin</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 0;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    background-color: #f5f5f5;
                }
                .container {
                    text-align: center;
                    padding: 2rem;
                    background-color: white;
                    border-radius: 10px;
                    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    max-width: 600px;
                }
                h1 {
                    color: #2c3e50;
                }
                p {
                    color: #7f8c8d;
                    margin-bottom: 1.5rem;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>SwissAirDry Admin-Bereich</h1>
                <p>Dies ist ein Platzhalter für den Admin-Bereich. In einer vollständigen Installation wäre hier das Admin-Dashboard verfügbar.</p>
            </div>
        </body>
    </html>
    """)

# Server starten
if __name__ == "__main__":
    print("SwissAirDry API Server startet...")
    
    # Konfiguration aus Umgebungsvariablen laden
    host = os.getenv("API_HOST", "0.0.0.0")
    port = int(os.getenv("API_PORT", 5000))
    debug = os.getenv("DEBUG", "False").lower() == "true"
    
    # Server starten
    uvicorn.run("run:app", host=host, port=port, reload=debug)

