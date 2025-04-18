"""
SwissAirDry - API-Schemas

Enthält alle Pydantic-Schemas für die SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import List, Optional, Dict, Any, Union
from datetime import datetime
from pydantic import BaseModel, Field


# --- Basis-Schemas ---

class Message(BaseModel):
    """Schema für einfache Nachrichtenantworten"""
    message: str


# --- Geräte-Schemas ---

class DeviceBase(BaseModel):
    """Basis-Schema für Geräte"""
    device_id: str
    name: str
    description: Optional[str] = None
    type: str = "standard"
    location: Optional[str] = None
    customer_id: Optional[int] = None
    job_id: Optional[int] = None
    mqtt_topic: Optional[str] = None
    firmware_version: Optional[str] = None
    configuration: Optional[Dict[str, Any]] = None


class DeviceCreate(DeviceBase):
    """Schema für das Erstellen eines Geräts"""
    api_key: Optional[str] = None


class DeviceUpdate(BaseModel):
    """Schema für das Aktualisieren eines Geräts"""
    name: Optional[str] = None
    description: Optional[str] = None
    type: Optional[str] = None
    location: Optional[str] = None
    customer_id: Optional[int] = None
    job_id: Optional[int] = None
    api_key: Optional[str] = None
    mqtt_topic: Optional[str] = None
    firmware_version: Optional[str] = None
    configuration: Optional[Dict[str, Any]] = None
    status: Optional[str] = None
    last_seen: Optional[datetime] = None


class Device(DeviceBase):
    """Schema für ein Gerät mit allen Feldern"""
    id: int
    api_key: Optional[str] = None
    status: str
    last_seen: Optional[datetime] = None
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Sensordaten-Schemas ---

class SensorDataBase(BaseModel):
    """Basis-Schema für Sensordaten"""
    timestamp: Optional[datetime] = Field(default_factory=datetime.now)
    temperature: Optional[float] = None
    humidity: Optional[float] = None
    power: Optional[float] = None
    energy: Optional[float] = None
    relay_state: Optional[bool] = None
    runtime: Optional[int] = None
    extra_data: Optional[Dict[str, Any]] = None


class SensorDataCreate(SensorDataBase):
    """Schema für das Erstellen von Sensordaten"""
    pass


class SensorData(SensorDataBase):
    """Schema für Sensordaten mit allen Feldern"""
    id: int
    device_id: int

    class Config:
        orm_mode = True


class SensorDataResponse(BaseModel):
    """Schema für die Antwort auf Sensordaten-Übermittlung"""
    status: str
    relay_control: Optional[bool] = None
    message: Optional[str] = None


# --- Kunden-Schemas ---

class CustomerBase(BaseModel):
    """Basis-Schema für Kunden"""
    name: str
    email: Optional[str] = None
    phone: Optional[str] = None
    address: Optional[str] = None
    postal_code: Optional[str] = None
    city: Optional[str] = None
    country: Optional[str] = "Schweiz"
    notes: Optional[str] = None


class CustomerCreate(CustomerBase):
    """Schema für das Erstellen eines Kunden"""
    external_id: Optional[str] = None


class CustomerUpdate(BaseModel):
    """Schema für das Aktualisieren eines Kunden"""
    name: Optional[str] = None
    email: Optional[str] = None
    phone: Optional[str] = None
    address: Optional[str] = None
    postal_code: Optional[str] = None
    city: Optional[str] = None
    country: Optional[str] = None
    notes: Optional[str] = None
    external_id: Optional[str] = None


class Customer(CustomerBase):
    """Schema für einen Kunden mit allen Feldern"""
    id: int
    external_id: Optional[str] = None
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Auftrags-Schemas ---

class JobBase(BaseModel):
    """Basis-Schema für Aufträge"""
    customer_id: int
    title: str
    description: Optional[str] = None
    location: Optional[str] = None
    status: str = "offen"
    start_date: Optional[datetime] = None
    end_date: Optional[datetime] = None
    notes: Optional[str] = None
    photos: Optional[List[str]] = None


class JobCreate(JobBase):
    """Schema für das Erstellen eines Auftrags"""
    external_id: Optional[str] = None
    invoice_id: Optional[str] = None


class JobUpdate(BaseModel):
    """Schema für das Aktualisieren eines Auftrags"""
    customer_id: Optional[int] = None
    title: Optional[str] = None
    description: Optional[str] = None
    location: Optional[str] = None
    status: Optional[str] = None
    start_date: Optional[datetime] = None
    end_date: Optional[datetime] = None
    notes: Optional[str] = None
    photos: Optional[List[str]] = None
    external_id: Optional[str] = None
    invoice_id: Optional[str] = None


class Job(JobBase):
    """Schema für einen Auftrag mit allen Feldern"""
    id: int
    external_id: Optional[str] = None
    invoice_id: Optional[str] = None
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Bericht-Schemas ---

class ReportBase(BaseModel):
    """Basis-Schema für Berichte"""
    job_id: int
    title: str
    content: Optional[str] = None
    report_type: str = "standard"
    author: Optional[str] = None


class ReportCreate(ReportBase):
    """Schema für das Erstellen eines Berichts"""
    pass


class Report(ReportBase):
    """Schema für einen Bericht mit allen Feldern"""
    id: int
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Benutzer-Schemas ---

class UserBase(BaseModel):
    """Basis-Schema für Benutzer"""
    username: str
    email: str
    full_name: Optional[str] = None
    role: str = "user"


class UserCreate(UserBase):
    """Schema für das Erstellen eines Benutzers"""
    password: str


class User(UserBase):
    """Schema für einen Benutzer mit allen Feldern"""
    id: int
    is_active: bool
    last_login: Optional[datetime] = None
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Warnung-Schemas ---

class AlertBase(BaseModel):
    """Basis-Schema für Warnungen"""
    device_id: int
    alert_type: str
    message: str
    value: Optional[float] = None
    threshold: Optional[float] = None
    is_active: bool = True


class AlertCreate(AlertBase):
    """Schema für das Erstellen einer Warnung"""
    pass


class Alert(AlertBase):
    """Schema für eine Warnung mit allen Feldern"""
    id: int
    acknowledged: bool
    acknowledged_by: Optional[int] = None
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Energiekosten-Schemas ---

class EnergyCostBase(BaseModel):
    """Basis-Schema für Energiekosten"""
    name: str
    rate_kwh: float
    valid_from: datetime
    valid_to: Optional[datetime] = None
    is_default: bool = False


class EnergyCostCreate(EnergyCostBase):
    """Schema für das Erstellen von Energiekosten"""
    pass


class EnergyCost(EnergyCostBase):
    """Schema für Energiekosten mit allen Feldern"""
    id: int
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- API-Schlüssel-Schemas ---

class APIKeyBase(BaseModel):
    """Basis-Schema für API-Schlüssel"""
    name: str
    user_id: Optional[int] = None
    scopes: Optional[str] = None
    expires_at: Optional[datetime] = None
    is_active: bool = True


class APIKeyCreate(APIKeyBase):
    """Schema für das Erstellen eines API-Schlüssels"""
    pass


class APIKey(APIKeyBase):
    """Schema für einen API-Schlüssel mit allen Feldern"""
    id: int
    key: str
    created_at: datetime
    updated_at: Optional[datetime] = None

    class Config:
        orm_mode = True


# --- Gerätebefehle-Schema ---

class DeviceCommand(BaseModel):
    """Schema für Gerätebefehle"""
    command: str
    value: Any