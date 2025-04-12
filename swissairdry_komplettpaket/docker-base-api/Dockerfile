FROM python:3.10-slim

WORKDIR /app

# System-Abhängigkeiten installieren
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    libpq-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Python-Abhängigkeiten installieren
COPY app/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Anwendungsdateien kopieren
COPY app/ .

# Port freigeben
EXPOSE 5000

# Umgebungsvariablen setzen
ENV PYTHONUNBUFFERED=1

# Anwendung starten
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "5000"]