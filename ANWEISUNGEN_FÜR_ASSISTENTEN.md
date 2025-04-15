# Anweisungen für Assistenten

## Übersicht

Dieses Dokument enthält Anweisungen für den KI-Assistenten bezüglich der Arbeit mit dem SwissAirDry-Repository. Es beschreibt, in welchem Ordner welche Aufgaben ausgeführt werden sollen und wie die Repository-Struktur organisiert ist.

## Repository-Struktur

Die aktuelle Repository-Struktur ist in 5 Hauptordner organisiert:

```
swissairdry_core/
├── api/                # FastAPI Backend-Server
├── web/                # Web-Anwendung (Browser)
├── mobile/             # Android-Anwendung (Kotlin)
├── nextcloud/          # Nextcloud-Integration
└── docs/               # Dokumentation und Anleitungen
```

## Anweisungen für bestimmte Aufgaben

### 1. API-Entwicklung

**Arbeitsordner:** `swissairdry_core/api/`

In diesem Ordner:
- FastAPI-Server-Code entwickeln und anpassen
- SQL-Datenbankschema verwalten
- API-Endpunkte implementieren
- Authentifizierung und Autorisierung steuern

**Wichtige Dateien:**
- `app/run.py` - Hauptserver-Datei
- `app/config.py` - Konfigurationseinstellungen
- `app/models/` - Datenbankmodelle
- `app/routes/` - API-Endpunkte

### 2. Web-Anwendung

**Arbeitsordner:** `swissairdry_core/web/`

In diesem Ordner:
- HTML, CSS und JavaScript-Dateien für die Webanwendung bearbeiten
- Webkomponenten entwickeln
- Frontend-Logik implementieren
- UI/UX der Webanwendung verbessern

**Wichtige Dateien:**
- `index.html` - Haupteinstiegspunkt der Web-App
- `app.js` - JavaScript-Hauptdatei
- `css/` - Styling-Dateien
- `components/` - Wiederverwendbare UI-Komponenten

### 3. Mobile App-Entwicklung

**Arbeitsordner:** `swissairdry_core/mobile/`

In diesem Ordner:
- Kotlin-basierte Android-App entwickeln
- Mobile UI-Komponenten erstellen
- REST-API-Integration implementieren
- Offline-Funktionalität verbessern

**Wichtige Dateien:**
- `app/src/main/` - Hauptquellcode
- `app/src/main/kotlin/` - Kotlin-Code
- `app/src/main/res/` - Ressourcendateien
- `build.gradle` - Build-Konfiguration

### 4. Nextcloud-Integration

**Arbeitsordner:** `swissairdry_core/nextcloud/`

In diesem Ordner:
- Nextcloud-App-Integration entwickeln
- Kompatibilität mit AppAPI sicherstellen
- Dateimanagement-Funktionen implementieren
- OAuth2-Authentifizierung konfigurieren

**Wichtige Dateien:**
- `appapi.json` - App-API-Konfiguration
- `index.php` - Haupteinstiegspunkt
- `api.php` - API-Schnittstelle
- `templates/` - Vorlagen für die Nextcloud-Ansicht

### 5. Dokumentation

**Arbeitsordner:** `swissairdry_core/docs/`

In diesem Ordner:
- Installations- und Konfigurationsanleitungen erstellen
- API-Dokumentation aktualisieren
- Benutzerhandbücher schreiben
- Fehlerbehebungsanweisungen pflegen

**Wichtige Dateien:**
- `SWISSAIRDRY_KOMPLETTANLEITUNG.md` - Hauptdokumentation

### 6. Allgemeine Konfiguration

**Arbeitsordner:** `swissairdry_core/`

In diesem Ordner:
- Docker-Konfiguration (`docker-compose.yml`) anpassen
- Umgebungsvariablen (`.env`) konfigurieren
- MQTT-Broker-Einstellungen (`mqtt/config/mosquitto.conf`) verwalten
- Grundlegende README-Informationen aktualisieren

## Häufige Aufgaben und Standardvorgehensweisen

### Installation einrichten

```bash
cd swissairdry_core
cp .env.example .env
# Anpassen der .env-Datei
docker-compose up -d
```

### API-Server entwickeln

```bash
cd swissairdry_core/api
# Code bearbeiten
docker-compose restart api
```

### Mobile App kompilieren

```bash
cd swissairdry_core/mobile
./gradlew assembleDebug
```

### Nextcloud-App installieren

```bash
# Auf dem Nextcloud-Server:
cd swissairdry_core/nextcloud
# Dateien ins Nextcloud-Apps-Verzeichnis kopieren
sudo -u www-data php /var/www/nextcloud/occ app:enable swissairdry
```

## Problemlösungsansätze

1. Bei API-Serverproblemen:
   - Logs in `swissairdry_core/api/logs/` überprüfen
   - Docker-Container neustarten: `docker-compose restart api`

2. Bei Datenbankproblemen:
   - PostgreSQL-Verbindung prüfen: `docker-compose exec postgres psql -U swissairdry -d swissairdry`
   - Backup erstellen: `docker-compose exec postgres pg_dump -U swissairdry swissairdry > backup.sql`

3. Bei Nextcloud-Integrationsproblemen:
   - Nextcloud-Logs überprüfen: `/var/www/nextcloud/data/nextcloud.log`
   - AppAPI-Status prüfen: `sudo -u www-data php /var/www/nextcloud/occ app:list | grep app_api`

## Sicherheitshinweise

1. Keine Passwörter oder API-Schlüssel hartcodieren
2. Produktionsumgebungen immer mit HTTPS absichern
3. Keine sensiblen Daten in öffentliche Repositories einchecken
4. `.env`-Datei niemals ins Repository einchecken

## Aktualisierungen und Versionierung

1. Semantische Versionierung für API-Änderungen verwenden
2. Migrationen für Datenbankschema-Änderungen erstellen
3. Änderungsprotokoll (`CHANGELOG.md`) für signifikante Änderungen führen
4. Bei Docker-Image-Updates backward-kompatible Updates bevorzugen

---

Diese Anweisungen dienen als Leitfaden für den KI-Assistenten und können bei Bedarf aktualisiert werden.