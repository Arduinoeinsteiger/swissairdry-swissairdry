from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    PROJECT_NAME: str = "SwissAirDry API"
    VERSION: str = "0.1.0"
    API_V1_STR: str = "/api/v1"

    # MQTT Einstellungen
    MQTT_BROKER: str = "mqtt"
    MQTT_PORT: int = 1883
    MQTT_USERNAME: str = "swissairdry"
    MQTT_PASSWORD: str = "swissairdry"

    # Datenbank Einstellungen
    POSTGRES_SERVER: str = "db"
    POSTGRES_USER: str = "swissairdry"
    POSTGRES_PASSWORD: str = "swissairdry"
    POSTGRES_DB: str = "swissairdry"
    SQLALCHEMY_DATABASE_URI: str = f"postgresql://{POSTGRES_USER}:{POSTGRES_PASSWORD}@{POSTGRES_SERVER}/{POSTGRES_DB}"

    class Config:
        case_sensitive = True
        env_file = ".env"

settings = Settings() 