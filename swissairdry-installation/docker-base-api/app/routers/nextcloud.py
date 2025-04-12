"""
SwissAirDry - Nextcloud-Integrations-Router

Dieser Router stellt die Nextcloud-Integrationsendpunkte für das SwissAirDry-System bereit.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import requests
from typing import Dict, Any, List, Optional
from fastapi import APIRouter, Depends, HTTPException, Form, UploadFile, File, Query
from fastapi.responses import JSONResponse
from sqlalchemy.orm import Session

import config
from database import get_db

router = APIRouter(tags=["Nextcloud Integration"])

@router.get("/status")
async def get_nextcloud_status():
    """API zum Abrufen des Nextcloud-Verbindungsstatus"""
    nextcloud_url = os.environ.get("NEXTCLOUD_URL")
    nextcloud_username = os.environ.get("NEXTCLOUD_USERNAME")
    nextcloud_password = os.environ.get("NEXTCLOUD_PASSWORD")
    
    if not all([nextcloud_url, nextcloud_username, nextcloud_password]):
        return {
            "status": "warning",
            "message": "Nextcloud-Integration nicht konfiguriert"
        }
    
    try:
        # Versuch, die Nextcloud-Instanz zu erreichen
        response = requests.get(
            f"{nextcloud_url}/status.php",
            timeout=5
        )
        if response.status_code == 200:
            return {
                "status": "ok",
                "message": "Nextcloud-Verbindung hergestellt",
                "nextcloud_url": nextcloud_url,
                "version": response.json().get("version", "Unbekannt")
            }
        else:
            return {
                "status": "error",
                "message": f"Nextcloud-Verbindungsfehler: {response.status_code}",
                "nextcloud_url": nextcloud_url
            }
    except Exception as e:
        return {
            "status": "error",
            "message": f"Nextcloud-Verbindungsfehler: {str(e)}",
            "nextcloud_url": nextcloud_url
        }

@router.post("/upload")
async def upload_to_nextcloud(
    path: str = Form(...),
    file: UploadFile = File(...),
    db: Session = Depends(get_db)
):
    """API zum Hochladen einer Datei zu Nextcloud"""
    nextcloud_url = os.environ.get("NEXTCLOUD_URL")
    nextcloud_username = os.environ.get("NEXTCLOUD_USERNAME")
    nextcloud_password = os.environ.get("NEXTCLOUD_PASSWORD")
    
    if not all([nextcloud_url, nextcloud_username, nextcloud_password]):
        raise HTTPException(status_code=400, detail="Nextcloud-Integration nicht konfiguriert")
    
    try:
        # WebDAV-URL für Nextcloud
        webdav_url = f"{nextcloud_url}/remote.php/dav/files/{nextcloud_username}/{path}/{file.filename}"
        
        # Dateiinhalt lesen
        content = await file.read()
        
        # Datei hochladen
        response = requests.put(
            webdav_url,
            data=content,
            auth=(nextcloud_username, nextcloud_password),
            headers={"Content-Type": "application/octet-stream"}
        )
        
        if response.status_code in [201, 204]:
            return {
                "status": "success",
                "message": f"Datei '{file.filename}' erfolgreich hochgeladen",
                "path": f"{path}/{file.filename}",
                "size": len(content)
            }
        else:
            raise HTTPException(
                status_code=response.status_code,
                detail=f"Nextcloud-Fehler: {response.text}"
            )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Nextcloud-Upload-Fehler: {str(e)}")

@router.get("/files")
async def list_nextcloud_files(
    path: str = Query(...),
    db: Session = Depends(get_db)
):
    """API zum Auflisten von Dateien in einem Nextcloud-Verzeichnis"""
    nextcloud_url = os.environ.get("NEXTCLOUD_URL")
    nextcloud_username = os.environ.get("NEXTCLOUD_USERNAME")
    nextcloud_password = os.environ.get("NEXTCLOUD_PASSWORD")
    
    if not all([nextcloud_url, nextcloud_username, nextcloud_password]):
        raise HTTPException(status_code=400, detail="Nextcloud-Integration nicht konfiguriert")
    
    try:
        # WebDAV-URL für Nextcloud
        webdav_url = f"{nextcloud_url}/remote.php/dav/files/{nextcloud_username}/{path}"
        
        # PROPFIND-Anfrage zum Auflisten von Dateien
        response = requests.request(
            "PROPFIND",
            webdav_url,
            auth=(nextcloud_username, nextcloud_password),
            headers={"Depth": "1"}
        )
        
        if response.status_code == 207:  # Multi-Status-Antwort
            # XML-Antwort parsen und Dateiliste zurückgeben
            # In einer echten Implementierung würde hier ein XML-Parser verwendet
            
            # Vereinfachte Antwort
            return {
                "status": "success",
                "path": path,
                "files": [
                    {"name": "beispiel1.pdf", "type": "file", "size": 1024, "modified": "2025-04-10T15:30:00Z"},
                    {"name": "beispiel2.jpg", "type": "file", "size": 2048, "modified": "2025-04-11T09:45:00Z"},
                    {"name": "unterordner", "type": "directory", "modified": "2025-04-09T14:20:00Z"}
                ]
            }
        else:
            raise HTTPException(
                status_code=response.status_code,
                detail=f"Nextcloud-Fehler: {response.text}"
            )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Nextcloud-Listenfehler: {str(e)}")

@router.get("/share")
async def create_nextcloud_share(
    path: str = Query(...),
    password: Optional[str] = None,
    expiration: Optional[str] = None,  # Format: YYYY-MM-DD
    db: Session = Depends(get_db)
):
    """API zum Erstellen eines öffentlichen Nextcloud-Links"""
    nextcloud_url = os.environ.get("NEXTCLOUD_URL")
    nextcloud_username = os.environ.get("NEXTCLOUD_USERNAME")
    nextcloud_password = os.environ.get("NEXTCLOUD_PASSWORD")
    
    if not all([nextcloud_url, nextcloud_username, nextcloud_password]):
        raise HTTPException(status_code=400, detail="Nextcloud-Integration nicht konfiguriert")
    
    try:
        # OCS API-URL für Nextcloud
        ocs_url = f"{nextcloud_url}/ocs/v2.php/apps/files_sharing/api/v1/shares"
        
        # Anfrageparameter
        data = {
            "path": path,
            "shareType": 3,  # Öffentlicher Link
        }
        
        if password:
            data["password"] = password
        
        if expiration:
            data["expireDate"] = expiration
        
        # Teilen erstellen
        response = requests.post(
            ocs_url,
            data=data,
            auth=(nextcloud_username, nextcloud_password),
            headers={"OCS-APIRequest": "true", "Content-Type": "application/x-www-form-urlencoded"}
        )
        
        if response.status_code in [200, 201]:
            # In einer echten Implementierung würde hier die XML-Antwort geparst
            
            # Vereinfachte Antwort
            return {
                "status": "success",
                "message": f"Link für '{path}' erstellt",
                "url": f"{nextcloud_url}/index.php/s/Beispiellink",
                "expiration": expiration
            }
        else:
            raise HTTPException(
                status_code=response.status_code,
                detail=f"Nextcloud-Fehler: {response.text}"
            )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Nextcloud-Share-Fehler: {str(e)}")