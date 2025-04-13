"""
SwissAirDry - Basismodelle

Dieses Modul enthält Basisklassen für alle Datenbankmodelle.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, Integer, DateTime
from sqlalchemy.sql import func

from database import Base

class TimestampMixin:
    """Mixin-Klasse für Zeitstempel in Modellen"""
    created_at = Column(DateTime(timezone=True), server_default=func.now(), nullable=False)
    updated_at = Column(DateTime(timezone=True), onupdate=func.now(), nullable=True)

class CrudMixin:
    """
    Mixin-Klasse für grundlegende CRUD-Methoden.
    """
    id = Column(Integer, primary_key=True, index=True)
    
    @classmethod
    def get_by_id(cls, db, id):
        """Objekt anhand der ID abrufen"""
        return db.query(cls).filter(cls.id == id).first()
    
    @classmethod
    def get_all(cls, db, skip=0, limit=100):
        """Alle Objekte abrufen"""
        return db.query(cls).offset(skip).limit(limit).all()
    
    @classmethod
    def create(cls, db, **kwargs):
        """Neues Objekt erstellen"""
        obj = cls(**kwargs)
        db.add(obj)
        db.commit()
        db.refresh(obj)
        return obj
    
    def update(self, db, **kwargs):
        """Objekt aktualisieren"""
        for key, value in kwargs.items():
            setattr(self, key, value)
        db.commit()
        db.refresh(self)
        return self
    
    def delete(self, db):
        """Objekt löschen"""
        db.delete(self)
        db.commit()
        return True