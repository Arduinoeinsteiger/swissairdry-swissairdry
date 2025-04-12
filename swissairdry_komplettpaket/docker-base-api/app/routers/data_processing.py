"""
SwissAirDry - Datenverarbeitungs-API-Router

Dieser Router bietet Endpunkte für die Verarbeitung und Analyse von CSV-Daten,
insbesondere für den Import von Bestandsdaten aus CSV-Exporten.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import csv
import shutil
import pandas as pd
from typing import List, Dict, Any, Optional
from datetime import datetime
import tempfile
from fastapi import APIRouter, UploadFile, File, Depends, BackgroundTasks, HTTPException, Query
from fastapi.responses import JSONResponse
from sqlalchemy.orm import Session
from database import get_db
from config import UPLOAD_FOLDER
from models.csv_import_logs import CsvImportLog

router = APIRouter()

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
    # Dateipfad für die hochgeladene Datei erstellen
    upload_time = datetime.now().strftime("%Y%m%d%H%M%S")
    file_name = f"{upload_time}_{file.filename}"
    file_path = os.path.join(UPLOAD_FOLDER, file_name)
    
    # Datei speichern
    with open(file_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
    
    # Importprotokoll erstellen
    import_log = CsvImportLog(
        filename=file.filename,
        stored_path=file_path,
        status="Wird verarbeitet",
        import_type="Einzeldatei"
    )
    db.add(import_log)
    db.commit()
    db.refresh(import_log)
    
    # Hintergrundaufgabe für die Verarbeitung erstellen
    background_tasks.add_task(process_csv_file, file_path, import_log.id, db)
    
    return {
        "status": "ok", 
        "message": f"CSV-Datei {file.filename} erfolgreich hochgeladen und wird verarbeitet",
        "import_log_id": import_log.id
    }

@router.post("/upload-multiple-csv")
async def upload_multiple_csv(
    background_tasks: BackgroundTasks,
    files: List[UploadFile] = File(...),
    db: Session = Depends(get_db)
):
    """
    Lädt mehrere CSV-Dateien hoch und verarbeitet sie zusammen.
    
    Diese Methode ist besonders nützlich für zusammenhängende Datensätze, die in mehreren CSV-Dateien vorliegen.
    Sie erkennt automatisch SwissAirDry-CSV-Dateien und verarbeitet diese mit dem entsprechenden Prozessor.
    """
    file_paths = []
    file_names = []
    
    # Alle Dateien speichern
    upload_time = datetime.now().strftime("%Y%m%d%H%M%S")
    for i, file in enumerate(files):
        file_name = f"{upload_time}_{i+1}_{file.filename}"
        file_path = os.path.join(UPLOAD_FOLDER, file_name)
        
        with open(file_path, "wb") as buffer:
            shutil.copyfileobj(file.file, buffer)
        
        file_paths.append(file_path)
        file_names.append(file.filename)
    
    # Importprotokoll erstellen
    import_log = CsvImportLog(
        filename=", ".join(file_names),
        stored_path=", ".join(file_paths),
        status="Wird verarbeitet",
        import_type="Multi-Datei"
    )
    db.add(import_log)
    db.commit()
    db.refresh(import_log)
    
    # Hintergrundaufgabe für die Verarbeitung erstellen
    background_tasks.add_task(process_csv_files, file_paths, import_log.id, db)
    
    return {
        "status": "ok", 
        "message": f"{len(files)} CSV-Dateien erfolgreich hochgeladen und werden verarbeitet",
        "import_log_id": import_log.id
    }

@router.get("/csv-import-logs")
async def get_csv_import_logs(
    limit: int = 10,
    db: Session = Depends(get_db)
):
    """
    Ruft die Protokolleinträge für vergangene CSV-Imports ab.
    
    Dies ist nützlich, um den Status vorheriger Imports einzusehen und Probleme zu diagnostizieren.
    """
    logs = db.query(CsvImportLog).order_by(CsvImportLog.created_at.desc()).limit(limit).all()
    return [log.to_dict() for log in logs]

@router.get("/drying-progress/{job_id}")
async def get_drying_progress(job_id: str, db: Session = Depends(get_db)):
    """
    Ruft den Trocknungsfortschritt für einen bestimmten Auftrag ab.
    
    Diese Funktion analysiert die Feuchtigkeitsmessungen eines Auftrags und berechnet
    den Trocknungsfortschritt für verschiedene Materialien und Standorte.
    """
    # In einer vollständigen Implementierung würde hier Code folgen, um:
    # 1. Alle Messungen für den Auftrag abzurufen
    # 2. Die Trocknungskurve für verschiedene Materialien zu berechnen
    # 3. Den geschätzten Abschluss auf Basis des aktuellen Fortschritts zu berechnen
    
    # Zu Demonstrationszwecken wird ein Beispielergebnis zurückgegeben
    return {
        "job_id": job_id,
        "overall_progress": 67,  # Prozent
        "materials": [
            {
                "name": "Estrich",
                "progress": 82,
                "measurements": [
                    {"date": "2023-08-01", "value": 95},
                    {"date": "2023-08-10", "value": 60},
                    {"date": "2023-08-20", "value": 18}
                ]
            },
            {
                "name": "Wandputz",
                "progress": 53,
                "measurements": [
                    {"date": "2023-08-01", "value": 85},
                    {"date": "2023-08-10", "value": 70},
                    {"date": "2023-08-20", "value": 47}
                ]
            }
        ],
        "estimated_completion": "2023-09-05"
    }

@router.get("/energy-consumption/{job_id}")
async def get_energy_consumption(job_id: str, db: Session = Depends(get_db)):
    """
    Berechnet den Energieverbrauch für einen bestimmten Auftrag basierend auf den Geräteeinsätzen.
    
    Diese Funktion analysiert die Zählerstände der eingesetzten Geräte und berechnet
    den Gesamtenergieverbrauch sowie die Kosten basierend auf Standardtarifen.
    """
    # In einer vollständigen Implementierung würde hier Code folgen, um:
    # 1. Alle Geräteeinsätze für den Auftrag abzurufen
    # 2. Die Zählerstände auszuwerten und den Verbrauch zu berechnen
    # 3. Kosten basierend auf konfigurierten Tarifen zu berechnen
    
    # Zu Demonstrationszwecken wird ein Beispielergebnis zurückgegeben
    return {
        "job_id": job_id,
        "total_consumption": 1250.5,  # kWh
        "devices": [
            {
                "device_id": "AD-104",
                "device_type": "Luftentfeuchter",
                "consumption": 850.2,
                "operating_hours": 470,
                "cost": 255.06
            },
            {
                "device_id": "HE-33",
                "device_type": "Heizgerät",
                "consumption": 400.3,
                "operating_hours": 210,
                "cost": 120.09
            }
        ],
        "total_cost": 375.15,  # EUR
        "co2_equivalent": 625.25  # kg
    }

@router.get("/job-reports/{job_id}")
async def get_job_reports(job_id: str, db: Session = Depends(get_db)):
    """
    Ruft alle Berichte und zugehörigen Bilder für einen bestimmten Auftrag ab.
    """
    # In einer vollständigen Implementierung würde hier Code folgen, um:
    # 1. Alle Berichte für den Auftrag abzurufen
    # 2. Die zugehörigen Bilder zu sammeln
    # 3. Alles in einem strukturierten Format zurückzugeben
    
    # Zu Demonstrationszwecken wird ein Beispielergebnis zurückgegeben
    return {
        "job_id": job_id,
        "reports": [
            {
                "id": "REP-2023-123",
                "type": "Erstbesichtigung",
                "date": "2023-08-01",
                "author": "Max Mustermann",
                "content": "Wasserschaden im Badezimmer, Küche und Flur. Leckage am Wasseranschluss der Waschmaschine.",
                "images": [
                    {"id": "IMG-001", "path": "/uploads/job123/img001.jpg", "caption": "Wasserschaden Badezimmer"},
                    {"id": "IMG-002", "path": "/uploads/job123/img002.jpg", "caption": "Leckage Waschmaschinenanschluss"}
                ]
            },
            {
                "id": "REP-2023-145",
                "type": "Fortschrittsbericht",
                "date": "2023-08-10",
                "author": "Erika Musterfrau",
                "content": "Trocknungsfortschritt nach Woche 1 liegt im Soll. Messungen zeigen erste Erfolge.",
                "images": [
                    {"id": "IMG-023", "path": "/uploads/job123/img023.jpg", "caption": "Messgerät Badezimmer"}
                ]
            }
        ]
    }

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
    # In einer vollständigen Implementierung würde hier Code folgen, um:
    # 1. Filterbedingungen basierend auf dem Datumsbereich zu erstellen
    # 2. Daten aus verschiedenen Tabellen abzufragen und zu aggregieren
    # 3. Statistiken zu berechnen
    
    # Zu Demonstrationszwecken wird ein Beispielergebnis zurückgegeben
    return {
        "period": {
            "start_date": start_date or "2023-01-01",
            "end_date": end_date or "2023-08-31"
        },
        "jobs": {
            "total": 156,
            "in_progress": 42,
            "completed": 114,
            "average_duration": 18.5  # Tage
        },
        "devices": {
            "total": 254,
            "deployed": 187,
            "in_maintenance": 12,
            "available": 55,
            "utilization_rate": 73.6  # Prozent
        },
        "customers": {
            "total": 87,
            "new": 14,
            "recurring": 73
        },
        "financial": {
            "total_revenue": 245750.50,
            "average_job_value": 1574.68,
            "operating_costs": 156325.20,
            "profit_margin": 36.4  # Prozent
        }
    }

# Hilfsfunktionen für die Hintergrundverarbeitung

def process_csv_file(file_path: str, import_log_id: int, db: Session):
    """Verarbeitet eine einzelne CSV-Datei"""
    try:
        # CSV-Datei einlesen
        df = pd.read_csv(file_path, encoding='utf-8', sep=';')
        
        # Automatische Dateityperkennung basierend auf Spalten oder Dateinamen
        file_type = detect_csv_type(df, os.path.basename(file_path))
        
        # Je nach Dateityp unterschiedlichen Prozessor verwenden
        if file_type == "KUNDENSTAMM":
            process_customer_data(df, db)
        elif file_type == "GERAETESTAMMVERZEICHNISS":
            process_device_data(df, db)
        elif file_type == "AUFTRAGSPROTOKOLL":
            process_job_data(df, db)
        elif file_type == "MESSPROTOKOLLWERTERFASSUNG":
            process_measurement_data(df, db)
        elif file_type == "GERAETESTANDORTWECHSELPROTOKOLL":
            process_device_deployment_data(df, db)
        else:
            # Unbekannter Dateityp
            update_import_log(db, import_log_id, "Fehler", f"Unbekannter CSV-Dateityp: {file_type}")
            return
        
        # Importprotokoll aktualisieren
        update_import_log(db, import_log_id, "Abgeschlossen", f"CSV-Datei vom Typ {file_type} erfolgreich importiert")
        
    except Exception as e:
        # Fehler protokollieren
        update_import_log(db, import_log_id, "Fehler", str(e))
        raise

def process_csv_files(file_paths: List[str], import_log_id: int, db: Session):
    """Verarbeitet mehrere CSV-Dateien als zusammenhängenden Datensatz"""
    try:
        # Dateien nach Typ gruppieren
        file_types = {}
        for file_path in file_paths:
            try:
                df = pd.read_csv(file_path, encoding='utf-8', sep=';')
                file_type = detect_csv_type(df, os.path.basename(file_path))
                
                if file_type in file_types:
                    file_types[file_type].append((file_path, df))
                else:
                    file_types[file_type] = [(file_path, df)]
            except Exception as e:
                update_import_log(db, import_log_id, "Warnung", f"Fehler beim Lesen von {os.path.basename(file_path)}: {str(e)}")
        
        # Jede Dateigruppe verarbeiten
        for file_type, file_dfs in file_types.items():
            if file_type == "KUNDENSTAMM":
                for _, df in file_dfs:
                    process_customer_data(df, db)
            elif file_type == "GERAETESTAMMVERZEICHNISS":
                for _, df in file_dfs:
                    process_device_data(df, db)
            elif file_type == "AUFTRAGSPROTOKOLL":
                for _, df in file_dfs:
                    process_job_data(df, db)
            elif file_type == "MESSPROTOKOLLWERTERFASSUNG":
                for _, df in file_dfs:
                    process_measurement_data(df, db)
            elif file_type == "GERAETESTANDORTWECHSELPROTOKOLL":
                for _, df in file_dfs:
                    process_device_deployment_data(df, db)
            else:
                update_import_log(db, import_log_id, "Warnung", f"Unbekannter CSV-Dateityp: {file_type}")
        
        # Importprotokoll aktualisieren
        update_import_log(db, import_log_id, "Abgeschlossen", f"{len(file_paths)} CSV-Dateien erfolgreich importiert")
        
    except Exception as e:
        # Fehler protokollieren
        update_import_log(db, import_log_id, "Fehler", str(e))
        raise

def detect_csv_type(df: pd.DataFrame, filename: str) -> str:
    """Erkennt den Typ der CSV-Datei basierend auf Spalten oder Dateinamen"""
    # Basierend auf Dateinamen
    if "KUNDENSTAMM" in filename:
        return "KUNDENSTAMM"
    elif "GERAETESTAMMVERZEICHNISS" in filename:
        return "GERAETESTAMMVERZEICHNISS"
    elif "AUFTRAGSPROTOKOLL" in filename:
        return "AUFTRAGSPROTOKOLL"
    elif "MESSPROTOKOLLWERTERFASSUNG" in filename:
        return "MESSPROTOKOLLWERTERFASSUNG"
    elif "GERAETESTANDORTWECHSELPROTOKOLL" in filename:
        return "GERAETESTANDORTWECHSELPROTOKOLL"
    
    # Basierend auf Spalten
    columns = set(df.columns)
    
    if {"KUNDENNUMMER", "FIRMENNAME", "ANSPRECHPARTNER"} <= columns:
        return "KUNDENSTAMM"
    elif {"GERAETNUMMER", "GERAETETYP", "SERIENNUMMER"} <= columns:
        return "GERAETESTAMMVERZEICHNISS"
    elif {"AUFTRAGSNUMMER", "KUNDENNUMMER", "SCHADENART"} <= columns:
        return "AUFTRAGSPROTOKOLL"
    elif {"MESSPROTOKOLLNUMMER", "AUFTRAGSNUMMER", "MESSPUNKT"} <= columns:
        return "MESSPROTOKOLLWERTERFASSUNG"
    elif {"GERAETNUMMER", "STANDORT", "EINSATZDATUM"} <= columns:
        return "GERAETESTANDORTWECHSELPROTOKOLL"
    
    # Standardwert, wenn kein Typ erkannt wird
    return "UNKNOWN"

def update_import_log(db: Session, import_log_id: int, status: str, message: str):
    """Aktualisiert den Status und die Nachricht eines Import-Logs"""
    import_log = db.query(CsvImportLog).filter(CsvImportLog.id == import_log_id).first()
    if import_log:
        import_log.status = status
        import_log.message = message
        import_log.updated_at = datetime.now()
        db.commit()

# Prozessoren für verschiedene CSV-Dateitypen
# In einer vollständigen Implementierung würden diese Funktionen die Daten in die entsprechenden
# Tabellen einfügen oder aktualisieren

def process_customer_data(df: pd.DataFrame, db: Session):
    """Verarbeitet Kundendaten aus einer CSV-Datei"""
    # Implementation würde hier folgen
    pass

def process_device_data(df: pd.DataFrame, db: Session):
    """Verarbeitet Gerätedaten aus einer CSV-Datei"""
    # Implementation würde hier folgen
    pass

def process_job_data(df: pd.DataFrame, db: Session):
    """Verarbeitet Auftragsdaten aus einer CSV-Datei"""
    # Implementation würde hier folgen
    pass

def process_measurement_data(df: pd.DataFrame, db: Session):
    """Verarbeitet Messdaten aus einer CSV-Datei"""
    # Implementation würde hier folgen
    pass

def process_device_deployment_data(df: pd.DataFrame, db: Session):
    """Verarbeitet Geräteeinsatzdaten aus einer CSV-Datei"""
    # Implementation würde hier folgen
    pass