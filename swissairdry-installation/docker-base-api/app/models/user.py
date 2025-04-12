"""
SwissAirDry - Benutzermodelle

Dieses Modul enthält die Datenbankmodelle für Benutzer und Authentifizierung.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import bcrypt
from sqlalchemy import Column, String, Boolean, Integer, ForeignKey, Table
from sqlalchemy.orm import relationship
from sqlalchemy.sql import func
from typing import Dict, Any, List, Optional

from database import Base
from models.base import TimestampMixin, CrudMixin

# Hilfstabelle für die Benutzer-Gruppen-Beziehung
user_groups = Table(
    'user_groups',
    Base.metadata,
    Column('user_id', Integer, ForeignKey('users.id'), primary_key=True),
    Column('group_id', Integer, ForeignKey('groups.id'), primary_key=True)
)

class User(Base, TimestampMixin, CrudMixin):
    """Benutzermodell für Authentifizierung und Autorisierung"""
    __tablename__ = 'users'

    username = Column(String(50), unique=True, nullable=False, index=True)
    email = Column(String(100), unique=True, nullable=False, index=True)
    password_hash = Column(String(128), nullable=False)
    full_name = Column(String(100), nullable=True)
    is_active = Column(Boolean, default=True, nullable=False)
    is_admin = Column(Boolean, default=False, nullable=False)
    last_login = Column(String(50), nullable=True)  # ISO-Format Zeitstempel

    # Beziehungen
    groups = relationship("Group", secondary=user_groups, back_populates="users")
    
    def set_password(self, password: str) -> None:
        """Setzt das Passwort (gehashed)"""
        password_bytes = password.encode('utf-8')
        salt = bcrypt.gensalt()
        self.password_hash = bcrypt.hashpw(password_bytes, salt).decode('utf-8')
    
    def check_password(self, password: str) -> bool:
        """Überprüft, ob das gegebene Passwort korrekt ist"""
        password_bytes = password.encode('utf-8')
        hash_bytes = self.password_hash.encode('utf-8')
        return bcrypt.checkpw(password_bytes, hash_bytes)
    
    def has_permission(self, permission: str) -> bool:
        """Überprüft, ob der Benutzer die angegebene Berechtigung hat"""
        # Admin hat alle Berechtigungen
        if self.is_admin:
            return True
        
        # Überprüfe Gruppenberechtigungen
        for group in self.groups:
            for perm in group.permissions:
                if perm.name == permission:
                    return True
        
        return False
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert den Benutzer in ein Dictionary"""
        return {
            "id": self.id,
            "username": self.username,
            "email": self.email,
            "fullName": self.full_name,
            "isActive": self.is_active,
            "isAdmin": self.is_admin,
            "lastLogin": self.last_login,
            "groups": [group.name for group in self.groups],
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }
    
    @classmethod
    def authenticate(cls, db, username: str, password: str) -> Optional['User']:
        """Authentifiziert einen Benutzer anhand von Benutzername und Passwort"""
        user = db.query(cls).filter(cls.username == username).first()
        if user and user.is_active and user.check_password(password):
            # Aktualisiere last_login
            user.last_login = func.now()
            db.commit()
            return user
        return None