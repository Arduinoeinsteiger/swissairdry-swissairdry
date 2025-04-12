"""
SwissAirDry - Gruppenmodelle

Dieses Modul enthält die Datenmodelle für Benutzergruppen
und Berechtigungen im SwissAirDry-System.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, Integer, String, Text, DateTime, JSON, ForeignKey, Table
from sqlalchemy.orm import relationship
from sqlalchemy.sql import func
from database import Base
from typing import Dict, Any, List

# Zwischentabelle für Benutzer und Gruppen
user_groups = Table(
    'user_groups',
    Base.metadata,
    Column('user_id', Integer, ForeignKey('users.id'), primary_key=True),
    Column('group_id', Integer, ForeignKey('groups.id'), primary_key=True)
)

# Zwischentabelle für Gruppen und API-Berechtigungen
group_api_permissions = Table(
    'group_api_permissions',
    Base.metadata,
    Column('group_id', Integer, ForeignKey('groups.id'), primary_key=True),
    Column('permission_id', Integer, ForeignKey('api_permissions.id'), primary_key=True)
)

# Zwischentabelle für Gruppen und Dashboard-Module
group_dashboard_modules = Table(
    'group_dashboard_modules',
    Base.metadata,
    Column('group_id', Integer, ForeignKey('groups.id'), primary_key=True),
    Column('module_id', Integer, ForeignKey('dashboard_modules.id'), primary_key=True)
)

class Group(Base):
    """Benutzergruppe mit Berechtigungen"""
    __tablename__ = 'groups'
    
    id = Column(Integer, primary_key=True)
    name = Column(String(50), unique=True, nullable=False)
    description = Column(Text, nullable=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    
    # Beziehungen
    users = relationship("User", secondary=user_groups, back_populates="groups")
    api_permissions = relationship("ApiPermission", secondary=group_api_permissions, back_populates="groups")
    dashboard_modules = relationship("DashboardModule", secondary=group_dashboard_modules, back_populates="groups")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Gruppe in ein Dictionary"""
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "created_at": self.created_at.isoformat() if self.created_at else None,
            "updated_at": self.updated_at.isoformat() if self.updated_at else None,
            "user_count": len(self.users),
            "permissions": [p.to_dict() for p in self.api_permissions],
            "modules": [m.to_dict() for m in self.dashboard_modules]
        }

class ApiPermission(Base):
    """API-Berechtigung für Benutzergruppen"""
    __tablename__ = 'api_permissions'
    
    id = Column(Integer, primary_key=True)
    endpoint = Column(String(255), nullable=False)
    methods = Column(JSON, nullable=False)  # Liste der erlaubten HTTP-Methoden
    description = Column(Text, nullable=True)
    
    # Beziehungen
    groups = relationship("Group", secondary=group_api_permissions, back_populates="api_permissions")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die API-Berechtigung in ein Dictionary"""
        return {
            "id": self.id,
            "endpoint": self.endpoint,
            "methods": self.methods,
            "description": self.description
        }

class DashboardModule(Base):
    """Dashboard-Modul für Benutzergruppen"""
    __tablename__ = 'dashboard_modules'
    
    id = Column(Integer, primary_key=True)
    module_id = Column(String(50), unique=True, nullable=False)  # z.B. "auftraege"
    name = Column(String(100), nullable=False)
    description = Column(Text, nullable=True)
    icon = Column(String(50), nullable=True)
    path = Column(String(255), nullable=False)
    requires_permissions = Column(JSON, nullable=True)  # Liste der erforderlichen API-Berechtigungen
    
    # Beziehungen
    groups = relationship("Group", secondary=group_dashboard_modules, back_populates="dashboard_modules")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das Dashboard-Modul in ein Dictionary"""
        return {
            "id": self.id,
            "module_id": self.module_id,
            "name": self.name,
            "description": self.description,
            "icon": self.icon,
            "path": self.path,
            "requires_permissions": self.requires_permissions
        }