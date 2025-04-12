"""
SwissAirDry - API Server

Hauptmodul zum Starten des SwissAirDry API-Servers.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from typing import Dict, Any
import uvicorn
from fastapi import FastAPI, Request, HTTPException, Depends
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session
import requests
from dotenv import load_dotenv

from database import get_db, engine, Base
from config import API_VERSION, API_PREFIX, ADMIN_PREFIX, CORS_ORIGINS, NEXTCLOUD_URL
import models
from routers import admin_ui, data_processing, devices, customers, jobs, measurements, users

# Umgebungsvariablen laden
load_dotenv()

# App-Instanz erstellen
app = FastAPI(
    title="SwissAirDry API",
    description="API für die SwissAirDry-Anwendung",
    version=API_VERSION,
    docs_url="/docs",
    redoc_url="/redoc",
)

# Templates konfigurieren
templates = Jinja2Templates(directory="templates")

# Statische Dateien einbinden
app.mount("/static", StaticFiles(directory="static"), name="static")

# CORS-Middleware hinzufügen
app.add_middleware(
    CORSMiddleware,
    allow_origins=CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Datenbanktabellen erstellen
Base.metadata.create_all(bind=engine)

# Router einbinden
app.include_router(admin_ui.router, prefix=ADMIN_PREFIX)
app.include_router(data_processing.router, prefix=f"{API_PREFIX}/data")
app.include_router(devices.router, prefix=f"{API_PREFIX}/devices")
app.include_router(customers.router, prefix=f"{API_PREFIX}/customers")
app.include_router(jobs.router, prefix=f"{API_PREFIX}/jobs")
app.include_router(measurements.router, prefix=f"{API_PREFIX}/measurements")
app.include_router(users.router, prefix=f"{API_PREFIX}/users")

@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    """Globaler Exception-Handler für HTTP-Exceptions."""
    return JSONResponse(
        status_code=exc.status_code,
        content={"status": "error", "message": str(exc.detail)},
    )

@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    return templates.TemplateResponse(
        "index.html", {"request": request, "api_version": API_VERSION}
    )

@app.get("/health")
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    return {"status": "ok", "version": API_VERSION}

@app.get("/nextcloud-connect")
async def nextcloud_connect():
    """Endpunkt zur Überprüfung der Nextcloud-Verbindung."""
    try:
        response = requests.get(NEXTCLOUD_URL, timeout=5)
        if response.status_code == 200:
            return {"status": "ok", "nextcloud_url": NEXTCLOUD_URL}
        else:
            return {
                "status": "warning",
                "message": f"Nextcloud-Server antwortet mit Status {response.status_code}",
                "nextcloud_url": NEXTCLOUD_URL,
            }
    except requests.RequestException as e:
        return {
            "status": "error",
            "message": f"Verbindung zu Nextcloud fehlgeschlagen: {str(e)}",
            "nextcloud_url": NEXTCLOUD_URL,
        }

if __name__ == "__main__":
    print("SwissAirDry API Server startet...")
    uvicorn.run("main:app", host="0.0.0.0", port=5000, reload=True)