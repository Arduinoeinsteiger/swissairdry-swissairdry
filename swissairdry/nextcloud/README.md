# SwissAirDry Nextcloud-Integration

Integration der SwissAirDry-Anwendung in Nextcloud über die AppAPI.

## Übersicht

Die Nextcloud-Integration ermöglicht die Verwendung von SwissAirDry direkt aus der Nextcloud-Umgebung heraus. Sie stellt eine nahtlose Benutzererfahrung bereit, bei der die SwissAirDry-Funktionen in die Nextcloud-Oberfläche integriert sind.

## Funktionen

- Nahtlose Integration in die Nextcloud-Oberfläche
- Single Sign-On mit Nextcloud-Benutzerkonten
- Zugriff auf SwissAirDry-Funktionen direkt aus Nextcloud
- Dateiverwaltung für Fotos und Berichte über Nextcloud
- Konsistente Benutzeroberfläche im Nextcloud-Design

## Technologien

- PHP für die Backend-Integration
- JavaScript, HTML und CSS für das Frontend
- AppAPI für die Nextcloud-Integration
- REST-API-Kommunikation mit dem SwissAirDry-Server
- MQTT-Anbindung für Echtzeit-Updates

## Installation

Siehe [INSTALLATION.md](./INSTALLATION.md) für detaillierte Installationsanweisungen.

### Voraussetzungen

- Nextcloud 25 oder höher mit aktivierter AppAPI
- Zugriff auf den SwissAirDry-API-Server
- PHP 8.0 oder höher
- Docker für die Containerverwaltung

### Konfiguration

Nach der Installation kann die App über die Nextcloud-Einstellungen konfiguriert werden. Hier können die Verbindungsdaten für den API-Server und den MQTT-Broker hinterlegt werden.