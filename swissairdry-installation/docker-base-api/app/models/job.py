"""
SwissAirDry - Auftragsmodelle

Dieses Modul enthält die Datenbankmodelle für Aufträge und Messungen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, String, Float, Integer, DateTime, ForeignKey, Text, Table
from sqlalchemy.types import JSON
from sqlalchemy.orm import relationship
from typing import Dict, Any, List, Optional
import datetime

from database import Base
from models.base import TimestampMixin, CrudMixin

# Hilfstabelle für die Auftrags-Techniker-Beziehung
job_technicians = Table(
    'job_technicians',
    Base.metadata,
    Column('job_id', Integer, ForeignKey('jobs.id'), primary_key=True),
    Column('user_id', Integer, ForeignKey('users.id'), primary_key=True)
)

class Customer(Base, TimestampMixin, CrudMixin):
    """Kundenmodell"""
    __tablename__ = 'customers'

    customer_id = Column(String(50), unique=True, nullable=False, index=True)  # z.B. "CUST-001"
    name = Column(String(100), nullable=False, index=True)
    contact_person = Column(String(100), nullable=True)
    email = Column(String(100), nullable=True)
    phone = Column(String(50), nullable=True)
    address = Column(String(255), nullable=True)
    city = Column(String(100), nullable=True)
    postal_code = Column(String(20), nullable=True)
    country = Column(String(100), nullable=True, default="Schweiz")
    customer_type = Column(String(50), nullable=True)  # z.B. "business", "private"
    notes = Column(Text, nullable=True)
    
    # Beziehungen
    jobs = relationship("Job", back_populates="customer")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert den Kunden in ein Dictionary"""
        return {
            "id": self.id,
            "customerId": self.customer_id,
            "name": self.name,
            "contactPerson": self.contact_person,
            "email": self.email,
            "phone": self.phone,
            "address": self.address,
            "city": self.city,
            "postalCode": self.postal_code,
            "country": self.country,
            "customerType": self.customer_type,
            "notes": self.notes,
            "jobCount": len(self.jobs) if self.jobs else 0,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class Job(Base, TimestampMixin, CrudMixin):
    """Auftragsmodell"""
    __tablename__ = 'jobs'

    job_id = Column(String(50), unique=True, nullable=False, index=True)  # z.B. "JOB-001"
    customer_id = Column(Integer, ForeignKey('customers.id'), nullable=False)
    title = Column(String(255), nullable=False)
    description = Column(Text, nullable=True)
    location = Column(String(255), nullable=False)
    building_type = Column(String(100), nullable=True)  # z.B. "Wohngebäude", "Büro"
    rooms = Column(JSON, nullable=True)  # Liste der betroffenen Räume
    damage_type = Column(String(100), nullable=True)  # z.B. "Wasserschaden", "Bautrocknung"
    damage_extent = Column(String(50), nullable=True)  # z.B. "gering", "mittel", "schwer"
    status = Column(String(50), nullable=False, default="neu")  # neu, aktiv, pausiert, abgeschlossen
    start_date = Column(DateTime, nullable=True)
    end_date = Column(DateTime, nullable=True)
    estimated_end_date = Column(DateTime, nullable=True)
    
    # Beziehungen
    customer = relationship("Customer", back_populates="jobs")
    devices = relationship("Device", back_populates="job")
    measurements = relationship("Measurement", back_populates="job")
    notes = relationship("JobNote", back_populates="job")
    technicians = relationship("User", secondary=job_technicians)
    documents = relationship("JobDocument", back_populates="job")
    
    def update_status(self, new_status: str) -> None:
        """Aktualisiert den Status des Auftrags"""
        self.status = new_status
        if new_status == "aktiv" and not self.start_date:
            self.start_date = datetime.datetime.now()
        elif new_status == "abgeschlossen" and not self.end_date:
            self.end_date = datetime.datetime.now()
    
    def add_device(self, device_id: int) -> None:
        """Fügt ein Gerät zum Auftrag hinzu"""
        # Diese Methode würde in der tatsächlichen Implementierung Device.assign_to_job aufrufen
        pass
    
    def remove_device(self, device_id: int) -> None:
        """Entfernt ein Gerät vom Auftrag"""
        # Diese Methode würde in der tatsächlichen Implementierung Device.unassign_from_job aufrufen
        pass
    
    def calculate_duration(self) -> Optional[int]:
        """Berechnet die Dauer des Auftrags in Tagen"""
        if not self.start_date:
            return None
        
        end = self.end_date or datetime.datetime.now()
        duration = end - self.start_date
        return duration.days
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert den Auftrag in ein Dictionary"""
        return {
            "id": self.id,
            "jobId": self.job_id,
            "customer": self.customer.to_dict() if self.customer else None,
            "title": self.title,
            "description": self.description,
            "location": self.location,
            "buildingType": self.building_type,
            "rooms": self.rooms,
            "damageType": self.damage_type,
            "damageExtent": self.damage_extent,
            "status": self.status,
            "startDate": self.start_date.isoformat() if self.start_date else None,
            "endDate": self.end_date.isoformat() if self.end_date else None,
            "estimatedEndDate": self.estimated_end_date.isoformat() if self.estimated_end_date else None,
            "duration": self.calculate_duration(),
            "devices": [device.to_dict() for device in self.devices] if self.devices else [],
            "technicians": [{"id": tech.id, "name": tech.full_name or tech.username} for tech in self.technicians] if self.technicians else [],
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class Measurement(Base, TimestampMixin, CrudMixin):
    """Messungsmodell"""
    __tablename__ = 'measurements'

    job_id = Column(Integer, ForeignKey('jobs.id'), nullable=False)
    location = Column(String(255), nullable=False)  # z.B. "Wohnzimmer", "Keller"
    position = Column(String(100), nullable=True)  # Genauere Position innerhalb eines Raums
    material = Column(String(100), nullable=True)  # z.B. "Estrich", "Wand"
    temperature = Column(Float, nullable=True)  # °C
    humidity = Column(Float, nullable=True)  # % Luftfeuchtigkeit
    material_moisture = Column(Float, nullable=True)  # % Materialfeuchte
    dew_point = Column(Float, nullable=True)  # °C Taupunkt
    measurement_type = Column(String(50), nullable=True)  # z.B. "manuell", "automatisch"
    measured_by = Column(Integer, ForeignKey('users.id'), nullable=True)
    notes = Column(Text, nullable=True)
    
    # Beziehungen
    job = relationship("Job", back_populates="measurements")
    user = relationship("User")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Messung in ein Dictionary"""
        return {
            "id": self.id,
            "jobId": self.job_id,
            "date": self.created_at.isoformat() if self.created_at else None,
            "location": self.location,
            "position": self.position,
            "material": self.material,
            "temperature": self.temperature,
            "humidity": self.humidity,
            "materialMoisture": self.material_moisture,
            "dewPoint": self.dew_point,
            "measurementType": self.measurement_type,
            "measuredBy": self.user.full_name or self.user.username if self.user else None,
            "notes": self.notes
        }

class JobNote(Base, TimestampMixin, CrudMixin):
    """Modell für Auftragsnotizen"""
    __tablename__ = 'job_notes'

    job_id = Column(Integer, ForeignKey('jobs.id'), nullable=False)
    user_id = Column(Integer, ForeignKey('users.id'), nullable=False)
    note_type = Column(String(50), nullable=True)  # z.B. "info", "warning", "problem"
    text = Column(Text, nullable=False)
    
    # Beziehungen
    job = relationship("Job", back_populates="notes")
    user = relationship("User")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Notiz in ein Dictionary"""
        return {
            "id": self.id,
            "jobId": self.job_id,
            "date": self.created_at.isoformat() if self.created_at else None,
            "author": self.user.full_name or self.user.username if self.user else None,
            "noteType": self.note_type,
            "text": self.text
        }

class JobDocument(Base, TimestampMixin, CrudMixin):
    """Modell für Auftragsdokumente"""
    __tablename__ = 'job_documents'

    job_id = Column(Integer, ForeignKey('jobs.id'), nullable=False)
    user_id = Column(Integer, ForeignKey('users.id'), nullable=False)
    name = Column(String(255), nullable=False)
    document_type = Column(String(50), nullable=False)  # z.B. "pdf", "image"
    storage_path = Column(String(255), nullable=False)
    nextcloud_path = Column(String(255), nullable=True)
    size_kb = Column(Integer, nullable=True)
    description = Column(Text, nullable=True)
    
    # Beziehungen
    job = relationship("Job", back_populates="documents")
    user = relationship("User")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das Dokument in ein Dictionary"""
        return {
            "id": self.id,
            "jobId": self.job_id,
            "name": self.name,
            "documentType": self.document_type,
            "url": f"/api/documents/{self.id}",
            "nextcloudUrl": self.nextcloud_path,
            "sizeKb": self.size_kb,
            "description": self.description,
            "uploadedBy": self.user.full_name or self.user.username if self.user else None,
            "uploadedAt": self.created_at.isoformat() if self.created_at else None
        }