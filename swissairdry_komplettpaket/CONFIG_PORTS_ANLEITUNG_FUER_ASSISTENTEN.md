# Konfigurationsanleitung für Assistenten: Porteinstellungen für SwissAirDry

## Übersicht der Porteinstellungen

Diese Anleitung beschreibt, wie Sie als Assistent die Porteinstellungen des SwissAirDry-Systems anpassen können, um Konflikte mit anderen auf dem Server laufenden Diensten zu vermeiden.

## Datei für Porteinstellungen

Alle Porteinstellungen befinden sich in der Datei `.env.ports`. Diese Datei wurde speziell erstellt, damit Sie als Assistent die Ports einfach anpassen können, ohne andere Konfigurationen zu beeinflussen.

## Verfügbare Ports und ihre Standardwerte

| Port-Variable | Standardwert | Beschreibung |
|---------------|--------------|--------------|
| API_PORT | 5000 | Hauptport für den API-Server |
| API_HTTP_PORT | 80 | HTTP-Port |
| API_HTTPS_PORT | 443 | HTTPS-Port |
| MQTT_PORT | 1883 | MQTT-Broker-Port |
| MQTT_WSS_PORT | 8083 | MQTT WebSocket-Port |
| ESP32_UPDATE_PORT | 8070 | ESP32-Firmware-Update-Port |
| NEXTCLOUD_HTTP_PORT | 8080 | HTTP-Port für Nextcloud |
| NEXTCLOUD_HTTPS_PORT | 8443 | HTTPS-Port für Nextcloud |

## Anpassung der Porteinstellungen

Führen Sie die folgenden Schritte aus, um die Porteinstellungen anzupassen:

1. Öffnen Sie die Datei `.env.ports`
2. Ändern Sie die Werte der Ports nach Bedarf
3. Speichern Sie die Datei
4. Führen Sie einen Neustart der Docker-Container durch:

```bash
docker-compose down
docker-compose up -d
```

## Beispiel für Portänderungen

Wenn der Port 5000 bereits belegt ist und Sie den API-Server auf Port 5001 ausführen möchten:

```
# Original in .env.ports
API_PORT=5000

# Geändert in .env.ports
API_PORT=5001
```

## Verifizierung der Portänderungen

Nach dem Neustart der Docker-Container können Sie prüfen, ob die Portänderungen wirksam wurden:

```bash
docker-compose ps
```

Oder prüfen Sie direkt, ob der gewünschte Port geöffnet ist:

```bash
netstat -tulpn | grep <PORT>
```

## Häufige Fehlerbehebungen bei Portkonflikten

1. **Problem**: Port ist bereits belegt
   **Lösung**: Wählen Sie einen anderen freien Port

2. **Problem**: Keine Verbindung trotz Portänderung
   **Lösung**: Prüfen Sie die Firewall-Einstellungen und stellen Sie sicher, dass der neue Port freigegeben ist

3. **Problem**: Dienst startet nicht nach Portänderung
   **Lösung**: Überprüfen Sie die Docker-Logs auf Fehler:
   ```bash
   docker-compose logs
   ```

## Besondere Hinweise

- Die Ports 80 und 443 erfordern möglicherweise Root-Berechtigungen. Wenn Sie keine Berechtigungen für diese Ports haben, verwenden Sie Ports über 1024.
- Stellen Sie sicher, dass die Ports in Ihrer Firewall freigegeben sind.
- Achten Sie darauf, dass die Portänderungen konsistent in der gesamten Konfiguration sind.

Bei weiteren Fragen oder Problemen stehen Sie bitte zur Verfügung.