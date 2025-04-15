# SwissAirDry Web-App

Eine moderne Weboberfläche für die SwissAirDry-Anwendung.

## Übersicht

Die Web-App bietet eine benutzerfreundliche Oberfläche für die Verwaltung von Trocknungsgeräten, Sensoren und Aufträgen über einen Webbrowser. Sie ist mit einer responsiven Benutzeroberfläche ausgestattet, die sowohl auf Desktop- als auch auf Mobilgeräten funktioniert.

## Funktionen

- Dashboard mit Echtzeit-Überblick über alle Geräte und Sensoren
- Detaillierte Geräteverwaltung
- Auftragsverwaltung und -tracking
- Sensorverwaltung und -überwachung
- Berichterstellung und Datenexport
- Benutzerverwaltung und Zugriffssteuerung
- Dark Mode für bessere Sichtbarkeit bei Nacht oder in dunklen Umgebungen

## Technologien

- HTML5, CSS3 und JavaScript
- API-Anbindung an den SwissAirDry-API-Server
- Responsive Design mit CSS Grid und Flexbox
- Chart.js für Diagramme und Visualisierungen

## Entwicklung

### Lokale Entwicklung

1. In das Verzeichnis wechseln:
   ```
   cd swissairdry/app
   ```

2. Einen lokalen Webserver starten:
   ```
   python -m http.server 8000
   ```

3. Die App ist nun unter http://localhost:8000 verfügbar

### Produktion

Für die Produktion kann die App über den Docker-Container bereitgestellt werden oder in einen beliebigen Webserver integriert werden.