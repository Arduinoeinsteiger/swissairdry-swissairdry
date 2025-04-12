"""
SwissAirDry - Datenbankanbindung

Modul zur Anbindung an die PostgreSQL-Datenbank.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from dotenv import load_dotenv

# Umgebungsvariablen laden
load_dotenv()

# Datenbankverbindung konfigurieren
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://swissairdry:swissairdry@localhost:5432/swissairdry")

# SQLAlchemy-Engine erstellen
engine = create_engine(
    DATABASE_URL,
    pool_pre_ping=True, # Prüft, ob die Verbindung noch aktiv ist
    pool_recycle=300  # Verbindung nach 5 Minuten recyclen
)

# SessionLocal-Klasse erstellen
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# Base-Klasse für Modelle
Base = declarative_base()

# Abhängigkeit für Datenbankverbindung in Endpunkten
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