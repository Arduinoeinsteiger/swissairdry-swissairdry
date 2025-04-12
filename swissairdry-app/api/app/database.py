"""
SwissAirDry - Datenbankanbindung

Modul zur Anbindung an die PostgreSQL-Datenbank.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from typing import Generator

import sqlalchemy
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session
from sqlalchemy.pool import QueuePool

import config

# Datenbankverbindung erstellen
engine = create_engine(
    config.DATABASE_URL,
    pool_pre_ping=True,
    pool_recycle=3600,
    pool_size=5,
    max_overflow=10,
    poolclass=QueuePool,
)

# Sessionmaker konfigurieren
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# Basisklasse für alle Modelle
Base = declarative_base()

def get_db() -> Generator[Session, None, None]:
    """
    Erstellt eine neue Datenbankverbindung für jeden Request und schließt sie danach.
    Wird als Dependency Injection in den FastAPI-Endpunkten verwendet.
    """
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()