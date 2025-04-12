"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from fastapi import FastAPI, Request, Depends, HTTPException, status
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session
from dotenv import load_dotenv
from database import get_db, engine
import models
from routers import admin_ui, data_processing, devices, customers, jobs, measurements, users

# Umgebungsvariablen laden
load_dotenv()

# Datenbanktabellen erstellen
models.Base.metadata.create_all(bind=engine)

# FastAPI-Anwendung erstellen
app = FastAPI(
    title="SwissAirDry API",
    description="REST API für das SwissAirDry-System zur Bautrocknung",
    version="1.0.0"
)

# CORS-Konfiguration
origins = [
    "http://localhost",
    "http://localhost:5000",
    "http://localhost:8080",
    os.getenv("NEXTCLOUD_URL", "")
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Templates und statische Dateien einrichten
templates = Jinja2Templates(directory="templates")
app.mount("/static", StaticFiles(directory="static"), name="static")

# Router einbinden
app.include_router(admin_ui.router, prefix="/admin", tags=["Admin"])
app.include_router(data_processing.router, prefix="/api/data", tags=["Datenverarbeitung"])
app.include_router(devices.router, prefix="/api/devices", tags=["Geräte"])
app.include_router(customers.router, prefix="/api/customers", tags=["Kunden"])
app.include_router(jobs.router, prefix="/api/jobs", tags=["Aufträge"])
app.include_router(measurements.router, prefix="/api/measurements", tags=["Messungen"])
app.include_router(users.router, prefix="/api/users", tags=["Benutzer"])

@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/health")
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    return {"status": "ok", "message": "API-Server läuft", "service": "swissairdry-api"}

@app.get("/nextcloud-connect")
async def nextcloud_connect():
    """Endpunkt zur Überprüfung der Nextcloud-Verbindung."""
    nextcloud_url = os.getenv("NEXTCLOUD_URL", "")
    
    if not nextcloud_url:
        return {"status": "error", "message": "Nextcloud-URL nicht konfiguriert"}
    
    return {
        "status": "ok", 
        "message": "Nextcloud-Verbindung konfiguriert", 
        "nextcloud_url": nextcloud_url
    }

@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    """Globaler Exception-Handler für HTTP-Exceptions."""
    return JSONResponse(
        status_code=exc.status_code,
        content={"status": "error", "message": exc.detail},
    )

# Server starten
if __name__ == "__main__":
    import uvicorn
    
    # Starte den Server mit uvicorn
    port = int(os.getenv("API_PORT", "5000"))
    uvicorn.run("main:app", host="0.0.0.0", port=port, reload=True)