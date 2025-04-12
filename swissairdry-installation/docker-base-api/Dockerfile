FROM python:3.9-slim

WORKDIR /app

# Umgebungsvariablen
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1
ENV PYTHONPATH=/app

# Installiere Abhängigkeiten
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libpq-dev \
    netcat-traditional \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Kopiere requirements.txt und installiere Python-Abhängigkeiten
COPY app/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Kopiere den Anwendungscode
COPY app/ .

# Einfache Überprüfung, ob der Server gestartet werden kann
RUN python -m compileall .

# Verzeichnisse erstellen und Berechtigungen setzen
RUN mkdir -p /app/static /app/uploads \
    && chmod -R 777 /app/static /app/uploads

# Set-up script hinzufügen
COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

EXPOSE 5000

ENTRYPOINT ["docker-entrypoint.sh"]
CMD ["python", "run.py"]