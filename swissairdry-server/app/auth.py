"""
SwissAirDry - Authentifizierung

Dieses Modul enthält die Authentifizierungsfunktionen für die SwissAirDry-API.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import logging
import jwt
from datetime import datetime, timedelta
from typing import Dict, Any, Optional
from fastapi import Depends, HTTPException, Header, status
from fastapi.security import OAuth2PasswordBearer
from sqlalchemy.orm import Session
from .database import get_db
from .models.user import User

# Logger einrichten
logger = logging.getLogger(__name__)

# OAuth2 Bearer-Token-Scheme
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

# JWT-Konfiguration
JWT_SECRET = os.getenv("JWT_SECRET", "entwicklung_geheim_key")  # In Produktion sicherer Schlüssel notwendig
JWT_ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 60 * 24  # 24 Stunden Token-Gültigkeit


def create_access_token(data: dict, expires_delta: Optional[timedelta] = None) -> str:
    """
    Erstellt ein JWT-Token mit den angegebenen Daten und einer Ablaufzeit.
    """
    to_encode = data.copy()
    
    if expires_delta:
        expire = datetime.utcnow() + expires_delta
    else:
        expire = datetime.utcnow() + timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
    
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, JWT_SECRET, algorithm=JWT_ALGORITHM)
    
    return encoded_jwt


def verify_token(token: str) -> Dict[str, Any]:
    """
    Überprüft ein JWT-Token und gibt den Payload zurück, wenn das Token gültig ist.
    """
    try:
        payload = jwt.decode(token, JWT_SECRET, algorithms=[JWT_ALGORITHM])
        return payload
    except jwt.PyJWTError as e:
        logger.error(f"Token-Verifizierungsfehler: {str(e)}")
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Ungültiges Authentifizierungstoken",
            headers={"WWW-Authenticate": "Bearer"},
        )


def get_current_user(token: str = Depends(oauth2_scheme), db: Session = Depends(get_db)) -> Dict[str, Any]:
    """
    Ermittelt den aktuellen Benutzer basierend auf dem übergebenen JWT-Token.
    """
    payload = verify_token(token)
    
    username = payload.get("sub")
    if username is None:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Ungültiges Token-Format",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    # Benutzer aus der Datenbank laden
    user = db.query(User).filter(User.username == username).first()
    if user is None:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Benutzer nicht gefunden",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    # Prüfen, ob der Benutzer aktiv ist
    if not user.is_active:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Benutzer ist deaktiviert",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    # Benutzerinformationen zurückgeben
    return {
        "id": user.id,
        "username": user.username,
        "email": user.email,
        "is_admin": user.is_admin,
        "role": "admin" if user.is_admin else "user",
        "group_ids": [group.id for group in user.groups],
        "groups": [group.name for group in user.groups]
    }


def verify_nextcloud_token(token: str) -> Dict[str, Any]:
    """
    Verifiziert ein von Nextcloud gesendetes Token.
    
    In einer realen Implementierung würde dies möglicherweise einen
    kryptografischen Schlüssel prüfen oder eine Nextcloud-API aufrufen,
    um das Token zu validieren.
    """
    try:
        # In dieser Beispielimplementierung wird das Token direkt als JWT behandelt
        payload = jwt.decode(token, JWT_SECRET, algorithms=[JWT_ALGORITHM])
        
        # Prüfen, ob es sich um ein Nextcloud-Token handelt
        if payload.get("iss") != "nextcloud":
            raise ValueError("Token wurde nicht von Nextcloud ausgestellt")
        
        return {
            "user_id": payload.get("sub"),
            "display_name": payload.get("name"),
            "email": payload.get("email"),
            "groups": payload.get("groups", []),
        }
    except Exception as e:
        logger.error(f"Nextcloud-Token-Verifizierungsfehler: {str(e)}")
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Ungültiges Nextcloud-Token",
            headers={"WWW-Authenticate": "Bearer"},
        )