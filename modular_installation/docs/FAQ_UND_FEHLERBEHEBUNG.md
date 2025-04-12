# SwissAirDry FAQ und Fehlerbehebung

## Häufig gestellte Fragen

### Installation und Einrichtung

**F: Welche Systemvoraussetzungen hat SwissAirDry?**

A: SwissAirDry benötigt mindestens:
- Docker und Docker Compose
- Eine laufende Nextcloud-Installation (Version 25 oder höher)
- Die Cloud-Py-Api App aus dem Nextcloud App Store
- Etwa 2 GB RAM und 20 GB Festplattenspeicher für die komplette Installation

---

**F: Kann ich SwissAirDry mit meiner bestehenden Nextcloud-Installation verwenden?**

A: Ja, die modulare Installation ist speziell dafür konzipiert, sich in eine bestehende Nextcloud-Umgebung zu integrieren. Die Integration erfolgt über die Cloud-Py-Api und erfordert keine direkten Änderungen an der Nextcloud-Installation selbst.

---

**F: Muss ich den API-Dienst auf demselben Server wie Nextcloud betreiben?**

A: Es wird empfohlen, den API-Dienst auf demselben Server zu betreiben, um eine optimale Leistung zu erzielen. Bei Verwendung von Docker-Containern werden beide Dienste idealerweise im selben Docker-Netzwerk betrieben.

---

**F: Wie kann ich die Installation auf einem Produktivsystem absichern?**

A: Für Produktivumgebungen empfehlen wir:
- Verwenden Sie sichere Passwörter für die Datenbankverbindung
- Führen Sie die API nur über HTTPS zugänglich
- Beschränken Sie den Zugriff auf die Docker-Container über Firewalls
- Richten Sie regelmäßige Backups der Datenbank und Konfigurationsdateien ein

---

**F: Kann ich einen externen Datenbankserver verwenden?**

A: Ja, Sie können einen externen PostgreSQL-Server verwenden. Ändern Sie dazu die DATABASE_URL-Umgebungsvariable im API-Service, um auf Ihren externen Datenbankserver zu verweisen.

### Datenimport und -migration

**F: Welche CSV-Dateiformate werden für den Import unterstützt?**

A: SwissAirDry kann folgende CSV-Formate importieren:
- Kundenstammdaten (KUNDENSTAMM.csv)
- Gerätestamm (GERAETESTAMMVERZEICHNISS.csv)
- Auftragsprotokolle (AUFTRAGSPROTOKOLL.csv)
- Gerätestandortwechsel (GERAETESTANDORTWECHSELPROTOKOLL.csv)
- Leistungserfassung (LEISTUNGSERFASSUNG.csv)
- Messprotokollwerterfassung (MESSPROTOKOLLWERTERFASSUNG.csv)

---

**F: Wie importiere ich Daten aus einer älteren SwissAirDry-Version?**

A: Verwenden Sie das Skript `05_import_data.sh`, um Daten aus CSV-Exporten der älteren Version zu importieren. Das Skript erkennt automatisch das Format und importiert die Daten in die entsprechenden Tabellen.

---

**F: Können Dateien und Bilder aus der alten Installation übernommen werden?**

A: Ja, Sie können Bilder und Dateien manuell in das Verzeichnis `/app/uploads` des API-Containers kopieren. Die Pfadinformationen müssen jedoch in der Datenbank aktualisiert werden, falls sich die Verzeichnisstruktur geändert hat.

### Fehlerbehebung

## Fehlerbehebung

### API-Dienst-Probleme

**Problem: API-Container startet nicht**

Symptome:
- Der Container wird nicht in `docker ps` angezeigt
- In den Logs erscheinen Fehlermeldungen

Mögliche Lösungen:
1. Prüfen Sie die Logs:
   ```bash
   docker logs swissairdry-api
   ```

2. Überprüfen Sie die Datenbankverbindung:
   ```bash
   docker exec -it swissairdry-api python -c "import os; print(os.environ.get('DATABASE_URL'))"
   ```

3. Stellen Sie sicher, dass die Ports nicht belegt sind:
   ```bash
   netstat -tuln | grep 5000
   ```

**Problem: API-Dienst ist nicht erreichbar**

Symptome:
- Die API antwortet nicht auf Anfragen
- 502-Fehler oder Connection Timeout

Mögliche Lösungen:
1. Prüfen Sie, ob der Container läuft:
   ```bash
   docker ps | grep swissairdry-api
   ```

2. Überprüfen Sie die Netzwerkkonfiguration:
   ```bash
   docker network inspect <netzwerk_name>
   ```

3. Testen Sie die Verbindung innerhalb des Docker-Netzwerks:
   ```bash
   docker exec -it nextcloud curl http://swissairdry-api:5000/health
   ```

### Nextcloud-Integration-Probleme

**Problem: Cloud-Py-Api ist nicht installiert oder aktiviert**

Symptome:
- Fehlermeldungen bei der Installation der SwissAirDry-App
- Die App erscheint nicht im Nextcloud-Menü

Mögliche Lösungen:
1. Prüfen Sie den Status der Cloud-Py-Api:
   ```bash
   curl -s -u "admin:password" -H "OCS-APIRequest: true" \
     "https://nextcloud.example.com/ocs/v1.php/cloud/apps/cloud_py_api"
   ```

2. Installieren Sie die App manuell über die Nextcloud-Admin-Oberfläche:
   - Gehen Sie zu Einstellungen > Apps > App Store
   - Suchen Sie nach "Cloud Py Api"
   - Klicken Sie auf "Installieren"

3. Aktivieren Sie die App:
   ```bash
   curl -X POST -u "admin:password" -H "OCS-APIRequest: true" \
     "https://nextcloud.example.com/ocs/v1.php/cloud/apps/cloud_py_api"
   ```

**Problem: SwissAirDry-App erscheint nicht in Nextcloud**

Symptome:
- Die App ist installiert, aber nicht im Menü sichtbar
- Beim Aufruf der App-URL erscheint ein Fehler

Mögliche Lösungen:
1. Überprüfen Sie, ob die App aktiviert ist:
   ```bash
   curl -s -u "admin:password" -H "OCS-APIRequest: true" \
     "https://nextcloud.example.com/ocs/v1.php/cloud/apps/swissairdry"
   ```

2. Prüfen Sie die Cloud-Py-Api-Logs in der Nextcloud-Admin-Oberfläche

3. Installieren Sie die App erneut mit dem Skript:
   ```bash
   ./03_install_nextcloud_app.sh
   ```

### Datenbank-Probleme

**Problem: Datenbankverbindungsfehler**

Symptome:
- API-Dienst meldet Verbindungsfehler
- In den Logs erscheinen PostgreSQL-Fehlermeldungen

Mögliche Lösungen:
1. Überprüfen Sie die Datenbankverbindungsdaten:
   ```bash
   cat /opt/swissairdry-api/.env | grep DATABASE_URL
   ```

2. Testen Sie die Datenbankverbindung:
   ```bash
   docker exec -it swissairdry-api python -c "
   import os
   from sqlalchemy import create_engine
   db_url = os.environ.get('DATABASE_URL')
   engine = create_engine(db_url)
   try:
       connection = engine.connect()
       print('Verbindung erfolgreich')
       connection.close()
   except Exception as e:
       print(f'Fehler: {str(e)}')
   "
   ```

3. Prüfen Sie, ob die Datenbank läuft:
   ```bash
   docker ps | grep postgres
   ```

**Problem: Fehler beim Datenimport**

Symptome:
- Import-Skript bricht mit Fehlern ab
- Daten werden nicht in der Datenbank angezeigt

Mögliche Lösungen:
1. Prüfen Sie das Format der CSV-Dateien:
   ```bash
   head -n 5 *.csv
   ```

2. Überprüfen Sie die Zeichenkodierung:
   ```bash
   file -i *.csv
   ```

3. Importieren Sie die Dateien manuell im Container:
   ```bash
   docker cp *.csv swissairdry-api:/app/data/
   docker exec -it swissairdry-api python -c "
   import pandas as pd
   from sqlalchemy import create_engine
   import os
   
   db_url = os.environ.get('DATABASE_URL')
   engine = create_engine(db_url)
   
   df = pd.read_csv('/app/data/KUNDENSTAMM.csv', encoding='utf-8')
   df.to_sql('kundenstamm', engine, if_exists='replace', index=False)
   print(f'Importiert: {len(df)} Datensätze')
   "
   ```

## Aktualisierung und Wartung

**F: Wie aktualisiere ich die SwissAirDry-Installation?**

A: Um eine bestehende Installation zu aktualisieren:
1. Erstellen Sie ein Backup der Datenbank und Konfigurationsdateien
2. Laden Sie die neueste Version von SwissAirDry herunter
3. Stoppen Sie die Docker-Container: `docker-compose down`
4. Ersetzen Sie die API-Dateien durch die neue Version
5. Starten Sie die Container neu: `docker-compose up -d`
6. Führen Sie ggf. Datenbankmigrationen durch

---

**F: Wie sichere ich meine SwissAirDry-Daten?**

A: Empfohlene Backup-Strategie:
1. Regelmäßige Sicherung der Datenbank:
   ```bash
   docker exec -it swissairdry-db pg_dump -U swissairdry swissairdry > backup_$(date +%Y%m%d).sql
   ```
2. Sicherung der Uploads und Konfigurationsdateien:
   ```bash
   docker cp swissairdry-api:/app/uploads ./backup_uploads
   docker cp swissairdry-api:/app/config ./backup_config
   ```
3. Externes Backup des gesamten Verzeichnisses `/opt/swissairdry-api`

---

**F: Welche regelmäßigen Wartungsarbeiten sind empfehlenswert?**

A: Folgende Wartungsarbeiten sollten regelmäßig durchgeführt werden:
1. Überprüfung der Protokolldateien auf Fehler und Warnungen
2. Bereinigung alter Protokolldateien und temporärer Daten
3. Aktualisierung der Docker-Container und Basis-Images
4. Optimierung der Datenbank (VACUUM ANALYZE)
5. Überprüfung des Festplattenspeichers und ggf. Bereinigung