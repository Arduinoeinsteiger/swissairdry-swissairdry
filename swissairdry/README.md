# SwissAirDry Projekt

Eine umfassende Lösung für die Verwaltung von Trocknungsgeräten und Feldservice-Operationen mit IoT-Integration.

## Projektstruktur

Das Projekt besteht aus vier Hauptkomponenten:

1. **API** - FastAPI-Backend-Server mit PostgreSQL-Datenbank und MQTT-Broker
2. **App** - Web-Frontend (Vue.js) für die Hauptanwendung
3. **Nextcloud** - Integration als External App für Nextcloud via AppAPI
4. **Mobile** - Android-App für Feldtechniker (Kotlin)

## Installation

Siehe `docs/INSTALLATION_ANLEITUNG.md` für detaillierte Installationsanweisungen.

## Entwicklung

### API-Server starten
```bash
cd swissairdry
docker-compose up -d
```

### Zugriff auf die APIs
- Haupt-API: http://localhost:5000
- Backup-API: http://swissairdry.replit.app (falls konfiguriert)

## Fehlersuche

Siehe `docs/FEHLERBEHEBUNG.md` für Informationen zur Behebung häufiger Probleme.

## Kontakt

Bei Fragen oder Problemen wenden Sie sich an das Swiss Air Dry Team.