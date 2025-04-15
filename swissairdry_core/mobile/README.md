# SwissAirDry Mobile App

Android-Anwendung für SwissAirDry, entwickelt für Feldtechniker zur Überwachung und Steuerung von Trocknungsgeräten.

## Architektur

Die App verwendet eine moderne MVVM-Architektur mit Repository-Pattern:

- **Model**: Datenmodelle und Repository-Klassen
- **View**: Activities und Fragments für die Benutzeroberfläche
- **ViewModel**: ViewModels zur Kommunikation zwischen Model und View

## Hauptfunktionen

- Echtzeit-Überwachung von Trocknungsgeräten
- Auftragsmanagement
- Fotoaufnahme für Dokumentation
- Offline-Funktionalität
- Push-Benachrichtigungen
- QR-Code-Scannen zur Geräteidentifikation

## API-Verbindung

Die App verbindet sich mit dem SwissAirDry-API-Server und enthält einen automatischen Failover-Mechanismus für erhöhte Zuverlässigkeit:

- Primärer API-Server: https://api.vgnc.org
- Backup-API-Server: https://swissairdry.replit.app

## Entwicklung

### Voraussetzungen

- Android Studio 4.2+
- Java JDK 11+
- Gradle 7.0+
- Android SDK 30+

### Build-Anweisungen

1. Projekt in Android Studio öffnen
2. Gradle-Sync durchführen
3. Anwendung auf Emulator oder Gerät ausführen

## Verteilung

Die APK-Datei kann über den Google Play Store oder direkt als APK verteilt werden.