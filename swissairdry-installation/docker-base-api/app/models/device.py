"""
SwissAirDry - Gerätemodelle

Dieses Modul enthält die Datenbankmodelle für Geräte und Sensoren.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, String, Float, Integer, Boolean, ForeignKey, DateTime, Text
from sqlalchemy.types import JSON
from sqlalchemy.orm import relationship
from typing import Dict, Any, List, Optional
import datetime

from database import Base
from models.base import TimestampMixin, CrudMixin

class DeviceType(Base, TimestampMixin, CrudMixin):
    """Gerätetypmodell"""
    __tablename__ = 'device_types'

    name = Column(String(100), unique=True, nullable=False, index=True)
    description = Column(Text, nullable=True)
    manufacturer = Column(String(100), nullable=True)
    model = Column(String(100), nullable=True)
    technical_specs = Column(JSON, nullable=True)
    energy_consumption = Column(Float, nullable=True)  # kWh
    
    # Beziehungen
    devices = relationship("Device", back_populates="device_type")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert den Gerätetyp in ein Dictionary"""
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "manufacturer": self.manufacturer,
            "model": self.model,
            "technicalSpecs": self.technical_specs,
            "energyConsumption": self.energy_consumption,
            "deviceCount": len(self.devices) if self.devices else 0,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class Device(Base, TimestampMixin, CrudMixin):
    """Gerätemodell"""
    __tablename__ = 'devices'

    device_id = Column(String(50), unique=True, nullable=False, index=True)  # z.B. "DEV-001"
    name = Column(String(100), nullable=False)
    serial_number = Column(String(100), unique=True, nullable=False)
    device_type_id = Column(Integer, ForeignKey('device_types.id'), nullable=False)
    status = Column(String(50), nullable=False, default="available")  # available, active, maintenance, broken
    location = Column(String(255), nullable=True)
    job_id = Column(String(50), ForeignKey('jobs.job_id'), nullable=True)
    energy_consumption = Column(Float, nullable=True)  # Aktueller Verbrauch in kWh
    operational_hours = Column(Integer, nullable=False, default=0)
    firmware_version = Column(String(50), nullable=True)
    mqtt_topic = Column(String(255), nullable=True)
    is_mqtt_enabled = Column(Boolean, nullable=False, default=False)
    last_maintenance = Column(DateTime, nullable=True)
    next_maintenance = Column(DateTime, nullable=True)
    
    # Beziehungen
    device_type = relationship("DeviceType", back_populates="devices")
    job = relationship("Job", back_populates="devices")
    maintenance_history = relationship("DeviceMaintenance", back_populates="device")
    sensor_data = relationship("DeviceSensorData", back_populates="device")
    
    def update_status(self, new_status: str) -> None:
        """Aktualisiert den Status des Geräts"""
        self.status = new_status
    
    def assign_to_job(self, job_id: str, location: str = None) -> None:
        """Weist das Gerät einem Auftrag zu"""
        self.job_id = job_id
        if location:
            self.location = location
        self.status = "active"
    
    def unassign_from_job(self) -> None:
        """Entfernt das Gerät von einem Auftrag"""
        self.job_id = None
        self.status = "available"
    
    def add_operational_hours(self, hours: int) -> None:
        """Fügt Betriebsstunden hinzu"""
        self.operational_hours += hours
    
    def schedule_maintenance(self, date: datetime.datetime = None) -> None:
        """Plant die nächste Wartung"""
        if date:
            self.next_maintenance = date
        else:
            # Standardmäßig 6 Monate in der Zukunft
            self.next_maintenance = datetime.datetime.now() + datetime.timedelta(days=180)
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das Gerät in ein Dictionary"""
        return {
            "id": self.id,
            "deviceId": self.device_id,
            "name": self.name,
            "serialNumber": self.serial_number,
            "type": self.device_type.name if self.device_type else None,
            "status": self.status,
            "location": self.location,
            "jobId": self.job_id,
            "energyConsumption": self.energy_consumption,
            "operationalHours": self.operational_hours,
            "firmwareVersion": self.firmware_version,
            "mqttTopic": self.mqtt_topic,
            "isMqttEnabled": self.is_mqtt_enabled,
            "lastMaintenance": self.last_maintenance.isoformat() if self.last_maintenance else None,
            "nextMaintenance": self.next_maintenance.isoformat() if self.next_maintenance else None,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class DeviceMaintenance(Base, TimestampMixin, CrudMixin):
    """Gerätewartungsmodell"""
    __tablename__ = 'device_maintenance'

    device_id = Column(Integer, ForeignKey('devices.id'), nullable=False)
    maintenance_type = Column(String(100), nullable=False)  # z.B. "routine", "repair"
    technician = Column(String(100), nullable=True)
    notes = Column(Text, nullable=True)
    parts_replaced = Column(JSON, nullable=True)
    
    # Beziehungen
    device = relationship("Device", back_populates="maintenance_history")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Gerätewartung in ein Dictionary"""
        return {
            "id": self.id,
            "deviceId": self.device_id,
            "maintenanceType": self.maintenance_type,
            "technician": self.technician,
            "notes": self.notes,
            "partsReplaced": self.parts_replaced,
            "date": self.created_at.isoformat() if self.created_at else None
        }

class DeviceSensorData(Base, TimestampMixin, CrudMixin):
    """Modell für Gerätesensordaten"""
    __tablename__ = 'device_sensor_data'

    device_id = Column(Integer, ForeignKey('devices.id'), nullable=False)
    timestamp = Column(DateTime, nullable=False, index=True)
    temperature = Column(Float, nullable=True)  # °C
    humidity = Column(Float, nullable=True)  # %
    power_consumption = Column(Float, nullable=True)  # Watt
    water_extraction = Column(Float, nullable=True)  # Liter
    data = Column(JSON, nullable=True)  # Für zusätzliche Sensordaten
    
    # Beziehungen
    device = relationship("Device", back_populates="sensor_data")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Sensordaten in ein Dictionary"""
        return {
            "id": self.id,
            "deviceId": self.device_id,
            "timestamp": self.timestamp.isoformat(),
            "temperature": self.temperature,
            "humidity": self.humidity,
            "powerConsumption": self.power_consumption,
            "waterExtraction": self.water_extraction,
            "data": self.data
        }