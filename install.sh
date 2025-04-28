#!/bin/bash

echo "Starte Installation..."

# Erstelle notwendige Verzeichnisse
mkdir -p swissairdry/{api,simple-api,mosquitto/config}

# Kopiere Docker-Compose-Datei
cp docker-compose.yml swissairdry/

# Kopiere API-Dateien
cp -r api/* swissairdry/api/
cp -r simple-api/* swissairdry/simple-api/
cp -r mosquitto/config/* swissairdry/mosquitto/config/

# Wechsle in das swissairdry Verzeichnis
cd swissairdry

# Starte die Container
echo "Starte Docker-Container..."
docker-compose up -d

echo "Installation abgeschlossen!"
echo "Die Dienste sind unter folgenden Ports erreichbar:"
echo "- API: http://localhost:5000"
echo "- Simple API: http://localhost:5001"
echo "- MQTT Broker: mqtt://localhost:1883" 