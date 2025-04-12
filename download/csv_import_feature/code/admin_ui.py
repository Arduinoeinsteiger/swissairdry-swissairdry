"""
SwissAirDry - Admin UI Router

Dieser Router stellt die Admin-Benutzeroberfläche für das SwissAirDry-System bereit,
inklusive Benutzer- und Gruppenverwaltung, sowie CSV-Import-Funktionen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from fastapi import APIRouter, Depends, HTTPException, Request, status
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from typing import Dict, List, Any, Optional
import logging
import os

# Pfad zur App-Directory hinzufügen
import sys
parent_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if parent_path not in sys.path:
    sys.path.append(parent_path)
    
import config
from database import get_db
from models import User, Group, user_groups

# Logger konfigurieren
logger = logging.getLogger(__name__)

# Router erstellen
router = APIRouter(
    prefix="/admin",
    tags=["Admin UI"]
)

# Templates initialisieren
templates_dir = os.path.join(parent_path, "templates")
os.makedirs(templates_dir, exist_ok=True)
templates = Jinja2Templates(directory=templates_dir)

@router.get("/", response_class=HTMLResponse)
async def admin_home(request: Request, db: Session = Depends(get_db)):
    """Admin-Dashboard-Startseite"""
    return templates.TemplateResponse(
        "admin_dashboard.html", 
        {
            "request": request, 
            "page_title": "Admin Dashboard",
            "api_base_url": config.API_BASE_URL,
            "nextcloud_url": config.NEXTCLOUD_FULL_URL
        }
    )

@router.get("/users", response_class=HTMLResponse)
async def user_management(request: Request, db: Session = Depends(get_db)):
    """Benutzerverwaltung"""
    users = db.query(User).all()
    
    # Benutzergruppen laden
    user_data = []
    for user in users:
        groups = db.query(Group).join(user_groups).filter(user_groups.c.user_id == user.id).all()
        group_names = [group.name for group in groups]
        
        user_data.append({
            "id": user.id,
            "username": user.username,
            "email": user.email,
            "full_name": user.full_name,
            "is_active": user.is_active,
            "is_admin": user.is_admin,
            "groups": group_names
        })
    
    return templates.TemplateResponse(
        "user_management.html", 
        {
            "request": request, 
            "page_title": "Benutzerverwaltung",
            "users": user_data,
            "api_base_url": config.API_BASE_URL,
            "nextcloud_url": config.NEXTCLOUD_FULL_URL
        }
    )

@router.get("/groups", response_class=HTMLResponse)
async def group_management(request: Request, db: Session = Depends(get_db)):
    """Gruppenverwaltung"""
    groups = db.query(Group).all()
    users = db.query(User).all()
    
    # Gruppendaten vorbereiten
    group_data = []
    for group in groups:
        # Benutzer in dieser Gruppe abrufen
        group_users = db.query(User).join(user_groups).filter(user_groups.c.group_id == group.id).all()
        user_list = [{"id": user.id, "username": user.username} for user in group_users]
        
        group_data.append({
            "id": group.id,
            "name": group.name,
            "description": group.description,
            "users": user_list,
            "user_count": len(user_list)
        })
    
    return templates.TemplateResponse(
        "group_management.html", 
        {
            "request": request, 
            "page_title": "Gruppenverwaltung",
            "groups": group_data,
            "all_users": users,
            "api_base_url": config.API_BASE_URL,
            "nextcloud_url": config.NEXTCLOUD_FULL_URL
        }
    )

@router.get("/user/{user_id}", response_class=HTMLResponse)
async def edit_user(user_id: int, request: Request, db: Session = Depends(get_db)):
    """Benutzer bearbeiten"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Benutzer mit ID {user_id} nicht gefunden"
        )
    
    # Alle verfügbaren Gruppen laden
    all_groups = db.query(Group).all()
    
    # Gruppen des Benutzers laden
    user_groups_db = db.query(Group).join(user_groups).filter(user_groups.c.user_id == user.id).all()
    user_group_ids = [group.id for group in user_groups_db]
    
    return templates.TemplateResponse(
        "edit_user.html", 
        {
            "request": request, 
            "page_title": f"Benutzer bearbeiten: {user.username}",
            "user": user,
            "all_groups": all_groups,
            "user_group_ids": user_group_ids,
            "api_base_url": config.API_BASE_URL,
            "nextcloud_url": config.NEXTCLOUD_FULL_URL
        }
    )

