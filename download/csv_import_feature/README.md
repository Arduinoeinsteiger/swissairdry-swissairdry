# SwissAirDry CSV-Import-Funktionalität

## Inhalt des Pakets

Dieses Paket enthält die notwendigen Dateien und Anleitungen zur Integration der CSV-Import-Funktionalität in Ihre bestehende SwissAirDry-Installation.

### Verzeichnisstruktur

```
csv_import_feature/
├── code/                     # Quellcode-Dateien
│   ├── admin_ui.py           # Admin-UI-Router mit CSV-Import-Endpunkt
│   ├── data_processing.py    # Datenverarbeitungs-Router für CSV-Upload
│   ├── csv_import.html       # HTML-Template für CSV-Import-Seite
│   └── utils/
│       └── csv_processor.py  # CSV-Verarbeitungsklasse
├── docs/                     # Dokumentation
│   ├── INSTALLATION.md       # Installationsanleitung
│   └── FUNKTIONEN.md         # Funktionsübersicht
└── README.md                 # Diese Datei
```

## Schnellstart

1. Lesen Sie die [Installationsanleitung](docs/INSTALLATION.md) für detaillierte Anweisungen.
2. Kopieren Sie die Dateien aus dem `code/`-Verzeichnis in Ihre SwissAirDry-Installation.
3. Starten Sie Ihren Docker-Container neu.
4. Greifen Sie auf die CSV-Import-Seite zu unter `/admin/csv-import`.

## Dokumentation

- [Installationsanleitung](docs/INSTALLATION.md) - Schritte zur Integration der Funktionalität
- [Funktionsübersicht](docs/FUNKTIONEN.md) - Detaillierte Beschreibung der Funktionalität

## Unterstützte CSV-Dateien

Die Funktionalität unterstützt folgende SwissAirDry-CSV-Dateien:

- SwissAirDry_KUNDENSTAMM.csv
- SwissAirDry_GERAETESTAMMVERZEICHNISS.csv
- SwissAirDry_AUFTRAGSPROTOKOLL.csv
- SwissAirDry_GERAETESTANDORTWECHSELPROTOKOLL.csv
- SwissAirDry_MESSPROTOKOLLWERTERFASSUNG.csv

## Version

Version: 1.0.0
Datum: 11. April 2025
Erstellt für: SwissAirDry Docker Base API

## Kontakt

Bei Fragen oder Problemen wenden Sie sich an:
- E-Mail: info@swissairdry.com