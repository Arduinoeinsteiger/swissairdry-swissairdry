# SwissAirDry Fehlerbehebung

Diese Anleitung hilft Ihnen bei der Behebung häufiger Probleme, die bei der Installation oder dem Betrieb des SwissAirDry-Systems auftreten können.

## Verbindungsprobleme

### API-Server nicht erreichbar

**Symptom:** Die API ist unter http://ihre-domain:5000/ nicht erreichbar.

**Lösungen:**

1. Prüfen Sie, ob der Port 5000 in Ihrer Firewall freigegeben ist:
   ```bash
   sudo iptables -L -n | grep 5000
   ```

2. Prüfen Sie, ob der API-Container läuft:
   ```bash
   docker-compose ps
   ```

3. Überprüfen Sie die Logs des API-Containers:
   ```bash
   docker-compose logs api
   ```

4. Starten Sie den API-Container neu:
   ```bash
   docker-compose restart api
   ```

### MQTT-Broker nicht erreichbar

**Symptom:** Geräte können keine Verbindung zum MQTT-Broker herstellen.

**Lösungen:**

1. Prüfen Sie, ob Port 1883 freigegeben ist:
   ```bash
   sudo iptables -L -n | grep 1883
   ```

2. Überprüfen Sie die MQTT-Broker-Logs:
   ```bash
   docker-compose logs mqtt
   ```

3. Prüfen Sie, ob die Passwortdatei korrekt erstellt wurde:
   ```bash
   ls -la swissairdry-docker/mosquitto/config/mosquitto.passwd
   ```

4. Testen Sie die MQTT-Verbindung mit einem Client:
   ```bash
   docker exec -it swissairdry-mqtt mosquitto_sub -h localhost -p 1883 -u swissairdry -P IhrPasswort -t test
   ```

## Datenbank-Probleme

### Verbindung zur Datenbank fehlgeschlagen

**Symptom:** Die API kann keine Verbindung zur Datenbank herstellen.

**Lösungen:**

1. Prüfen Sie, ob der Datenbank-Container läuft:
   ```bash
   docker-compose ps db
   ```

2. Prüfen Sie die Datenbank-Logs:
   ```bash
   docker-compose logs db
   ```

3. Prüfen Sie die Verbindungsparameter in der .env-Datei:
   ```bash
   grep POSTGRES .env
   ```

4. Stellen Sie eine direkte Verbindung zur Datenbank her, um zu prüfen, ob sie funktioniert:
   ```bash
   docker-compose exec db psql -U swissairdry -d swissairdry -c "SELECT 1;"
   ```

### Datenbankmigration fehlgeschlagen

**Symptom:** Fehler bei der automatischen Migration der Datenbank.

**Lösungen:**

1. Prüfen Sie die API-Logs auf spezifische Migrationsfehler:
   ```bash
   docker-compose logs api | grep -i migrat
   ```

2. Starten Sie die Container neu, um die Migration erneut zu versuchen:
   ```bash
   docker-compose down
   docker-compose up -d
   ```

## Failover-System-Probleme

### Automatischer Wechsel funktioniert nicht

**Symptom:** Das System wechselt nicht automatisch zum Backup-Server, wenn der primäre Server ausfällt.

**Lösungen:**

1. Prüfen Sie die aktuellen Server-Einstellungen:
   ```bash
   curl -u benutzer:passwort http://ihre-domain:5000/api/v1/api-status
   ```

2. Prüfen Sie die API-Logs auf Failover-Meldungen:
   ```bash
   docker-compose logs api | grep -i server
   ```

3. Führen Sie einen manuellen Wechsel durch:
   ```bash
   curl -X POST -u benutzer:passwort http://ihre-domain:5000/api/v1/api-status/switch-to-backup
   ```

4. Überprüfen Sie die Umgebungsvariablen in der .env-Datei:
   ```bash
   grep API_ .env
   ```

## Nextcloud-Probleme

### Nextcloud-App wird nicht angezeigt

**Symptom:** Die SwissAirDry-App erscheint nicht in Nextcloud.

**Lösungen:**

1. Prüfen Sie, ob Nextcloud richtig läuft:
   ```bash
   docker-compose ps nextcloud
   ```

2. Überprüfen Sie, ob das App-Verzeichnis korrekt eingebunden wurde:
   ```bash
   docker exec -it swissairdry-nextcloud ls -la /var/www/html/custom_apps
   ```

3. Aktivieren Sie die App manuell in Nextcloud:
   ```bash
   docker exec -it swissairdry-nextcloud php occ app:enable swissairdry
   ```

4. Prüfen Sie die Nextcloud-Logs:
   ```bash
   docker-compose logs nextcloud
   ```

## Container-Management

### Container neu starten

Um alle Container neu zu starten:

```bash
docker-compose down
docker-compose up -d
```

Um einen spezifischen Container neu zu starten:

```bash
docker-compose restart [container-name]
```

### Container-Logs anzeigen

Um die Logs eines Containers anzuzeigen:

```bash
docker-compose logs [container-name]
```

Um die letzten 100 Zeilen der Logs anzuzeigen und fortzufahren:

```bash
docker-compose logs -f --tail=100 [container-name]
```

## Support

Wenn Sie ein Problem nicht selbst lösen können, wenden Sie sich bitte an den Support:

- E-Mail: support@swissairdry.com
- Telefon: +41 XXXX XXXX XX

Bitte halten Sie folgende Informationen bereit:
- Die Version des SwissAirDry-Systems
- Relevante Logfiles
- Eine genaue Beschreibung des Problems
- Schritte zur Reproduktion des Fehlers, falls möglich