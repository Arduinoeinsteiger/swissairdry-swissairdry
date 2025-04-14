-- Initiale Datenbankskripte für SwissAirDry
-- Dieses Skript wird beim ersten Start des Containers ausgeführt

-- Nextcloud-Datenbank und Benutzer erstellen, falls sie noch nicht existieren
CREATE DATABASE IF NOT EXISTS nextcloud;
CREATE USER IF NOT EXISTS nextcloud WITH ENCRYPTED PASSWORD 'nextcloud';
GRANT ALL PRIVILEGES ON DATABASE nextcloud TO nextcloud;

-- Hauptschema für die SwissAirDry-Anwendung
CREATE SCHEMA IF NOT EXISTS swissairdry;

-- Tabellen erstellen

-- Benutzer-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) NOT NULL UNIQUE,
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL DEFAULT 'user',
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    last_login TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Geräte-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.devices (
    id SERIAL PRIMARY KEY,
    serial_number VARCHAR(100) NOT NULL UNIQUE,
    name VARCHAR(255) NOT NULL,
    device_type VARCHAR(100) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'offline',
    last_seen TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Sensor-Daten-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.sensor_data (
    id SERIAL PRIMARY KEY,
    device_id INTEGER NOT NULL REFERENCES swissairdry.devices(id),
    temperature FLOAT,
    humidity FLOAT,
    pressure FLOAT,
    battery_level INTEGER,
    timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Kunden-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.customers (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    contact_person VARCHAR(255),
    email VARCHAR(255),
    phone VARCHAR(100),
    address TEXT,
    postal_code VARCHAR(20),
    city VARCHAR(100),
    country VARCHAR(100) DEFAULT 'Schweiz',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Job-Tabelle (Aufträge)
CREATE TABLE IF NOT EXISTS swissairdry.jobs (
    id SERIAL PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    customer_id INTEGER NOT NULL REFERENCES swissairdry.customers(id),
    location TEXT,
    status VARCHAR(50) NOT NULL DEFAULT 'pending',
    start_date TIMESTAMP,
    end_date TIMESTAMP,
    assigned_to INTEGER REFERENCES swissairdry.users(id),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Job-Geräte-Zuordnung (viele-zu-viele)
CREATE TABLE IF NOT EXISTS swissairdry.job_devices (
    id SERIAL PRIMARY KEY,
    job_id INTEGER NOT NULL REFERENCES swissairdry.jobs(id),
    device_id INTEGER NOT NULL REFERENCES swissairdry.devices(id),
    added_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(job_id, device_id)
);

-- Messungen-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.measurements (
    id SERIAL PRIMARY KEY,
    job_id INTEGER NOT NULL REFERENCES swissairdry.jobs(id),
    measured_by INTEGER REFERENCES swissairdry.users(id),
    temperature FLOAT,
    humidity FLOAT,
    notes TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Berichte-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.reports (
    id SERIAL PRIMARY KEY,
    job_id INTEGER NOT NULL REFERENCES swissairdry.jobs(id),
    created_by INTEGER REFERENCES swissairdry.users(id),
    title VARCHAR(255) NOT NULL,
    content TEXT,
    report_type VARCHAR(50) NOT NULL DEFAULT 'standard',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Bilder-Tabelle
CREATE TABLE IF NOT EXISTS swissairdry.images (
    id SERIAL PRIMARY KEY,
    job_id INTEGER NOT NULL REFERENCES swissairdry.jobs(id),
    report_id INTEGER REFERENCES swissairdry.reports(id),
    uploaded_by INTEGER REFERENCES swissairdry.users(id),
    filename VARCHAR(255) NOT NULL,
    file_path VARCHAR(255) NOT NULL,
    file_size INTEGER,
    mime_type VARCHAR(100),
    description TEXT,
    uploaded_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Administratorbenutzer einfügen, falls noch nicht vorhanden
INSERT INTO swissairdry.users (username, email, password_hash, role, first_name, last_name)
VALUES (
    'admin',
    'admin@swissairdry.com',
    -- Passwort: admin (in Produktion ändern!)
    '$2b$12$Nt0FNkOhSIJmpKa1CQ44QOCgYKaURhHFVJ.J6QwXDCfCciyGHLkfe',
    'admin',
    'Admin',
    'User'
) ON CONFLICT (username) DO NOTHING;

-- Hinweis: Weitere Demonutzer oder -daten können hier hinzugefügt werden.