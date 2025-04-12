"""
SwissAirDry - Admin UI Router

Dieser Router stellt die Admin-Benutzeroberfläche für das SwissAirDry-System bereit,
inklusive Benutzer- und Gruppenverwaltung, sowie CSV-Import-Funktionen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import List, Optional, Dict, Any
from fastapi import APIRouter, Request, Depends, HTTPException, Form, status
from fastapi.responses import HTMLResponse, JSONResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from sqlalchemy.exc import IntegrityError

from database import get_db
from models.users import User
from models.groups import Group, ApiPermission, DashboardModule

router = APIRouter()
templates = Jinja2Templates(directory="templates")

@router.get("/", response_class=HTMLResponse)
async def admin_home(request: Request, db: Session = Depends(get_db)):
    """Admin-Dashboard-Startseite"""
    return templates.TemplateResponse(
        "admin/index.html", {"request": request, "active_tab": "dashboard"}
    )

@router.get("/users", response_class=HTMLResponse)
async def user_management(request: Request, db: Session = Depends(get_db)):
    """Benutzerverwaltung"""
    users = db.query(User).order_by(User.username).all()
    groups = db.query(Group).order_by(Group.name).all()
    
    return templates.TemplateResponse(
        "admin/users.html", 
        {
            "request": request, 
            "active_tab": "users", 
            "users": users,
            "groups": groups
        }
    )

@router.get("/groups", response_class=HTMLResponse)
async def group_management(request: Request, db: Session = Depends(get_db)):
    """Gruppenverwaltung"""
    groups = db.query(Group).order_by(Group.name).all()
    permissions = db.query(ApiPermission).order_by(ApiPermission.endpoint).all()
    modules = db.query(DashboardModule).order_by(DashboardModule.name).all()
    
    return templates.TemplateResponse(
        "admin/groups.html", 
        {
            "request": request, 
            "active_tab": "groups", 
            "groups": groups,
            "permissions": permissions,
            "modules": modules
        }
    )

@router.get("/users/{user_id}", response_class=HTMLResponse)
async def edit_user(user_id: int, request: Request, db: Session = Depends(get_db)):
    """Benutzer bearbeiten"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(status_code=404, detail="Benutzer nicht gefunden")
    
    groups = db.query(Group).order_by(Group.name).all()
    user_groups = [g.id for g in user.groups]
    
    return templates.TemplateResponse(
        "admin/edit_user.html", 
        {
            "request": request, 
            "active_tab": "users", 
            "user": user,
            "groups": groups,
            "user_groups": user_groups
        }
    )

@router.post("/api/users/{user_id}/groups")
async def update_user_groups(
    user_id: int, 
    group_ids: List[int] = [], 
    db: Session = Depends(get_db)
):
    """Aktualisiert die Gruppenzuweisungen eines Benutzers"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(status_code=404, detail="Benutzer nicht gefunden")
    
    # Aktuelle Gruppen löschen
    user.groups = []
    
    # Neue Gruppen zuweisen
    for group_id in group_ids:
        group = db.query(Group).filter(Group.id == group_id).first()
        if group:
            user.groups.append(group)
    
    db.commit()
    return {"status": "ok", "message": f"Gruppen für Benutzer {user.username} aktualisiert"}

@router.get("/api/users")
async def get_users(db: Session = Depends(get_db)):
    """API zum Abrufen aller Benutzer mit ihren Gruppen"""
    users = db.query(User).all()
    return [user.to_dict(include_groups=True) for user in users]

@router.get("/api/groups")
async def get_groups(db: Session = Depends(get_db)):
    """API zum Abrufen aller Gruppen"""
    groups = db.query(Group).all()
    return [group.to_dict() for group in groups]

@router.get("/api/users/{user_id}/groups")
async def get_user_groups(user_id: int, db: Session = Depends(get_db)):
    """API zum Abrufen der Gruppen eines Benutzers"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(status_code=404, detail="Benutzer nicht gefunden")
    
    return [group.to_dict() for group in user.groups]

@router.post("/api/users/{user_id}/groups/{group_id}")
async def add_user_to_group(user_id: int, group_id: int, db: Session = Depends(get_db)):
    """API zum Hinzufügen eines Benutzers zu einer Gruppe"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(status_code=404, detail="Benutzer nicht gefunden")
    
    group = db.query(Group).filter(Group.id == group_id).first()
    if not group:
        raise HTTPException(status_code=404, detail="Gruppe nicht gefunden")
    
    if group not in user.groups:
        user.groups.append(group)
        db.commit()
        return {"status": "ok", "message": f"Benutzer {user.username} zur Gruppe {group.name} hinzugefügt"}
    else:
        return {"status": "warning", "message": f"Benutzer {user.username} ist bereits Mitglied der Gruppe {group.name}"}

@router.delete("/api/users/{user_id}/groups/{group_id}")
async def remove_user_from_group(user_id: int, group_id: int, db: Session = Depends(get_db)):
    """API zum Entfernen eines Benutzers aus einer Gruppe"""
    user = db.query(User).filter(User.id == user_id).first()
    if not user:
        raise HTTPException(status_code=404, detail="Benutzer nicht gefunden")
    
    group = db.query(Group).filter(Group.id == group_id).first()
    if not group:
        raise HTTPException(status_code=404, detail="Gruppe nicht gefunden")
    
    if group in user.groups:
        user.groups.remove(group)
        db.commit()
        return {"status": "ok", "message": f"Benutzer {user.username} aus Gruppe {group.name} entfernt"}
    else:
        return {"status": "warning", "message": f"Benutzer {user.username} ist kein Mitglied der Gruppe {group.name}"}

@router.get("/csv-import", response_class=HTMLResponse)
async def csv_import_page(request: Request, db: Session = Depends(get_db)):
    """CSV-Import-Seite"""
    return templates.TemplateResponse(
        "admin/csv_import.html", {"request": request, "active_tab": "csv_import"}
    )