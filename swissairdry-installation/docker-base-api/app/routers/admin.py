"""
SwissAirDry - Admin-Router

Dieser Router stellt die Admin-API und Benutzeroberfläche für das SwissAirDry-System bereit.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List
from fastapi import APIRouter, Depends, Request, HTTPException, Body, Form, UploadFile, File
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session

import config
from database import get_db

router = APIRouter(tags=["Admin"])
templates = Jinja2Templates(directory="templates")

@router.get("/", response_class=HTMLResponse)
async def admin_dashboard(request: Request, db: Session = Depends(get_db)):
    """Admin-Dashboard-Startseite"""
    return templates.TemplateResponse("admin.html", {
        "request": request,
        "page": "admin",
        "title": "Admin-Dashboard"
    })

@router.get("/users", response_class=HTMLResponse)
async def users_management(request: Request, db: Session = Depends(get_db)):
    """Benutzer-Verwaltungsseite"""
    return templates.TemplateResponse("users.html", {
        "request": request,
        "page": "users",
        "title": "Benutzerverwaltung"
    })

@router.get("/groups", response_class=HTMLResponse)
async def groups_management(request: Request, db: Session = Depends(get_db)):
    """Gruppen-Verwaltungsseite"""
    return templates.TemplateResponse("groups.html", {
        "request": request,
        "page": "groups",
        "title": "Gruppenverwaltung"
    })

@router.get("/csv-import", response_class=HTMLResponse)
async def csv_import_page(request: Request, db: Session = Depends(get_db)):
    """CSV-Import-Seite"""
    return templates.TemplateResponse("csv_import.html", {
        "request": request,
        "page": "csv-import",
        "title": "CSV-Import"
    })

# API-Endpunkte

@router.get("/api/users")
async def get_users(db: Session = Depends(get_db)):
    """API zum Abrufen aller Benutzer"""
    # In einer echten Implementierung würden hier Benutzer aus der Datenbank abgerufen
    users = [
        {
            "id": 1,
            "username": "admin",
            "email": "admin@swissairdry.com",
            "fullName": "Administrator",
            "isActive": True,
            "isAdmin": True,
            "lastLogin": "2025-04-01T10:30:00Z",
            "groups": ["Administrator"]
        },
        {
            "id": 2,
            "username": "techniker",
            "email": "techniker@swissairdry.com",
            "fullName": "Test Techniker",
            "isActive": True,
            "isAdmin": False,
            "lastLogin": "2025-03-30T08:15:00Z",
            "groups": ["Trocknungsspezialist"]
        }
    ]
    return {"users": users}

@router.get("/api/groups")
async def get_groups(db: Session = Depends(get_db)):
    """API zum Abrufen aller Gruppen"""
    # In einer echten Implementierung würden hier Gruppen aus der Datenbank abgerufen
    groups = [
        {
            "id": 1,
            "name": "Administrator",
            "description": "Voller Zugriff auf alle Funktionen",
            "userCount": 1
        },
        {
            "id": 2,
            "name": "Trocknungsspezialist",
            "description": "Zugriff auf Aufträge, Messungen und Geräte",
            "userCount": 1
        },
        {
            "id": 3,
            "name": "Logistikmitarbeiter",
            "description": "Zugriff auf Geräte und Transport",
            "userCount": 0
        }
    ]
    return {"groups": groups}

@router.post("/api/csv-upload")
async def upload_csv(file: UploadFile = File(...), db: Session = Depends(get_db)):
    """API zum Hochladen einer CSV-Datei"""
    if not file.filename.endswith('.csv'):
        raise HTTPException(status_code=400, detail="Nur CSV-Dateien sind erlaubt")
    
    # Beispiel-Antwort ohne tatsächlichen Import
    return {
        "status": "success",
        "message": f"CSV-Datei '{file.filename}' wurde hochgeladen",
        "details": {
            "filename": file.filename,
            "size": file.size
        }
    }