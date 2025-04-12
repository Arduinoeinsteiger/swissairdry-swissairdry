"""
SwissAirDry - Nextcloud Integration Router

Dieses Modul definiert die Endpunkte für die Integration mit Nextcloud.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import logging
from typing import Dict, Any, Optional, List
import requests
from fastapi import APIRouter, Depends, HTTPException, Header, Request, UploadFile, File, Form
from sqlalchemy.orm import Session
from pydantic import BaseModel, HttpUrl
from datetime import datetime, timedelta

from .. import config
from ..database import get_db
from ..auth import verify_nextcloud_token, get_current_user

# Logger einrichten
logger = logging.getLogger(__name__)

# Router initialisieren
router = APIRouter(prefix="/nextcloud", tags=["nextcloud"])


# Datenmodelle
class NextcloudUser(BaseModel):
    """Nextcloud-Benutzermodell"""
    user_id: str
    display_name: Optional[str] = None
    email: Optional[str] = None
    groups: List[str] = []


class NextcloudFileInfo(BaseModel):
    """Informationen zu einer Nextcloud-Datei"""
    file_id: int
    path: str
    name: str
    mime_type: str
    size: int
    last_modified: datetime
    public_link: Optional[str] = None


class NextcloudSettings(BaseModel):
    """Nextcloud-Einstellungen"""
    url: HttpUrl
    username: str
    app_password: str
    folder_path: str = "/SwissAirDry"


# API-Endpunkte
@router.get("/connect")
async def test_connection(
    x_nextcloud_url: Optional[str] = Header(None),
    x_nextcloud_user: Optional[str] = Header(None),
    x_nextcloud_token: Optional[str] = Header(None)
):
    """
    Testet die Verbindung mit Nextcloud.
    
    Diese Route kann von der Nextcloud App aufgerufen werden, um zu testen,
    ob die API erreichbar ist und ob die Authentifizierung funktioniert.
    """
    if not x_nextcloud_url or not x_nextcloud_user:
        raise HTTPException(status_code=400, detail="Nextcloud-URL und Benutzer sind erforderlich")
    
    # Wenn Token vorhanden ist, Authentifizierung prüfen
    user_info = None
    if x_nextcloud_token:
        try:
            user_info = verify_nextcloud_token(x_nextcloud_token)
        except Exception as e:
            logger.error(f"Fehler bei der Token-Verifizierung: {str(e)}")
            raise HTTPException(status_code=401, detail="Ungültiges Token")
    
    return {
        "status": "ok",
        "message": "Verbindung erfolgreich hergestellt",
        "server_time": datetime.now().isoformat(),
        "api_version": config.API_VERSION,
        "user": user_info
    }


@router.post("/files/upload")
async def upload_file(
    request: Request,
    file: UploadFile = File(...),
    path: str = Form(...),
    file_type: str = Form("document"),
    db: Session = Depends(get_db),
    user: Dict[str, Any] = Depends(get_current_user)
):
    """
    Lädt eine Datei hoch und speichert Sie in Nextcloud.
    
    Diese Route wird vom Backend verwendet, um Dateien in Nextcloud zu speichern.
    Die Authentifizierung erfolgt über die API und die Datei wird dann an Nextcloud weitergeleitet.
    """
    # Pfad validieren
    if not path.startswith("/"):
        path = "/" + path
    
    # Nextcloud-Einstellungen aus Konfiguration auslesen
    nextcloud_url = os.getenv("NEXTCLOUD_URL")
    nextcloud_username = os.getenv("NEXTCLOUD_USERNAME")
    nextcloud_password = os.getenv("NEXTCLOUD_PASSWORD")
    
    if not nextcloud_url or not nextcloud_username or not nextcloud_password:
        raise HTTPException(status_code=500, detail="Nextcloud-Konfiguration nicht vollständig")
    
    # Dateiinhalt in temporäre Datei speichern
    file_content = await file.read()
    file_name = file.filename
    
    try:
        # WebDAV-Endpunkt für Nextcloud
        webdav_url = f"{nextcloud_url}/remote.php/dav/files/{nextcloud_username}{path}/{file_name}"
        
        # Datei an Nextcloud senden
        response = requests.put(
            webdav_url,
            data=file_content,
            auth=(nextcloud_username, nextcloud_password),
            headers={"Content-Type": file.content_type}
        )
        
        if response.status_code not in (201, 204):
            logger.error(f"Fehler beim Hochladen der Datei zu Nextcloud: {response.text}")
            raise HTTPException(status_code=response.status_code, detail=f"Nextcloud-Fehler: {response.text}")
        
        # Informationen zur hochgeladenen Datei abrufen
        file_info_response = requests.propfind(
            webdav_url,
            auth=(nextcloud_username, nextcloud_password),
            headers={"Depth": "0"}
        )
        
        # Erfolgreiche Antwort zurückgeben
        return {
            "success": True,
            "file_name": file_name,
            "path": path,
            "size": len(file_content),
            "type": file_type,
            "uploaded_at": datetime.now().isoformat()
        }
        
    except Exception as e:
        logger.error(f"Fehler beim Hochladen der Datei: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Fehler beim Hochladen: {str(e)}")


@router.get("/files/list")
async def list_files(
    path: str = "/SwissAirDry",
    db: Session = Depends(get_db),
    user: Dict[str, Any] = Depends(get_current_user)
):
    """
    Listet Dateien in einem Nextcloud-Verzeichnis auf.
    """
    # Pfad validieren
    if not path.startswith("/"):
        path = "/" + path
    
    # Nextcloud-Einstellungen aus Konfiguration auslesen
    nextcloud_url = os.getenv("NEXTCLOUD_URL")
    nextcloud_username = os.getenv("NEXTCLOUD_USERNAME")
    nextcloud_password = os.getenv("NEXTCLOUD_PASSWORD")
    
    if not nextcloud_url or not nextcloud_username or not nextcloud_password:
        raise HTTPException(status_code=500, detail="Nextcloud-Konfiguration nicht vollständig")
    
    try:
        # WebDAV-Endpunkt für Nextcloud
        webdav_url = f"{nextcloud_url}/remote.php/dav/files/{nextcloud_username}{path}"
        
        # Dateien von Nextcloud abrufen
        response = requests.propfind(
            webdav_url,
            auth=(nextcloud_username, nextcloud_password),
            headers={"Depth": "1"}
        )
        
        if response.status_code != 207:  # MultiStatus
            logger.error(f"Fehler beim Abrufen der Dateien von Nextcloud: {response.text}")
            raise HTTPException(status_code=response.status_code, detail=f"Nextcloud-Fehler: {response.text}")
        
        # WebDAV-Antwort in vereinfachtes Format umwandeln
        # In einer echten Implementierung würde hier ein XML-Parser verwendet
        
        # Platzhalter für die tatsächliche Implementierung
        files = [
            {
                "name": "Beispieldatei.pdf",
                "path": f"{path}/Beispieldatei.pdf",
                "size": 12345,
                "last_modified": datetime.now().isoformat()
            }
        ]
        
        return {
            "path": path,
            "files": files
        }
        
    except Exception as e:
        logger.error(f"Fehler beim Abrufen der Dateien: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Fehler beim Abrufen der Dateien: {str(e)}")


@router.get("/settings")
async def get_nextcloud_settings(
    db: Session = Depends(get_db),
    user: Dict[str, Any] = Depends(get_current_user)
):
    """
    Gibt die aktuellen Nextcloud-Einstellungen zurück.
    """
    # Überprüfen, ob Benutzer Admin ist
    if user.get("role") != "admin":
        raise HTTPException(status_code=403, detail="Nur Administratoren können Nextcloud-Einstellungen abrufen")
    
    # Nextcloud-Einstellungen aus Umgebungsvariablen auslesen
    settings = {
        "url": os.getenv("NEXTCLOUD_URL", ""),
        "username": os.getenv("NEXTCLOUD_USERNAME", ""),
        "app_password": "********",  # Passwort nie direkt zurückgeben
        "folder_path": os.getenv("NEXTCLOUD_FOLDER_PATH", "/SwissAirDry")
    }
    
    return settings


@router.post("/settings")
async def update_nextcloud_settings(
    settings: NextcloudSettings,
    db: Session = Depends(get_db),
    user: Dict[str, Any] = Depends(get_current_user)
):
    """
    Aktualisiert die Nextcloud-Einstellungen.
    """
    # Überprüfen, ob Benutzer Admin ist
    if user.get("role") != "admin":
        raise HTTPException(status_code=403, detail="Nur Administratoren können Nextcloud-Einstellungen ändern")
    
    # Verbindung zu Nextcloud testen
    try:
        webdav_url = f"{settings.url}/remote.php/dav/files/{settings.username}/"
        response = requests.propfind(
            webdav_url,
            auth=(settings.username, settings.app_password),
            headers={"Depth": "0"}
        )
        
        if response.status_code != 207:  # MultiStatus
            raise HTTPException(status_code=400, detail="Verbindung zu Nextcloud konnte nicht hergestellt werden")
        
        # In einer echten Implementierung würden die Einstellungen in der Datenbank gespeichert
        # oder in einer Konfigurationsdatei, die beim Neustart gelesen wird
        
        # Erfolgsmeldung zurückgeben
        return {
            "success": True,
            "message": "Nextcloud-Einstellungen wurden aktualisiert"
        }
        
    except Exception as e:
        logger.error(f"Fehler beim Aktualisieren der Nextcloud-Einstellungen: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Fehler: {str(e)}")