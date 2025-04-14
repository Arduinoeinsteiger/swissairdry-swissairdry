"""
SwissAirDry - API Status Router

Dieses Modul enthält die API-Endpunkte für den Status der API-Server.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from fastapi import APIRouter, Depends, HTTPException
from pydantic import BaseModel
from typing import Dict, List, Optional

import config
from auth import get_current_user

router = APIRouter(
    prefix="/api-status",
    tags=["api-status"],
    responses={404: {"description": "Not found"}},
)


class ApiStatusResponse(BaseModel):
    """API-Statusmodell für die Antwort."""
    primary_server: str
    backup_server: str
    active_server: str
    using_backup: bool


@router.get("/", response_model=ApiStatusResponse)
async def get_api_status():
    """
    Gibt den aktuellen Status der API-Server zurück.
    
    Dieser Endpunkt prüft, ob der primäre und der Backup-Server erreichbar sind,
    und gibt zurück, welcher Server aktuell verwendet wird.
    """
    # Prüfe den Status der Server
    status = config.get_api_status()
    
    # Füge zusätzliche Informationen hinzu
    status["using_backup"] = config.is_using_backup_server()
    
    return status


@router.post("/switch-to-backup")
async def switch_to_backup(user: Dict = Depends(get_current_user)):
    """
    Wechselt manuell zum Backup-Server.
    
    Dieser Endpunkt erfordert Administrator-Rechte.
    """
    # Prüfe, ob der Benutzer Administrator ist
    if user.get("role") != "admin":
        raise HTTPException(status_code=403, detail="Nur Administratoren können diese Aktion ausführen")
    
    # Setze globale Variablen auf den Backup-Server
    config._active_api_host = config.BACKUP_API_HOST
    config._active_api_port = config.BACKUP_API_PORT
    config._active_api_scheme = config.BACKUP_API_SCHEME
    config._active_api_prefix = config.BACKUP_API_PREFIX
    config._is_using_backup = True
    
    return {"message": "Auf Backup-Server umgeschaltet", "active_server": "backup"}


@router.post("/switch-to-primary")
async def switch_to_primary(user: Dict = Depends(get_current_user)):
    """
    Wechselt manuell zum primären Server.
    
    Dieser Endpunkt erfordert Administrator-Rechte.
    """
    # Prüfe, ob der Benutzer Administrator ist
    if user.get("role") != "admin":
        raise HTTPException(status_code=403, detail="Nur Administratoren können diese Aktion ausführen")
    
    # Prüfe, ob der primäre Server erreichbar ist
    if not config.check_api_availability(
        config.PRIMARY_API_HOST, 
        config.PRIMARY_API_PORT, 
        config.PRIMARY_API_SCHEME, 
        config.PRIMARY_API_PREFIX
    ):
        raise HTTPException(status_code=503, detail="Primärer Server ist nicht erreichbar")
    
    # Setze globale Variablen auf den primären Server
    config._active_api_host = config.PRIMARY_API_HOST
    config._active_api_port = config.PRIMARY_API_PORT
    config._active_api_scheme = config.PRIMARY_API_SCHEME
    config._active_api_prefix = config.PRIMARY_API_PREFIX
    config._is_using_backup = False
    
    return {"message": "Auf primären Server umgeschaltet", "active_server": "primary"}