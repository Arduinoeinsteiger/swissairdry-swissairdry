"""
SwissAirDry - CRUD-Operationen

Enthält alle Datenbankoperationen für die SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import List, Optional, Dict, Any, Union
from datetime import datetime
from sqlalchemy.orm import Session

from . import models, schemas


# --- Geräte-Operationen ---

def get_device(db: Session, device_id: int) -> Optional[models.Device]:
    """Gibt ein Gerät anhand seiner ID zurück."""
    return db.query(models.Device).filter(models.Device.id == device_id).first()


def get_device_by_device_id(db: Session, device_id: str) -> Optional[models.Device]:
    """Gibt ein Gerät anhand seiner Geräte-ID zurück."""
    return db.query(models.Device).filter(models.Device.device_id == device_id).first()


def get_devices(db: Session, skip: int = 0, limit: int = 100) -> List[models.Device]:
    """Gibt eine Liste von Geräten zurück."""
    return db.query(models.Device).offset(skip).limit(limit).all()


def create_device(db: Session, device: schemas.DeviceCreate) -> models.Device:
    """Erstellt ein neues Gerät."""
    db_device = models.Device(
        device_id=device.device_id,
        name=device.name,
        description=device.description,
        type=device.type,
        location=device.location,
        customer_id=device.customer_id,
        job_id=device.job_id,
        api_key=device.api_key,
        mqtt_topic=device.mqtt_topic,
        firmware_version=device.firmware_version,
        configuration=device.configuration
    )
    db.add(db_device)
    db.commit()
    db.refresh(db_device)
    return db_device


def update_device(db: Session, device_id: str, device: schemas.DeviceUpdate) -> models.Device:
    """Aktualisiert ein Gerät."""
    db_device = get_device_by_device_id(db, device_id)
    
    # Aktualisiere nur die angegebenen Felder
    update_data = device.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(db_device, key, value)
    
    db.commit()
    db.refresh(db_device)
    return db_device


def delete_device(db: Session, device_id: str) -> None:
    """Löscht ein Gerät."""
    db_device = get_device_by_device_id(db, device_id)
    db.delete(db_device)
    db.commit()


# --- Sensordaten-Operationen ---

def get_sensor_data(db: Session, sensor_data_id: int) -> Optional[models.SensorData]:
    """Gibt Sensordaten anhand ihrer ID zurück."""
    return db.query(models.SensorData).filter(models.SensorData.id == sensor_data_id).first()


def get_sensor_data_by_device(
    db: Session, device_id: int, limit: int = 100
) -> List[models.SensorData]:
    """Gibt die Sensordaten eines Geräts zurück."""
    return (
        db.query(models.SensorData)
        .filter(models.SensorData.device_id == device_id)
        .order_by(models.SensorData.timestamp.desc())
        .limit(limit)
        .all()
    )


def create_sensor_data(
    db: Session, sensor_data: schemas.SensorDataCreate, device_id: int
) -> models.SensorData:
    """Erstellt neue Sensordaten für ein Gerät."""
    db_sensor_data = models.SensorData(
        device_id=device_id,
        timestamp=sensor_data.timestamp,
        temperature=sensor_data.temperature,
        humidity=sensor_data.humidity,
        power=sensor_data.power,
        energy=sensor_data.energy,
        relay_state=sensor_data.relay_state,
        runtime=sensor_data.runtime,
        extra_data=sensor_data.extra_data
    )
    db.add(db_sensor_data)
    db.commit()
    db.refresh(db_sensor_data)
    return db_sensor_data


# --- Kunden-Operationen ---

def get_customer(db: Session, customer_id: int) -> Optional[models.Customer]:
    """Gibt einen Kunden anhand seiner ID zurück."""
    return db.query(models.Customer).filter(models.Customer.id == customer_id).first()


def get_customers(db: Session, skip: int = 0, limit: int = 100) -> List[models.Customer]:
    """Gibt eine Liste von Kunden zurück."""
    return db.query(models.Customer).offset(skip).limit(limit).all()


def create_customer(db: Session, customer: schemas.CustomerCreate) -> models.Customer:
    """Erstellt einen neuen Kunden."""
    db_customer = models.Customer(
        name=customer.name,
        email=customer.email,
        phone=customer.phone,
        address=customer.address,
        postal_code=customer.postal_code,
        city=customer.city,
        country=customer.country,
        notes=customer.notes,
        external_id=customer.external_id
    )
    db.add(db_customer)
    db.commit()
    db.refresh(db_customer)
    return db_customer


def update_customer(
    db: Session, customer_id: int, customer: schemas.CustomerUpdate
) -> models.Customer:
    """Aktualisiert einen Kunden."""
    db_customer = get_customer(db, customer_id)
    
    # Aktualisiere nur die angegebenen Felder
    update_data = customer.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(db_customer, key, value)
    
    db.commit()
    db.refresh(db_customer)
    return db_customer


def delete_customer(db: Session, customer_id: int) -> None:
    """Löscht einen Kunden."""
    db_customer = get_customer(db, customer_id)
    db.delete(db_customer)
    db.commit()


def get_customer_devices(db: Session, customer_id: int) -> List[models.Device]:
    """Gibt die Geräte eines Kunden zurück."""
    return (
        db.query(models.Device)
        .filter(models.Device.customer_id == customer_id)
        .all()
    )


# --- Auftrags-Operationen ---

def get_job(db: Session, job_id: int) -> Optional[models.Job]:
    """Gibt einen Auftrag anhand seiner ID zurück."""
    return db.query(models.Job).filter(models.Job.id == job_id).first()


def get_jobs(db: Session, skip: int = 0, limit: int = 100) -> List[models.Job]:
    """Gibt eine Liste von Aufträgen zurück."""
    return db.query(models.Job).offset(skip).limit(limit).all()


def create_job(db: Session, job: schemas.JobCreate) -> models.Job:
    """Erstellt einen neuen Auftrag."""
    db_job = models.Job(
        customer_id=job.customer_id,
        title=job.title,
        description=job.description,
        location=job.location,
        status=job.status,
        start_date=job.start_date,
        end_date=job.end_date,
        notes=job.notes,
        photos=job.photos,
        external_id=job.external_id,
        invoice_id=job.invoice_id
    )
    db.add(db_job)
    db.commit()
    db.refresh(db_job)
    return db_job


def update_job(db: Session, job_id: int, job: schemas.JobUpdate) -> models.Job:
    """Aktualisiert einen Auftrag."""
    db_job = get_job(db, job_id)
    
    # Aktualisiere nur die angegebenen Felder
    update_data = job.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(db_job, key, value)
    
    db.commit()
    db.refresh(db_job)
    return db_job


def delete_job(db: Session, job_id: int) -> None:
    """Löscht einen Auftrag."""
    db_job = get_job(db, job_id)
    db.delete(db_job)
    db.commit()


# --- Berichts-Operationen ---

def get_report(db: Session, report_id: int) -> Optional[models.Report]:
    """Gibt einen Bericht anhand seiner ID zurück."""
    return db.query(models.Report).filter(models.Report.id == report_id).first()


def get_reports(db: Session, skip: int = 0, limit: int = 100) -> List[models.Report]:
    """Gibt eine Liste von Berichten zurück."""
    return db.query(models.Report).offset(skip).limit(limit).all()


def create_report(db: Session, report: schemas.ReportCreate) -> models.Report:
    """Erstellt einen neuen Bericht."""
    db_report = models.Report(
        job_id=report.job_id,
        title=report.title,
        content=report.content,
        report_type=report.report_type,
        author=report.author
    )
    db.add(db_report)
    db.commit()
    db.refresh(db_report)
    return db_report


# --- Energiekosten-Operationen ---

def get_energy_cost(db: Session, energy_cost_id: int) -> Optional[models.EnergyCost]:
    """Gibt einen Energiekosten-Eintrag anhand seiner ID zurück."""
    return db.query(models.EnergyCost).filter(models.EnergyCost.id == energy_cost_id).first()


def get_energy_costs(db: Session) -> List[models.EnergyCost]:
    """Gibt eine Liste aller Energiekosten-Einträge zurück."""
    return db.query(models.EnergyCost).order_by(models.EnergyCost.valid_from.desc()).all()


def get_current_energy_cost(db: Session) -> Optional[models.EnergyCost]:
    """Gibt den aktuellen Energiekosten-Eintrag zurück."""
    now = datetime.now()
    return (
        db.query(models.EnergyCost)
        .filter(models.EnergyCost.valid_from <= now)
        .filter(
            (models.EnergyCost.valid_to.is_(None)) | (models.EnergyCost.valid_to >= now)
        )
        .order_by(models.EnergyCost.valid_from.desc())
        .first()
    ) or db.query(models.EnergyCost).filter(models.EnergyCost.is_default.is_(True)).first()


def create_energy_cost(
    db: Session, energy_cost: schemas.EnergyCostCreate
) -> models.EnergyCost:
    """Erstellt einen neuen Energiekosten-Eintrag."""
    db_energy_cost = models.EnergyCost(
        name=energy_cost.name,
        rate_kwh=energy_cost.rate_kwh,
        valid_from=energy_cost.valid_from,
        valid_to=energy_cost.valid_to,
        is_default=energy_cost.is_default
    )
    db.add(db_energy_cost)
    db.commit()
    db.refresh(db_energy_cost)
    return db_energy_cost