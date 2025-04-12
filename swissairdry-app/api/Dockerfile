# SwissAirDry API Server Dockerfile
FROM python:3.11-slim

# Setze Arbeitsverzeichnis
WORKDIR /app

# Umgebungsvariablen setzen
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1
ENV STANDALONE_MODE=True
ENV DATABASE_URL="postgresql://swissairdry:swissairdry@postgres:5432/swissairdry"

# Erforderliche Pakete installieren
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libpq-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Abhängigkeiten installieren
COPY requirements.txt .
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Anwendungsdateien kopieren
COPY app/ .

# Port freigeben
EXPOSE 5000

# Startbefehl
CMD ["python", "run.py"]