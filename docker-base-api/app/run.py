#!/usr/bin/env python3
"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import uvicorn
from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from dotenv import load_dotenv

# Umgebungsvariablen laden
load_dotenv()

# FastAPI-Anwendung erstellen
app = FastAPI(
    title="SwissAirDry API",
    description="REST API für das SwissAirDry-System zur Bautrocknung",
    version="1.0.0"
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
    print(f"Warnung: Konnte Templates oder statische Dateien nicht einrichten: {e}")

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

