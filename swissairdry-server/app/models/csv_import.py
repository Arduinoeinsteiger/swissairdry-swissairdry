"""
SwissAirDry - CSV-Import-Modelle

Dieses Modul enthält die Datenbankmodelle für CSV-Import-Protokolle und -Zuordnungen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, String, Integer, ForeignKey, Text
from sqlalchemy.types import JSON
from sqlalchemy.orm import relationship
from typing import Dict, Any, List, Optional

from database import Base
from models.base import TimestampMixin, CrudMixin

class CsvImportLog(Base, TimestampMixin, CrudMixin):
    """Protokoll für CSV-Importvorgänge"""
    __tablename__ = 'csv_import_logs'

    filename = Column(String(255), nullable=False)
    stored_path = Column(String(255), nullable=False)
    status = Column(String(50), nullable=False)  # z.B. "processing", "completed", "error"
    message = Column(Text, nullable=True)
    import_type = Column(String(50), nullable=False)  # z.B. "single", "multi"
    details = Column(JSON, nullable=True)  # z.B. Anzahl der importierten Datensätze
    user_id = Column(Integer, ForeignKey('users.id'), nullable=True)
    
    # Beziehungen
    user = relationship("User")
    mappings = relationship("CsvMapping", back_populates="import_log")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das Import-Protokoll in ein Dictionary"""
        return {
            "id": self.id,
            "filename": self.filename,
            "storedPath": self.stored_path,
            "status": self.status,
            "message": self.message,
            "importType": self.import_type,
            "details": self.details,
            "userId": self.user_id,
            "user": self.user.username if self.user else None,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class CsvMapping(Base, TimestampMixin, CrudMixin):
    """Zuordnung für CSV-Spalten zu Datenbankfeldern"""
    __tablename__ = 'csv_mappings'

    import_log_id = Column(Integer, ForeignKey('csv_import_logs.id'), nullable=False)
    csv_column = Column(String(255), nullable=False)
    db_field = Column(String(255), nullable=False)
    model_name = Column(String(255), nullable=False)  # z.B. "Customer", "Device"
    transformation = Column(String(255), nullable=True)  # z.B. "uppercase", "date_format"
    notes = Column(Text, nullable=True)
    
    # Beziehungen
    import_log = relationship("CsvImportLog", back_populates="mappings")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die CSV-Zuordnung in ein Dictionary"""
        return {
            "id": self.id,
            "importLogId": self.import_log_id,
            "csvColumn": self.csv_column,
            "dbField": self.db_field,
            "modelName": self.model_name,
            "transformation": self.transformation,
            "notes": self.notes
        }

class CsvTemplate(Base, TimestampMixin, CrudMixin):
    """Vorlage für CSV-Importe"""
    __tablename__ = 'csv_templates'

    name = Column(String(255), nullable=False, unique=True)
    description = Column(Text, nullable=True)
    target_model = Column(String(255), nullable=False)  # z.B. "Customer", "Device"
    column_mappings = Column(JSON, nullable=False)  # z.B. {"Name": "name", "Adresse": "address"}
    has_header = Column(Integer, nullable=False, default=1)  # 0=false, 1=true
    delimiter = Column(String(10), nullable=False, default=";")
    example_file_path = Column(String(255), nullable=True)
    user_id = Column(Integer, ForeignKey('users.id'), nullable=True)
    
    # Beziehungen
    user = relationship("User")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die CSV-Vorlage in ein Dictionary"""
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "targetModel": self.target_model,
            "columnMappings": self.column_mappings,
            "hasHeader": bool(self.has_header),
            "delimiter": self.delimiter,
            "exampleFilePath": self.example_file_path,
            "userId": self.user_id,
            "user": self.user.username if self.user else None,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }