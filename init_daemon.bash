#!/bin/bash

PROG_FILE="webserv"
SERVICE_NAME="webserv"
SOURCE_DIR="daemon/"
PLIST_FILE="dev.maxencegama.webserv.plist"
SERVICE_FILE="webserv.service"

# Fonction pour configurer et démarrer le service avec launchd (macOS)
setup_launchd() {
    echo "Setting up $SERVICE_NAME with launchd (macOS)..."
    if [ ! -f "$SOURCE_DIR$PLIST_FILE" ]; then
        echo "Error: $PLIST_FILE not found!"
        exit 1
    fi
    sudo cp "$SOURCE_DIR$PLIST_FILE" /Library/LaunchDaemons/
    sudo launchctl load /Library/LaunchDaemons/"$PLIST_FILE"
    sudo launchctl start "dev.maxencegama.$SERVICE_NAME"
    echo "$SERVICE_NAME started with launchd."
}

# Fonction pour configurer et démarrer le service avec systemd (Linux)
setup_systemd() {
    echo "Setting up $SERVICE_NAME with systemd (Linux)..."
    if [ ! -f "$SOURCE_DIR$SERVICE_FILE" ]; then
        echo "Error: $SERVICE_FILE not found!"
        exit 1
    fi
    sudo cp "$SOURCE_DIR$SERVICE_FILE" /etc/systemd/system/
    sudo systemctl daemon-reload
    sudo systemctl enable "$SERVICE_FILE"
    sudo systemctl start "$SERVICE_NAME"
    echo "$SERVICE_NAME started with systemd."
}

sudo mkdir -p /etc/webserv

sudo cp $PROG_FILE /usr/local/bin

# Détection du système d'exploitation et appel des fonctions appropriées
if [[ "$OSTYPE" == "darwin"* ]]; then
    setup_launchd
elif [[ -f /etc/debian_version || -f /etc/redhat-release ]]; then
    setup_systemd
else
    echo "Unsupported OS."
    exit 1
fi
