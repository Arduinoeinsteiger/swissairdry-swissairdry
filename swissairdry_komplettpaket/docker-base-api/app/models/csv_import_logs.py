"""
SwissAirDry - CSV-Import-Logs

Dieses Modul enthält das Datenbankmodell für CSV-Import-Logs,
um den Status und die Ergebnisse von CSV-Importvorgängen zu verfolgen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, Integer, String, Text, DateTime, JSON
from sqlalchemy.sql import func
from database import Base
from typing import Dict, Any

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
            "status": self.status,
            "message": self.message,
            "import_type": self.import_type,
            "details": self.details,
            "created_at": self.created_at.isoformat() if self.created_at else None,
            "updated_at": self.updated_at.isoformat() if self.updated_at else None
        }