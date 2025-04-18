"""
SwissAirDry API - Einfache Version

Eine vereinfachte Version der SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import json
import asyncio
import logging
from datetime import datetime
from typing import Dict, Any, List, Optional

from fastapi import FastAPI, Request, BackgroundTasks
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

# Eigene MQTT-Client-Klasse importieren
try:
    from mqtt_client import MQTTClient
except ImportError:
    # Falls die Datei nicht gefunden wird
    MQTTClient = None
    print("MQTT-Client-Modul nicht gefunden. MQTT-Funktionalität deaktiviert.")

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

# MQTT-Client initialisieren
mqtt_client = None


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


@app.on_event("startup")
async def startup_event():
    """Wird beim Start der Anwendung aufgerufen."""
    global mqtt_client
    
    print("API-Server wird gestartet...")
    
    # MQTT-Client initialisieren, wenn MQTTClient und paho.mqtt.client verfügbar sind
    if MQTTClient is not None:
        try:
            # MQTT-Verbindungsdaten aus Umgebungsvariablen oder Standardwerten
            mqtt_host = os.getenv("MQTT_HOST", "localhost")
            mqtt_port = int(os.getenv("MQTT_PORT", "1883"))
            mqtt_user = os.getenv("MQTT_USER", "")
            mqtt_password = os.getenv("MQTT_PASSWORD", "")
            
            print(f"Verbinde mit MQTT-Broker {mqtt_host}:{mqtt_port}...")
            mqtt_client = MQTTClient(mqtt_host, mqtt_port, mqtt_user, mqtt_password)
            
            # Callback für SwissAirDry-Nachrichten
            mqtt_client.add_message_callback("swissairdry/#", mqtt_message_handler)
            
            # Verbindung herstellen
            connected = await mqtt_client.connect()
            if connected:
                print(f"MQTT-Client verbunden mit {mqtt_host}:{mqtt_port}")
                
                # Standard-Themen abonnieren
                await mqtt_client.subscribe("swissairdry/+/data")
                await mqtt_client.subscribe("swissairdry/+/status")
                await mqtt_client.subscribe("swissairdry/+/config")
            else:
                print("MQTT-Verbindung fehlgeschlagen")
        except Exception as e:
            print(f"Fehler bei der MQTT-Initialisierung: {e}")
    
    print("API-Server erfolgreich gestartet")


@app.on_event("shutdown")
async def shutdown_event():
    """Wird beim Herunterfahren der Anwendung aufgerufen."""
    global mqtt_client
    
    print("API-Server wird heruntergefahren...")
    
    # MQTT-Verbindung trennen
    if mqtt_client and mqtt_client.is_connected():
        await mqtt_client.disconnect()
        print("MQTT-Client getrennt")


def mqtt_message_handler(topic: str, payload: Any):
    """
    Callback-Funktion für MQTT-Nachrichten.
    
    Args:
        topic: MQTT-Thema
        payload: Nachrichteninhalt (JSON-Objekt oder String)
    """
    print(f"MQTT-Nachricht empfangen: {topic}")
    
    try:
        # Topic-Teile extrahieren (swissairdry/device_id/type)
        topic_parts = topic.split("/")
        if len(topic_parts) >= 3:
            device_id = topic_parts[1]
            message_type = topic_parts[2]
            
            if message_type == "data":
                # Sensordaten verarbeiten
                if isinstance(payload, dict):
                    process_sensor_data(device_id, payload)
                else:
                    print(f"Ungültiges Payload-Format für Sensordaten: {payload}")
            
            elif message_type == "status":
                # Statusmeldung verarbeiten
                update_device_status(device_id, payload)
            
            elif message_type == "config":
                # Konfigurationsdaten verarbeiten
                update_device_config(device_id, payload)
    
    except Exception as e:
        print(f"Fehler bei der Verarbeitung der MQTT-Nachricht: {e}")


def process_sensor_data(device_id: str, data: Dict[str, Any]):
    """
    Verarbeitet Sensordaten von einem Gerät.
    
    Args:
        device_id: Geräte-ID
        data: Sensordaten
    """
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
            "name": f"MQTT Gerät: {device_id}",
            "type": "standard",
            "status": "online",
            "last_seen": datetime.now().isoformat(),
            "created_at": datetime.now().isoformat(),
        }
        devices.append(new_device)
        print(f"Neues Gerät erstellt: {device_id}")
    
    # Sensordaten erstellen
    new_data = {
        "timestamp": datetime.now().isoformat(),
        "temperature": data.get("temperature"),
        "humidity": data.get("humidity"),
        "power": data.get("power"),
        "energy": data.get("energy"),
        "relay_state": data.get("relay_state"),
        "runtime": data.get("runtime")
    }
    
    # Daten hinzufügen
    if device_id not in sensor_data:
        sensor_data[device_id] = []
    
    # Begrenze die Anzahl der Datenpunkte auf 1000
    sensor_data[device_id].append(new_data)
    if len(sensor_data[device_id]) > 1000:
        sensor_data[device_id] = sensor_data[device_id][-1000:]
    
    print(f"Sensordaten gespeichert für Gerät: {device_id}")


def update_device_status(device_id: str, status: Any):
    """
    Aktualisiert den Status eines Geräts.
    
    Args:
        device_id: Geräte-ID
        status: Statusdaten
    """
    for device in devices:
        if device["device_id"] == device_id:
            if isinstance(status, dict):
                # Status-Attribute aktualisieren
                if "status" in status:
                    device["status"] = status["status"]
                device["last_seen"] = datetime.now().isoformat()
            elif isinstance(status, str):
                # Einfacher Status-String
                device["status"] = status
                device["last_seen"] = datetime.now().isoformat()
            print(f"Status aktualisiert für Gerät: {device_id}")
            return
    
    # Wenn das Gerät nicht existiert, ignoriere die Statusmeldung
    print(f"Statusmeldung ignoriert: Gerät {device_id} nicht gefunden")


def update_device_config(device_id: str, config: Any):
    """
    Aktualisiert die Konfiguration eines Geräts.
    
    Args:
        device_id: Geräte-ID
        config: Konfigurationsdaten
    """
    for device in devices:
        if device["device_id"] == device_id:
            if isinstance(config, dict):
                # Konfiguration in Gerätedaten speichern
                if "configuration" not in device:
                    device["configuration"] = {}
                device["configuration"].update(config)
                print(f"Konfiguration aktualisiert für Gerät: {device_id}")
                return
    
    # Wenn das Gerät nicht existiert, ignoriere die Konfiguration
    print(f"Konfiguration ignoriert: Gerät {device_id} nicht gefunden")


@app.post("/api/device/{device_id}/command")
async def send_device_command(device_id: str, command: DeviceCommand):
    """Sendet einen Befehl an ein Gerät über MQTT."""
    # Prüfen, ob das Gerät existiert
    device_exists = False
    for device in devices:
        if device["device_id"] == device_id:
            device_exists = True
            break
    
    if not device_exists:
        return JSONResponse(status_code=404, content={"detail": "Gerät nicht gefunden"})
    
    # Befehl über MQTT senden, wenn der Client verbunden ist
    if mqtt_client and mqtt_client.is_connected():
        topic = f"swissairdry/{device_id}/cmd/{command.command}"
        try:
            await mqtt_client.publish(topic, command.value)
            print(f"MQTT-Befehl gesendet: {topic} = {command.value}")
            return {"message": "Befehl gesendet"}
        except Exception as e:
            print(f"Fehler beim Senden des MQTT-Befehls: {e}")
            return JSONResponse(
                status_code=500, 
                content={"detail": f"Fehler beim Senden des Befehls: {str(e)}"}
            )
    else:
        # Fallback, falls MQTT nicht verfügbar ist
        print(f"MQTT nicht verfügbar. Simuliere Befehl: {device_id}: {command.command} = {command.value}")
        return {"message": "Befehl simuliert (MQTT nicht verfügbar)"}