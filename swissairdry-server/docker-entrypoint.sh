#!/bin/bash
set -e

# Warte auf die Datenbank
if [ "$DATABASE_URL" ]; then
  echo "Warte auf die Datenbank..."
  HOST=$(echo $DATABASE_URL | awk -F[@//] '{print $4}')
  PORT=$(echo $DATABASE_URL | awk -F[@://] '{print $6}' | awk -F/ '{print $1}')
  
  until nc -z $HOST $PORT; do
    echo "Datenbank ist noch nicht verfügbar - warte..."
    sleep 3
  done
  
  echo "Datenbank ist verfügbar!"
fi

# Erstelle statische Verzeichnisse, falls noch nicht vorhanden
mkdir -p /app/static /app/uploads /app/templates
chmod -R 777 /app/static /app/uploads

# Führe den übergebenen Befehl aus
exec "$@"