FROM python:3.11-slim

WORKDIR /app

# System-Abhängigkeiten installieren
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        libpq-dev \
        netcat-traditional \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Python-Abhängigkeiten kopieren und installieren
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Warten auf Datenbank-Script
COPY wait-for-db.sh .
RUN chmod +x wait-for-db.sh

# App-Dateien kopieren
COPY app/ .

# Verzeichnisse für Uploads und Logs erstellen
RUN mkdir -p uploads logs

# Netzwerk-Port freigeben
EXPOSE 5000

# Warten auf Datenbank und dann App starten
CMD ["./wait-for-db.sh", "db", "5432", "python", "run.py"]