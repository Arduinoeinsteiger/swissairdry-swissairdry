# SwissAirDry - Komplettpaket

Dieses Komplettpaket enthält alle Komponenten und Anleitungen für die Installation und Einrichtung des SwissAirDry-Systems. Das Paket ist in folgende Bereiche gegliedert:

## Ordnerstruktur

- **installation/** - Modulare Installationsskripte und Dokumentation
  - **scripts/** - Installationsskripte für verschiedene Komponenten
  - **docs/** - Detaillierte Dokumentation zur Installation
  - **config/** - Konfigurationsvorlagen
  - **nextcloud_integration/** - Dateien für die Nextcloud-Integration
  - **api_service/** - Dateien für den API-Dienst

- **docker-base-api/** - Vollständiger Quellcode des API-Dienstes
  - **app/** - Hauptanwendungscode des API-Dienstes

- **nextcloud-app/** - Dateien für die Nextcloud-App

- **anleitungen/** - Allgemeine Anleitungen zur Verwendung des Systems

## Schnellstart

Für eine einfache Installation mit einer bestehenden Nextcloud-Docker-Instanz:

```bash
cd installation/scripts
chmod +x *.sh
./docker_nextcloud_integration.sh
```

Dieser Assistent führt Sie durch die Integration mit Ihrer bestehenden Nextcloud-Installation.

## Detaillierte Anleitungen

Bitte lesen Sie die folgenden Dokumente für detaillierte Informationen:

1. [Installationsanleitung](installation/docs/INSTALLATION.md) - Vollständige Installationsanleitung
2. [Docker-Nextcloud-Integration](installation/docs/DOCKER_NEXTCLOUD_INTEGRATION.md) - Spezifische Anleitung für Docker-Umgebungen
3. [FAQ und Fehlerbehebung](installation/docs/FAQ_UND_FEHLERBEHEBUNG.md) - Häufig gestellte Fragen und Problemlösungen

## Systemanforderungen

- Docker und Docker Compose
- Nextcloud-Installation (Version 25+)
- Cloud-Py-Api App aus dem Nextcloud App Store
- PostgreSQL-Datenbank
- 2 GB RAM und 20 GB Festplattenspeicher

## Support

Bei Fragen oder Problemen wenden Sie sich bitte an:
- Support-E-Mail: info@swissairdry.com
