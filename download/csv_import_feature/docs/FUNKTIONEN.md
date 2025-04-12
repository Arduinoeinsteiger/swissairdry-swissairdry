# SwissAirDry CSV-Import-Funktionalität - Funktionsübersicht

## Hauptfunktionen

Die CSV-Import-Funktionalität bietet folgende Hauptfunktionen:

1. **Hochladen von CSV-Dateien**: Ein benutzerfreundliches Interface zum Hochladen von SwissAirDry-CSV-Dateien.
2. **Automatische Erkennung und Verarbeitung**: Erkennt automatisch den Dateityp und wendet die entsprechende Verarbeitungslogik an.
3. **Daten-Mapping**: Ordnet die Spalten aus den alten CSV-Dateien den neuen Datenbankmodellen zu.
4. **Batch-Verarbeitung**: Ermöglicht das gleichzeitige Hochladen und Verarbeiten mehrerer Dateien.
5. **Import-Protokollierung**: Protokolliert Import-Vorgänge und zeigt den Verlauf früherer Imports an.

## CSV-Dateitypen und Mapping

### SwissAirDry_KUNDENSTAMM.csv
- Enthält Kundendaten, die in das `Customer`-Modell importiert werden
- Wichtige Felder: Kundennummer, Name, Kontaktperson, Adresse, Telefon, E-Mail

### SwissAirDry_GERAETESTAMMVERZEICHNISS.csv
- Enthält Geräteinformationen, die in das `Device`-Modell importiert werden
- Wichtige Felder: Geräte-ID, Name, Typ, Status, Firmware-Version

### SwissAirDry_AUFTRAGSPROTOKOLL.csv
- Enthält Auftrags-/Einsatzdaten, die in das `Job`-Modell importiert werden
- Wichtige Felder: Auftragsnummer, Kundenreferenz, Beschreibung, Start-/Enddatum, Status

### SwissAirDry_GERAETESTANDORTWECHSELPROTOKOLL.csv
- Enthält Gerätezuweisungen, die in das `JobDevice`-Modell importiert werden
- Wichtige Felder: Auftragsnummer, Geräte-ID, Start-/Enddatum, Standort

### SwissAirDry_MESSPROTOKOLLWERTERFASSUNG.csv
- Enthält Messwerte, die in das `Measurement`-Modell importiert werden
- Wichtige Felder: Geräte-ID, Zeitstempel, Temperatur, Luftfeuchtigkeit, Energieverbrauch

## Benutzeroberfläche

Die CSV-Import-Funktionalität bietet eine intuitive Benutzeroberfläche mit folgenden Elementen:

- **Dateiauswahl**: Multi-File-Upload-Funktion für CSV-Dateien
- **Fortschrittsanzeige**: Visualisierung des Upload- und Verarbeitungsfortschritts
- **Ergebnisanzeige**: Detaillierte Anzeige der Import-Ergebnisse
- **Import-Verlauf**: Tabelle mit Informationen zu früheren Import-Vorgängen

## Technische Details

### Verarbeitungsprozess

1. **Hochladen**: Dateien werden temporär auf dem Server gespeichert
2. **Header-Analyse**: CSV-Header werden analysiert, um den Dateityp zu bestimmen
3. **Daten-Extraktion**: Relevante Daten werden aus den CSV-Dateien extrahiert
4. **Datentransformation**: Daten werden in das Format der neuen Datenbankmodelle transformiert
5. **Datenvalidierung**: Daten werden auf Gültigkeit und Konsistenz geprüft
6. **Import**: Gültige Daten werden in die Datenbank importiert
7. **Protokollierung**: Import-Vorgang wird protokolliert und in der Benutzeroberfläche angezeigt

### API-Endpunkte

- `POST /api/data/csv/upload-batch`: Hochladen und Verarbeiten mehrerer CSV-Dateien
- `GET /api/data/csv/import-logs`: Abrufen der Import-Protokolle

### Dateizugriff und Sicherheit

- Temporäre Dateien werden nach der Verarbeitung automatisch gelöscht
- Import-Vorgänge werden im Hintergrund ausgeführt, um die Benutzeroberfläche nicht zu blockieren
- Fehlerhafte Datensätze werden protokolliert, aber der Import wird fortgesetzt

## Integration in die Benutzeroberfläche

Die CSV-Import-Funktionalität ist nahtlos in die bestehende Admin-UI integriert:

- Ein neuer Menüpunkt "CSV-Import" wurde zur Sidebar hinzugefügt
- Die Seite verwendet das bestehende Layout und Design
- Die Funktionalität ist mobil-responsiv gestaltet

## Zusammenfassung der Vorteile

- **Zeitersparnis**: Automatisierte Datenübernahme aus dem alten System
- **Fehlerreduzierung**: Standardisierter Prozess mit automatischer Validierung
- **Benutzerfreundlichkeit**: Einfache Benutzeroberfläche ohne technische Vorkenntnisse
- **Nachverfolgbarkeit**: Protokollierung aller Import-Vorgänge
- **Flexibilität**: Unterstützung verschiedener CSV-Dateitypen