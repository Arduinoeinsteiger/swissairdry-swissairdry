"""
SwissAirDry - Datenbankverbindung

Enthält die Datenbankverbindung und Session für die SwissAirDry API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

from dotenv import load_dotenv

# Lade Umgebungsvariablen aus .env Datei
load_dotenv()

# Datenbankverbindung
DATABASE_URL = os.getenv("DATABASE_URL")

# Für SQLite, falls kein DATABASE_URL definiert ist (nur für Entwicklung)
if not DATABASE_URL:
    DATABASE_URL = "sqlite:///./swissairdry.db"
    engine = create_engine(
        DATABASE_URL, connect_args={"check_same_thread": False}
    )
else:
    # Für PostgreSQL
    engine = create_engine(
        DATABASE_URL,
        pool_size=5,
        max_overflow=10,
        pool_recycle=3600,
        pool_pre_ping=True
    )

# Session erstellen
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# Basisklasse für SQLAlchemy-Modelle
Base = declarative_base()


# Hilfsfunktion zum Abrufen der Datenbank-Session
def get_db():
    """Liefert eine Datenbank-Session und schließt sie nach Verwendung"""
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()