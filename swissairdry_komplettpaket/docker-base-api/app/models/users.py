"""
SwissAirDry - Benutzermodelle

Dieses Modul enthält die Datenmodelle für Benutzer
und Authentifizierung im SwissAirDry-System.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, Integer, String, Text, Boolean, DateTime, JSON, ForeignKey
from sqlalchemy.orm import relationship
from sqlalchemy.sql import func
from database import Base
from typing import Dict, Any, List
from passlib.context import CryptContext
from datetime import datetime, timedelta

from .groups import user_groups

# Passwort-Hashing-Kontext
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

class User(Base):
    """Benutzermodell mit Authentifizierung und Gruppenberechtigungen"""
    __tablename__ = 'users'
    
    id = Column(Integer, primary_key=True)
    username = Column(String(50), unique=True, nullable=False)
    email = Column(String(100), unique=True, nullable=False)
    password_hash = Column(String(255), nullable=False)
    full_name = Column(String(100), nullable=True)
    company = Column(String(100), nullable=True)
    phone = Column(String(20), nullable=True)
    is_active = Column(Boolean, default=True, nullable=False)
    is_admin = Column(Boolean, default=False, nullable=False)
    last_login = Column(DateTime(timezone=True), nullable=True)
    profile_data = Column(JSON, nullable=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    
    # Beziehungen
    groups = relationship("Group", secondary=user_groups, back_populates="users")
    
    def set_password(self, password: str) -> None:
        """Setzt das Passwort des Benutzers (gehashed)"""
        self.password_hash = pwd_context.hash(password)
    
    def verify_password(self, password: str) -> bool:
        """Überprüft, ob das eingegebene Passwort korrekt ist"""
        return pwd_context.verify(password, self.password_hash)
    
    def to_dict(self, include_groups: bool = False) -> Dict[str, Any]:
        """Konvertiert den Benutzer in ein Dictionary"""
        user_dict = {
            "id": self.id,
            "username": self.username,
            "email": self.email,
            "full_name": self.full_name,
            "company": self.company,
            "phone": self.phone,
            "is_active": self.is_active,
            "is_admin": self.is_admin,
            "last_login": self.last_login.isoformat() if self.last_login else None,
            "created_at": self.created_at.isoformat() if self.created_at else None,
            "updated_at": self.updated_at.isoformat() if self.updated_at else None,
        }
        
        if include_groups:
            user_dict["groups"] = [g.to_dict() for g in self.groups]
        
        return user_dict
    
    def has_permission(self, endpoint: str, method: str) -> bool:
        """Überprüft, ob der Benutzer die angegebene Berechtigung hat"""
        # Admin hat alle Berechtigungen
        if self.is_admin:
            return True
        
        # Prüfen, ob eine der Gruppen des Benutzers die Berechtigung hat
        for group in self.groups:
            for permission in group.api_permissions:
                if permission.endpoint == endpoint or permission.endpoint == "*":
                    if method in permission.methods or "*" in permission.methods:
                        return True
        
        return False
    
    def has_module_access(self, module_id: str) -> bool:
        """Überprüft, ob der Benutzer Zugriff auf ein bestimmtes Dashboard-Modul hat"""
        # Admin hat Zugriff auf alle Module
        if self.is_admin:
            return True
        
        # Prüfen, ob eine der Gruppen des Benutzers Zugriff auf das Modul hat
        for group in self.groups:
            for module in group.dashboard_modules:
                if module.module_id == module_id:
                    return True
        
        return False