@router.post("/user/{user_id}/update_groups")
async def update_user_groups(
    user_id: int, 
    group_ids: List[int] = [], 
    db: Session = Depends(get_db)
):
    """Aktualisiert die Gruppenzuweisungen eines Benutzers"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Benutzer mit ID {user_id} nicht gefunden"
        )
    
    # Bestehende Gruppenzuweisungen entfernen
    db.execute(user_groups.delete().where(user_groups.c.user_id == user_id))
    
    # Neue Gruppenzuweisungen erstellen
    for group_id in group_ids:
        db.execute(
            user_groups.insert().values(user_id=user_id, group_id=group_id)
        )
    
    db.commit()
    
    logger.info(f"Gruppen für Benutzer {user.username} aktualisiert: {group_ids}")
    
    return {"success": True, "message": "Gruppen erfolgreich aktualisiert"}

@router.get("/api/users", response_class=HTMLResponse)
async def get_users(db: Session = Depends(get_db)):
    """API zum Abrufen aller Benutzer mit ihren Gruppen"""
    users = db.query(User).all()
    
    user_data = []
    for user in users:
        groups = db.query(Group).join(user_groups).filter(user_groups.c.user_id == user.id).all()
        group_names = [group.name for group in groups]
        
        user_data.append({
            "id": user.id,
            "username": user.username,
            "email": user.email,
            "full_name": user.full_name,
            "is_active": user.is_active,
            "is_admin": user.is_admin,
            "groups": group_names
        })
    
    return {"users": user_data}

@router.get("/api/groups")
async def get_groups(db: Session = Depends(get_db)):
    """API zum Abrufen aller Gruppen"""
    groups = db.query(Group).all()
    
    group_data = []
    for group in groups:
        user_count = db.query(user_groups).filter(user_groups.c.group_id == group.id).count()
        group_data.append({
            "id": group.id,
            "name": group.name,
            "description": group.description,
            "user_count": user_count
        })
    
    return {"groups": group_data}

@router.get("/api/user/{user_id}/groups")
async def get_user_groups(user_id: int, db: Session = Depends(get_db)):
    """API zum Abrufen der Gruppen eines Benutzers"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Benutzer mit ID {user_id} nicht gefunden"
        )
    
    # Gruppen des Benutzers laden
    groups = db.query(Group).join(user_groups).filter(user_groups.c.user_id == user.id).all()
    group_data = [{"id": group.id, "name": group.name} for group in groups]
    
    return {
        "user_id": user_id,
        "username": user.username,
        "groups": group_data
    }

@router.post("/api/user/{user_id}/add_to_group/{group_id}")
async def add_user_to_group(user_id: int, group_id: int, db: Session = Depends(get_db)):
    """API zum Hinzufügen eines Benutzers zu einer Gruppe"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Benutzer mit ID {user_id} nicht gefunden"
        )
    
    group = db.query(Group).filter(Group.id == group_id).first()
    if not group:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Gruppe mit ID {group_id} nicht gefunden"
        )
    
    # Prüfen, ob der Benutzer bereits in der Gruppe ist
    existing = db.query(user_groups).filter(
        user_groups.c.user_id == user_id,
        user_groups.c.group_id == group_id
    ).first()
    
    if not existing:
        # Benutzer zur Gruppe hinzufügen
        db.execute(
            user_groups.insert().values(user_id=user_id, group_id=group_id)
        )
        db.commit()
        
        logger.info(f"Benutzer {user.username} zur Gruppe {group.name} hinzugefügt")
        return {"success": True, "message": f"Benutzer zur Gruppe hinzugefügt"}
    else:
        return {"success": False, "message": "Benutzer ist bereits in dieser Gruppe"}

@router.post("/api/user/{user_id}/remove_from_group/{group_id}")
async def remove_user_from_group(user_id: int, group_id: int, db: Session = Depends(get_db)):
    """API zum Entfernen eines Benutzers aus einer Gruppe"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Benutzer mit ID {user_id} nicht gefunden"
        )
    
    group = db.query(Group).filter(Group.id == group_id).first()
    if not group:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Gruppe mit ID {group_id} nicht gefunden"
        )
    
    # Benutzer aus der Gruppe entfernen
    db.execute(
        user_groups.delete().where(
            user_groups.c.user_id == user_id,
            user_groups.c.group_id == group_id
        )
    )
    db.commit()
    
    logger.info(f"Benutzer {user.username} aus der Gruppe {group.name} entfernt")
    return {"success": True, "message": f"Benutzer aus der Gruppe entfernt"}

@router.get("/csv-import", response_class=HTMLResponse)
async def csv_import_page(request: Request, db: Session = Depends(get_db)):
    """CSV-Import-Seite"""
    return templates.TemplateResponse(
        "csv_import.html", 
        {
            "request": request, 
            "page_title": "CSV-Import",
            "api_base_url": config.API_BASE_URL,
            "nextcloud_url": config.NEXTCLOUD_FULL_URL
        }
    )