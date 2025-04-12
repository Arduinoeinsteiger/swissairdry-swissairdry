"""
SwissAirDry - Datenbankanbindung

Modul zur Anbindung an die PostgreSQL-Datenbank.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from sqlalchemy.pool import QueuePool

from config import DATABASE_URL

# SQLAlchemy-Engine mit Connection-Pool erstellen
engine = create_engine(
    DATABASE_URL,
    pool_size=5,
    max_overflow=10,
    pool_timeout=30,
    pool_recycle=1800,
    pool_pre_ping=True,
    poolclass=QueuePool
)

# SessionLocal-Klasse erstellen, die die Datenbanksitzungen erstellt
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# Basis-Klasse für die Modelle erstellen
Base = declarative_base()

def get_db():
    """
    Erstellt eine neue Datenbankverbindung für jeden Request und schließt sie danach.
    Wird als Dependency Injection in den FastAPI-Endpunkten verwendet.
    """
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()