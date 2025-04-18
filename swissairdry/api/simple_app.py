"""
SwissAirDry API - Einfache Version

Eine vereinfachte Version der SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from datetime import datetime
from typing import Dict, Any, List, Optional

from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

# FastAPI-App erstellen
app = FastAPI(
    title="SwissAirDry API",
    description="API für die Verwaltung von SwissAirDry-Geräten und -Daten",
    version="1.0.0",
)

# CORS Middleware hinzufügen
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In Produktion einschränken
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Templates und statische Dateien einrichten
current_dir = os.path.dirname(os.path.abspath(__file__))
templates_dir = os.path.join(current_dir, "app", "templates")
static_dir = os.path.join(current_dir, "app", "static")

if os.path.exists(templates_dir):
    templates = Jinja2Templates(directory=templates_dir)
    app.mount("/static", StaticFiles(directory=static_dir), name="static")
else:
    # Fallback, wenn die Verzeichnisse nicht existieren
    templates = None
    print(f"Warnung: Verzeichnis {templates_dir} nicht gefunden")

# Simulierte Daten für die API
devices = [
    {
        "id": 1,
        "device_id": "device001",
        "name": "Luftentfeuchter 1",
        "type": "standard",
        "status": "online",
        "last_seen": datetime.now().isoformat(),
        "created_at": datetime.now().isoformat(),
    },
    {
        "id": 2,
        "device_id": "device002",
        "name": "Luftentfeuchter 2",
        "type": "premium",
        "status": "offline",
        "last_seen": datetime.now().isoformat(),
        "created_at": datetime.now().isoformat(),
    }
]

# Sensordaten für die Geräte
sensor_data = {
    "device001": [
        {
            "timestamp": datetime.now().isoformat(),
            "temperature": 22.5,
            "humidity": 65.8,
            "power": 450.0,
            "energy": 12.5,
            "relay_state": True,
            "runtime": 3600
        }
    ],
    "device002": [
        {
            "timestamp": datetime.now().isoformat(),
            "temperature": 21.0,
            "humidity": 70.2,
            "power": 0.0,
            "energy": 8.3,
            "relay_state": False,
            "runtime": 7200
        }
    ]
}

# Status-Variablen
server_start_time = datetime.now()
api_stats = {
    "request_count": 0,
    "error_count": 0,
    "last_request": None,
}


@app.middleware("http")
async def log_requests(request: Request, call_next):
    """Middleware zum Loggen aller Anfragen"""
    start_time = datetime.now()
    
    # Request-Statistik aktualisieren
    api_stats["request_count"] += 1
    api_stats["last_request"] = datetime.now()
    
    response = await call_next(request)
    
    # Bei Fehler die Fehlerstatistik erhöhen
    if response.status_code >= 400:
        api_stats["error_count"] += 1
    
    process_time = (datetime.now() - start_time).total_seconds()
    response.headers["X-Process-Time"] = str(process_time)
    
    print(
        f"{request.client.host}:{request.client.port} - "
        f"{request.method} {request.url.path} - "
        f"{response.status_code} - {process_time:.4f}s"
    )
    
    return response


@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    """Root-Endpoint, liefert eine einfache HTML-Seite zurück."""
    if templates:
        return templates.TemplateResponse("index.html", {"request": request})
    else:
        return HTMLResponse(content="""
        <html>
            <head>
                <title>SwissAirDry API</title>
                <style>
                    body { font-family: Arial, sans-serif; margin: 40px; }
                    h1 { color: #0066cc; }
                </style>
            </head>
            <body>
                <h1>SwissAirDry API</h1>
                <p>Willkommen bei der SwissAirDry API.</p>
                <p>
                    Verfügbare Endpoints:
                    <ul>
                        <li><a href="/docs">/docs</a> - API-Dokumentation</li>
                        <li><a href="/health">/health</a> - API-Status</li>
                        <li><a href="/api/devices">/api/devices</a> - Liste aller Geräte</li>
                    </ul>
                </p>
            </body>
        </html>
        """)


@app.get("/health", response_model=Dict[str, Any])
async def health_check():
    """Health-Check-Endpunkt für Monitoring."""
    return {
        "status": "ok",
        "version": "1.0.0",
        "uptime": (datetime.now() - server_start_time).total_seconds(),
        "stats": api_stats,
    }


@app.get("/api/devices", response_model=List[Dict[str, Any]])
async def get_devices():
    """Gibt eine Liste aller Geräte zurück."""
    return devices


@app.get("/api/devices/{device_id}", response_model=Dict[str, Any])
async def get_device(device_id: str):
    """Gibt ein Gerät anhand seiner ID zurück."""
    for device in devices:
        if device["device_id"] == device_id:
            return device
    return JSONResponse(status_code=404, content={"detail": "Gerät nicht gefunden"})


@app.get("/api/device/{device_id}/data", response_model=List[Dict[str, Any]])
async def get_sensor_data(device_id: str):
    """Gibt die Sensordaten eines Geräts zurück."""
    if device_id in sensor_data:
        return sensor_data[device_id]
    return JSONResponse(status_code=404, content={"detail": "Keine Daten für dieses Gerät gefunden"})


class SensorDataCreate(BaseModel):
    """Schema für Sensordaten"""
    temperature: Optional[float] = None
    humidity: Optional[float] = None
    power: Optional[float] = None
    energy: Optional[float] = None
    relay_state: Optional[bool] = None
    runtime: Optional[int] = None


@app.post("/api/device/{device_id}/data")
async def create_sensor_data(device_id: str, data: SensorDataCreate):
    """Speichert neue Sensordaten für ein Gerät."""
    # Prüfen, ob das Gerät existiert
    device_exists = False
    for device in devices:
        if device["device_id"] == device_id:
            device_exists = True
            device["status"] = "online"
            device["last_seen"] = datetime.now().isoformat()
            break
    
    # Wenn das Gerät nicht existiert, automatisch erstellen
    if not device_exists:
        new_device = {
            "id": len(devices) + 1,
            "device_id": device_id,
            "name": f"Automatisch erstellt: {device_id}",
            "type": "standard",
            "status": "online",
            "last_seen": datetime.now().isoformat(),
            "created_at": datetime.now().isoformat(),
        }
        devices.append(new_device)
    
    # Sensordaten erstellen
    new_data = {
        "timestamp": datetime.now().isoformat(),
        "temperature": data.temperature,
        "humidity": data.humidity,
        "power": data.power,
        "energy": data.energy,
        "relay_state": data.relay_state,
        "runtime": data.runtime
    }
    
    # Daten hinzufügen
    if device_id not in sensor_data:
        sensor_data[device_id] = []
    
    sensor_data[device_id].append(new_data)
    
    return {"status": "ok"}


class DeviceCommand(BaseModel):
    """Schema für Gerätebefehle"""
    command: str
    value: Any


@app.post("/api/device/{device_id}/command")
async def send_device_command(device_id: str, command: DeviceCommand):
    """Sendet einen Befehl an ein Gerät."""
    # Prüfen, ob das Gerät existiert
    device_exists = False
    for device in devices:
        if device["device_id"] == device_id:
            device_exists = True
            break
    
    if not device_exists:
        return JSONResponse(status_code=404, content={"detail": "Gerät nicht gefunden"})
    
    # In einer echten Implementierung würde hier eine MQTT-Nachricht gesendet
    print(f"Sende Befehl an Gerät {device_id}: {command.command} = {command.value}")
    
    return {"message": "Befehl gesendet"}