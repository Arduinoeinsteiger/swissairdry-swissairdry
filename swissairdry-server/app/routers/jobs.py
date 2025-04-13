"""
SwissAirDry - Aufträge-Router

Dieser Router stellt die Aufträge-API für das SwissAirDry-System bereit.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List, Optional
from fastapi import APIRouter, Depends, HTTPException, Query, Path
from sqlalchemy.orm import Session

from database import get_db

router = APIRouter(tags=["Jobs"])

@router.get("/")
async def get_jobs(
    status: Optional[str] = None,
    customer_id: Optional[int] = None,
    limit: int = 100,
    offset: int = 0,
    db: Session = Depends(get_db)
):
    """API zum Abrufen aller Aufträge mit optionaler Filterung"""
    # In einer echten Implementierung würden hier Aufträge aus der Datenbank abgerufen
    jobs = [
        {
            "id": "JOB-456",
            "customer": {
                "id": 101,
                "name": "Müller GmbH",
                "contact": "Hans Müller",
                "phone": "+49 123 456789"
            },
            "location": "Hauptstraße 1, München",
            "status": "Aktiv",
            "start_date": "2025-04-05T08:00:00Z",
            "estimated_end_date": "2025-04-19T16:00:00Z",
            "devices": ["DEV-123"],
            "type": "Wasserschaden",
            "description": "Wasserschaden nach Rohrbruch im Erdgeschoss"
        },
        {
            "id": "JOB-457",
            "customer": {
                "id": 102,
                "name": "Schmidt KG",
                "contact": "Anna Schmidt",
                "phone": "+49 234 567890"
            },
            "location": "Industrieweg 42, Berlin",
            "status": "Aktiv",
            "start_date": "2025-04-03T10:30:00Z",
            "estimated_end_date": "2025-04-17T16:00:00Z",
            "devices": ["DEV-124"],
            "type": "Bautrocknung",
            "description": "Trocknung von Estrich nach Neubau"
        },
        {
            "id": "JOB-458",
            "customer": {
                "id": 103,
                "name": "Bauer AG",
                "contact": "Peter Bauer",
                "phone": "+49 345 678901"
            },
            "location": "Dorfplatz 3, Hamburg",
            "status": "Abgeschlossen",
            "start_date": "2025-03-20T09:00:00Z",
            "end_date": "2025-04-10T16:45:00Z",
            "devices": [],
            "type": "Wasserschaden",
            "description": "Wasserschaden nach Unwetter im Kellergeschoss"
        }
    ]
    
    # Filterung anwenden (in einer echten Implementierung würde dies in der Datenbankabfrage geschehen)
    if status:
        jobs = [j for j in jobs if j["status"] == status]
    if customer_id:
        jobs = [j for j in jobs if j["customer"]["id"] == customer_id]
    
    # Pagination anwenden
    total_count = len(jobs)
    jobs = jobs[offset:offset+limit]
    
    return {
        "jobs": jobs,
        "total": total_count,
        "limit": limit,
        "offset": offset
    }

@router.get("/{job_id}")
async def get_job(job_id: str = Path(...), db: Session = Depends(get_db)):
    """API zum Abrufen eines einzelnen Auftrags anhand seiner ID"""
    # In einer echten Implementierung würde hier der Auftrag aus der Datenbank abgerufen
    if job_id == "JOB-456":
        return {
            "id": "JOB-456",
            "customer": {
                "id": 101,
                "name": "Müller GmbH",
                "contact": "Hans Müller",
                "phone": "+49 123 456789",
                "email": "hans.mueller@mueller-gmbh.de",
                "address": "Hauptstraße 1, 80331 München"
            },
            "location": {
                "address": "Hauptstraße 1, 80331 München",
                "building": "Bürogebäude",
                "floor": "Erdgeschoss",
                "rooms": ["Empfangsbereich", "Büro 1", "Büro 2"]
            },
            "status": "Aktiv",
            "start_date": "2025-04-05T08:00:00Z",
            "estimated_end_date": "2025-04-19T16:00:00Z",
            "devices": [
                {
                    "id": "DEV-123",
                    "name": "Luftentfeuchter A1",
                    "type": "Luftentfeuchter",
                    "location": "Empfangsbereich",
                    "status": "Aktiv"
                }
            ],
            "type": "Wasserschaden",
            "description": "Wasserschaden nach Rohrbruch im Erdgeschoss",
            "damage_extent": "Mittelschwer",
            "technicians": [
                {"id": 201, "name": "Max Mustermann", "role": "Trocknungsspezialist"},
                {"id": 202, "name": "Lisa Schmidt", "role": "Assistentin"}
            ],
            "measurements": [
                {"date": "2025-04-05T10:30:00Z", "location": "Empfangsbereich", "temperature": 21.5, "humidity": 75.3, "material_moisture": 15.8},
                {"date": "2025-04-06T09:15:00Z", "location": "Empfangsbereich", "temperature": 22.0, "humidity": 68.7, "material_moisture": 14.2},
                {"date": "2025-04-07T11:00:00Z", "location": "Empfangsbereich", "temperature": 22.3, "humidity": 62.4, "material_moisture": 12.9},
                {"date": "2025-04-08T10:45:00Z", "location": "Empfangsbereich", "temperature": 22.5, "humidity": 58.1, "material_moisture": 11.6},
                {"date": "2025-04-09T09:30:00Z", "location": "Empfangsbereich", "temperature": 22.6, "humidity": 54.3, "material_moisture": 10.8},
                {"date": "2025-04-10T11:15:00Z", "location": "Empfangsbereich", "temperature": 22.7, "humidity": 51.2, "material_moisture": 10.1},
                {"date": "2025-04-11T10:00:00Z", "location": "Empfangsbereich", "temperature": 22.8, "humidity": 48.5, "material_moisture": 9.5},
                {"date": "2025-04-12T09:45:00Z", "location": "Empfangsbereich", "temperature": 22.5, "humidity": 45.8, "material_moisture": 8.9}
            ],
            "notes": [
                {"date": "2025-04-05T08:30:00Z", "author": "Max Mustermann", "text": "Wasserschaden begutachtet, Trocknungsgeräte installiert."},
                {"date": "2025-04-08T09:00:00Z", "author": "Lisa Schmidt", "text": "Trocknungsfortschritt kontrolliert, alles verläuft planmäßig."},
                {"date": "2025-04-12T10:00:00Z", "author": "Max Mustermann", "text": "Trocknungsgrad verbessert sich gut, Gerät belassen."}
            ],
            "documents": [
                {"id": "DOC-1001", "name": "Schadensprotokoll", "type": "PDF", "url": "/api/documents/DOC-1001", "uploaded": "2025-04-05T12:30:00Z"},
                {"id": "DOC-1002", "name": "Fotos vom Schaden", "type": "ZIP", "url": "/api/documents/DOC-1002", "uploaded": "2025-04-05T12:45:00Z"},
                {"id": "DOC-1003", "name": "Messprotokoll", "type": "PDF", "url": "/api/documents/DOC-1003", "uploaded": "2025-04-12T11:00:00Z"}
            ]
        }
    else:
        raise HTTPException(status_code=404, detail=f"Auftrag mit ID '{job_id}' nicht gefunden")

@router.get("/{job_id}/measurements")
async def get_job_measurements(
    job_id: str = Path(...),
    start_date: Optional[str] = None,
    end_date: Optional[str] = None,
    db: Session = Depends(get_db)
):
    """API zum Abrufen aller Messungen für einen Auftrag"""
    # In einer echten Implementierung würden hier die Messungen aus der Datenbank abgerufen
    if job_id == "JOB-456":
        return {
            "job_id": "JOB-456",
            "measurements": [
                {"date": "2025-04-05T10:30:00Z", "location": "Empfangsbereich", "temperature": 21.5, "humidity": 75.3, "material_moisture": 15.8},
                {"date": "2025-04-06T09:15:00Z", "location": "Empfangsbereich", "temperature": 22.0, "humidity": 68.7, "material_moisture": 14.2},
                {"date": "2025-04-07T11:00:00Z", "location": "Empfangsbereich", "temperature": 22.3, "humidity": 62.4, "material_moisture": 12.9},
                {"date": "2025-04-08T10:45:00Z", "location": "Empfangsbereich", "temperature": 22.5, "humidity": 58.1, "material_moisture": 11.6},
                {"date": "2025-04-09T09:30:00Z", "location": "Empfangsbereich", "temperature": 22.6, "humidity": 54.3, "material_moisture": 10.8},
                {"date": "2025-04-10T11:15:00Z", "location": "Empfangsbereich", "temperature": 22.7, "humidity": 51.2, "material_moisture": 10.1},
                {"date": "2025-04-11T10:00:00Z", "location": "Empfangsbereich", "temperature": 22.8, "humidity": 48.5, "material_moisture": 9.5},
                {"date": "2025-04-12T09:45:00Z", "location": "Empfangsbereich", "temperature": 22.5, "humidity": 45.8, "material_moisture": 8.9}
            ]
        }
    else:
        raise HTTPException(status_code=404, detail=f"Auftrag mit ID '{job_id}' nicht gefunden")