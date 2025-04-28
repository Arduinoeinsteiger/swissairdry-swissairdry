#!/bin/bash

echo "Stoppe und entferne alle Container..."
docker-compose down -v

echo "Entferne alle Docker-Volumes..."
docker volume prune -f

echo "Entferne alle Docker-Netzwerke..."
docker network prune -f

echo "Entferne alle Docker-Images..."
docker image prune -af

echo "Deinstallation abgeschlossen!" 