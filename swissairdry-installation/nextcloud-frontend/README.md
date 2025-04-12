# SwissAirDry - Nextcloud Frontend Integration

Diese Komponente stellt die Frontend-Integration für Nextcloud bereit, um die SwissAirDry-Anwendung innerhalb einer bestehenden Nextcloud-Installation zu nutzen.

## Übersicht

Die SwissAirDry Nextcloud Frontend Integration ermöglicht es Benutzern, die SwissAirDry-Anwendung direkt innerhalb ihrer Nextcloud-Umgebung zu verwenden. Die eigentliche Anwendungsfunktionalität wird über die Docker Base API bereitgestellt, während das Frontend als Nextcloud-App implementiert ist.

## Voraussetzungen

* Nextcloud 25 oder höher
* Zugriff auf die Nextcloud-Administratoroberfläche
* Docker Base API Server läuft und ist von Nextcloud aus erreichbar

## Installation

### Automatische Installation (empfohlen)

1. Melden Sie sich als Administrator in Ihrer Nextcloud-Instanz an
2. Gehen Sie zu "Apps" > "App-Verwaltung"
3. Wählen Sie die Kategorie "Werkzeuge" aus
4. Suchen Sie nach "SwissAirDry"
5. Klicken Sie auf "Installieren"

### Manuelle Installation

1. Laden Sie das neueste Release von der [Release-Seite](https://github.com/swissairdry/nextcloud-frontend/releases) herunter
2. Entpacken Sie das Archiv in das Verzeichnis `apps` Ihrer Nextcloud-Installation:
   ```
   cd /pfad/zu/nextcloud/apps/
   tar -xzf /pfad/zum/download/swissairdry-nextcloud.tar.gz
   ```
3. Stellen Sie sicher, dass die Berechtigungen korrekt gesetzt sind:
   ```
   chown -R www-data:www-data /pfad/zu/nextcloud/apps/swissairdry/
   ```
4. Aktivieren Sie die App über die Nextcloud-Administratoroberfläche oder die occ-Befehlszeile:
   ```
   cd /pfad/zu/nextcloud/
   sudo -u www-data php occ app:enable swissairdry
   ```

## Konfiguration

Nach der Installation muss die App konfiguriert werden:

1. Gehen Sie zu "Einstellungen" > "Verwaltung" > "SwissAirDry"
2. Geben Sie die URL des SwissAirDry API-Servers ein (z.B. `http://api-server:5000`)
3. Speichern Sie die Einstellungen

## Benutzerberechtigungen

In Nextcloud können folgende Berechtigungsgruppen für die App konfiguriert werden:

* `Admin-Gruppe`: Vollzugriff auf alle Funktionen
* `Techniker-Gruppe`: Zugriff auf Aufträge, Messungen und Berichte
* `Benutzer-Gruppe`: Eingeschränkter Zugriff nur auf bestimmte Funktionen

Die Berechtigungen können unter "Einstellungen" > "Verwaltung" > "Berechtigungen" konfiguriert werden.

## Integration mit anderen Nextcloud-Apps

Die SwissAirDry App kann mit folgenden Nextcloud-Apps integriert werden:

* **Nextcloud Dateien**: Automatische Speicherung von Berichten und Fotos
* **Nextcloud Kalender**: Terminplanung für Techniker-Einsätze
* **Nextcloud Aufgaben**: Verwaltung von Aufgaben im Zusammenhang mit Aufträgen

## Entwicklung

### Entwicklungsumgebung einrichten

1. Klonen Sie das Repository:
   ```
   git clone https://github.com/swissairdry/nextcloud-frontend.git
   ```

2. Installieren Sie die Entwicklungsabhängigkeiten:
   ```
   cd nextcloud-frontend
   npm install
   ```

3. Starten Sie den Entwicklungsserver:
   ```
   npm run dev
   ```

### Build-Prozess

Um ein Produktions-Build zu erstellen:

```
npm run build
```

### Tests durchführen

```
npm run test
```

## Fehlerbehebung

### App wird nicht angezeigt

Wenn die App nach der Installation nicht in der Nextcloud-Navigation angezeigt wird:

1. Überprüfen Sie, ob die App aktiviert ist:
   ```
   sudo -u www-data php occ app:list | grep swissairdry
   ```

2. Prüfen Sie die Nextcloud-Logs:
   ```
   tail -f /pfad/zu/nextcloud/data/nextcloud.log
   ```

3. Stellen Sie sicher, dass die App-Version mit Ihrer Nextcloud-Version kompatibel ist

### Verbindungsprobleme mit der API

Wenn keine Verbindung zur API hergestellt werden kann:

1. Überprüfen Sie die API-URL in den Einstellungen
2. Stellen Sie sicher, dass der API-Server läuft und von Nextcloud aus erreichbar ist
3. Prüfen Sie die CORS-Einstellungen des API-Servers

## Support

Bei Fragen oder Problemen wenden Sie sich bitte an:

* E-Mail: info@swissairdry.com
* Support-Telefon: +41 123 456 789