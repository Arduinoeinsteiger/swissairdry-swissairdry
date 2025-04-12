"""
SwissAirDry - Datenverarbeitungs-API-Router

Dieser Router bietet Endpunkte für die Verarbeitung und Analyse von CSV-Daten,
insbesondere für den Import von Bestandsdaten aus CSV-Exporten.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from fastapi import APIRouter, File, UploadFile, Form, HTTPException, Depends, BackgroundTasks
from fastapi.responses import JSONResponse
from typing import List, Optional, Dict, Any
from sqlalchemy.orm import Session
import os
import tempfile
import shutil
import logging
from datetime import datetime, timedelta
import pandas as pd

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from database import get_db
from utils.csv_processor import CSVProcessor
import models

# Logger konfigurieren
logger = logging.getLogger(__name__)

router = APIRouter(
    prefix="/api/data",
    tags=["Datenverarbeitung"]
)

@router.post("/csv/upload", summary="CSV-Datei hochladen und verarbeiten")
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
    
    # Temporäre Datei zum Speichern der hochgeladenen CSV
    temp_dir = tempfile.mkdtemp()
    try:
        temp_file_path = os.path.join(temp_dir, file.filename)
        
        # Datei speichern
        with open(temp_file_path, "wb") as temp_file:
            content = await file.read()
            temp_file.write(content)
        
        # Datei verarbeiten
        processor = CSVProcessor(db)
        success, errors = processor.process_directory(temp_dir)
        
        return {
            "success": success > 0,
            "message": f"Datei verarbeitet: {success} Erfolge, {errors} Fehler",
            "filename": file.filename
        }
    
    except Exception as e:
        logger.error(f"Fehler bei der Verarbeitung von {file.filename}: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Fehler bei der Verarbeitung: {str(e)}")
    
    finally:
        # Temporäres Verzeichnis aufräumen
        shutil.rmtree(temp_dir)

@router.post("/csv/upload-batch", summary="Mehrere CSV-Dateien hochladen und verarbeiten")
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
    if not files:
        raise HTTPException(status_code=400, detail="Keine Dateien hochgeladen")
    
    # Prüfen, ob alle Dateien CSV sind
    for file in files:
        if not file.filename.endswith('.csv'):
            raise HTTPException(status_code=400, detail=f"Datei {file.filename} ist keine CSV-Datei")
    
    # Temporäres Verzeichnis für alle Dateien
    temp_dir = tempfile.mkdtemp()
    try:
        # Alle Dateien speichern
        for file in files:
            temp_file_path = os.path.join(temp_dir, file.filename)
            with open(temp_file_path, "wb") as temp_file:
                content = await file.read()
                temp_file.write(content)
        
        # Verarbeitung im Hintergrund starten
        def process_csv_files():
            try:
                processor = CSVProcessor(next(get_db()))
                success, errors = processor.process_directory(temp_dir)
                
                # Speichere Import-Informationen in der Datenbank für spätere Abfrage
                try:
                    db_session = next(get_db())
                    for filename in os.listdir(temp_dir):
                        if filename.endswith('.csv'):
                            log_entry = models.SystemLog(
                                log_type="csv_import",
                                description=f"CSV-Import: {filename}",
                                timestamp=datetime.utcnow(),
                                details=f"Import erfolgreich: {success > 0}, Fehler: {errors}"
                            )
                            db_session.add(log_entry)
                    db_session.commit()
                except Exception as log_error:
                    logger.error(f"Fehler beim Speichern der Import-Protokolle: {str(log_error)}")
                
                logger.info(f"Batch-Verarbeitung von {len(files)} Dateien abgeschlossen: {success} erfolgreich, {errors} Fehler")
            except Exception as e:
                logger.error(f"Fehler bei der Batch-Verarbeitung: {str(e)}")
            finally:
                # Temporäres Verzeichnis aufräumen
                shutil.rmtree(temp_dir)
        
        background_tasks.add_task(process_csv_files)
        
        return {
            "success": True,
            "message": f"Verarbeitung von {len(files)} Dateien gestartet",
            "files": [file.filename for file in files]
        }
    
    except Exception as e:
        # Bei Fehler aufräumen und Ausnahme weitergeben
        shutil.rmtree(temp_dir)
        logger.error(f"Fehler beim Hochladen der Dateien: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Fehler beim Hochladen: {str(e)}")
        
@router.get("/csv/import-logs", summary="CSV-Import-Protokolle abrufen")
async def get_csv_import_logs(
    limit: int = 10,
    db: Session = Depends(get_db)
):
    """
    Ruft die Protokolleinträge für vergangene CSV-Imports ab.
    
    Dies ist nützlich, um den Status vorheriger Imports einzusehen und Probleme zu diagnostizieren.
    """
    logs = db.query(models.SystemLog).filter(
        models.SystemLog.log_type == "csv_import"
    ).order_by(models.SystemLog.timestamp.desc()).limit(limit).all()
    
    result = []
    for log in logs:
        result.append({
            "id": log.id,
            "timestamp": log.timestamp,
            "description": log.description,
            "details": log.details
        })
    
    return {
        "logs": result,
        "count": len(result)
    }

@router.get("/job/{job_id}/drying-progress", summary="Trocknungsfortschritt eines Auftrags abrufen")
async def get_drying_progress(job_id: str, db: Session = Depends(get_db)):
    """
    Ruft den Trocknungsfortschritt für einen bestimmten Auftrag ab.
    
    Diese Funktion analysiert die Feuchtigkeitsmessungen eines Auftrags und berechnet
    den Trocknungsfortschritt für verschiedene Materialien und Standorte.
    """
    # Auftrag in der Datenbank suchen
    job = db.query(models.Job).filter(models.Job.name == job_id).first()
    if not job:
        raise HTTPException(status_code=404, detail=f"Auftrag {job_id} nicht gefunden")
    
    # Systemlogs mit Messdaten für diesen Auftrag abrufen
    logs = db.query(models.SystemLog).filter(
        models.SystemLog.message.like(f"Baustofffeuchtemessung für Auftrag {job_id}%") |
        models.SystemLog.message.like(f"Dämmschichtmessung für Auftrag {job_id}%")
    ).order_by(models.SystemLog.timestamp).all()
    
    if not logs:
        raise HTTPException(status_code=404, detail=f"Keine Feuchtigkeitsmessungen für Auftrag {job_id} gefunden")
    
    # Extrahiere Informationen aus den Log-Nachrichten
    results = []
    for log in logs:
        entry = {
            "timestamp": log.timestamp,
            "type": "Baustofffeuchte" if "Baustofffeuchtemessung" in log.message else "Dämmschicht",
            "measurements": []
        }
        
        lines = log.message.split('\n')
        
        # Materialtyp und Ort extrahieren
        material = None
        location = None
        for line in lines:
            if "Baustoff:" in line:
                material = line.split("Baustoff:")[1].strip()
            elif "Dämmstoffart:" in line:
                material = line.split("Dämmstoffart:")[1].strip()
            elif "Ort:" in line:
                location = line.split("Ort:")[1].strip()
        
        # Messwerte extrahieren
        measurements = {}
        for line in lines:
            if "Feuchtigkeit % (Mittel):" in line:
                try:
                    humidity = float(line.split("Feuchtigkeit % (Mittel):")[1].strip())
                    measurements["humidity"] = humidity
                except:
                    pass
            elif "Referenz Feuchtigkeit %:" in line:
                try:
                    ref_humidity = float(line.split("Referenz Feuchtigkeit %:")[1].strip())
                    measurements["reference_humidity"] = ref_humidity
                except:
                    pass
            elif "Temperatur °C:" in line or "Temperatur:" in line:
                try:
                    if "Temperatur °C:" in line:
                        temp = float(line.split("Temperatur °C:")[1].strip())
                    else:
                        temp = float(line.split("Temperatur:")[1].strip())
                    measurements["temperature"] = temp
                except:
                    pass
        
        # Fortschritt berechnen, wenn Referenz- und aktuelle Werte vorhanden sind
        if "humidity" in measurements and "reference_humidity" in measurements:
            reference = measurements["reference_humidity"]
            current = measurements["humidity"]
            
            # Verhältnis berechnen (je niedriger der Wert desto trockener)
            # Überschreiten des Referenzwerts bedeutet "feucht"
            # Erreichen des Referenzwerts bedeutet "trocken"
            if reference > 0:
                moisture_ratio = current / reference
                progress = max(0, min(100, (1 - moisture_ratio) * 100)) if moisture_ratio > 1 else 100
                is_dry = current <= reference
            else:
                progress = 0
                is_dry = False
            
            entry["material"] = material
            entry["location"] = location
            entry["current_humidity"] = current
            entry["reference_humidity"] = reference
            entry["progress"] = progress
            entry["is_dry"] = is_dry
            entry["temperature"] = measurements.get("temperature")
            
            results.append(entry)
    
    return results

@router.get("/job/{job_id}/energy-consumption", summary="Energieverbrauch eines Auftrags abrufen")
async def get_energy_consumption(job_id: str, db: Session = Depends(get_db)):
    """
    Berechnet den Energieverbrauch für einen bestimmten Auftrag basierend auf den Geräteeinsätzen.
    
    Diese Funktion analysiert die Zählerstände der eingesetzten Geräte und berechnet
    den Gesamtenergieverbrauch sowie die Kosten basierend auf Standardtarifen.
    """
    # Auftrag in der Datenbank suchen
    job = db.query(models.Job).filter(models.Job.name == job_id).first()
    if not job:
        raise HTTPException(status_code=404, detail=f"Auftrag {job_id} nicht gefunden")
    
    # Geräte-Auftrag-Verknüpfungen abrufen
    job_devices = db.query(models.JobDevice).filter(models.JobDevice.job_id == job.id).all()
    if not job_devices:
        raise HTTPException(status_code=404, detail=f"Keine Geräteeinsätze für Auftrag {job_id} gefunden")
    
    # Gerätedaten und Messungen für jeden Einsatz sammeln
    results = []
    for job_device in job_devices:
        device = db.query(models.Device).filter(models.Device.id == job_device.device_id).first()
        if not device:
            continue
        
        # Messungen für dieses Gerät finden
        measurements = db.query(models.Measurement).filter(
            models.Measurement.device_id == device.id,
            models.Measurement.timestamp >= job_device.start_date,
            models.Measurement.timestamp <= (job_device.end_date or datetime.utcnow())
        ).order_by(models.Measurement.timestamp).all()
        
        if not measurements:
            continue
        
        # Ersten und letzten Zählerstand finden
        first_reading = None
        last_reading = None
        
        for m in measurements:
            if m.energy_consumption is not None:
                if first_reading is None:
                    first_reading = m.energy_consumption
                last_reading = m.energy_consumption
        
        if first_reading is not None and last_reading is not None:
            # Energieverbrauch berechnen
            energy_consumed = max(0, last_reading - first_reading)
            
            # Standardkosten berechnen (0.35 CHF pro kWh)
            cost = energy_consumed * 0.35
            
            # Einsatzdauer berechnen
            start_date = job_device.start_date
            end_date = job_device.end_date or datetime.utcnow()
            duration_days = (end_date - start_date).days
            
            results.append({
                "device_id": device.id,
                "device_name": device.name,
                "device_type": device.type,
                "start_date": start_date,
                "end_date": end_date,
                "duration_days": duration_days,
                "energy_consumed_kwh": energy_consumed,
                "estimated_cost_chf": cost,
                "location": job_device.notes or "Unbekannt"
            })
    
    # Gesamtverbrauch und -kosten berechnen
    total_energy = sum(r["energy_consumed_kwh"] for r in results)
    total_cost = sum(r["estimated_cost_chf"] for r in results)
    
    return {
        "job_id": job_id,
        "devices": results,
        "total_energy_kwh": total_energy,
        "total_cost_chf": total_cost
    }

@router.get("/job/{job_id}/reports", summary="Berichte eines Auftrags abrufen")
async def get_job_reports(job_id: str, db: Session = Depends(get_db)):
    """
    Ruft alle Berichte und zugehörigen Bilder für einen bestimmten Auftrag ab.
    """
    # Auftrag in der Datenbank suchen
    job = db.query(models.Job).filter(models.Job.name == job_id).first()
    if not job:
        raise HTTPException(status_code=404, detail=f"Auftrag {job_id} nicht gefunden")
    
    # Berichte für diesen Auftrag abrufen
    reports = db.query(models.Report).filter(models.Report.job_id == job.id).all()
    if not reports:
        return {"job_id": job_id, "reports": []}
    
    # Berichte mit Bildern zusammenführen
    report_list = []
    for report in reports:
        images = db.query(models.ReportImage).filter(
            models.ReportImage.report_id == report.id
        ).order_by(models.ReportImage.order).all()
        
        report_data = {
            "id": report.id,
            "title": report.title,
            "content": report.content,
            "type": report.type,
            "created_at": report.created_at,
            "images": [
                {
                    "id": img.id,
                    "path": img.image_path,
                    "caption": img.caption,
                    "order": img.order
                }
                for img in images
            ]
        }
        
        report_list.append(report_data)
    
    return {
        "job_id": job_id,
        "reports": report_list
    }

@router.get("/statistics/dashboard", summary="Dashboard-Statistiken abrufen")
async def get_dashboard_statistics(
    start_date: Optional[str] = None,
    end_date: Optional[str] = None,
    db: Session = Depends(get_db)
):
    """
    Ruft aggregierte Statistiken für das Dashboard ab.
    
    Umfasst Auftragszahlen, Durchschnittszeiten, Gerätenutzung und mehr.
    """
    # Datumsfilter erstellen
    query_start = None
    query_end = None
    
    if start_date:
        try:
            query_start = datetime.strptime(start_date, "%Y-%m-%d")
        except:
            raise HTTPException(status_code=400, detail="Ungültiges Startdatum. Verwenden Sie das Format YYYY-MM-DD")
    
    if end_date:
        try:
            query_end = datetime.strptime(end_date, "%Y-%m-%d")
        except:
            raise HTTPException(status_code=400, detail="Ungültiges Enddatum. Verwenden Sie das Format YYYY-MM-DD")
    
    # Standardzeitraum: Letzte 30 Tage
    if not query_start:
        query_start = datetime.utcnow() - timedelta(days=30)
    
    if not query_end:
        query_end = datetime.utcnow()
    
    # 1. Auftragsstatistiken
    jobs_query = db.query(models.Job)
    
    # Filter anwenden
    if query_start:
        jobs_query = jobs_query.filter(models.Job.created_at >= query_start)
    if query_end:
        jobs_query = jobs_query.filter(models.Job.created_at <= query_end)
    
    total_jobs = jobs_query.count()
    active_jobs = jobs_query.filter(models.Job.status == 'active').count()
    completed_jobs = jobs_query.filter(models.Job.status == 'completed').count()
    
    # 2. Gerätestatistiken
    active_devices = db.query(models.Device).filter(models.Device.status == 'active').count()
    total_devices = db.query(models.Device).count()
    
    # Aktive Geräte nach Typ
    device_types = db.query(
        models.Device.type, 
        db.func.count(models.Device.id).label('count')
    ).group_by(models.Device.type).all()
    
    # 3. Laufende und abgeschlossene Trocknungen
    drying_jobs_active = jobs_query.filter(
        models.Job.status == 'active',
        db.exists().where(
            models.JobDevice.job_id == models.Job.id,
            models.Device.id == models.JobDevice.device_id,
            models.Device.type.in_(['dehumidifier', 'fan', 'heater', 'insulation_dryer'])
        )
    ).count()
    
    # Zusammenfassung
    return {
        "date_range": {
            "start": query_start,
            "end": query_end
        },
        "jobs": {
            "total": total_jobs,
            "active": active_jobs,
            "completed": completed_jobs,
            "active_drying": drying_jobs_active
        },
        "devices": {
            "total": total_devices,
            "active": active_devices,
            "by_type": [{"type": t[0], "count": t[1]} for t in device_types]
        }
    }