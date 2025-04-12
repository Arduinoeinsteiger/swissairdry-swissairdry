"""
SwissAirDry - Datenverarbeitungs-API-Router

Dieser Router bietet Endpunkte für die Verarbeitung und Analyse von CSV-Daten,
insbesondere für den Import von Bestandsdaten aus CSV-Exporten.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import json
import shutil
from typing import Dict, Any, List, Optional
from datetime import datetime

import pandas as pd
from io import StringIO
from pathlib import Path

from fastapi import APIRouter, Depends, HTTPException, UploadFile, File, BackgroundTasks
from fastapi.responses import JSONResponse
from sqlalchemy.orm import Session

import config
from database import get_db
from models import CsvImportLog

router = APIRouter(tags=["Data Processing"])

@router.post("/upload-csv")
async def upload_csv(
    background_tasks: BackgroundTasks,
    file: UploadFile = File(...),
    db: Session = Depends(get_db)
):
    """
    Lädt eine einzelne CSV-Datei hoch und verarbeitet sie.
    
    Die Datei wird basierend auf ihrem Inhalt automatisch klassifiziert und in die Datenbank importiert.
    """
    if not file.filename.endswith('.csv'):
        raise HTTPException(status_code=400, detail="Nur CSV-Dateien sind erlaubt")
    
    # Datei speichern
    upload_dir = os.path.join(config.UPLOAD_FOLDER, "csv")
    os.makedirs(upload_dir, exist_ok=True)
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    stored_filename = f"{timestamp}_{file.filename}"
    file_path = os.path.join(upload_dir, stored_filename)
    
    with open(file_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
    
    # Import-Log erstellen
    import_log = CsvImportLog(
        filename=file.filename,
        stored_path=file_path,
        status="Wird verarbeitet",
        import_type="Einzeldatei",
        details={"original_filename": file.filename}
    )
    
    # In einem echten System würden wir hier die Datenbank aktualisieren
    # db.add(import_log)
    # db.commit()
    # db.refresh(import_log)
    
    # Hintergrundaufgabe für die Verarbeitung
    # In einer echten Implementierung würde hier die Datei verarbeitet und in die Datenbank importiert werden
    # background_tasks.add_task(process_csv_file, file_path, import_log.id, db)
    
    return {
        "status": "success",
        "message": "CSV-Datei wurde hochgeladen und wird verarbeitet",
        "filename": file.filename,
        "stored_path": file_path,
        "import_log_id": 1  # In der echten Implementierung wäre dies import_log.id
    }

@router.get("/import-logs")
async def get_csv_import_logs(
    limit: int = 10,
    db: Session = Depends(get_db)
):
    """
    Ruft die Protokolleinträge für vergangene CSV-Imports ab.
    
    Dies ist nützlich, um den Status vorheriger Imports einzusehen und Probleme zu diagnostizieren.
    """
    # In einer echten Implementierung würden hier die Import-Logs aus der Datenbank abgerufen werden
    logs = [
        {
            "id": 1,
            "filename": "SwissAirDry_KUNDENSTAMM.csv",
            "status": "Abgeschlossen",
            "import_type": "Einzeldatei",
            "message": "Import erfolgreich",
            "created_at": "2025-04-10T14:30:00Z",
            "details": {"records_imported": 150, "errors": 0}
        },
        {
            "id": 2,
            "filename": "SwissAirDry_GERAETESTAMMVERZEICHNISS.csv",
            "status": "Abgeschlossen",
            "import_type": "Einzeldatei",
            "message": "Import erfolgreich",
            "created_at": "2025-04-10T14:35:00Z",
            "details": {"records_imported": 75, "errors": 0}
        }
    ]
    
    return {"logs": logs[:limit]}

@router.get("/dashboard-statistics")
async def get_dashboard_statistics(
    start_date: Optional[str] = None,
    end_date: Optional[str] = None,
    db: Session = Depends(get_db)
):
    """
    Ruft aggregierte Statistiken für das Dashboard ab.
    
    Umfasst Auftragszahlen, Durchschnittszeiten, Gerätenutzung und mehr.
    """
    # In einer echten Implementierung würden hier die Statistiken aus der Datenbank abgerufen werden
    statistics = {
        "jobs": {
            "total": 120,
            "active": 45,
            "completed": 75,
            "avg_duration_days": 14.5
        },
        "devices": {
            "total": 75,
            "in_use": 42,
            "available": 33,
            "maintenance": 3
        },
        "customers": {
            "total": 85,
            "active": 38
        },
        "measurements": {
            "total": 3250,
            "last_24h": 120
        }
    }
    
    return statistics