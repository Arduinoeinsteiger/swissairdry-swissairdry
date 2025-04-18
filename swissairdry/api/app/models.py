"""
SwissAirDry - Datenbankmodelle

Enthält alle SQLAlchemy-Modelle für die SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, Integer, String, Float, Boolean, DateTime, ForeignKey, Text, JSON
from sqlalchemy.orm import relationship
from sqlalchemy.sql import func

from .database import Base


class Device(Base):
    """Modell für ein SwissAirDry-Gerät"""
    __tablename__ = "devices"
    
    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(String(64), unique=True, index=True, nullable=False)
    name = Column(String(255), nullable=False)
    description = Column(Text, nullable=True)
    type = Column(String(50), nullable=False, default="standard")
    location = Column(String(255), nullable=True)
    customer_id = Column(Integer, ForeignKey("customers.id"), nullable=True)
    job_id = Column(Integer, ForeignKey("jobs.id"), nullable=True)
    api_key = Column(String(64), unique=True, nullable=True)
    mqtt_topic = Column(String(255), nullable=True)
    firmware_version = Column(String(20), nullable=True)
    configuration = Column(JSON, nullable=True)
    status = Column(String(20), nullable=False, default="offline")
    last_seen = Column(DateTime, nullable=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())
    
    # Beziehungen
    customer = relationship("Customer", back_populates="devices")
    job = relationship("Job", back_populates="devices")
    sensor_data = relationship("SensorData", back_populates="device", cascade="all, delete-orphan")
    alerts = relationship("Alert", back_populates="device", cascade="all, delete-orphan")


class SensorData(Base):
    """Modell für Sensordaten eines Geräts"""
    __tablename__ = "sensor_data"
    
    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(Integer, ForeignKey("devices.id"), nullable=False)
    timestamp = Column(DateTime, nullable=False, default=func.now())
    temperature = Column(Float, nullable=True)
    humidity = Column(Float, nullable=True)
    power = Column(Float, nullable=True)
    energy = Column(Float, nullable=True)
    relay_state = Column(Boolean, nullable=True)
    runtime = Column(Integer, nullable=True)  # Laufzeit in Sekunden
    extra_data = Column(JSON, nullable=True)
    
    # Beziehungen
    device = relationship("Device", back_populates="sensor_data")


class Customer(Base):
    """Modell für einen Kunden"""
    __tablename__ = "customers"
    
    id = Column(Integer, primary_key=True, index=True)
    external_id = Column(String(64), unique=True, nullable=True)  # z.B. Bexio-ID
    name = Column(String(255), nullable=False)
    email = Column(String(255), nullable=True)
    phone = Column(String(50), nullable=True)
    address = Column(String(255), nullable=True)
    postal_code = Column(String(20), nullable=True)
    city = Column(String(100), nullable=True)
    country = Column(String(100), nullable=True, default="Schweiz")
    notes = Column(Text, nullable=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())
    
    # Beziehungen
    devices = relationship("Device", back_populates="customer")
    jobs = relationship("Job", back_populates="customer")


class Job(Base):
    """Modell für einen Auftrag"""
    __tablename__ = "jobs"
    
    id = Column(Integer, primary_key=True, index=True)
    external_id = Column(String(64), unique=True, nullable=True)  # z.B. Bexio-ID
    customer_id = Column(Integer, ForeignKey("customers.id"), nullable=False)
    title = Column(String(255), nullable=False)
    description = Column(Text, nullable=True)
    location = Column(String(255), nullable=True)
    status = Column(String(50), nullable=False, default="offen")
    start_date = Column(DateTime, nullable=True)
    end_date = Column(DateTime, nullable=True)
    notes = Column(Text, nullable=True)
    photos = Column(JSON, nullable=True)  # Array von Foto-URLs/Pfaden
    invoice_id = Column(String(64), nullable=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())
    
    # Beziehungen
    customer = relationship("Customer", back_populates="jobs")
    devices = relationship("Device", back_populates="job")
    reports = relationship("Report", back_populates="job", cascade="all, delete-orphan")


class Report(Base):
    """Modell für einen Bericht"""
    __tablename__ = "reports"
    
    id = Column(Integer, primary_key=True, index=True)
    job_id = Column(Integer, ForeignKey("jobs.id"), nullable=False)
    title = Column(String(255), nullable=False)
    content = Column(Text, nullable=True)
    report_type = Column(String(50), nullable=False, default="standard")
    author = Column(String(255), nullable=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())
    
    # Beziehungen
    job = relationship("Job", back_populates="reports")


class User(Base):
    """Modell für einen Benutzer"""
    __tablename__ = "users"
    
    id = Column(Integer, primary_key=True, index=True)
    username = Column(String(50), unique=True, index=True, nullable=False)
    email = Column(String(255), unique=True, index=True, nullable=False)
    hashed_password = Column(String(255), nullable=False)
    full_name = Column(String(255), nullable=True)
    role = Column(String(20), nullable=False, default="user")  # admin, user, technician
    is_active = Column(Boolean, nullable=False, default=True)
    last_login = Column(DateTime, nullable=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())


class Alert(Base):
    """Modell für eine Warnung/Benachrichtigung"""
    __tablename__ = "alerts"
    
    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(Integer, ForeignKey("devices.id"), nullable=False)
    alert_type = Column(String(50), nullable=False)
    message = Column(Text, nullable=False)
    value = Column(Float, nullable=True)
    threshold = Column(Float, nullable=True)
    is_active = Column(Boolean, nullable=False, default=True)
    acknowledged = Column(Boolean, nullable=False, default=False)
    acknowledged_by = Column(Integer, ForeignKey("users.id"), nullable=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())
    
    # Beziehungen
    device = relationship("Device", back_populates="alerts")


class EnergyCost(Base):
    """Modell für Energiekosten-Berechnung"""
    __tablename__ = "energy_costs"
    
    id = Column(Integer, primary_key=True, index=True)
    name = Column(String(255), nullable=False)
    rate_kwh = Column(Float, nullable=False)  # Preis pro kWh in CHF
    valid_from = Column(DateTime, nullable=False)
    valid_to = Column(DateTime, nullable=True)
    is_default = Column(Boolean, nullable=False, default=False)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())


class APIKey(Base):
    """Modell für API-Schlüssel"""
    __tablename__ = "api_keys"
    
    id = Column(Integer, primary_key=True, index=True)
    key = Column(String(64), unique=True, index=True, nullable=False)
    name = Column(String(255), nullable=False)
    user_id = Column(Integer, ForeignKey("users.id"), nullable=True)
    scopes = Column(String(255), nullable=True)  # Berechtigungen, durch Komma getrennt
    expires_at = Column(DateTime, nullable=True)
    is_active = Column(Boolean, nullable=False, default=True)
    created_at = Column(DateTime, server_default=func.now())
    updated_at = Column(DateTime, onupdate=func.now())