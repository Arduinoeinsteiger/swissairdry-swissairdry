# SwissAirDry Installation und Konfiguration - Anweisungen für Assistenten

## Systemüberblick

SwissAirDry ist ein umfassendes System zur Verwaltung von Trocknungsgeräten und Trocknungsaufträgen mit folgenden Komponenten:

- **API-Server**: FastAPI-basierter Backend-Server
- **Nextcloud-Integration**: Anbindung an bestehende Nextcloud-Instanz
- **MQTT-Broker**: Für die Kommunikation mit ESP32-Geräten
- **Postgresql-Datenbank**: Zur Speicherung aller Daten
- **ESP32-Firmware**: Für die Steuerung der Trocknungsgeräte
- **Benutzeroberfläche**: Responsive Design mit Dark Mode-Unterstützung

## Schritt 1: Umgebungsvariablen konfigurieren

1. Erstelle eine `.env` Datei basierend auf der `.env.example` Vorlage
2. Passe die Portkonfiguration in der `.env.ports` Datei an, falls Standardports bereits belegt sind
3. Stelle sicher, dass die Domain-Konfiguration für `api.vgnc.org` korrekt ist

## Schritt 2: Ports und Firewall konfigurieren

Stelle sicher, dass folgende Ports in der Firewall freigegeben sind:

- `5000`: API-Server (intern)
- `80`: HTTP
- `443`: HTTPS
- `1883`: MQTT
- `8083`: MQTT-WebSocket
- `8070`: ESP32-Update-Server

Die genauen Ports können in der `.env.ports` Datei angepasst werden.

## Schritt 3: Docker-Umgebung einrichten

1. Stelle sicher, dass Docker und Docker Compose installiert sind
2. Navigiere zum Hauptverzeichnis des Projekts
3. Starte die Docker-Container:
   ```bash
   docker-compose up -d
   ```
4. Überprüfe, ob alle Container gestartet wurden:
   ```bash
   docker-compose ps
   ```

## Schritt 4: Nextcloud-Integration

1. Installiere das cloud-py-api Plugin aus dem Nextcloud App Store
2. Konfiguriere die Verbindung zwischen API-Server und Nextcloud:
   ```bash
   cd installation/scripts
   ./docker_nextcloud_integration.sh
   ```

## Schritt 5: Nginx und SSL konfigurieren

1. Kopiere die `nginx.conf` in das entsprechende Verzeichnis deines Nginx-Servers
2. Besorge SSL-Zertifikate für `api.vgnc.org`
3. Platziere die Zertifikate in `/etc/nginx/ssl/`:
   - `api.vgnc.org.crt`
   - `api.vgnc.org.key`
4. Starte Nginx neu:
   ```bash
   sudo systemctl restart nginx
   ```

## Schritt 6: Ersteinrichtung des Systems

1. Öffne einen Webbrowser und navigiere zu `https://api.vgnc.org`
2. Folge dem Einrichtungsassistenten, um:
   - Administrator-Konto zu erstellen
   - Benutzergruppen einzurichten
   - Standardmodule zu aktivieren

## Schritt 7: CSV-Datenimport

1. Navigiere im Admin-Bereich zum Menüpunkt "CSV-Import"
2. Lade die folgenden CSV-Dateien hoch:
   - SwissAirDry_KUNDENSTAMM.csv
   - SwissAirDry_GERAETESTAMMVERZEICHNISS.csv
   - SwissAirDry_AUFTRAGSPROTOKOLL.csv
   - SwissAirDry_MESSPROTOKOLLWERTERFASSUNG.csv
   - SwissAirDry_GERAETESTANDORTWECHSELPROTOKOLL.csv

## Schritt 8: Dark Mode konfigurieren

Die Benutzeroberfläche unterstützt einen Dark Mode, der über einen Toggle-Switch aktiviert werden kann:

1. Der Toggle-Switch befindet sich in der rechten oberen Ecke des Headers (zwischen Sonnen- und Mond-Icon)
2. Der gewählte Modus wird automatisch im LocalStorage des Browsers gespeichert
3. Die Einstellung bleibt auch nach einem Neustart der Anwendung erhalten
4. Das Design passt sich automatisch an helle und dunkle Umgebungen an

## Häufig auftretende Probleme und Lösungen

### Problem: Docker-Container starten nicht
Lösung: Überprüfe die Docker-Logs:
```bash
docker-compose logs
```

### Problem: Portfreigabe funktioniert nicht
Lösung: Ändere die Ports in der `.env.ports` Datei und starte die Container neu.

### Problem: Verbindung zur Nextcloud schlägt fehl
Lösung: Überprüfe die Nextcloud-URLs und -Zugangsdaten in der `.env` Datei.

### Problem: SSL-Zertifikate funktionieren nicht
Lösung: Stelle sicher, dass die Zertifikate korrekt eingerichtet sind und der Nginx-Server Zugriff auf die Dateien hat.

## Anpassung der Portkonfiguration

Die Portkonfiguration kann in der `.env.ports` Datei angepasst werden. Nach Änderungen müssen die Docker-Container neu gestartet werden:

```bash
docker-compose down
docker-compose up -d
```

## Backups erstellen

Es wird empfohlen, regelmäßige Backups der Datenbank zu erstellen:

```bash
docker-compose exec postgres pg_dump -U swissairdry swissairdry > backup_$(date +%Y%m%d).sql
```

## Systemaktualisierungen

Um das System zu aktualisieren:

```bash
git pull
docker-compose build
docker-compose down
docker-compose up -d
```

Bei Fragen oder Problemen stehe ich gerne zur Verfügung!