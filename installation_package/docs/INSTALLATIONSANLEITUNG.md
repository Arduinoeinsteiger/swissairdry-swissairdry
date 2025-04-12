# SwissAirDry-System: Installationsanleitung

Diese Anleitung führt Sie durch die Installation und Konfiguration des SwissAirDry-Systems für Ihr Unternehmen.

## Inhaltsverzeichnis

1. [Systemvoraussetzungen](#1-systemvoraussetzungen)
2. [Installationsschritte](#2-installationsschritte)
3. [Konfiguration](#3-konfiguration)
4. [Erststart und Initialisierung](#4-erststart-und-initialisierung)
5. [Fehlerbehebung](#5-fehlerbehebung)
6. [Updates](#6-updates)

## 1. Systemvoraussetzungen

### Hardware-Anforderungen
- Server mit mindestens 4 CPU-Kernen
- Mindestens 8 GB RAM
- 50 GB freier Festplattenspeicher
- Internet-Verbindung

### Software-Voraussetzungen
- Linux-Betriebssystem (Debian 12 oder Ubuntu 22.04 LTS empfohlen)
- Docker CE (Version 23.0 oder höher)
- Docker Compose (Version 2.0 oder höher)
- Python 3.10 oder höher
- Git

## 2. Installationsschritte

### 2.1 Installationsskript verwenden (empfohlen)

Die einfachste Methode zur Installation ist unser automatisiertes Skript:

```bash
# Zum Installationsverzeichnis wechseln
cd installation_package/scripts

# Ausführbar machen
chmod +x install_swissairdry.sh

# Als Administrator ausführen
sudo ./install_swissairdry.sh
```

Das Skript führt automatisch folgende Schritte durch:
- Prüfung der Systemvoraussetzungen
- Installation fehlender Abhängigkeiten
- Einrichtung der PostgreSQL-Datenbank
- Konfiguration von Docker und Volumes
- Herunterladen und Entpacken der Anwendungskomponenten
- Konfiguration der Umgebungsvariablen

### 2.2 Manuelle Installation

Falls Sie lieber manuell vorgehen möchten:

1. **Docker und Docker Compose installieren:**
   ```bash
   sudo apt update
   sudo apt install docker.io docker-compose
   sudo systemctl enable docker
   sudo systemctl start docker
   sudo usermod -aG docker $USER
   ```

2. **Projektverzeichnis erstellen:**
   ```bash
   mkdir -p /opt/swissairdry
   cd /opt/swissairdry
   ```

3. **Komponenten entpacken:**
   ```bash
   # Kopieren Sie alle ZIP-Dateien in das Verzeichnis
   cp installation_package/files/*.zip /opt/swissairdry/
   
   # Entpacken
   unzip swissairdry_admin_dashboard.zip
   unzip docker-base-api.zip
   unzip mobile-api.zip
   unzip esp32-firmware.zip
   unzip nextcloud-frontend.zip
   ```

4. **Datenbank-Container erstellen:**
   ```bash
   docker volume create swissairdry_pgdata
   docker-compose -f docker-compose.yml up -d postgres
   ```

## 3. Konfiguration

### 3.1 Umgebungsvariablen

Bearbeiten Sie die Datei `.env` im Hauptverzeichnis:

```env
# API-Konfiguration
API_BASE_URL=https://api.vgnc.org
PORT=5000
DEBUG=False

# Datenbank-Verbindung
DATABASE_URL=postgresql://swissairdry:password@postgres:5432/swissairdry

# Nextcloud-Integration
NEXTCLOUD_FULL_URL=https://vgnc.org/apps/swissairdry
NEXTCLOUD_USERNAME=admin
NEXTCLOUD_PASSWORD=your_password

# MQTT-Konfiguration
MQTT_BROKER_HOST=mosquitto
MQTT_BROKER_PORT=1883
MQTT_USERNAME=swissairdry
MQTT_PASSWORD=your_password

# JWT-Secrets
JWT_SECRET_KEY=your_secure_random_string
JWT_ALGORITHM=HS256
JWT_EXPIRATION_MINUTES=1440
```

Ersetzen Sie alle Passwörter und Secrets durch sichere Werte!

### 3.2 Domaineinrichtung

Die Anwendung verwendet folgende Domains, die Sie in Ihrer Netzwerkumgebung konfigurieren müssen:

- `api.vgnc.org` - Backend-API
- `vgnc.org` - Nextcloud-Integration

Für Entwicklungs- und Testzwecke können Sie diese Domains in Ihrer lokalen `/etc/hosts`-Datei einrichten oder einen Reverse-Proxy wie Nginx verwenden.

### 3.3 TLS/SSL-Konfiguration

Für eine sichere Produktionsumgebung empfehlen wir die Einrichtung von TLS/SSL:

1. Letsencrypt Zertifikate beziehen:
   ```bash
   # Im Skript enthalten oder manuell mit certbot
   ```

2. Docker-Compose mit TLS-Konfiguration aktualisieren (siehe `docker-compose.prod.yml`)

## 4. Erststart und Initialisierung

### 4.1 Starten aller Dienste

```bash
cd /opt/swissairdry
docker-compose up -d
```

### 4.2 Datenbank-Initialisierung

```bash
# Container-Shell öffnen
docker-compose exec docker-base-api bash

# Datenbank initialisieren
cd app
python init_db.py
python init_sample_users.py
```

### 4.3 Erstanmeldung

Öffnen Sie die Admin-Oberfläche unter `https://api.vgnc.org/admin` und melden Sie sich an:

- Benutzername: `admin`
- Passwort: `admin123` (ändern Sie dies sofort nach der ersten Anmeldung!)

## 5. Fehlerbehebung

### Häufige Probleme

1. **Docker-Container starten nicht:**
   ```bash
   # Logs prüfen
   docker-compose logs
   ```

2. **Datenbank-Verbindungsprobleme:**
   ```bash
   # Datenbank-Logs prüfen
   docker-compose logs postgres
   
   # Verbindung testen
   docker-compose exec postgres psql -U swissairdry -d swissairdry -c "SELECT 1;"
   ```

3. **API ist nicht erreichbar:**
   ```bash
   # Prüfen, ob der Port geöffnet ist
   curl http://localhost:5000/health
   
   # API-Logs prüfen
   docker-compose logs docker-base-api
   ```

## 6. Updates

### Updates installieren

```bash
# Zum Installationsverzeichnis wechseln
cd /opt/swissairdry

# Aktuelle Container stoppen
docker-compose down

# Code aktualisieren (bei Updates)
cp installation_package/files/*.zip .
unzip -o swissairdry_admin_dashboard.zip
# weitere ZIP-Dateien nach Bedarf

# Container neu starten
docker-compose up -d

# Datenbankmigrationen ausführen
docker-compose exec docker-base-api bash -c "cd app && python migrations.py"
```

Für weitere Unterstützung wenden Sie sich bitte an:
support@swissairdry.com