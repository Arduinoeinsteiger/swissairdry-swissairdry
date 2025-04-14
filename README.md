# SwissAirDry Projekt

## Überblick

SwissAirDry ist eine umfassende Lösung für die Verwaltung von Trocknungsgeräten und Feldservice-Operationen mit IoT-Integration. Das Projekt umfasst eine API-Server-Komponente, eine Weboberfläche, eine Nextcloud-Integration und eine Android-Mobilanwendung.

## Aufgeräumte Projektstruktur

Das Projekt wurde in eine aufgeräumte, modulare Struktur reorganisiert:

```
swissairdry/
├── api/                # FastAPI Backend-Server
├── app/                # Web-Frontend 
├── nextcloud/          # Nextcloud-Integration
├── mobile/             # Android-App
├── docs/               # Dokumentation
├── docker-compose.yml  # Docker-Konfiguration
└── .env.example        # Beispiel-Umgebungsvariablen
```

## Schnellstart

1. Ins Projektverzeichnis wechseln:
   ```
   cd swissairdry
   ```

2. Umgebungsvariablen konfigurieren:
   ```
   cp .env.example .env
   # .env-Datei nach Bedarf anpassen
   ```

3. Docker-Container starten:
   ```
   docker-compose up -d
   ```

4. Die Anwendung ist nun verfügbar unter:
   - API: http://localhost:5000
   - Nextcloud App: http://localhost:8080

## Dokumentation

Weitere Details finden Sie in der Dokumentation:

- [Projektübersicht](swissairdry/docs/PROJEKT_UEBERSICHT.md)
- [Installationsanleitung](swissairdry/docs/INSTALLATION_ANLEITUNG.md)
- [Fehlerbehebung](swissairdry/docs/FEHLERBEHEBUNG.md)

## Hinweis

Dies ist die aufgeräumte Version des SwissAirDry-Projekts. Die ursprünglichen Ordner sind weiterhin vorhanden, werden aber nicht mehr aktiv verwendet.