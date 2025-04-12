"""
SwissAirDry - CSV-Datenverarbeitungsmodul

Dieses Modul bietet Funktionen zur Verarbeitung und Integration von CSV-Daten
für das SwissAirDry-System. Es ermöglicht den Import von Kundendaten, Geräteinformationen,
Messwerten und Projektdaten aus strukturierten CSV-Dateien wie sie aus der bestehenden
Lösung exportiert werden.

@author Swiss Air Dry Team <info@swissairdry.com>
@copyright 2023-2025 Swiss Air Dry Team
"""

import os
import csv
import logging
import pandas as pd
from datetime import datetime
from typing import Dict, List, Optional, Tuple, Any, Union
from sqlalchemy.orm import Session

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from database import get_db
from models import (
    Customer, Device, Job, JobDevice, Measurement, 
    Report, ReportImage, Invoice, SystemLog
)

# Logger konfigurieren
logger = logging.getLogger(__name__)

class CSVProcessor:
    """
    Klasse zur Verarbeitung von CSV-Dateien für das SwissAirDry-System
    """
    
    def __init__(self, db_session: Session):
        """
        Initialisierung mit Datenbankverbindung

        Args:
            db_session: Aktive Datenbankverbindung
        """
        self.db = db_session
        self.processed_files = set()
        self.error_count = 0
        self.success_count = 0
    
    def process_directory(self, directory_path: str) -> Tuple[int, int]:
        """
        Verarbeitet alle CSV-Dateien in einem Verzeichnis

        Args:
            directory_path: Pfad zum Verzeichnis mit CSV-Dateien

        Returns:
            Tuple mit Anzahl erfolgreicher Verarbeitungen und Fehler
        """
        if not os.path.exists(directory_path):
            logger.error(f"Verzeichnis nicht gefunden: {directory_path}")
            return 0, 1
        
        # Zähler zurücksetzen
        self.error_count = 0
        self.success_count = 0
        
        # CSV-Dateien im Verzeichnis identifizieren
        csv_files = [f for f in os.listdir(directory_path) if f.endswith('.csv')]
        
        # SwissAirDry CSV-Dateien verarbeiten
        swissairdry_files = [f for f in csv_files if "SwissAirDry_" in f]
        if swissairdry_files:
            logger.info(f"SwissAirDry CSV-Dateien gefunden: {len(swissairdry_files)}")
            return self._process_swissairdry_files(directory_path, swissairdry_files)
        
        # Wenn keine SwissAirDry-Dateien gefunden wurden, versuche es mit den FirstDry-Dateien (Legacy)
        # Sortierte Verarbeitung nach CSV-Typ
        # 1. Zuerst Kundendaten
        customer_files = [f for f in csv_files if "FirstDry_" in f and "_1_" in f]
        for file in customer_files:
            file_path = os.path.join(directory_path, file)
            self._process_customer_file(file_path)
        
        # 2. Dann Auftragsdaten/Jobs
        job_files = [f for f in csv_files if "FirstDry_" in f and not any(suffix in f for suffix in ["_1_", "_2_", "_3_", "_4_", "_5_", "_6_", "_7_", "_8_", "_9_"])]
        for file in job_files:
            file_path = os.path.join(directory_path, file)
            self._process_job_file(file_path)
        
        # 3. Gerätedaten
        device_files = [f for f in csv_files if "FirstDry_" in f and "_5_" in f]
        for file in device_files:
            file_path = os.path.join(directory_path, file)
            self._process_device_file(file_path)
        
        # 4. Gerätemessungen/Projektdaten
        measurement_files = [f for f in csv_files if "FirstDry_" in f and "_6_" in f]
        for file in measurement_files:
            file_path = os.path.join(directory_path, file)
            self._process_measurement_file(file_path)
        
        # 5. Materialfeuchtemessungen
        moisture_files = [f for f in csv_files if "FirstDry_" in f and "_8_" in f]
        for file in moisture_files:
            file_path = os.path.join(directory_path, file)
            self._process_moisture_file(file_path)
        
        # 6. Dämmschicht-Messungen
        insulation_files = [f for f in csv_files if "FirstDry_" in f and "_9_" in f]
        for file in insulation_files:
            file_path = os.path.join(directory_path, file)
            self._process_insulation_file(file_path)
        
        # 7. Berichtsdaten und Bilder
        report_files = [f for f in csv_files if "FirstDry_" in f and "_4_" in f]
        for file in report_files:
            file_path = os.path.join(directory_path, file)
            self._process_report_file(file_path)
            
        # 8. Arbeitszeiten und Aktivitäten
        activities_files = [f for f in csv_files if "FirstDry_" in f and "_3_" in f]
        for file in activities_files:
            file_path = os.path.join(directory_path, file)
            self._process_activities_file(file_path)
        
        logger.info(f"CSV-Verarbeitung abgeschlossen: {self.success_count} erfolgreich, {self.error_count} Fehler")
        return self.success_count, self.error_count
        
    def _process_swissairdry_files(self, directory_path: str, files: List[str]) -> Tuple[int, int]:
        """
        Verarbeitet SwissAirDry CSV-Dateien in der richtigen Reihenfolge
        
        Args:
            directory_path: Pfad zum Verzeichnis mit CSV-Dateien
            files: Liste der SwissAirDry CSV-Dateien
            
        Returns:
            Tuple mit Anzahl erfolgreicher Verarbeitungen und Fehler
        """
        # 1. Verarbeitung der Kundenstammdaten (Kontaktdaten)
        kundenstamm_files = [f for f in files if "AUFTRAGSPROTOKOLL" in f]
        for file in kundenstamm_files:
            file_path = os.path.join(directory_path, file)
            self._process_swissairdry_kundenstamm(file_path)
            
        # 2. Verarbeitung der Gerätestammdaten
        geraetestamm_files = [f for f in files if "GERAETESTAMMVERZEICHNISS" in f]
        for file in geraetestamm_files:
            file_path = os.path.join(directory_path, file)
            self._process_swissairdry_geraetestamm(file_path)
            
        # 3. Verarbeitung der Auftragsdaten (Job/Kundenaufträge)
        auftrag_files = [f for f in files if "KUNDENSTAMM" in f]
        for file in auftrag_files:
            file_path = os.path.join(directory_path, file)
            self._process_swissairdry_auftraege(file_path)
            
        # 4. Verarbeitung der Gerätezuweisungen zu Aufträgen
        geraetezuweisung_files = [f for f in files if "GERAETESTANDORTWECHSELPROTOKOLL" in f]
        for file in geraetezuweisung_files:
            file_path = os.path.join(directory_path, file)
            self._process_swissairdry_geraetezuweisung(file_path)
            
        # 5. Verarbeitung der Messwerterfassungen
        messwert_files = [f for f in files if "MESSPROTOKOLLWERTERFASSUNG" in f or "MESSWERTERFASSUNGEN" in f]
        for file in messwert_files:
            file_path = os.path.join(directory_path, file)
            self._process_swissairdry_messwerte(file_path)
            
        # 6. Sonstige Dateien
        other_files = [f for f in files if not any(t in f for t in [
            "AUFTRAGSPROTOKOLL", "GERAETESTAMMVERZEICHNISS", "KUNDENSTAMM", 
            "GERAETESTANDORTWECHSELPROTOKOLL", "MESSPROTOKOLLWERTERFASSUNG", "MESSWERTERFASSUNGEN"
        ])]
        for file in other_files:
            file_path = os.path.join(directory_path, file)
            logger.info(f"Unbekannter Dateityp, wird übersprungen: {file}")
        
        logger.info(f"SwissAirDry CSV-Verarbeitung abgeschlossen: {self.success_count} erfolgreich, {self.error_count} Fehler")
        return self.success_count, self.error_count
        
    def _process_swissairdry_kundenstamm(self, file_path: str) -> None:
        """
        Verarbeitet die AUFTRAGSPROTOKOLL.csv Datei mit Kundendaten im SwissAirDry-Format
        
        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überprüfe, ob die notwendigen Schlüssel vorhanden sind
                if not all(key in row for key in ['UID', 'Firma', 'Vorname', 'Nachname']):
                    continue
                
                # Überspringen Sie leere Zeilen (identifiziert durch leere UID)
                if pd.isna(row['UID']) or not str(row['UID']).strip():
                    continue
                
                # Prüfen, ob Kunde bereits existiert
                existing_customer = self.db.query(Customer).filter_by(bexio_id=str(row['UID'])).first()
                
                if existing_customer:
                    # Update bestehender Kunde
                    self._update_customer(existing_customer, row)
                else:
                    # Neuer Kunde
                    self._create_customer(row)
            
            logger.info(f"SwissAirDry Kundendatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der SwissAirDry Kundendatei {file_path}: {str(e)}")
            self.error_count += 1
            
    def _process_swissairdry_geraetestamm(self, file_path: str) -> None:
        """
        Verarbeitet die GERAETESTAMMVERZEICHNISS.csv Datei mit Gerätedaten im SwissAirDry-Format
        
        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überprüfe, ob die notwendigen Schlüssel vorhanden sind
                if not all(key in row for key in ['Gerätenummer', 'Kategorie', 'Name']):
                    continue
                
                # Überspringen Sie leere Zeilen (identifiziert durch leere Gerätenummer)
                if pd.isna(row['Gerätenummer']) or not str(row['Gerätenummer']).strip():
                    continue
                
                # Prüfen, ob Gerät bereits existiert
                existing_device = self.db.query(Device).filter_by(serial_number=str(row['Gerätenummer'])).first()
                
                if existing_device:
                    # Update bestehendes Gerät
                    self._update_device(existing_device, row)
                else:
                    # Neues Gerät
                    self._create_swissairdry_device(row)
            
            logger.info(f"SwissAirDry Gerätedatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der SwissAirDry Gerätedatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_swissairdry_device(self, row: Dict) -> Device:
        """
        Erstellt ein neues Gerät aus SwissAirDry CSV-Daten
        
        Args:
            row: Dictionary mit Gerätedaten
            
        Returns:
            Neue Device-Instanz
        """
        # Preis auslesen und in numerischen Wert umwandeln
        price = 0.0
        if 'Preis pro Tag' in row and not pd.isna(row['Preis pro Tag']):
            price_str = str(row['Preis pro Tag'])
            # Extrahiere numerischen Wert aus Preisformat (z.B. "CHF 16.00")
            import re
            if re.search(r'(\d+[,.]?\d*)', price_str):
                price = float(re.search(r'(\d+[,.]?\d*)', price_str).group(1).replace(',', '.'))
        
        # Beschreibung zusammenstellen
        description_parts = []
        if 'Beschreibung' in row and not pd.isna(row['Beschreibung']):
            description_parts.append(str(row['Beschreibung']).strip())
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']):
            description_parts.append(str(row['Bemerkungen']).strip())
        
        description = " - ".join(description_parts) if description_parts else None
        
        # Geräteinformationen
        device = Device(
            name=str(row['Name']).strip() if not pd.isna(row.get('Name')) else "Unbekanntes Gerät",
            device_type=str(row['Kategorie']).strip() if not pd.isna(row.get('Kategorie')) else "Sonstiges",
            serial_number=str(row['Gerätenummer']).strip(),
            description=description,
            status="available",  # Standardmäßig verfügbar
            price_per_day=price,
            manufacturer=None,  # Nicht in CSV enthalten
            purchase_date=None,  # Nicht in CSV enthalten
            last_maintenance_date=None,  # Nicht in CSV enthalten
            next_maintenance_date=None,  # Nicht in CSV enthalten
        )
        
        self.db.add(device)
        self.db.commit()
        logger.info(f"Neues Gerät erstellt: {device.name} (ID: {device.id})")
        
        return device
        
    def _process_swissairdry_auftraege(self, file_path: str) -> None:
        """
        Verarbeitet die KUNDENSTAMM.csv Datei mit Auftragsdaten im SwissAirDry-Format
        
        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne Auftragsnummer
                if 'Auftragsnummer' not in row or pd.isna(row['Auftragsnummer']) or not str(row['Auftragsnummer']).strip():
                    continue
                
                # Prüfen, ob Auftrag bereits existiert
                existing_job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
                
                if existing_job:
                    # Update bestehender Auftrag
                    self._update_swissairdry_job(existing_job, row)
                else:
                    # Neuer Auftrag
                    self._create_swissairdry_job(row)
            
            logger.info(f"SwissAirDry Auftragsdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der SwissAirDry Auftragsdatei {file_path}: {str(e)}")
            self.error_count += 1
            
    def _create_swissairdry_job(self, row: Dict) -> Job:
        """
        Erstellt einen neuen Auftrag aus SwissAirDry CSV-Daten
        
        Args:
            row: Dictionary mit Auftragsdaten
            
        Returns:
            Neue Job-Instanz
        """
        # Kundenidentifikation - wir versuchen, einen passenden Kunden zu finden
        # oder erstellen einen neuen, falls erforderlich
        customer = None
        
        # Versuche, den Kunden anhand der Kontaktreferenzen zu finden
        for contact_field in ['Auftraggeber Kontakt', 'Eigentümer 1 Kontakt', 'Bewohner 1 Kontakt']:
            if contact_field in row and not pd.isna(row[contact_field]) and row[contact_field]:
                customer = self.db.query(Customer).filter_by(bexio_id=str(row[contact_field])).first()
                if customer:
                    break
                    
        # Wenn kein Kunde gefunden wurde, erstelle einen generischen
        # basierend auf der Adresse und anderen verfügbaren Informationen
        if not customer:
            customer_name_parts = []
            if 'Bewohner 1 Kontakt' in row and not pd.isna(row['Bewohner 1 Kontakt']):
                customer_name_parts.append(f"Kontakt: {row['Bewohner 1 Kontakt']}")
            
            if 'Adresse' in row and not pd.isna(row['Adresse']):
                customer_name_parts.append(f"Adresse: {row['Adresse']}")
                
            customer_name = " - ".join(customer_name_parts) if customer_name_parts else f"Kunde für Auftrag {row['Auftragsnummer']}"
            
            customer = Customer(
                name=customer_name,
                address=str(row['Adresse']) if 'Adresse' in row and not pd.isna(row['Adresse']) else None,
                contact_person=None,
                phone=None,
                email=None,
                bexio_id=str(row['Auftraggeber Kontakt']) if 'Auftraggeber Kontakt' in row and not pd.isna(row['Auftraggeber Kontakt']) else None
            )
            
            self.db.add(customer)
            self.db.commit()
            logger.info(f"Neuer Kunde für Auftrag erstellt: {customer.name} (ID: {customer.id})")
        
        # Auftragsbeschreibung zusammenstellen
        description_parts = []
        
        if 'Schadensart' in row and not pd.isna(row['Schadensart']) and row['Schadensart']:
            description_parts.append(f"Schadensart: {row['Schadensart']}")
        
        if 'Räume' in row and not pd.isna(row['Räume']) and row['Räume']:
            description_parts.append(f"Betroffene Räume: {row['Räume']}")
            
        if 'Wohnung' in row and not pd.isna(row['Wohnung']) and row['Wohnung']:
            description_parts.append(f"Wohnung: {row['Wohnung']}")
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            description_parts.append(f"Bemerkungen: {row['Bemerkungen']}")
        
        description = "\n".join(description_parts) if description_parts else None
        
        # Auftragsstatus aus CSV-Status ableiten
        status_mapping = {
            'aktiv': 'active',
            'abgeschlossen': 'completed',
            'in Bearbeitung': 'active',
            'offen': 'pending',
            'storniert': 'canceled'
        }
        
        csv_status = row.get('Status', '').lower() if 'Status' in row and not pd.isna(row['Status']) else ''
        db_status = status_mapping.get(csv_status, 'pending')
        
        # Datum parsen
        job_date = None
        if 'Datum Erstbegehung' in row and not pd.isna(row['Datum Erstbegehung']):
            try:
                job_date = datetime.strptime(str(row['Datum Erstbegehung']), '%m/%d/%Y')
            except ValueError:
                try:
                    job_date = datetime.strptime(str(row['Datum Erstbegehung']), '%d.%m.%Y')
                except:
                    logger.warning(f"Konnte Datum nicht parsen: {row['Datum Erstbegehung']}")
        
        # Neuen Auftrag erstellen
        job = Job(
            name=str(row['Auftragsnummer']),
            customer_id=customer.id,
            description=description,
            status=db_status,
            priority="medium",  # Standard-Priorität
            start_date=job_date,
            end_date=None,  # Nicht aus CSV ableitbar
            address=str(row['Adresse']) if 'Adresse' in row and not pd.isna(row['Adresse']) else None,
            insurance_number=str(row['Schadennummer Versicherung']) if 'Schadennummer Versicherung' in row and not pd.isna(row['Schadennummer Versicherung']) else None,
            damage_type=str(row['Schadensart']) if 'Schadensart' in row and not pd.isna(row['Schadensart']) else None,
            rooms=str(row['Räume']) if 'Räume' in row and not pd.isna(row['Räume']) else None,
        )
        
        self.db.add(job)
        self.db.commit()
        logger.info(f"Neuer Auftrag erstellt: {job.name} (ID: {job.id})")
        
        return job
        
    def _update_swissairdry_job(self, job: Job, row: Dict) -> None:
        """
        Aktualisiert einen bestehenden Auftrag mit neuen SwissAirDry CSV-Daten
        
        Args:
            job: Bestehende Job-Instanz
            row: Dictionary mit aktualisierten Auftragsdaten
        """
        modified = False
        
        # Auftragsbeschreibung aktualisieren
        description_parts = []
        
        if 'Schadensart' in row and not pd.isna(row['Schadensart']) and row['Schadensart']:
            description_parts.append(f"Schadensart: {row['Schadensart']}")
        
        if 'Räume' in row and not pd.isna(row['Räume']) and row['Räume']:
            description_parts.append(f"Betroffene Räume: {row['Räume']}")
            
        if 'Wohnung' in row and not pd.isna(row['Wohnung']) and row['Wohnung']:
            description_parts.append(f"Wohnung: {row['Wohnung']}")
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            description_parts.append(f"Bemerkungen: {row['Bemerkungen']}")
        
        new_description = "\n".join(description_parts) if description_parts else None
        
        if job.description != new_description:
            job.description = new_description
            modified = True
        
        # Status aktualisieren
        status_mapping = {
            'aktiv': 'active',
            'abgeschlossen': 'completed',
            'in Bearbeitung': 'active',
            'offen': 'pending',
            'storniert': 'canceled'
        }
        
        csv_status = row.get('Status', '').lower() if 'Status' in row and not pd.isna(row['Status']) else ''
        new_status = status_mapping.get(csv_status, job.status)  # Behalte aktuellen Status, wenn kein Mapping
        
        if job.status != new_status:
            job.status = new_status
            modified = True
            
        # Schadennummer der Versicherung aktualisieren
        if 'Schadennummer Versicherung' in row and not pd.isna(row['Schadennummer Versicherung']):
            new_insurance_number = str(row['Schadennummer Versicherung'])
            if job.insurance_number != new_insurance_number:
                job.insurance_number = new_insurance_number
                modified = True
                
        # Adresse aktualisieren
        if 'Adresse' in row and not pd.isna(row['Adresse']):
            new_address = str(row['Adresse'])
            if job.address != new_address:
                job.address = new_address
                modified = True
                
        # Schadensart aktualisieren
        if 'Schadensart' in row and not pd.isna(row['Schadensart']):
            new_damage_type = str(row['Schadensart'])
            if job.damage_type != new_damage_type:
                job.damage_type = new_damage_type
                modified = True
                
        # Räume aktualisieren
        if 'Räume' in row and not pd.isna(row['Räume']):
            new_rooms = str(row['Räume'])
            if job.rooms != new_rooms:
                job.rooms = new_rooms
                modified = True
        
        if modified:
            job.updated_at = datetime.utcnow()
            self.db.commit()
            logger.info(f"Auftrag aktualisiert: {job.name} (ID: {job.id})")
            
    def _process_swissairdry_geraetezuweisung(self, file_path: str) -> None:
        """
        Verarbeitet die GERAETESTANDORTWECHSELPROTOKOLL.csv Datei mit Gerätezuweisungen zu Aufträgen
        
        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überprüfe, ob die notwendigen Schlüssel vorhanden sind
                if not all(key in row for key in ['Gerätenummer', 'Auftragsnummer', 'Datum']):
                    continue
                
                # Überspringen von Zeilen ohne Gerätenummer oder Auftragsnummer
                if (pd.isna(row['Gerätenummer']) or not str(row['Gerätenummer']).strip() or
                    pd.isna(row['Auftragsnummer']) or not str(row['Auftragsnummer']).strip()):
                    continue
                
                # Finde zugehöriges Gerät und Auftrag
                device = self.db.query(Device).filter_by(serial_number=str(row['Gerätenummer'])).first()
                job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
                
                if not device or not job:
                    logger.warning(f"Gerät {row.get('Gerätenummer')} oder Auftrag {row.get('Auftragsnummer')} nicht gefunden")
                    continue
                
                # Prüfen, ob die Verknüpfung bereits existiert
                existing_job_device = self.db.query(JobDevice).filter_by(
                    job_id=job.id, 
                    device_id=device.id
                ).first()
                
                if existing_job_device:
                    # Update der bestehenden Verknüpfung
                    self._update_job_device(existing_job_device, row)
                else:
                    # Neue Verknüpfung erstellen
                    self._create_swissairdry_job_device(row, job.id, device.id)
            
            logger.info(f"SwissAirDry Gerätezuweisungsdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der SwissAirDry Gerätezuweisungsdatei {file_path}: {str(e)}")
            self.error_count += 1
            
    def _create_swissairdry_job_device(self, row: Dict, job_id: int, device_id: int) -> JobDevice:
        """
        Erstellt eine neue Verknüpfung zwischen Auftrag und Gerät aus SwissAirDry CSV-Daten
        
        Args:
            row: Dictionary mit Einsatzdaten
            job_id: ID des Auftrags
            device_id: ID des Geräts
            
        Returns:
            Neue JobDevice-Instanz
        """
        # Datum parsen
        installation_date = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                installation_date = datetime.strptime(str(row['Datum']), '%m/%d/%Y')
            except ValueError:
                try:
                    installation_date = datetime.strptime(str(row['Datum']), '%d.%m.%Y')
                except:
                    logger.warning(f"Konnte Datum nicht parsen: {row['Datum']}")
                    
        # Art der Installation bestimmen (Montage/Demontage)
        installation_type = 'installation'  # Standard
        if 'Art' in row and not pd.isna(row['Art']):
            if str(row['Art']).lower() == 'demontage':
                installation_type = 'removal'
        
        # Standortinformationen
        location = None
        location_parts = []
        if 'Wohnung' in row and not pd.isna(row['Wohnung']) and row['Wohnung']:
            location_parts.append(str(row['Wohnung']))
        if 'Standort' in row and not pd.isna(row['Standort']) and row['Standort']:
            location_parts.append(str(row['Standort']))
            
        location = ", ".join(location_parts) if location_parts else None
        
        # Zählerstand
        meter_reading = None
        if 'Zählerstand kWh' in row and not pd.isna(row['Zählerstand kWh']):
            try:
                meter_reading = float(str(row['Zählerstand kWh']).replace(',', '.'))
            except ValueError:
                logger.warning(f"Konnte Zählerstand nicht parsen: {row['Zählerstand kWh']}")
        
        # Neue Verknüpfung erstellen
        job_device = JobDevice(
            job_id=job_id,
            device_id=device_id,
            installation_date=installation_date,
            removal_date=None,  # Wird bei einer Demontage später aktualisiert
            location=location,
            status=installation_type,
            initial_reading=meter_reading,
            final_reading=None,  # Wird bei einer Demontage aktualisiert
            notes=str(row['Bemerkungen']) if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) else None
        )
        
        self.db.add(job_device)
        self.db.commit()
        logger.info(f"Neue Gerätezuweisung erstellt: Job #{job_id}, Gerät #{device_id}")
        
        return job_device
        
    def _process_swissairdry_messwerte(self, file_path: str) -> None:
        """
        Verarbeitet die MESSPROTOKOLLWERTERFASSUNG.csv oder MESSWERTERFASSUNGEN.csv Datei 
        mit Arbeitszeiten und Messungen
        
        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überprüfe, ob die notwendigen Schlüssel vorhanden sind
                if 'Auftragsnummer' not in row or pd.isna(row['Auftragsnummer']) or not str(row['Auftragsnummer']).strip():
                    continue
                
                # Finde den zugehörigen Auftrag
                job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
                if not job:
                    logger.warning(f"Auftrag {row.get('Auftragsnummer')} nicht gefunden")
                    continue
                
                # Verarbeite die Arbeitszeiten als SystemLog-Einträge
                self._create_swissairdry_activity_log(row, job.id)
            
            logger.info(f"SwissAirDry Messwertdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der SwissAirDry Messwertdatei {file_path}: {str(e)}")
            self.error_count += 1
            
    def _create_swissairdry_activity_log(self, row: Dict, job_id: int) -> None:
        """
        Erstellt SystemLog-Einträge für Arbeitszeiten aus SwissAirDry CSV-Daten
        
        Args:
            row: Dictionary mit Aktivitätsdaten
            job_id: ID des zugehörigen Auftrags
        """
        # Datum parsen
        activity_date = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                activity_date = datetime.strptime(str(row['Datum']), '%m/%d/%Y')
            except ValueError:
                try:
                    activity_date = datetime.strptime(str(row['Datum']), '%d.%m.%Y')
                except:
                    logger.warning(f"Konnte Datum nicht parsen: {row['Datum']}")
                    # Falls das Datum nicht geparst werden kann, verwenden wir das aktuelle Datum
                    activity_date = datetime.now()
        else:
            # Falls kein Datum vorhanden ist, verwenden wir das aktuelle Datum
            activity_date = datetime.now()
            
        # Benutzer/Mitarbeiter identifizieren
        user_email = str(row['Mitarbeiter']) if 'Mitarbeiter' in row and not pd.isna(row['Mitarbeiter']) else "unbekannt@swissairdry.com"
        
        # Aktivitäten extrahieren und verarbeiten
        activity_fields = [
            'Schadenaufnahme', 'Messung/Kontrolle', 'Sofortmassnahmen', 'Leckortung', 
            'Montage', 'Demontage', 'Wassersaugen', 'Fahrzeit', 'Bericht', 
            'Bauleitung', 'Schimmelbehandlung', 'Abdeckarbeiten', 'Besprechung', 
            'Endabnahme', 'Bauprogramm', 'Abbruch'
        ]
        
        for field in activity_fields:
            if field in row and not pd.isna(row[field]) and row[field]:
                try:
                    # Versuche, die Stundenzahl zu parsen
                    hours = float(str(row[field]).replace(',', '.'))
                    if hours > 0:
                        # Erstelle einen Log-Eintrag für diese Aktivität
                        activity_description = f"Aktivität: {field}, Dauer: {hours} Stunden"
                        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
                            activity_description += f", Bemerkungen: {row['Bemerkungen']}"
                        
                        log_entry = SystemLog(
                            job_id=job_id,
                            user_email=user_email,
                            log_type="activity",
                            description=activity_description,
                            timestamp=activity_date,
                            hours=hours,
                            activity_type=field
                        )
                        
                        self.db.add(log_entry)
                        
                except ValueError:
                    logger.warning(f"Konnte Stundenwert nicht parsen: {field}={row[field]}")
        
        # Speichere alle Änderungen
        self.db.commit()
        logger.info(f"Aktivitätslog für Auftrag #{job_id} erstellt")
    
    def _safe_csv_read(self, file_path: str) -> Optional[List[Dict]]:
        """
        Liest CSV-Datei sicher ein und behandelt potenzielle Fehler

        Args:
            file_path: Pfad zur CSV-Datei

        Returns:
            Liste von Zeilen als Dictionaries oder None bei Fehler
        """
        try:
            # Überprüfen, ob die Datei bereits verarbeitet wurde
            if file_path in self.processed_files:
                logger.info(f"Datei wurde bereits verarbeitet: {file_path}")
                return None
            
            # Versuche CSV mit pandas zu lesen für bessere Fehlertoleranz
            df = pd.read_csv(file_path, encoding='utf-8', low_memory=False)
            
            # Konvertiere zu Liste von Dictionaries
            records = df.to_dict('records')
            
            self.processed_files.add(file_path)
            self.success_count += 1
            return records
            
        except Exception as e:
            logger.error(f"Fehler beim Lesen der CSV-Datei {file_path}: {str(e)}")
            self.error_count += 1
            return None
    
    def _process_customer_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Kundendaten

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überprüfe, ob die notwendigen Schlüssel vorhanden sind
                if not all(key in row for key in ['UID', 'Firma', 'Vorname', 'Nachname']):
                    continue
                
                # Überspringen Sie leere Zeilen (identifiziert durch leere UID)
                if pd.isna(row['UID']) or not str(row['UID']).strip():
                    continue
                
                # Prüfen, ob Kunde bereits existiert
                existing_customer = self.db.query(Customer).filter_by(bexio_id=str(row['UID'])).first()
                
                if existing_customer:
                    # Update bestehender Kunde
                    self._update_customer(existing_customer, row)
                else:
                    # Neuer Kunde
                    self._create_customer(row)
            
            logger.info(f"Kundendatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Kundendatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_customer(self, row: Dict) -> Customer:
        """
        Erstellt einen neuen Kunden aus CSV-Daten

        Args:
            row: Dictionary mit Kundendaten

        Returns:
            Neue Customer-Instanz
        """
        # Kombiniere Firma, Name und Vorname für den Customer Namen
        name_parts = []
        if not pd.isna(row.get('Firma')) and row.get('Firma'):
            name_parts.append(row['Firma'])
        
        person_name = []
        if not pd.isna(row.get('Vorname')) and row.get('Vorname'):
            person_name.append(row['Vorname'])
        if not pd.isna(row.get('Nachname')) and row.get('Nachname'):
            person_name.append(row['Nachname'])
        
        if person_name:
            name_parts.append(" ".join(person_name))
        
        customer_name = " - ".join(name_parts) if len(name_parts) > 1 else name_parts[0] if name_parts else "Unbekannt"
        
        # Adresse zusammenbauen
        address_parts = []
        for field in ['Adresszeile 1', 'Adresszeile 2', 'PLZ', 'Ort']:
            if field in row and not pd.isna(row[field]) and row[field]:
                address_parts.append(str(row[field]))
        
        address = ", ".join(address_parts)
        
        # Kontaktinformationen
        contact_person = " ".join([part for part in [row.get('Vorname'), row.get('Nachname')] 
                                  if part and not pd.isna(part)])
        
        email = row.get('E-Mail') if 'E-Mail' in row and not pd.isna(row['E-Mail']) else None
        
        # Telefonnummern zusammenführen
        phone_parts = []
        for field in ['Festnetz', 'Mobil']:
            if field in row and not pd.isna(row[field]) and row[field]:
                phone_parts.append(str(row[field]))
        
        phone = " / ".join(phone_parts) if phone_parts else None
        
        # Neuen Kunden erstellen
        customer = Customer(
            name=customer_name,
            contact_person=contact_person,
            email=email,
            phone=phone,
            address=address,
            bexio_id=str(row['UID'])
        )
        
        self.db.add(customer)
        self.db.commit()
        logger.info(f"Neuer Kunde erstellt: {customer_name} (ID: {customer.id})")
        
        return customer
    
    def _update_customer(self, customer: Customer, row: Dict) -> None:
        """
        Aktualisiert einen bestehenden Kunden mit neuen CSV-Daten

        Args:
            customer: Bestehende Customer-Instanz
            row: Dictionary mit aktualisierten Kundendaten
        """
        modified = False
        
        # Name aktualisieren wenn sich Firma oder Personenname geändert hat
        name_parts = []
        if not pd.isna(row.get('Firma')) and row.get('Firma'):
            name_parts.append(row['Firma'])
        
        person_name = []
        if not pd.isna(row.get('Vorname')) and row.get('Vorname'):
            person_name.append(row['Vorname'])
        if not pd.isna(row.get('Nachname')) and row.get('Nachname'):
            person_name.append(row['Nachname'])
        
        if person_name:
            name_parts.append(" ".join(person_name))
        
        new_name = " - ".join(name_parts) if len(name_parts) > 1 else name_parts[0] if name_parts else "Unbekannt"
        
        if customer.name != new_name:
            customer.name = new_name
            modified = True
        
        # Adresse aktualisieren
        address_parts = []
        for field in ['Adresszeile 1', 'Adresszeile 2', 'PLZ', 'Ort']:
            if field in row and not pd.isna(row[field]) and row[field]:
                address_parts.append(str(row[field]))
        
        new_address = ", ".join(address_parts)
        if customer.address != new_address:
            customer.address = new_address
            modified = True
        
        # Kontaktinformationen aktualisieren
        new_contact_person = " ".join([part for part in [row.get('Vorname'), row.get('Nachname')] 
                                     if part and not pd.isna(part)])
        if customer.contact_person != new_contact_person:
            customer.contact_person = new_contact_person
            modified = True
        
        new_email = row.get('E-Mail') if 'E-Mail' in row and not pd.isna(row['E-Mail']) else None
        if customer.email != new_email:
            customer.email = new_email
            modified = True
        
        # Telefonnummern aktualisieren
        phone_parts = []
        for field in ['Festnetz', 'Mobil']:
            if field in row and not pd.isna(row[field]) and row[field]:
                phone_parts.append(str(row[field]))
        
        new_phone = " / ".join(phone_parts) if phone_parts else None
        if customer.phone != new_phone:
            customer.phone = new_phone
            modified = True
        
        if modified:
            customer.updated_at = datetime.utcnow()
            self.db.commit()
            logger.info(f"Kunde aktualisiert: {customer.name} (ID: {customer.id})")
    
    def _process_job_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Auftragsdaten

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne Auftragsnummer
                if 'Auftragsnummer' not in row or pd.isna(row['Auftragsnummer']) or not str(row['Auftragsnummer']).strip():
                    continue
                
                # Prüfen, ob Auftrag bereits existiert
                existing_job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
                
                if existing_job:
                    # Update bestehender Auftrag
                    self._update_job(existing_job, row)
                else:
                    # Neuer Auftrag
                    self._create_job(row)
            
            logger.info(f"Auftragsdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Auftragsdatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_job(self, row: Dict) -> Job:
        """
        Erstellt einen neuen Auftrag aus CSV-Daten

        Args:
            row: Dictionary mit Auftragsdaten

        Returns:
            Neue Job-Instanz
        """
        # Kundenidentifikation
        customer = None
        
        # Versuche, den Kunden anhand der Kontaktreferenzen zu finden
        for contact_field in ['Auftraggeber Kontakt', 'Eigentümer 1 Kontakt', 'Bewohner 1 Kontakt']:
            if contact_field in row and not pd.isna(row[contact_field]) and row[contact_field]:
                customer = self.db.query(Customer).filter_by(bexio_id=str(row[contact_field])).first()
                if customer:
                    break
        
        # Wenn kein Kunde gefunden wurde, erstelle einen generischen
        if not customer and 'Auftraggeber Kontakt' in row and not pd.isna(row['Auftraggeber Kontakt']):
            customer = Customer(
                name=f"Kunde für Auftrag {row['Auftragsnummer']}",
                bexio_id=str(row['Auftraggeber Kontakt'])
            )
            self.db.add(customer)
            self.db.commit()
        
        # Auftragsbeschreibung zusammenstellen
        description_parts = []
        
        if 'Schadensart' in row and not pd.isna(row['Schadensart']) and row['Schadensart']:
            description_parts.append(f"Schadensart: {row['Schadensart']}")
        
        if 'Räume' in row and not pd.isna(row['Räume']) and row['Räume']:
            description_parts.append(f"Betroffene Räume: {row['Räume']}")
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            description_parts.append(f"Bemerkungen: {row['Bemerkungen']}")
        
        # Auftragsstatus aus CSV-Status ableiten
        status_mapping = {
            'aktiv': 'active',
            'abgeschlossen': 'completed',
            'in Bearbeitung': 'active',
            'offen': 'pending',
            'storniert': 'canceled'
        }
        
        csv_status = row.get('Status', '').lower() if 'Status' in row and not pd.isna(row['Status']) else ''
        db_status = status_mapping.get(csv_status, 'pending')
        
        # Rechnungsdatum als Ende verwenden, falls vorhanden
        end_date = None
        if 'Datum Rechnungsversand' in row and not pd.isna(row['Datum Rechnungsversand']):
            try:
                end_date = datetime.strptime(row['Datum Rechnungsversand'], '%m/%d/%Y')
            except:
                try:
                    end_date = datetime.strptime(row['Datum Rechnungsversand'], '%d.%m.%Y')
                except:
                    pass
        
        # Startdatum aus Erstbegehung
        start_date = None
        if 'Datum Erstbegehung' in row and not pd.isna(row['Datum Erstbegehung']):
            try:
                start_date = datetime.strptime(row['Datum Erstbegehung'], '%m/%d/%Y')
            except:
                try:
                    start_date = datetime.strptime(row['Datum Erstbegehung'], '%d.%m.%Y')
                except:
                    pass
        
        # Erstelle den Job
        job = Job(
            name=str(row['Auftragsnummer']),
            description="\n".join(description_parts),
            status=db_status,
            location=row.get('Adresse', '') if 'Adresse' in row and not pd.isna(row['Adresse']) else None,
            start_date=start_date,
            end_date=end_date,
            customer_id=customer.id if customer else None,
            bexio_id=int(row['Auftragsnummer'][3:]) if row['Auftragsnummer'].startswith('500') else None
        )
        
        self.db.add(job)
        self.db.commit()
        logger.info(f"Neuer Auftrag erstellt: {job.name} (ID: {job.id})")
        
        return job
    
    def _update_job(self, job: Job, row: Dict) -> None:
        """
        Aktualisiert einen bestehenden Auftrag mit neuen CSV-Daten

        Args:
            job: Bestehende Job-Instanz
            row: Dictionary mit aktualisierten Auftragsdaten
        """
        modified = False
        
        # Beschreibung aktualisieren wenn sich relevante Felder geändert haben
        description_parts = []
        
        if 'Schadensart' in row and not pd.isna(row['Schadensart']) and row['Schadensart']:
            description_parts.append(f"Schadensart: {row['Schadensart']}")
        
        if 'Räume' in row and not pd.isna(row['Räume']) and row['Räume']:
            description_parts.append(f"Betroffene Räume: {row['Räume']}")
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            description_parts.append(f"Bemerkungen: {row['Bemerkungen']}")
        
        new_description = "\n".join(description_parts)
        if job.description != new_description:
            job.description = new_description
            modified = True
        
        # Status aktualisieren
        status_mapping = {
            'aktiv': 'active',
            'abgeschlossen': 'completed',
            'in Bearbeitung': 'active',
            'offen': 'pending',
            'storniert': 'canceled'
        }
        
        csv_status = row.get('Status', '').lower() if 'Status' in row and not pd.isna(row['Status']) else ''
        new_status = status_mapping.get(csv_status, job.status)
        
        if job.status != new_status:
            job.status = new_status
            modified = True
        
        # Adresse/Standort aktualisieren
        new_location = row.get('Adresse', '') if 'Adresse' in row and not pd.isna(row['Adresse']) else None
        if job.location != new_location:
            job.location = new_location
            modified = True
        
        # Enddatum aktualisieren
        if 'Datum Rechnungsversand' in row and not pd.isna(row['Datum Rechnungsversand']):
            try:
                new_end_date = datetime.strptime(row['Datum Rechnungsversand'], '%m/%d/%Y')
                if job.end_date != new_end_date:
                    job.end_date = new_end_date
                    modified = True
            except:
                try:
                    new_end_date = datetime.strptime(row['Datum Rechnungsversand'], '%d.%m.%Y')
                    if job.end_date != new_end_date:
                        job.end_date = new_end_date
                        modified = True
                except:
                    pass
        
        # Startdatum aktualisieren
        if 'Datum Erstbegehung' in row and not pd.isna(row['Datum Erstbegehung']):
            try:
                new_start_date = datetime.strptime(row['Datum Erstbegehung'], '%m/%d/%Y')
                if job.start_date != new_start_date:
                    job.start_date = new_start_date
                    modified = True
            except:
                try:
                    new_start_date = datetime.strptime(row['Datum Erstbegehung'], '%d.%m.%Y')
                    if job.start_date != new_start_date:
                        job.start_date = new_start_date
                        modified = True
                except:
                    pass
        
        if modified:
            job.updated_at = datetime.utcnow()
            self.db.commit()
            logger.info(f"Auftrag aktualisiert: {job.name} (ID: {job.id})")
    
    def _process_device_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Gerätedaten

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne Gerätenummer
                if 'Gerätenummer' not in row or pd.isna(row['Gerätenummer']) or not str(row['Gerätenummer']).strip():
                    continue
                
                # Prüfen, ob Gerät bereits existiert
                existing_device = self.db.query(Device).filter_by(esp_id=str(row['Gerätenummer'])).first()
                
                if existing_device:
                    # Update bestehendes Gerät
                    self._update_device(existing_device, row)
                else:
                    # Neues Gerät
                    self._create_device(row)
            
            logger.info(f"Gerätedatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Gerätedatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_device(self, row: Dict) -> Device:
        """
        Erstellt ein neues Gerät aus CSV-Daten

        Args:
            row: Dictionary mit Gerätedaten

        Returns:
            Neue Device-Instanz
        """
        # Gerätetyp aus Kategorie ableiten
        device_type_mapping = {
            'Entfeuchter': 'dehumidifier',
            'Ventilator': 'fan',
            'Heizung': 'heater',
            'Dämmschichttrocknung': 'insulation_dryer',
            'Sensor': 'sensor'
        }
        
        category = row.get('Kategorie', '') if 'Kategorie' in row and not pd.isna(row['Kategorie']) else ''
        device_type = device_type_mapping.get(category, 'dehumidifier')  # Standardtyp, falls nicht zugeordnet
        
        # Name aus Modellbezeichnung oder Kategorie + Gerätenummer
        device_name = None
        if 'Name' in row and not pd.isna(row['Name']) and row['Name']:
            device_name = row['Name']
        else:
            device_name = f"{category} {row['Gerätenummer']}" if category else f"Gerät {row['Gerätenummer']}"
        
        # Erstelle das Gerät
        device = Device(
            name=device_name,
            esp_id=str(row['Gerätenummer']),
            type=device_type,
            status='inactive',  # Standardstatus für neue Geräte
            location=row.get('Einsatzort', '') if 'Einsatzort' in row and not pd.isna(row['Einsatzort']) else None,
            firmware_version='1.0'  # Standardversion für importierte Geräte
        )
        
        self.db.add(device)
        self.db.commit()
        logger.info(f"Neues Gerät erstellt: {device.name} (ID: {device.id})")
        
        return device
    
    def _update_device(self, device: Device, row: Dict) -> None:
        """
        Aktualisiert ein bestehendes Gerät mit neuen CSV-Daten

        Args:
            device: Bestehende Device-Instanz
            row: Dictionary mit aktualisierten Gerätedaten
        """
        modified = False
        
        # Name aktualisieren wenn sich Modellbezeichnung geändert hat
        new_name = None
        if 'Name' in row and not pd.isna(row['Name']) and row['Name']:
            new_name = row['Name']
        else:
            category = row.get('Kategorie', '') if 'Kategorie' in row and not pd.isna(row['Kategorie']) else ''
            new_name = f"{category} {row['Gerätenummer']}" if category else f"Gerät {row['Gerätenummer']}"
        
        if device.name != new_name:
            device.name = new_name
            modified = True
        
        # Gerätetyp aktualisieren
        device_type_mapping = {
            'Entfeuchter': 'dehumidifier',
            'Ventilator': 'fan',
            'Heizung': 'heater',
            'Dämmschichttrocknung': 'insulation_dryer',
            'Sensor': 'sensor'
        }
        
        category = row.get('Kategorie', '') if 'Kategorie' in row and not pd.isna(row['Kategorie']) else ''
        new_type = device_type_mapping.get(category, device.type)  # Beibehalten bei Unbekannt
        
        if device.type != new_type:
            device.type = new_type
            modified = True
        
        # Standort aktualisieren
        new_location = row.get('Einsatzort', '') if 'Einsatzort' in row and not pd.isna(row['Einsatzort']) else None
        if device.location != new_location:
            device.location = new_location
            modified = True
        
        if modified:
            device.updated_at = datetime.utcnow()
            self.db.commit()
            logger.info(f"Gerät aktualisiert: {device.name} (ID: {device.id})")
    
    def _process_measurement_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Messwerten/Geräteeinsätzen

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne UID oder Gerätenummer
                if any(key not in row or pd.isna(row[key]) or not str(row[key]).strip() 
                       for key in ['UID', 'Gerätenummer', 'Auftragsnummer']):
                    continue
                
                # Finde das Gerät
                device = self.db.query(Device).filter_by(esp_id=str(row['Gerätenummer'])).first()
                if not device:
                    logger.warning(f"Gerät nicht gefunden: {row['Gerätenummer']}")
                    continue
                
                # Finde den Auftrag
                job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
                if not job:
                    logger.warning(f"Auftrag nicht gefunden: {row['Auftragsnummer']}")
                    continue
                
                # Prüfen, ob es bereits eine Verknüpfung gibt
                existing_job_device = self.db.query(JobDevice).filter_by(
                    job_id=job.id, device_id=device.id
                ).first()
                
                # Messwert erstellen
                self._create_measurement(row, device)
                
                # Bei Montage oder Demontage den Geräte-Auftrag-Einsatz aktualisieren
                art = row.get('Art', '') if 'Art' in row and not pd.isna(row['Art']) else ''
                
                if art in ['Montage', 'Demontage']:
                    if existing_job_device:
                        self._update_job_device(existing_job_device, row)
                    else:
                        self._create_job_device(row, job.id, device.id)
            
            logger.info(f"Messwertdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Messwertdatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_measurement(self, row: Dict, device: Device) -> Measurement:
        """
        Erstellt einen neuen Messwert aus CSV-Daten

        Args:
            row: Dictionary mit Messwertdaten
            device: Zugehöriges Gerät

        Returns:
            Neue Measurement-Instanz
        """
        # Datum des Messwerts ermitteln
        timestamp = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                timestamp = datetime.strptime(row['Datum'], '%m/%d/%Y')
            except:
                try:
                    timestamp = datetime.strptime(row['Datum'], '%d.%m.%Y')
                except:
                    timestamp = datetime.utcnow()
        else:
            timestamp = datetime.utcnow()
        
        # Energieverbrauch aus Zählerstand, falls vorhanden
        energy_consumption = None
        if 'Zählerstand kWh' in row and not pd.isna(row['Zählerstand kWh']):
            try:
                energy_consumption = float(row['Zählerstand kWh'])
            except:
                pass
        
        # Messwert erstellen
        measurement = Measurement(
            device_id=device.id,
            timestamp=timestamp,
            energy_consumption=energy_consumption,
            # Die anderen Felder werden dynamisch befüllt, wenn verfügbar
            temperature=None,
            humidity=None,
            voltage=None,
            signal_strength=None
        )
        
        self.db.add(measurement)
        self.db.commit()
        logger.debug(f"Neuer Messwert erstellt für Gerät {device.name} (Messwert-ID: {measurement.id})")
        
        return measurement
    
    def _create_job_device(self, row: Dict, job_id: int, device_id: int) -> JobDevice:
        """
        Erstellt eine neue Verknüpfung zwischen Auftrag und Gerät

        Args:
            row: Dictionary mit Einsatzdaten
            job_id: ID des Auftrags
            device_id: ID des Geräts

        Returns:
            Neue JobDevice-Instanz
        """
        # Datum des Einsatzes ermitteln
        start_date = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                start_date = datetime.strptime(row['Datum'], '%m/%d/%Y')
            except:
                try:
                    start_date = datetime.strptime(row['Datum'], '%d.%m.%Y')
                except:
                    start_date = datetime.utcnow()
        else:
            start_date = datetime.utcnow()
        
        # Bei Demontage das Enddatum setzen
        art = row.get('Art', '') if 'Art' in row and not pd.isna(row['Art']) else ''
        end_date = start_date if art == 'Demontage' else None
        
        # Notizen zusammenstellen
        notes = []
        if 'Standort' in row and not pd.isna(row['Standort']) and row['Standort']:
            notes.append(f"Standort: {row['Standort']}")
        
        if 'Wohnung' in row and not pd.isna(row['Wohnung']) and row['Wohnung']:
            notes.append(f"Wohnung: {row['Wohnung']}")
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            notes.append(f"Bemerkungen: {row['Bemerkungen']}")
        
        # Verknüpfung erstellen
        job_device = JobDevice(
            job_id=job_id,
            device_id=device_id,
            start_date=start_date,
            end_date=end_date,
            notes="\n".join(notes) if notes else None
        )
        
        self.db.add(job_device)
        self.db.commit()
        logger.info(f"Neue Gerät-Auftrag-Verknüpfung erstellt (ID: {job_device.id})")
        
        return job_device
    
    def _update_job_device(self, job_device: JobDevice, row: Dict) -> None:
        """
        Aktualisiert eine bestehende Verknüpfung zwischen Auftrag und Gerät

        Args:
            job_device: Bestehende JobDevice-Instanz
            row: Dictionary mit aktualisierten Einsatzdaten
        """
        modified = False
        
        # Bei Demontage das Enddatum setzen
        art = row.get('Art', '') if 'Art' in row and not pd.isna(row['Art']) else ''
        
        if art == 'Demontage' and job_device.end_date is None:
            # Datum des Einsatzes ermitteln
            end_date = None
            if 'Datum' in row and not pd.isna(row['Datum']):
                try:
                    end_date = datetime.strptime(row['Datum'], '%m/%d/%Y')
                except:
                    try:
                        end_date = datetime.strptime(row['Datum'], '%d.%m.%Y')
                    except:
                        end_date = datetime.utcnow()
            else:
                end_date = datetime.utcnow()
            
            job_device.end_date = end_date
            modified = True
        
        # Notizen aktualisieren
        notes = []
        if 'Standort' in row and not pd.isna(row['Standort']) and row['Standort']:
            notes.append(f"Standort: {row['Standort']}")
        
        if 'Wohnung' in row and not pd.isna(row['Wohnung']) and row['Wohnung']:
            notes.append(f"Wohnung: {row['Wohnung']}")
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            notes.append(f"Bemerkungen: {row['Bemerkungen']}")
        
        new_notes = "\n".join(notes) if notes else None
        if job_device.notes != new_notes:
            # Bestehende Notizen ergänzen, nicht überschreiben
            if job_device.notes and new_notes:
                job_device.notes += f"\n\n{new_notes}"
            else:
                job_device.notes = new_notes
            
            modified = True
        
        if modified:
            self.db.commit()
            logger.info(f"Gerät-Auftrag-Verknüpfung aktualisiert (ID: {job_device.id})")
    
    def _process_moisture_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Materialfeuchtemessungen

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne UID oder Auftragsnummer
                if any(key not in row or pd.isna(row[key]) or not str(row[key]).strip() 
                       for key in ['UID', 'Auftragsnummer']):
                    continue
                
                # Baustoffmessungen als spezialisierte Messwerte speichern
                self._create_material_moisture_measurement(row)
            
            logger.info(f"Baustofffeuchte-Datei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Baustofffeuchte-Datei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_material_moisture_measurement(self, row: Dict) -> None:
        """
        Erstellt einen spezialisierten Messwert für Baustofffeuchte

        Args:
            row: Dictionary mit Messungsdaten
        """
        # Auftrag finden
        job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
        if not job:
            logger.warning(f"Auftrag nicht gefunden: {row['Auftragsnummer']}")
            return
        
        # Datum ermitteln
        timestamp = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                timestamp = datetime.strptime(row['Datum'], '%m/%d/%Y')
            except:
                try:
                    timestamp = datetime.strptime(row['Datum'], '%d.%m.%Y')
                except:
                    timestamp = datetime.utcnow()
        else:
            timestamp = datetime.utcnow()
        
        # Systemmeldung für die Baustofffeuchtemessung erstellen
        log_message = f"Baustofffeuchtemessung für Auftrag {row['Auftragsnummer']}"
        
        if 'Baustoff' in row and not pd.isna(row['Baustoff']) and row['Baustoff']:
            log_message += f"\nBaustoff: {row['Baustoff']}"
        
        if 'Ort' in row and not pd.isna(row['Ort']) and row['Ort']:
            log_message += f"\nOrt: {row['Ort']}"
        
        if 'Stelle' in row and not pd.isna(row['Stelle']) and row['Stelle']:
            log_message += f"\nStelle: {row['Stelle']}"
        
        # Messwerte hinzufügen
        measurement_fields = [
            ('Digits tiefster Wert', 'Digits (min)'),
            ('Digits Mittelwert', 'Digits (Mittel)'),
            ('Digits höchster Wert', 'Digits (max)'),
            ('% tiefster Wert', 'Feuchtigkeit % (min)'),
            ('% Mittelwert', 'Feuchtigkeit % (Mittel)'),
            ('% höchster Wert', 'Feuchtigkeit % (max)'),
            ('Temperatur', 'Temperatur °C'),
            ('Luftfeuchtigkeit', 'Luftfeuchtigkeit %')
        ]
        
        for csv_field, display_name in measurement_fields:
            if csv_field in row and not pd.isna(row[csv_field]) and row[csv_field]:
                try:
                    value = float(row[csv_field])
                    log_message += f"\n{display_name}: {value}"
                except:
                    log_message += f"\n{display_name}: {row[csv_field]}"
        
        # Bemerkungen hinzufügen
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            log_message += f"\n\nBemerkungen: {row['Bemerkungen']}"
        
        # Erfasser hinzufügen
        if 'Erfasser' in row and not pd.isna(row['Erfasser']) and row['Erfasser']:
            log_message += f"\n\nErfasser: {row['Erfasser']}"
        
        # Log-Eintrag erstellen
        system_log = SystemLog(
            level="INFO",
            message=log_message,
            source=f"CSV Import: Baustofffeuchte-Messung",
            timestamp=timestamp
        )
        
        self.db.add(system_log)
        self.db.commit()
        logger.debug(f"Baustofffeuchte-Messung importiert für Auftrag {row['Auftragsnummer']}")
    
    def _process_insulation_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Dämmschicht-Messungen

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne UID oder Auftragsnummer
                if any(key not in row or pd.isna(row[key]) or not str(row[key]).strip() 
                       for key in ['UID', 'Auftragsnummer']):
                    continue
                
                # Dämmschicht-Messungen als spezialisierte Messwerte speichern
                self._create_insulation_measurement(row)
            
            logger.info(f"Dämmschichtmessungs-Datei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Dämmschichtmessungs-Datei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_insulation_measurement(self, row: Dict) -> None:
        """
        Erstellt einen spezialisierten Messwert für Dämmschichtmessungen

        Args:
            row: Dictionary mit Messungsdaten
        """
        # Auftrag finden
        job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
        if not job:
            logger.warning(f"Auftrag nicht gefunden: {row['Auftragsnummer']}")
            return
        
        # Datum ermitteln
        timestamp = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                timestamp = datetime.strptime(row['Datum'], '%m/%d/%Y')
            except:
                try:
                    timestamp = datetime.strptime(row['Datum'], '%d.%m.%Y')
                except:
                    timestamp = datetime.utcnow()
        else:
            timestamp = datetime.utcnow()
        
        # Systemmeldung für die Dämmschichtmessung erstellen
        log_message = f"Dämmschichtmessung für Auftrag {row['Auftragsnummer']}"
        
        if 'Art' in row and not pd.isna(row['Art']) and row['Art']:
            log_message += f"\nDämmstoffart: {row['Art']}"
        
        if 'Ort' in row and not pd.isna(row['Ort']) and row['Ort']:
            log_message += f"\nOrt: {row['Ort']}"
        
        # Referenzwerte hinzufügen
        reference_fields = [
            ('Referenz %', 'Referenz Feuchtigkeit %'),
            ('Referenz °C', 'Referenz Temperatur °C'),
            ('Referenz g/m3', 'Referenz Absolutfeuchte g/m³')
        ]
        
        for csv_field, display_name in reference_fields:
            if csv_field in row and not pd.isna(row[csv_field]) and row[csv_field]:
                try:
                    value = float(row[csv_field])
                    log_message += f"\n{display_name}: {value}"
                except:
                    log_message += f"\n{display_name}: {row[csv_field]}"
        
        # Bohrungswerte hinzufügen
        for i in range(1, 21):  # Bohrungen 1-20
            if all(f'Bohrung {i} {suffix}' in row and not pd.isna(row[f'Bohrung {i} {suffix}']) and row[f'Bohrung {i} {suffix}']
                   for suffix in ['%', '°C']):
                try:
                    humidity = float(row[f'Bohrung {i} %'])
                    temperature = float(row[f'Bohrung {i} °C'])
                    
                    log_message += f"\n\nBohrung {i}:"
                    log_message += f"\n  Feuchtigkeit: {humidity}%"
                    log_message += f"\n  Temperatur: {temperature}°C"
                    
                    if f'Bohrung {i} g/m3' in row and not pd.isna(row[f'Bohrung {i} g/m3']) and row[f'Bohrung {i} g/m3']:
                        try:
                            abs_humidity = float(row[f'Bohrung {i} g/m3'])
                            log_message += f"\n  Absolutfeuchte: {abs_humidity} g/m³"
                        except:
                            pass
                except:
                    continue
        
        # Erfasser hinzufügen
        if 'Erfasser' in row and not pd.isna(row['Erfasser']) and row['Erfasser']:
            log_message += f"\n\nErfasser: {row['Erfasser']}"
        
        # Bemerkungen hinzufügen
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            log_message += f"\n\nBemerkungen: {row['Bemerkungen']}"
        
        # Log-Eintrag erstellen
        system_log = SystemLog(
            level="INFO",
            message=log_message,
            source=f"CSV Import: Dämmschichtmessung",
            timestamp=timestamp
        )
        
        self.db.add(system_log)
        self.db.commit()
        logger.debug(f"Dämmschichtmessung importiert für Auftrag {row['Auftragsnummer']}")
    
    def _process_report_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Berichtsdaten und Bildern

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne UID oder Auftragsnummer
                if any(key not in row or pd.isna(row[key]) or not str(row[key]).strip() 
                       for key in ['UID', 'Auftragsnummer']):
                    continue
                
                # Berichtsbilder als Report und ReportImage speichern
                self._create_report_entry(row)
            
            logger.info(f"Berichtsdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Berichtsdatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_report_entry(self, row: Dict) -> None:
        """
        Erstellt Berichtseinträge aus Bilddaten

        Args:
            row: Dictionary mit Berichtsdaten
        """
        # Auftrag finden
        job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
        if not job:
            logger.warning(f"Auftrag nicht gefunden: {row['Auftragsnummer']}")
            return
        
        # Datum ermitteln
        timestamp = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                timestamp = datetime.strptime(row['Datum'], '%m/%d/%Y')
            except:
                try:
                    timestamp = datetime.strptime(row['Datum'], '%d.%m.%Y')
                except:
                    timestamp = datetime.utcnow()
        else:
            timestamp = datetime.utcnow()
        
        # Prüfen ob es Bilder gibt
        has_images = False
        for i in range(1, 4):
            if f'Bild {i}' in row and not pd.isna(row[f'Bild {i}']) and row[f'Bild {i}']:
                has_images = True
                break
        
        if not has_images and 'Zeichnung 1' not in row:
            return  # Keine Bilder vorhanden
        
        # Berichtstitel generieren
        report_title = f"Bilddokumentation für Auftrag {row['Auftragsnummer']}"
        
        # Berichtsinhalt generieren
        report_content = "Automatisch importierte Bilddokumentation"
        
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            report_content += f"\n\nBemerkungen: {row['Bemerkungen']}"
        
        # Bestehenden Bericht finden oder erstellen
        existing_report = self.db.query(Report).filter_by(
            job_id=job.id, 
            created_at=timestamp
        ).first()
        
        if existing_report:
            report = existing_report
            
            # Bestehenden Inhalt ergänzen
            if report.content:
                report.content += f"\n\n---\n\n{report_content}"
            else:
                report.content = report_content
            
            report.updated_at = datetime.utcnow()
        else:
            # Neuen Bericht erstellen
            report = Report(
                job_id=job.id,
                title=report_title,
                content=report_content,
                type='regular',
                created_at=timestamp
            )
            self.db.add(report)
        
        self.db.commit()
        
        # Bilder zum Bericht hinzufügen
        for i in range(1, 4):
            if f'Bild {i}' in row and not pd.isna(row[f'Bild {i}']) and row[f'Bild {i}']:
                image_path = row[f'Bild {i}']
                
                # Prüfen ob das Bild bereits existiert
                existing_image = self.db.query(ReportImage).filter_by(
                    report_id=report.id, 
                    image_path=image_path
                ).first()
                
                if not existing_image:
                    # Neues Bild hinzufügen
                    report_image = ReportImage(
                        report_id=report.id,
                        image_path=image_path,
                        caption=f"Bild {i}",
                        order=i
                    )
                    self.db.add(report_image)
        
        # Zeichnung hinzufügen falls vorhanden
        if 'Zeichnung 1' in row and not pd.isna(row['Zeichnung 1']) and row['Zeichnung 1']:
            image_path = row['Zeichnung 1']
            
            # Prüfen ob das Bild bereits existiert
            existing_image = self.db.query(ReportImage).filter_by(
                report_id=report.id, 
                image_path=image_path
            ).first()
            
            if not existing_image:
                # Neue Zeichnung hinzufügen
                report_image = ReportImage(
                    report_id=report.id,
                    image_path=image_path,
                    caption="Zeichnung",
                    order=10  # Zeichnungen am Ende
                )
                self.db.add(report_image)
        
        self.db.commit()
        logger.debug(f"Berichtseintrag importiert für Auftrag {row['Auftragsnummer']}")
    
    def _process_activities_file(self, file_path: str) -> None:
        """
        Verarbeitet eine CSV-Datei mit Aktivitäten und Arbeitszeiten

        Args:
            file_path: Pfad zur CSV-Datei
        """
        records = self._safe_csv_read(file_path)
        if not records:
            return
        
        try:
            for row in records:
                # Überspringe Zeilen ohne UID oder Auftragsnummer
                if any(key not in row or pd.isna(row[key]) or not str(row[key]).strip() 
                       for key in ['UID', 'Auftragsnummer']):
                    continue
                
                # Aktivitäten als SystemLog-Einträge speichern
                self._create_activity_log(row)
            
            logger.info(f"Aktivitätsdatei erfolgreich verarbeitet: {file_path}")
        
        except Exception as e:
            logger.error(f"Fehler bei der Verarbeitung der Aktivitätsdatei {file_path}: {str(e)}")
            self.error_count += 1
    
    def _create_activity_log(self, row: Dict) -> None:
        """
        Erstellt SystemLog-Einträge aus Aktivitätsdaten

        Args:
            row: Dictionary mit Aktivitätsdaten
        """
        # Auftrag finden
        job = self.db.query(Job).filter_by(name=str(row['Auftragsnummer'])).first()
        if not job:
            logger.warning(f"Auftrag nicht gefunden: {row['Auftragsnummer']}")
            return
        
        # Datum ermitteln
        timestamp = None
        if 'Datum' in row and not pd.isna(row['Datum']):
            try:
                timestamp = datetime.strptime(row['Datum'], '%m/%d/%Y')
            except:
                try:
                    timestamp = datetime.strptime(row['Datum'], '%d.%m.%Y')
                except:
                    timestamp = datetime.utcnow()
        else:
            timestamp = datetime.utcnow()
        
        # Aktivitätstypen und -zeiten erfassen
        activity_fields = [
            'Schadenaufnahme', 'Messung/Kontrolle', 'Sofortmassnahmen', 'Leckortung',
            'Montage', 'Demontage', 'Wassersaugen', 'Fahrzeit', 'Bericht', 'Bauleitung',
            'Schimmelbehandlung', 'Abdeckarbeiten', 'Besprechung', 'Endabnahme', 'Bauprogramm', 'Abbruch'
        ]
        
        activities = []
        for field in activity_fields:
            if field in row and not pd.isna(row[field]) and row[field]:
                try:
                    hours = float(row[field])
                    if hours > 0:
                        activities.append(f"{field}: {hours}h")
                except:
                    continue
        
        if not activities:
            return  # Keine Aktivitäten vorhanden
        
        # Log-Nachricht erstellen
        log_message = f"Aktivitäten für Auftrag {row['Auftragsnummer']}"
        
        # Mitarbeiter hinzufügen
        if 'Mitarbeiter' in row and not pd.isna(row['Mitarbeiter']) and row['Mitarbeiter']:
            log_message += f"\nMitarbeiter: {row['Mitarbeiter']}"
        
        # Aktivitäten hinzufügen
        log_message += "\n\n" + "\n".join(activities)
        
        # Bemerkungen hinzufügen
        if 'Bemerkungen' in row and not pd.isna(row['Bemerkungen']) and row['Bemerkungen']:
            log_message += f"\n\nBemerkungen: {row['Bemerkungen']}"
        
        # Log-Eintrag erstellen
        system_log = SystemLog(
            level="INFO",
            message=log_message,
            source=f"CSV Import: Aktivitäten",
            timestamp=timestamp
        )
        
        self.db.add(system_log)
        self.db.commit()
        logger.debug(f"Aktivitäten importiert für Auftrag {row['Auftragsnummer']}")


def import_csv_data(directory_path: str) -> Tuple[int, int]:
    """
    Funktion zum Importieren von CSV-Daten aus einem Verzeichnis

    Args:
        directory_path: Pfad zum Verzeichnis mit CSV-Dateien

    Returns:
        Tuple mit Anzahl erfolgreicher Verarbeitungen und Fehler
    """
    db = next(get_db())
    processor = CSVProcessor(db)
    return processor.process_directory(directory_path)


# CLI-Interface, wenn direkt ausgeführt
if __name__ == "__main__":
    import sys
    
    # Logging konfigurieren
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    if len(sys.argv) < 2:
        print("Verwendung: python csv_processor.py <verzeichnis_mit_csv_dateien>")
        sys.exit(1)
    
    directory = sys.argv[1]
    
    success, errors = import_csv_data(directory)
    print(f"CSV-Import abgeschlossen: {success} erfolgreich, {errors} Fehler")