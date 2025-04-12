# SwissAirDry - Docker-Integration Kurzanleitung

Diese Kurzanleitung führt Sie durch die Integration von SwissAirDry in eine bestehende Nextcloud-Docker-Installation.

## Voraussetzungen

- Bestehende Nextcloud-Installation in Docker-Containern
- Docker und Docker Compose
- Cloud-Py-Api App aus dem Nextcloud App Store

## Schnellintegration

1. **Integration starten**

   ```bash
   cd installation/scripts
   chmod +x *.sh
   ./docker_nextcloud_integration.sh
   ```

2. **Docker-Netzwerk konfigurieren**

   Das Skript fragt nach dem Namen Ihres Nextcloud-Containers und verbindet den SwissAirDry-API-Container mit dem Nextcloud-Netzwerk.

3. **Docker-Compose-Datei aktualisieren**

   Das Skript aktualisiert Ihre bestehende docker-compose.yml, um die SwissAirDry-Services hinzuzufügen.

4. **Cloud-Py-Api installieren**

   Stellen Sie sicher, dass Cloud-Py-Api in Ihrer Nextcloud-Installation aktiviert ist.

5. **SwissAirDry-App installieren**

   Das Skript installiert die SwissAirDry-App über Cloud-Py-Api in Ihrer Nextcloud-Installation.

6. **Docker-Container neu starten**

   ```bash
   cd /pfad/zu/ihrer/docker-compose-datei
   docker-compose up -d
   ```

7. **Zugriff auf SwissAirDry**

   Nach erfolgreichem Abschluss können Sie auf SwissAirDry zugreifen unter:
   ```
   https://ihre-nextcloud.example.com/index.php/apps/swissairdry/
   ```

## Fehlersuche

Wenn Probleme auftreten:

1. **Netzwerkkonnektivität prüfen**
   ```bash
   docker exec -it nextcloud ping swissairdry-api
   ```

2. **API-Dienst prüfen**
   ```bash
   docker logs swissairdry-api
   ```

3. **Cloud-Py-Api prüfen**
   Überprüfen Sie in der Nextcloud-Admin-Oberfläche, ob Cloud-Py-Api korrekt installiert und aktiviert ist.

Weitere Informationen finden Sie in der vollständigen Dokumentation unter [Docker-Nextcloud-Integration](../installation/docs/DOCKER_NEXTCLOUD_INTEGRATION.md).
