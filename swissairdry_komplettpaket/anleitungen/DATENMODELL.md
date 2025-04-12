# SwissAirDry - Datenmodell

## Übersicht

Das SwissAirDry-System verwendet eine PostgreSQL-Datenbank mit folgendem Datenmodell:

## Haupttabellen

### Kunden (customers)
Speichert Informationen zu Kunden.

| Feld        | Typ        | Beschreibung                           |
|-------------|------------|----------------------------------------|
| id          | INTEGER    | Primärschlüssel                        |
| name        | VARCHAR    | Name des Kunden                        |
| contact     | VARCHAR    | Kontaktperson                          |
| address     | VARCHAR    | Adresse                                |
| postal_code | VARCHAR    | Postleitzahl                           |
| city        | VARCHAR    | Stadt                                  |
| phone       | VARCHAR    | Telefonnummer                          |
| email       | VARCHAR    | E-Mail-Adresse                         |
| created_at  | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at  | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

### Geräte (devices)
Speichert Informationen zu Trocknungsgeräten.

| Feld        | Typ        | Beschreibung                           |
|-------------|------------|----------------------------------------|
| id          | INTEGER    | Primärschlüssel                        |
| device_type | VARCHAR    | Gerätetyp                              |
| serial_no   | VARCHAR    | Seriennummer                           |
| manufacturer| VARCHAR    | Hersteller                             |
| model       | VARCHAR    | Modell                                 |
| wattage     | INTEGER    | Leistungsaufnahme in Watt              |
| status      | VARCHAR    | Status (aktiv, inaktiv, in Reparatur)  |
| created_at  | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at  | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

### Aufträge (jobs)
Speichert Informationen zu Trocknungsaufträgen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| customer_id    | INTEGER    | Fremdschlüssel auf Kunden              |
| job_number     | VARCHAR    | Auftragsnummer                         |
| status         | VARCHAR    | Status (offen, in Bearbeitung, abgeschlossen) |
| address        | VARCHAR    | Adresse des Einsatzortes               |
| start_date     | DATE       | Startdatum                             |
| end_date       | DATE       | Enddatum (geplant oder tatsächlich)    |
| description    | TEXT       | Beschreibung des Auftrags              |
| created_at     | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at     | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

### Geräteeinsätze (device_deployments)
Verknüpft Geräte mit Aufträgen und speichert Einsatzdaten.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| job_id         | INTEGER    | Fremdschlüssel auf Aufträge            |
| device_id      | INTEGER    | Fremdschlüssel auf Geräte              |
| location       | VARCHAR    | Standort innerhalb des Einsatzortes    |
| start_date     | TIMESTAMP  | Startzeitpunkt des Einsatzes           |
| end_date       | TIMESTAMP  | Endzeitpunkt des Einsatzes             |
| start_reading  | INTEGER    | Zählerstand bei Einsatzbeginn          |
| end_reading    | INTEGER    | Zählerstand bei Einsatzende            |
| created_at     | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at     | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

### Messungen (measurements)
Speichert Feuchtigkeitsmessungen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| job_id         | INTEGER    | Fremdschlüssel auf Aufträge            |
| location       | VARCHAR    | Messort                                |
| material       | VARCHAR    | Material (Estrich, Mauerwerk, etc.)    |
| measurement_date | TIMESTAMP | Messzeitpunkt                         |
| humidity       | FLOAT      | Gemessene Feuchtigkeit in %            |
| temperature    | FLOAT      | Gemessene Temperatur in °C             |
| notes          | TEXT       | Anmerkungen                            |
| created_at     | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at     | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

## Benutzer- und Gruppentabellen

### Benutzer (users)
Speichert Benutzerinformationen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| username       | VARCHAR    | Benutzername                           |
| email          | VARCHAR    | E-Mail-Adresse                         |
| password_hash  | VARCHAR    | Gehashtes Passwort                     |
| full_name      | VARCHAR    | Vollständiger Name                     |
| role           | VARCHAR    | Rolle (admin, user, etc.)              |
| is_active      | BOOLEAN    | Aktiv-Status                           |
| created_at     | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at     | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

### Gruppen (groups)
Speichert Benutzergruppen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| name           | VARCHAR    | Gruppenname                            |
| description    | TEXT       | Beschreibung                           |
| created_at     | TIMESTAMP  | Erstellungszeitpunkt                   |
| updated_at     | TIMESTAMP  | Letzter Aktualisierungszeitpunkt       |

### Benutzer-Gruppen (user_groups)
Verknüpft Benutzer mit Gruppen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| user_id        | INTEGER    | Fremdschlüssel auf Benutzer            |
| group_id       | INTEGER    | Fremdschlüssel auf Gruppen             |

### API-Berechtigungen (api_permissions)
Speichert API-Berechtigungen für Gruppen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| endpoint       | VARCHAR    | API-Endpunkt                           |
| methods        | JSON       | Erlaubte HTTP-Methoden                 |
| description    | TEXT       | Beschreibung                           |

### Gruppen-API-Berechtigungen (group_api_permissions)
Verknüpft Gruppen mit API-Berechtigungen.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| group_id       | INTEGER    | Fremdschlüssel auf Gruppen             |
| permission_id  | INTEGER    | Fremdschlüssel auf API-Berechtigungen  |

## Import-Tabellen

### CSV-Import-Logs (csv_import_logs)
Protokolliert CSV-Importvorgänge.

| Feld           | Typ        | Beschreibung                           |
|----------------|------------|----------------------------------------|
| id             | INTEGER    | Primärschlüssel                        |
| filename       | VARCHAR    | Dateiname                              |
| import_date    | TIMESTAMP  | Importzeitpunkt                        |
| status         | VARCHAR    | Status (erfolgreich, fehlgeschlagen)   |
| records_imported | INTEGER  | Anzahl importierter Datensätze         |
| error_message  | TEXT       | Fehlermeldung (falls vorhanden)        |
| user_id        | INTEGER    | Benutzer, der den Import durchgeführt hat |
