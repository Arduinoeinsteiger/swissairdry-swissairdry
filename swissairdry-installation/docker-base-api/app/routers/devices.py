"""
SwissAirDry - Geräte-Router

Dieser Router stellt die Geräte-API für das SwissAirDry-System bereit.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List, Optional
from fastapi import APIRouter, Depends, HTTPException, Query, Path
from sqlalchemy.orm import Session

from database import get_db

router = APIRouter(tags=["Devices"])

@router.get("/")
async def get_devices(
    status: Optional[str] = None,
    device_type: Optional[str] = None,
    limit: int = 100,
    offset: int = 0,
    db: Session = Depends(get_db)
):
    """API zum Abrufen aller Geräte mit optionaler Filterung"""
    # In einer echten Implementierung würden hier Geräte aus der Datenbank abgerufen
    devices = [
        {
            "id": "DEV-123",
            "name": "Luftentfeuchter A1",
            "type": "Luftentfeuchter",
            "serial": "LE-2025-001",
            "status": "Aktiv",
            "location": "Müller GmbH",
            "job": "JOB-456",
            "last_maintenance": "2025-02-15",
            "energy_consumption": 2.5,  # kWh pro Tag
            "operational_hours": 3560
        },
        {
            "id": "DEV-124",
            "name": "Ventilator B2",
            "type": "Ventilator",
            "serial": "VE-2025-002",
            "status": "Aktiv",
            "location": "Schmidt KG",
            "job": "JOB-457",
            "last_maintenance": "2025-03-10",
            "energy_consumption": 0.8,  # kWh pro Tag
            "operational_hours": 1250
        },
        {
            "id": "DEV-125",
            "name": "Heizgerät C3",
            "type": "Heizgerät",
            "serial": "HG-2025-003",
            "status": "Verfügbar",
            "location": "Lager",
            "job": None,
            "last_maintenance": "2025-03-01",
            "energy_consumption": 3.2,  # kWh pro Tag
            "operational_hours": 850
        }
    ]
    
    # Filterung anwenden (in einer echten Implementierung würde dies in der Datenbankabfrage geschehen)
    if status:
        devices = [d for d in devices if d["status"] == status]
    if device_type:
        devices = [d for d in devices if d["type"] == device_type]
    
    # Pagination anwenden
    total_count = len(devices)
    devices = devices[offset:offset+limit]
    
    return {
        "devices": devices,
        "total": total_count,
        "limit": limit,
        "offset": offset
    }

@router.get("/{device_id}")
async def get_device(device_id: str = Path(...), db: Session = Depends(get_db)):
    """API zum Abrufen eines einzelnen Geräts anhand seiner ID"""
    # In einer echten Implementierung würde hier das Gerät aus der Datenbank abgerufen
    if device_id == "DEV-123":
        return {
            "id": "DEV-123",
            "name": "Luftentfeuchter A1",
            "type": "Luftentfeuchter",
            "serial": "LE-2025-001",
            "status": "Aktiv",
            "location": "Müller GmbH",
            "job": "JOB-456",
            "last_maintenance": "2025-02-15",
            "next_maintenance": "2025-08-15",
            "energy_consumption": 2.5,  # kWh pro Tag
            "operational_hours": 3560,
            "technical_details": {
                "model": "SwissAir Pro 500",
                "manufacturer": "SwissAir Technologies",
                "year": 2024,
                "capacity": 50,  # l/Tag
                "power": 900,  # Watt
                "dimensions": "60x40x30 cm",
                "weight": 35  # kg
            },
            "maintenance_history": [
                {"date": "2025-02-15", "type": "Vollständige Wartung", "technician": "Max Müller", "notes": "Filter ersetzt, Gerät gereinigt"},
                {"date": "2024-08-20", "type": "Routinewartung", "technician": "Jan Schmidt", "notes": "Alles in Ordnung"}
            ],
            "sensor_data": {
                "temperature": 22.5,  # °C
                "humidity": 45.8,  # %
                "power_consumption": 850,  # Watt aktuell
                "last_update": "2025-04-12T08:30:00Z"
            }
        }
    else:
        raise HTTPException(status_code=404, detail=f"Gerät mit ID '{device_id}' nicht gefunden")

@router.get("/{device_id}/history")
async def get_device_history(
    device_id: str = Path(...),
    start_date: Optional[str] = None,
    end_date: Optional[str] = None,
    db: Session = Depends(get_db)
):
    """API zum Abrufen der Verlaufsdaten eines Geräts"""
    # In einer echten Implementierung würden hier die Verlaufsdaten aus der Datenbank abgerufen
    if device_id == "DEV-123":
        return {
            "device_id": "DEV-123",
            "name": "Luftentfeuchter A1",
            "history": [
                {"timestamp": "2025-04-11T12:00:00Z", "temperature": 22.5, "humidity": 45.2, "power": 850},
                {"timestamp": "2025-04-11T13:00:00Z", "temperature": 22.6, "humidity": 44.8, "power": 855},
                {"timestamp": "2025-04-11T14:00:00Z", "temperature": 22.7, "humidity": 44.3, "power": 860},
                {"timestamp": "2025-04-11T15:00:00Z", "temperature": 22.8, "humidity": 43.9, "power": 865},
                {"timestamp": "2025-04-11T16:00:00Z", "temperature": 22.7, "humidity": 43.5, "power": 860},
                {"timestamp": "2025-04-11T17:00:00Z", "temperature": 22.6, "humidity": 43.1, "power": 855},
                {"timestamp": "2025-04-11T18:00:00Z", "temperature": 22.5, "humidity": 42.7, "power": 850},
                {"timestamp": "2025-04-11T19:00:00Z", "temperature": 22.4, "humidity": 42.3, "power": 845},
                {"timestamp": "2025-04-11T20:00:00Z", "temperature": 22.3, "humidity": 41.9, "power": 840},
                {"timestamp": "2025-04-11T21:00:00Z", "temperature": 22.2, "humidity": 41.5, "power": 835},
                {"timestamp": "2025-04-11T22:00:00Z", "temperature": 22.1, "humidity": 41.1, "power": 830},
                {"timestamp": "2025-04-11T23:00:00Z", "temperature": 22.0, "humidity": 40.7, "power": 825},
                {"timestamp": "2025-04-12T00:00:00Z", "temperature": 21.9, "humidity": 40.3, "power": 820},
                {"timestamp": "2025-04-12T01:00:00Z", "temperature": 21.8, "humidity": 39.9, "power": 815},
                {"timestamp": "2025-04-12T02:00:00Z", "temperature": 21.7, "humidity": 39.5, "power": 810},
                {"timestamp": "2025-04-12T03:00:00Z", "temperature": 21.6, "humidity": 39.1, "power": 805},
                {"timestamp": "2025-04-12T04:00:00Z", "temperature": 21.5, "humidity": 38.7, "power": 800},
                {"timestamp": "2025-04-12T05:00:00Z", "temperature": 21.6, "humidity": 38.9, "power": 805},
                {"timestamp": "2025-04-12T06:00:00Z", "temperature": 21.8, "humidity": 39.3, "power": 815},
                {"timestamp": "2025-04-12T07:00:00Z", "temperature": 22.0, "humidity": 40.1, "power": 825},
                {"timestamp": "2025-04-12T08:00:00Z", "temperature": 22.3, "humidity": 41.2, "power": 840},
                {"timestamp": "2025-04-12T09:00:00Z", "temperature": 22.5, "humidity": 42.5, "power": 850},
                {"timestamp": "2025-04-12T10:00:00Z", "temperature": 22.6, "humidity": 43.8, "power": 855},
                {"timestamp": "2025-04-12T11:00:00Z", "temperature": 22.7, "humidity": 45.0, "power": 860}
            ]
        }
    else:
        raise HTTPException(status_code=404, detail=f"Gerät mit ID '{device_id}' nicht gefunden")