"""
SwissAirDry - MQTT-Modelle

Dieses Modul enthält die Datenbankmodelle für MQTT-Geräte und Sensoren.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, String, Float, Integer, Boolean, ForeignKey, DateTime, Text
from sqlalchemy.types import JSON
from sqlalchemy.orm import relationship
from typing import Dict, Any, List, Optional

from database import Base
from models.base import TimestampMixin, CrudMixin

class MqttDevice(Base, TimestampMixin, CrudMixin):
    """MQTT-Gerätemodell"""
    __tablename__ = 'mqtt_devices'

    device_id = Column(String(50), unique=True, nullable=False, index=True)  # z.B. "ESP32-001"
    name = Column(String(100), nullable=False)
    device_type = Column(String(50), nullable=False)  # z.B. "sensor", "gateway", "controller"
    topic = Column(String(255), nullable=False, index=True)
    status = Column(String(50), nullable=False, default="unknown")  # online, offline, error, unknown
    last_seen = Column(DateTime, nullable=True)
    firmware_version = Column(String(50), nullable=True)
    hardware_version = Column(String(50), nullable=True)
    battery_level = Column(Integer, nullable=True)  # %
    location = Column(String(255), nullable=True)
    job_id = Column(String(50), ForeignKey('jobs.job_id'), nullable=True)
    connected_device_ids = Column(JSON, nullable=True)  # Liste von verbundenen Geräte-IDs (für Gateways)
    settings = Column(JSON, nullable=True)  # Gerätespezifische Einstellungen
    
    # Beziehungen
    job = relationship("Job")
    messages = relationship("MqttMessage", back_populates="device")
    
    def update_status(self, new_status: str, last_seen: datetime.datetime = None) -> None:
        """Aktualisiert den Status des MQTT-Geräts"""
        self.status = new_status
        if last_seen:
            self.last_seen = last_seen
    
    def update_firmware(self, new_version: str) -> None:
        """Aktualisiert die Firmware-Version des Geräts"""
        self.firmware_version = new_version
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das MQTT-Gerät in ein Dictionary"""
        return {
            "id": self.id,
            "deviceId": self.device_id,
            "name": self.name,
            "deviceType": self.device_type,
            "topic": self.topic,
            "status": self.status,
            "lastSeen": self.last_seen.isoformat() if self.last_seen else None,
            "firmwareVersion": self.firmware_version,
            "hardwareVersion": self.hardware_version,
            "batteryLevel": self.battery_level,
            "location": self.location,
            "jobId": self.job_id,
            "connectedDeviceIds": self.connected_device_ids,
            "settings": self.settings,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class MqttMessage(Base, TimestampMixin, CrudMixin):
    """MQTT-Nachrichtenmodell"""
    __tablename__ = 'mqtt_messages'

    device_id = Column(Integer, ForeignKey('mqtt_devices.id'), nullable=False)
    topic = Column(String(255), nullable=False)
    payload = Column(JSON, nullable=False)
    qos = Column(Integer, nullable=False, default=0)
    retain = Column(Boolean, nullable=False, default=False)
    direction = Column(String(10), nullable=False)  # received, sent
    processed = Column(Boolean, nullable=False, default=False)
    
    # Beziehungen
    device = relationship("MqttDevice", back_populates="messages")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die MQTT-Nachricht in ein Dictionary"""
        return {
            "id": self.id,
            "deviceId": self.device_id,
            "topic": self.topic,
            "payload": self.payload,
            "qos": self.qos,
            "retain": self.retain,
            "direction": self.direction,
            "processed": self.processed,
            "timestamp": self.created_at.isoformat() if self.created_at else None
        }

class MqttSensorData(Base, TimestampMixin, CrudMixin):
    """MQTT-Sensordatenmodell"""
    __tablename__ = 'mqtt_sensor_data'

    device_id = Column(Integer, ForeignKey('mqtt_devices.id'), nullable=False)
    timestamp = Column(DateTime, nullable=False, index=True)
    data_type = Column(String(50), nullable=False)  # z.B. "temperature", "humidity"
    value = Column(Float, nullable=False)
    unit = Column(String(20), nullable=True)  # z.B. "°C", "%"
    
    # Beziehungen
    device = relationship("MqttDevice")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die MQTT-Sensordaten in ein Dictionary"""
        return {
            "id": self.id,
            "deviceId": self.device_id,
            "timestamp": self.timestamp.isoformat(),
            "dataType": self.data_type,
            "value": self.value,
            "unit": self.unit
        }

class MqttGateway(Base, TimestampMixin, CrudMixin):
    """MQTT-Gateway-Modell"""
    __tablename__ = 'mqtt_gateways'

    gateway_id = Column(String(50), unique=True, nullable=False, index=True)  # z.B. "GW-001"
    mqtt_device_id = Column(Integer, ForeignKey('mqtt_devices.id'), nullable=False)
    name = Column(String(100), nullable=False)
    location = Column(String(255), nullable=True)
    status = Column(String(50), nullable=False, default="unknown")  # online, offline, error, unknown
    ip_address = Column(String(50), nullable=True)
    mac_address = Column(String(50), nullable=True)
    firmware_version = Column(String(50), nullable=True)
    configuration = Column(JSON, nullable=True)
    
    # Beziehungen
    mqtt_device = relationship("MqttDevice")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das MQTT-Gateway in ein Dictionary"""
        return {
            "id": self.id,
            "gatewayId": self.gateway_id,
            "mqttDeviceId": self.mqtt_device_id,
            "name": self.name,
            "location": self.location,
            "status": self.status,
            "ipAddress": self.ip_address,
            "macAddress": self.mac_address,
            "firmwareVersion": self.firmware_version,
            "configuration": self.configuration,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }