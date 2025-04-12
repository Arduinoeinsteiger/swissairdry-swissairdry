"""
SwissAirDry - Datenbankmodelle

Dieses Paket enthält alle Datenbankmodelle für das SwissAirDry-System.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

# Importiere alle Modelle, damit sie beim Base.metadata.create_all() berücksichtigt werden
from models.base import TimestampMixin, CrudMixin
from models.user import User
from models.group import Group, Permission, DashboardModule
from models.device import DeviceType, Device, DeviceMaintenance, DeviceSensorData
from models.job import Customer, Job, Measurement, JobNote, JobDocument
from models.mqtt import MqttDevice, MqttMessage, MqttSensorData, MqttGateway
from models.csv_import import CsvImportLog, CsvMapping, CsvTemplate