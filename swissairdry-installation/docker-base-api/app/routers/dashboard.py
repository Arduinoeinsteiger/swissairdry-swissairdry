"""
SwissAirDry - Dashboard-Router

Dieser Router stellt die Dashboard-API für das SwissAirDry-System bereit,
die von der Nextcloud-Integration verwendet wird.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List
from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session

from database import get_db

router = APIRouter(tags=["Dashboard"])

@router.get("/modules")
async def get_dashboard_modules(db: Session = Depends(get_db)):
    """API zum Abrufen aller verfügbaren Dashboard-Module"""
    # In einer echten Implementierung würden hier Module aus der Datenbank abgerufen
    modules = [
        {
            "id": "admin",
            "name": "Administration",
            "icon": "settings",
            "path": "/admin",
            "description": "Verwaltung von Benutzern und Einstellungen"
        },
        {
            "id": "devices",
            "name": "Geräte",
            "icon": "devices",
            "path": "/devices",
            "description": "Verwaltung der Trocknungsgeräte"
        },
        {
            "id": "jobs",
            "name": "Aufträge",
            "icon": "assignment",
            "path": "/jobs",
            "description": "Verwaltung von Trocknungsaufträgen"
        },
        {
            "id": "customers",
            "name": "Kunden",
            "icon": "people",
            "path": "/customers",
            "description": "Kundenverwaltung"
        },
        {
            "id": "reports",
            "name": "Berichte",
            "icon": "assessment",
            "path": "/reports",
            "description": "Berichte und Analysen"
        }
    ]
    return {"modules": modules}

@router.get("/statistics")
async def get_dashboard_statistics(db: Session = Depends(get_db)):
    """API zum Abrufen von Dashboard-Statistiken"""
    # In einer echten Implementierung würden hier Statistiken aus der Datenbank abgerufen
    statistics = {
        "devices": {
            "total": 120,
            "active": 75,
            "maintenance": 5,
            "available": 40
        },
        "jobs": {
            "total": 250,
            "active": 35,
            "completed": 215,
            "avgDurationDays": 14.5
        },
        "customers": {
            "total": 85,
            "active": 42
        },
        "measurements": {
            "total": 12450,
            "last24h": 350
        }
    }
    return statistics

@router.get("/modules/{module_id}")
async def get_module_data(module_id: str, db: Session = Depends(get_db)):
    """API zum Abrufen von Daten für ein spezifisches Modul"""
    # Beispielantwort abhängig vom Modul-ID
    if module_id == "devices":
        return {
            "id": "devices",
            "name": "Geräte",
            "statistics": {
                "total": 120,
                "active": 75,
                "maintenance": 5,
                "available": 40
            },
            "recentChanges": [
                {"id": "DEV-123", "type": "Luftentfeuchter", "location": "Müller GmbH", "status": "Aktiv", "lastUpdate": "2025-04-11T15:30:00Z"},
                {"id": "DEV-124", "type": "Ventilator", "location": "Schmidt KG", "status": "Aktiv", "lastUpdate": "2025-04-10T09:15:00Z"},
                {"id": "DEV-125", "type": "Heizgerät", "location": "Lager", "status": "Verfügbar", "lastUpdate": "2025-04-09T14:45:00Z"}
            ]
        }
    elif module_id == "jobs":
        return {
            "id": "jobs",
            "name": "Aufträge",
            "statistics": {
                "total": 250,
                "active": 35,
                "completed": 215,
                "avgDurationDays": 14.5
            },
            "recentJobs": [
                {"id": "JOB-456", "customer": "Müller GmbH", "location": "Hauptstraße 1, München", "status": "Aktiv", "startDate": "2025-04-05T08:00:00Z"},
                {"id": "JOB-457", "customer": "Schmidt KG", "location": "Industrieweg 42, Berlin", "status": "Aktiv", "startDate": "2025-04-03T10:30:00Z"},
                {"id": "JOB-458", "customer": "Bauer AG", "location": "Dorfplatz 3, Hamburg", "status": "Abgeschlossen", "startDate": "2025-03-20T09:00:00Z", "endDate": "2025-04-10T16:45:00Z"}
            ]
        }
    else:
        raise HTTPException(status_code=404, detail=f"Modul '{module_id}' nicht gefunden")