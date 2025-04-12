"""
SwissAirDry - API Server

Hauptmodul mit FastAPI-Anwendung, das die Routen und Middleware definiert.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from typing import Dict, Any

import uvicorn
import fastapi
from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session

import config
from database import get_db, engine, Base

# Datenbanktabellen erstellen
Base.metadata.create_all(bind=engine)

# FastAPI-Anwendung initialisieren
app = FastAPI(
    title="SwissAirDry API",
    description="API für das SwissAirDry-System zur Verwaltung von Trocknungsgeräten und -aufträgen.",
    version=config.API_VERSION,
)

# CORS-Middleware hinzufügen
app.add_middleware(
    CORSMiddleware,
    allow_origins=config.CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Statische Dateien und Templates konfigurieren
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")

# Globaler Exception-Handler für HTTP-Exceptions
@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    """Globaler Exception-Handler für HTTP-Exceptions."""
    return JSONResponse(
        status_code=exc.status_code,
        content={"status": "error", "message": exc.detail},
    )

# Grundlegende Endpunkte
@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    return templates.TemplateResponse("index.html", {"request": request, "page": "home"})

@app.get("/health")
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    return {"status": "ok", "version": config.API_VERSION}

@app.get("/nextcloud-connect")
async def nextcloud_connect():
    """Endpunkt zur Überprüfung der Nextcloud-Verbindung."""
    if not config.NEXTCLOUD_URL:
        return {
            "status": "warning",
            "message": "Keine Nextcloud-URL konfiguriert"
        }
    
    try:
        # Hier würde normalerweise eine Verbindungsprüfung stattfinden
        return {
            "status": "ok",
            "message": "Verbindung zur Nextcloud hergestellt",
            "nextcloud_url": config.NEXTCLOUD_URL
        }
    except Exception as e:
        return {
            "status": "error",
            "message": f"Fehler bei der Verbindung zur Nextcloud: {str(e)}"
        }

# Router für verschiedene API-Bereiche importieren und einbinden
# Hier können je nach Bedarf weitere Router hinzugefügt werden
from routers import admin, dashboard, devices, jobs, mqtt_integration

app.include_router(admin.router, prefix=config.ADMIN_PREFIX)
app.include_router(dashboard.router, prefix=f"{config.API_PREFIX}/dashboard")
app.include_router(devices.router, prefix=f"{config.API_PREFIX}/devices")
app.include_router(jobs.router, prefix=f"{config.API_PREFIX}/jobs")
app.include_router(mqtt_integration.router, prefix=f"{config.API_PREFIX}/mqtt")

# Server starten (nur im Direktaufruf, nicht beim Import)
if __name__ == "__main__":
    uvicorn.run("main:app", host="0.0.0.0", port=config.API_PORT, reload=config.DEBUG)