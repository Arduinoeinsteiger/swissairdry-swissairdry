"""
SwissAirDry - MQTT-Integrations-Router

Dieser Router stellt die MQTT-Integrationsendpunkte für IoT-Geräte bereit.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import json
from typing import Dict, Any, List, Optional
from fastapi import APIRouter, Depends, HTTPException, WebSocket, WebSocketDisconnect
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
from sqlalchemy.orm import Session

import config
from database import get_db

router = APIRouter(tags=["MQTT Integration"])

# MQTT-Client Konfiguration
mqtt_client = None

def get_mqtt_client():
    """MQTT-Client initialisieren, falls noch nicht geschehen"""
    global mqtt_client
    if mqtt_client is None:
        mqtt_client = mqtt.Client()
        # Optional: Authentifizierung hinzufügen
        if config.MQTT_USERNAME and config.MQTT_PASSWORD:
            mqtt_client.username_pw_set(config.MQTT_USERNAME, config.MQTT_PASSWORD)
        try:
            mqtt_client.connect(config.MQTT_BROKER, config.MQTT_PORT, 60)
            mqtt_client.loop_start()
        except Exception as e:
            print(f"MQTT-Verbindungsfehler: {str(e)}")
    return mqtt_client

@router.get("/status")
async def get_mqtt_status():
    """API zum Abrufen des MQTT-Verbindungsstatus"""
    try:
        client = get_mqtt_client()
        return {
            "status": "connected" if client._state == mqtt.mqtt_cs_connected else "disconnected",
            "broker": config.MQTT_BROKER,
            "port": config.MQTT_PORT
        }
    except Exception as e:
        return {
            "status": "error",
            "message": str(e)
        }

@router.post("/publish")
async def publish_message(
    topic: str,
    message: Dict[str, Any],
    retain: bool = False,
    qos: int = 0
):
    """API zum Veröffentlichen einer Nachricht an ein MQTT-Topic"""
    try:
        payload = json.dumps(message)
        publish.single(
            topic=topic,
            payload=payload,
            qos=qos,
            retain=retain,
            hostname=config.MQTT_BROKER,
            port=config.MQTT_PORT,
            client_id="swissairdry-api",
            auth=None if not (config.MQTT_USERNAME and config.MQTT_PASSWORD) else 
                  {'username': config.MQTT_USERNAME, 'password': config.MQTT_PASSWORD}
        )
        return {
            "status": "success",
            "topic": topic,
            "message": message
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"MQTT-Fehler: {str(e)}")

@router.get("/devices")
async def get_mqtt_devices(db: Session = Depends(get_db)):
    """API zum Abrufen aller MQTT-fähigen Geräte"""
    # In einer echten Implementierung würden hier MQTT-Geräte aus der Datenbank abgerufen
    devices = [
        {
            "id": "MQTT-DEV-001",
            "name": "ESP32 Sensor 1",
            "type": "Temperatur- und Feuchtigkeitssensor",
            "topic": "swissairdry/sensors/esp32-001",
            "status": "Online",
            "last_seen": "2025-04-12T10:15:00Z",
            "firmware_version": "1.2.3",
            "battery": 78
        },
        {
            "id": "MQTT-DEV-002",
            "name": "ESP32 Gateway 1",
            "type": "Gateway",
            "topic": "swissairdry/gateways/esp32-001",
            "status": "Online",
            "last_seen": "2025-04-12T10:16:00Z",
            "firmware_version": "1.2.3",
            "connected_devices": 3
        },
        {
            "id": "MQTT-DEV-003",
            "name": "ESP32 Sensor 2",
            "type": "Temperatur- und Feuchtigkeitssensor",
            "topic": "swissairdry/sensors/esp32-002",
            "status": "Offline",
            "last_seen": "2025-04-11T22:45:00Z",
            "firmware_version": "1.2.2",
            "battery": 15
        }
    ]
    return {"devices": devices}

# WebSocket-Verbindung für Live-MQTT-Updates
class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)

    async def broadcast(self, message: str):
        for connection in self.active_connections:
            await connection.send_text(message)

manager = ConnectionManager()

@router.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket-Endpunkt für Live-MQTT-Updates"""
    await manager.connect(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            # In einer echten Implementierung würden wir hier auf MQTT-Nachrichten reagieren
            await websocket.send_text(f"Nachricht empfangen: {data}")
    except WebSocketDisconnect:
        manager.disconnect(websocket)