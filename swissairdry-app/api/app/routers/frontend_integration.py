"""
SwissAirDry - Frontend-Integrations-Router

Dieser Router stellt spezielle Endpunkte für die Integration mit dem Frontend bereit.
Er wird nur im Standalone-Modus verwendet, wenn keine Nextcloud-Integration vorliegt.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List, Optional
from fastapi import APIRouter, Depends, HTTPException, Body
from sqlalchemy.orm import Session
from pydantic import BaseModel

from database import get_db
from models import User, Group, DashboardModule

router = APIRouter(tags=["Frontend Integration"])

class DashboardData(BaseModel):
    """Datenmodell für Dashboard-Informationen"""
    modules: List[Dict[str, Any]]
    user_data: Dict[str, Any]

@router.get("/dashboard")
async def get_dashboard_data(db: Session = Depends(get_db)) -> DashboardData:
    """
    Liefert alle notwendigen Daten für das Dashboard der Frontend-App.
    """
    # Demo-Daten für Entwicklungszwecke
    # In der Produktion würden diese aus der Datenbank kommen
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
    
    user_data = {
        "id": 1,
        "username": "admin",
        "fullName": "Administrator",
        "email": "admin@swissairdry.com",
        "role": "Administrator",
        "isAdmin": True
    }
    
    return DashboardData(modules=modules, user_data=user_data)

@router.get("/modules/{module_id}")
async def get_module_data(module_id: str, db: Session = Depends(get_db)) -> Dict[str, Any]:
    """
    Liefert spezifische Daten für ein bestimmtes Modul.
    """
    # Beispielantwort für verschiedene Module
    if module_id == "devices":
        return {
            "id": "devices",
            "name": "Geräte",
            "items": [
                {"id": 1, "name": "Trockner A1", "type": "Luftentfeuchter", "status": "Aktiv"},
                {"id": 2, "name": "Trockner B2", "type": "Kondensationstrockner", "status": "Im Einsatz"},
                {"id": 3, "name": "Lüfter C3", "type": "Ventilator", "status": "Verfügbar"}
            ]
        }
    elif module_id == "jobs":
        return {
            "id": "jobs",
            "name": "Aufträge",
            "items": [
                {"id": 101, "customer": "Müller GmbH", "status": "In Bearbeitung", "startDate": "2025-03-15"},
                {"id": 102, "customer": "Schmidt AG", "status": "Abgeschlossen", "startDate": "2025-02-20"},
                {"id": 103, "customer": "Bauer KG", "status": "Neu", "startDate": "2025-04-01"}
            ]
        }
    elif module_id == "customers":
        return {
            "id": "customers",
            "name": "Kunden",
            "items": [
                {"id": 201, "name": "Müller GmbH", "contact": "Hans Müller", "phone": "+49 123 456789"},
                {"id": 202, "name": "Schmidt AG", "contact": "Anna Schmidt", "phone": "+49 234 567890"},
                {"id": 203, "name": "Bauer KG", "contact": "Peter Bauer", "phone": "+49 345 678901"}
            ]
        }
    else:
        raise HTTPException(status_code=404, detail=f"Modul {module_id} nicht gefunden")

@router.post("/auth/login")
async def login(
    username: str = Body(...),
    password: str = Body(...),
    db: Session = Depends(get_db)
) -> Dict[str, Any]:
    """
    Simuliert einen Login-Prozess für die Standalone-App.
    In der Produktion würde hier eine echte Authentifizierung stattfinden.
    """
    # Demo-Login für Entwicklungszwecke
    # In der Produktion würde ein echter Benutzer aus der Datenbank authentifiziert werden
    if username == "admin" and password == "admin":
        return {
            "status": "success",
            "message": "Login erfolgreich",
            "token": "demo_token_12345",
            "user": {
                "id": 1,
                "username": "admin",
                "fullName": "Administrator",
                "email": "admin@swissairdry.com",
                "role": "Administrator",
                "isAdmin": True
            }
        }
    else:
        raise HTTPException(
            status_code=401,
            detail="Ungültige Zugangsdaten"
        )