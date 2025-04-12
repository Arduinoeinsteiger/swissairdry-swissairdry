"""
SwissAirDry - CSV-Import-Logs

Dieses Modul enthält das Datenbankmodell für CSV-Import-Logs,
um den Status und die Ergebnisse von CSV-Importvorgängen zu verfolgen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Dict, Any, List
from datetime import datetime
from sqlalchemy import Column, Integer, String, Text, DateTime, ForeignKey, JSON
from sqlalchemy.orm import relationship
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.sql import func

from database import Base

class CsvImportLog(Base):
    """Protokoll für CSV-Importvorgänge"""
    __tablename__ = 'csv_import_logs'

    id = Column(Integer, primary_key=True)
    filename = Column(String(255), nullable=False)
    stored_path = Column(String(255), nullable=False)
    status = Column(String(50), nullable=False)  # z.B. "Wird verarbeitet", "Abgeschlossen", "Fehler"
    message = Column(Text, nullable=True)
    import_type = Column(String(50), nullable=False)  # z.B. "Einzeldatei", "Multi-Datei"
    details = Column(JSON, nullable=True)  # z.B. Anzahl der importierten Datensätze
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das Importprotokoll in ein Dictionary"""
        return {
            "id": self.id,
            "filename": self.filename,
            "stored_path": self.stored_path,
            "status": self.status,
            "message": self.message,
            "import_type": self.import_type,
            "details": self.details,
            "created_at": self.created_at.isoformat() if self.created_at else None,
            "updated_at": self.updated_at.isoformat() if self.updated_at else None
        }