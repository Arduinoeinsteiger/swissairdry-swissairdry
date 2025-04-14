FROM python:3.11-slim

WORKDIR /app

# Abhängigkeiten für psycopg2 und andere Pakete installieren
RUN apt-get update && apt-get install -y \
    gcc \
    python3-dev \
    libpq-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Kopiere requirements.txt und installiere Python-Abhängigkeiten
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Kopiere den Anwendungscode
COPY app/ .

# Port freigeben
EXPOSE 5000

# Umgebungsvariablen
ENV PYTHONUNBUFFERED=1
ENV APP_ENVIRONMENT=production
ENV API_HOST=0.0.0.0
ENV API_PORT=5000

# Startbefehl
CMD ["python", "run.py"]