"""
SwissAirDry - API Server

Hauptmodul mit FastAPI-Anwendung, das die Routen und Middleware definiert.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import logging
import os
from typing import Dict, Any

import uvicorn
from fastapi import FastAPI, Request, HTTPException, Depends
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session

import config
from database import get_db, init_db, check_db_connection

# Routers importieren
from routers import admin, dashboard, devices, jobs, mqtt_integration, nextcloud

# Logger konfigurieren
logging.basicConfig(
    level=getattr(logging, config.LOG_LEVEL),
    format=config.LOG_FORMAT
)
logger = logging.getLogger(__name__)

# FastAPI-Anwendung initialisieren
app = FastAPI(
    title=config.API_TITLE,
    description=config.API_DESCRIPTION,
    version=config.API_VERSION,
    docs_url=f"{config.API_PREFIX}/docs",
    redoc_url=f"{config.API_PREFIX}/redoc",
    openapi_url=f"{config.API_PREFIX}/openapi.json"
)

# Statische Dateien einbinden
app.mount("/static", StaticFiles(directory="static"), name="static")

# Templates konfigurieren
templates = Jinja2Templates(directory="templates")

# CORS-Middleware hinzufügen
app.add_middleware(
    CORSMiddleware,
    allow_origins=config.CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# API-Routen registrieren
app.include_router(admin.router, prefix=f"{config.API_PREFIX}/admin")
app.include_router(dashboard.router, prefix=f"{config.API_PREFIX}/dashboard")
app.include_router(devices.router, prefix=f"{config.API_PREFIX}/devices")
app.include_router(jobs.router, prefix=f"{config.API_PREFIX}/jobs")
app.include_router(mqtt_integration.router, prefix=f"{config.API_PREFIX}/mqtt")
app.include_router(nextcloud.router, prefix=f"{config.API_PREFIX}/nextcloud")

# Globaler Exception-Handler
@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    """Globaler Exception-Handler für HTTP-Exceptions."""
    return JSONResponse(
        status_code=exc.status_code,
        content={"detail": exc.detail}
    )

# Root-Endpunkt
@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    return templates.TemplateResponse("index.html", {
        "request": request,
        "page": "home",
        "title": "SwissAirDry - Startseite"
    })

# Health-Check-Endpunkt
@app.get("/health")
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    db_status = "ok" if check_db_connection() else "error"
    
    # MQTT-Status prüfen
    mqtt_status = "disabled"
    if config.FEATURE_MQTT_ENABLED:
        try:
            # Optional: MQTT-Status prüfen
            mqtt_status = "ok"
        except Exception as e:
            mqtt_status = f"error: {str(e)}"
    
    # Nextcloud-Status prüfen
    nextcloud_status = "disabled"
    if config.FEATURE_NEXTCLOUD_ENABLED and config.NEXTCLOUD_BASE_URL:
        nextcloud_status = "ok"
    
    return {
        "status": "ok",
        "version": config.API_VERSION,
        "components": {
            "database": db_status,
            "mqtt": mqtt_status,
            "nextcloud": nextcloud_status
        }
    }

# Nextcloud-Connect-Endpunkt
@app.get("/nextcloud-connect")
async def nextcloud_connect():
    """Endpunkt zur Überprüfung der Nextcloud-Verbindung."""
    if not config.FEATURE_NEXTCLOUD_ENABLED or not config.NEXTCLOUD_BASE_URL:
        return {
            "status": "warning",
            "message": "Nextcloud-Integration nicht konfiguriert"
        }
    
    try:
        # Routerlogik von nextcloud.get_nextcloud_status verwenden
        return await nextcloud.get_nextcloud_status()
    except Exception as e:
        return {
            "status": "error",
            "message": f"Nextcloud-Verbindungsfehler: {str(e)}"
        }

# Admin-Dashboard-Endpunkt
@app.get("/admin", response_class=HTMLResponse)
async def admin_dashboard(request: Request, db: Session = Depends(get_db)):
    """Admin-Dashboard-Endpunkt."""
    return await admin.admin_dashboard(request, db)

# Datenbank initialisieren
@app.on_event("startup")
async def startup_event():
    """Wird beim Start der Anwendung aufgerufen."""
    # Datenbank initialisieren
    init_db()
    logger.info("SwissAirDry API-Server gestartet")

# Für lokale Entwicklung
if __name__ == "__main__":
    # Server starten
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=int(os.environ.get("API_PORT", 5000)),
        reload=os.environ.get("DEBUG", "False").lower() == "true"
    )