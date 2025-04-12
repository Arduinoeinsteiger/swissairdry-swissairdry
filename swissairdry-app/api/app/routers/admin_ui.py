"""
SwissAirDry - Admin UI Router

Dieser Router stellt die Admin-Benutzeroberfläche für das SwissAirDry-System bereit,
inklusive Benutzer- und Gruppenverwaltung, sowie CSV-Import-Funktionen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List, Optional
from fastapi import APIRouter, Depends, Request, HTTPException, Body, Form, UploadFile, File
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from sqlalchemy.exc import SQLAlchemyError

import config
from database import get_db
from models import User, Group, ApiPermission, DashboardModule, CsvImportLog

router = APIRouter(tags=["Admin UI"])
templates = Jinja2Templates(directory="templates")

@router.get("/", response_class=HTMLResponse)
async def admin_home(request: Request, db: Session = Depends(get_db)):
    """Admin-Dashboard-Startseite"""
    return templates.TemplateResponse(
        "index.html", 
        {"request": request, "page": "admin"}
    )

@router.get("/users", response_class=HTMLResponse)
async def user_management(request: Request, db: Session = Depends(get_db)):
    """Benutzerverwaltung"""
    return templates.TemplateResponse(
        "index.html", 
        {"request": request, "page": "users"}
    )

@router.get("/groups", response_class=HTMLResponse)
async def group_management(request: Request, db: Session = Depends(get_db)):
    """Gruppenverwaltung"""
    return templates.TemplateResponse(
        "index.html", 
        {"request": request, "page": "groups"}
    )

@router.get("/csv-import", response_class=HTMLResponse)
async def csv_import_page(request: Request, db: Session = Depends(get_db)):
    """CSV-Import-Seite"""
    return templates.TemplateResponse(
        "index.html", 
        {"request": request, "page": "csv-import"}
    )

# API-Endpunkte für die Standalone-App

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