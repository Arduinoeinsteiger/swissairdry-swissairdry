"""
SwissAirDry - Datenbankanbindung

Modul zur Anbindung an die PostgreSQL-Datenbank.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import logging
from typing import Generator
import sqlalchemy
from sqlalchemy import create_engine, MetaData
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session
from sqlalchemy.pool import NullPool

import config

# Logger konfigurieren
logger = logging.getLogger(__name__)

# SQLAlchemy Basis-Modell definieren
Base = declarative_base()

# Engine und Session erstellen
try:
    engine = create_engine(
        config.DATABASE_URL,
        pool_pre_ping=True,
        pool_recycle=3600,
        pool_size=10,
        max_overflow=20,
        echo=(config.LOG_LEVEL == "DEBUG")
    )
    SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
    logger.info("Datenbankverbindung hergestellt")
except Exception as e:
    logger.error(f"Fehler beim Verbinden zur Datenbank: {str(e)}")
    # Verwende NullPool für Tests ohne Datenbank
    engine = create_engine("sqlite:///:memory:", poolclass=NullPool)
    SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

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

def init_db() -> None:
    """
    Initialisiert die Datenbank und erstellt alle Tabellen.
    """
    # Import der Modelle, damit sie registriert werden
    from models import base
    try:
        Base.metadata.create_all(bind=engine)
        logger.info("Datenbank-Tabellen erstellt")
    except Exception as e:
        logger.error(f"Fehler beim Erstellen der Datenbank-Tabellen: {str(e)}")

def check_db_connection() -> bool:
    """
    Überprüft, ob eine Verbindung zur Datenbank hergestellt werden kann.
    """
    try:
        with engine.connect() as conn:
            conn.execute(sqlalchemy.text("SELECT 1"))
        return True
    except Exception as e:
        logger.error(f"Datenbankverbindung fehlgeschlagen: {str(e)}")
        return False