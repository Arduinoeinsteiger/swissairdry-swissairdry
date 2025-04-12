"""
SwissAirDry - Gruppenmodelle

Dieses Modul enthält die Datenbankmodelle für Benutzergruppen und Berechtigungen.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import Column, String, Text, Integer, ForeignKey, Table
from sqlalchemy.types import JSON
from sqlalchemy.orm import relationship
from typing import Dict, Any, List

from database import Base
from models.base import TimestampMixin, CrudMixin

# Hilfstabelle für die Gruppen-Berechtigungen-Beziehung
group_permissions = Table(
    'group_permissions',
    Base.metadata,
    Column('group_id', Integer, ForeignKey('groups.id'), primary_key=True),
    Column('permission_id', Integer, ForeignKey('permissions.id'), primary_key=True)
)

# Hilfstabelle für die Gruppen-Dashboard-Module-Beziehung
group_dashboard_modules = Table(
    'group_dashboard_modules',
    Base.metadata,
    Column('group_id', Integer, ForeignKey('groups.id'), primary_key=True),
    Column('module_id', Integer, ForeignKey('dashboard_modules.id'), primary_key=True)
)

class Group(Base, TimestampMixin, CrudMixin):
    """Gruppenmodell für Benutzergruppen"""
    __tablename__ = 'groups'

    name = Column(String(50), unique=True, nullable=False, index=True)
    description = Column(Text, nullable=True)
    
    # Beziehungen
    users = relationship("User", secondary="user_groups", back_populates="groups")
    permissions = relationship("Permission", secondary=group_permissions, back_populates="groups")
    dashboard_modules = relationship("DashboardModule", secondary=group_dashboard_modules, back_populates="groups")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Gruppe in ein Dictionary"""
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "permissions": [p.name for p in self.permissions],
            "dashboardModules": [m.module_id for m in self.dashboard_modules],
            "userCount": len(self.users),
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class Permission(Base, TimestampMixin, CrudMixin):
    """Berechtigungsmodell für Benutzergruppen"""
    __tablename__ = 'permissions'

    name = Column(String(100), unique=True, nullable=False, index=True)
    description = Column(Text, nullable=True)
    resource = Column(String(100), nullable=False)  # z.B. "jobs", "devices"
    action = Column(String(50), nullable=False)  # z.B. "read", "write", "delete"
    
    # Beziehungen
    groups = relationship("Group", secondary=group_permissions, back_populates="permissions")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert die Berechtigung in ein Dictionary"""
        return {
            "id": self.id,
            "name": self.name,
            "description": self.description,
            "resource": self.resource,
            "action": self.action,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }

class DashboardModule(Base, TimestampMixin, CrudMixin):
    """Dashboard-Modul für Benutzergruppen"""
    __tablename__ = 'dashboard_modules'

    module_id = Column(String(50), unique=True, nullable=False, index=True)  # z.B. "devices"
    name = Column(String(100), nullable=False)
    description = Column(Text, nullable=True)
    icon = Column(String(50), nullable=True)
    path = Column(String(255), nullable=False)
    requires_permissions = Column(JSON, nullable=True)  # Liste der erforderlichen Berechtigungen
    
    # Beziehungen
    groups = relationship("Group", secondary=group_dashboard_modules, back_populates="dashboard_modules")
    
    def to_dict(self) -> Dict[str, Any]:
        """Konvertiert das Dashboard-Modul in ein Dictionary"""
        return {
            "id": self.id,
            "moduleId": self.module_id,
            "name": self.name,
            "description": self.description,
            "icon": self.icon,
            "path": self.path,
            "requiresPermissions": self.requires_permissions,
            "createdAt": self.created_at.isoformat() if self.created_at else None,
            "updatedAt": self.updated_at.isoformat() if self.updated_at else None
        }