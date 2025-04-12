# Fehlerbehebungsanleitung für SwissAirDry - Für Assistenten

Diese Anleitung hilft Ihnen als Assistent bei der Diagnose und Behebung häufiger Probleme, die bei der Installation oder dem Betrieb von SwissAirDry auftreten können.

## Hinweis zu Aktualisierungen
Diese Anleitung wurde zuletzt am 12.04.2025 aktualisiert und enthält nun auch Informationen zur Fehlerbehebung des Dark Mode.

## Inhaltsverzeichnis
1. [Diagnosewerkzeuge](#diagnosewerkzeuge)
2. [Allgemeine Docker-bezogene Probleme](#allgemeine-docker-bezogene-probleme)
3. [API-Server-Probleme](#api-server-probleme)
4. [Datenbankprobleme](#datenbankprobleme)
5. [Nextcloud-Integrationsprobleme](#nextcloud-integrationsprobleme)
6. [Netzwerkprobleme und Port-Konflikte](#netzwerkprobleme-und-port-konflikte)
7. [SSL/TLS-Probleme](#ssltls-probleme)
8. [MQTT und ESP32-bezogene Probleme](#mqtt-und-esp32-bezogene-probleme)
9. [Aktualisierungsprobleme](#aktualisierungsprobleme)
10. [Logging und Monitoring](#logging-und-monitoring)
11. [Benutzeroberfläche und Dark Mode](#benutzeroberfläche-und-dark-mode)

## Diagnosewerkzeuge

### Docker-Diagnose
```bash
# Container-Status prüfen
docker-compose ps

# Logs aller Container anzeigen
docker-compose logs

# Logs eines bestimmten Containers anzeigen
docker-compose logs api

# Logs kontinuierlich folgen
docker-compose logs -f api
```

### API-Server-Diagnose
```bash
# Gesundheitscheck des API-Servers
curl http://localhost:5000/health

# API-Dokumentation prüfen
curl http://localhost:5000/docs
```

### Datenbank-Diagnose
```bash
# Verbindung zur Datenbank testen
docker-compose exec postgres pg_isready -U swissairdry

# Interaktive PostgreSQL-Shell öffnen
docker-compose exec postgres psql -U swissairdry -d swissairdry
```

## Allgemeine Docker-bezogene Probleme

### Problem: Container starten nicht
**Symptome:**
- `docker-compose ps` zeigt, dass Container nicht laufen
- Status ist "Exit" oder "Created" statt "Up"

**Lösungen:**
1. Überprüfen Sie die Logs:
   ```bash
   docker-compose logs
   ```

2. Neu starten erzwingen:
   ```bash
   docker-compose down
   docker-compose up -d
   ```

3. Container neu erstellen:
   ```bash
   docker-compose down
   docker-compose build --no-cache
   docker-compose up -d
   ```

### Problem: Docker-Volumes haben keine korrekten Berechtigungen
**Symptome:**
- Fehlermeldungen bezüglich Berechtigungen in den Logs
- "Permission denied" Fehler

**Lösungen:**
1. Volumes neu erstellen:
   ```bash
   docker-compose down -v
   docker-compose up -d
   ```

2. Falls dies die Datenbank betrifft, sichern Sie zuerst die Daten:
   ```bash
   docker-compose exec postgres pg_dump -U swissairdry swissairdry > backup.sql
   ```

## API-Server-Probleme

### Problem: API-Server startet nicht
**Symptome:**
- 502 Bad Gateway bei Anfragen
- "Connection refused" Fehlermeldungen

**Lösungen:**
1. Überprüfen Sie die API-Server-Logs:
   ```bash
   docker-compose logs api
   ```

2. Überprüfen Sie die Python-Abhängigkeiten in `requirements.txt`

3. Prüfen Sie die Umgebungsvariablen in der `.env`-Datei

### Problem: FastAPI-Endpunkte sind nicht erreichbar
**Symptome:**
- 404 Not Found Fehler
- API-Endpunkte funktionieren nicht wie erwartet

**Lösungen:**
1. Überprüfen Sie die URL-Pfade in der API-Dokumentation:
   ```
   http://localhost:5000/docs
   ```

2. Prüfen Sie, ob die Router in `main.py` korrekt eingebunden sind

## Datenbankprobleme

### Problem: Verbindungsprobleme zur Datenbank
**Symptome:**
- "Connection refused" Fehlermeldungen
- Datenbankbezogene Fehler in API-Logs

**Lösungen:**
1. Prüfen Sie den Status des Postgres-Containers:
   ```bash
   docker-compose ps postgres
   ```

2. Überprüfen Sie die Datenbankverbindungszeichenfolge in `.env`:
   ```
   DATABASE_URL=postgresql://swissairdry:swissairdry@postgres:5432/swissairdry
   ```

3. Testen Sie die Datenbankverbindung:
   ```bash
   docker-compose exec postgres pg_isready -U swissairdry
   ```

### Problem: Migrationsprobleme oder Tabellenfehler
**Symptome:**
- "Table does not exist" Fehlermeldungen
- Fehler bei Datenbankabfragen

**Lösungen:**
1. Überprüfen Sie, ob die Tabellen existieren:
   ```sql
   \dt
   ```

2. Erzwingen Sie eine Neuinitialisierung der Datenbank (Vorsicht: Datenverlust!):
   ```bash
   docker-compose down
   docker volume rm swissairdry_postgres_data
   docker-compose up -d
   ```

## Nextcloud-Integrationsprobleme

### Problem: Keine Verbindung zur Nextcloud
**Symptome:**
- Nextcloud-Verbindungsfehler in den Logs
- API-Endpunkt `/nextcloud-connect` meldet Fehler

**Lösungen:**
1. Überprüfen Sie die Nextcloud-URL in `.env`:
   ```
   NEXTCLOUD_URL=https://nextcloud.vgnc.org
   ```

2. Prüfen Sie die Netzwerkverbindung zwischen API-Server und Nextcloud:
   ```bash
   docker-compose exec api ping nextcloud.vgnc.org
   ```

3. Überprüfen Sie die Zugangsdaten für Nextcloud in `.env`

### Problem: cloud-py-api Plugin funktioniert nicht
**Symptome:**
- Keine Anzeige der SwissAirDry-App in Nextcloud
- Fehlermeldungen in den Nextcloud-Logs

**Lösungen:**
1. Stellen Sie sicher, dass das Plugin in Nextcloud installiert und aktiviert ist
2. Führen Sie das Integrationsscript erneut aus:
   ```bash
   cd installation/scripts
   ./docker_nextcloud_integration.sh
   ```

## Netzwerkprobleme und Port-Konflikte

### Problem: Portkonflikte
**Symptome:**
- "Address already in use" Fehlermeldungen
- Container starten nicht wegen belegter Ports

**Lösungen:**
1. Identifizieren Sie, welcher Prozess den Port belegt:
   ```bash
   sudo netstat -tulpn | grep <PORT>
   ```

2. Ändern Sie die Ports in `.env.ports` und starten Sie die Container neu:
   ```bash
   docker-compose down
   # Ports in .env.ports anpassen
   docker-compose up -d
   ```

### Problem: Keine Verbindung von externen Geräten
**Symptome:**
- Lokaler Zugriff funktioniert, aber keine Verbindung von anderen Geräten
- Mobile Geräte können nicht auf die API zugreifen

**Lösungen:**
1. Überprüfen Sie die Firewall-Einstellungen:
   ```bash
   sudo ufw status
   ```

2. Stellen Sie sicher, dass die benötigten Ports freigegeben sind:
   ```bash
   sudo ufw allow <PORT>/tcp
   ```

## SSL/TLS-Probleme

### Problem: SSL-Zertifikate funktionieren nicht
**Symptome:**
- Browser meldet unsichere Verbindung
- "SSL handshake failed" Fehlermeldungen

**Lösungen:**
1. Überprüfen Sie die Zertifikatsdateien:
   ```bash
   ls -la /etc/nginx/ssl/
   ```

2. Stellen Sie sicher, dass die Zertifikate im korrekten Format vorliegen und gültig sind:
   ```bash
   openssl x509 -in /etc/nginx/ssl/api.vgnc.org.crt -text -noout
   ```

3. Überprüfen Sie die Nginx-Konfiguration auf korrekte SSL-Einstellungen

### Problem: Zertifikat ist abgelaufen
**Symptome:**
- Browser meldet abgelaufenes Zertifikat
- Datum des Zertifikats liegt in der Vergangenheit

**Lösungen:**
1. Erneuern Sie das Zertifikat (je nach Bezugsquelle unterschiedlich)
2. Bei Let's Encrypt:
   ```bash
   certbot renew
   ```

## MQTT und ESP32-bezogene Probleme

### Problem: MQTT-Broker ist nicht erreichbar
**Symptome:**
- ESP32-Geräte können keine Verbindung herstellen
- MQTT-Client-Fehler

**Lösungen:**
1. Überprüfen Sie, ob der MQTT-Broker läuft:
   ```bash
   docker-compose ps mqtt
   ```

2. Testen Sie die MQTT-Verbindung mit einem Testclient:
   ```bash
   mosquitto_pub -h localhost -p 1883 -t test -m "hello"
   ```

### Problem: ESP32-Firmware-Updates schlagen fehl
**Symptome:**
- Update-Prozess wird nicht abgeschlossen
- ESP32-Geräte erhalten keine Updates

**Lösungen:**
1. Überprüfen Sie den Update-Server-Port in `.env.ports`:
   ```
   ESP32_UPDATE_PORT=8070
   ```

2. Stellen Sie sicher, dass die ESP32-Firmware im korrekten Format vorliegt

## Aktualisierungsprobleme

### Problem: Nach Update funktioniert das System nicht mehr
**Symptome:**
- Nach `git pull` und Neustart treten Fehler auf
- Funktionen sind nicht mehr verfügbar

**Lösungen:**
1. Überprüfen Sie die Änderungen zwischen den Versionen:
   ```bash
   git log
   ```

2. Stellen Sie sicher, dass alle Abhängigkeiten aktualisiert wurden:
   ```bash
   docker-compose build --no-cache
   ```

3. Überprüfen Sie, ob Datenbankmigrationen erforderlich sind

## Logging und Monitoring

### Problem: Keine ausreichenden Logs für Fehlerdiagnose
**Symptome:**
- Fehler treten auf, aber keine hilfreichen Logs
- Unklare Fehlerursachen

**Lösungen:**
1. Erhöhen Sie das Log-Level in der Konfiguration:
   ```bash
   # In .env setzen
   DEBUG=True
   ```

2. Richten Sie ein zentrales Logging-System ein (z.B. ELK-Stack)

### Problem: System wird langsam oder instabil
**Symptome:**
- Lange Antwortzeiten
- Zufällige Abstürze oder Timeouts

**Lösungen:**
1. Überwachen Sie die Systemressourcen:
   ```bash
   docker stats
   ```

2. Überprüfen Sie die Datenbankleistung:
   ```sql
   SELECT * FROM pg_stat_activity;
   ```

3. Erwägen Sie ein Upgrade der Serverressourcen oder Optimierung der Datenbank

## Benutzeroberfläche und Dark Mode

### Problem: Dark Mode funktioniert nicht
**Symptome:**
- Toggle-Switch für den Dark Mode ist nicht sichtbar
- Umschalten zwischen Hell- und Dunkelmodus funktioniert nicht
- Design ändert sich nicht beim Klicken auf den Toggle-Switch

**Lösungen:**
1. Überprüfen Sie die CSS-Dateien:
   ```bash
   # Prüfen Sie, ob die CSS-Datei die Dark Mode Styles enthält
   cat docker-base-api/app/static/css/style.css
   ```

2. Stellen Sie sicher, dass das JavaScript für den Dark Mode geladen wird:
   ```bash
   # Prüfen Sie die HTML-Datei auf Dark Mode JavaScript
   cat docker-base-api/app/templates/index.html
   ```

3. Prüfen Sie die Browser-Konsole auf JavaScript-Fehler:
   - Öffnen Sie die Entwicklertools im Browser (F12)
   - Wechseln Sie zur Konsole und suchen Sie nach Fehlermeldungen

### Problem: Dark Mode wird nicht gespeichert
**Symptome:**
- Nach Neuladen der Seite wird die Einstellung zurückgesetzt
- LocalStorage speichert die Einstellung nicht

**Lösungen:**
1. Prüfen Sie, ob der LocalStorage funktioniert:
   ```javascript
   // In der Browser-Konsole ausführen
   localStorage.setItem('test', 'value');
   console.log(localStorage.getItem('test'));
   ```

2. Stellen Sie sicher, dass das Theme korrekt im LocalStorage gespeichert wird:
   ```javascript
   // In der Browser-Konsole ausführen
   console.log(localStorage.getItem('theme'));
   ```

3. Überprüfen Sie den JavaScript-Code für die Theme-Speicherung in der index.html

### Problem: Dark Mode sieht falsch aus
**Symptome:**
- Farben sind inkonsistent
- Text ist schlecht lesbar im Dark Mode
- Einige Elemente werden nicht korrekt umgestellt

**Lösungen:**
1. Überprüfen Sie die CSS-Variablen für den Dark Mode:
   ```css
   /* Diese sollten in der style.css definiert sein */
   [data-theme="dark"] {
     --bg-color: #121212;
     --text-color: #e0e0e0;
     /* ... weitere Variablen ... */
   }
   ```

2. Stellen Sie sicher, dass alle UI-Elemente CSS-Variablen verwenden und nicht hart kodierte Farben

3. Fügen Sie fehlende CSS-Variablen hinzu oder passen Sie bestehende an, um die Lesbarkeit zu verbessern

---

Bei weiteren Fragen oder Problemen stehen Sie als Assistent zur Verfügung, um gezielte Unterstützung zu bieten.