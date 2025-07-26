#!/bin/bash

# Resolve absolute paths
ROOTDIR="$(cd "$(dirname "$0")" && pwd)"
RESDIR="$ROOTDIR/res"

# Target locations
MIME_XML="$RESDIR/image-x-farbfeld.xml"
DESKTOP_FILE="$RESDIR/ffpirate.desktop"
MIME_DEST="$HOME/.local/share/mime/packages"
APP_DEST="$HOME/.local/share/applications"

echo "Installing MIME type..."

mkdir -p "$MIME_DEST"
cp "$MIME_XML" "$MIME_DEST/"

# Update the MIME database
update-mime-database "$HOME/.local/share/mime"

echo "Installing desktop entry..."

mkdir -p "$APP_DEST"
cp "$DESKTOP_FILE" "$APP_DEST/"

# Register MIME default handler
xdg-mime default ffpirate.desktop image/x-farbfeld

# Update .desktop database
update-desktop-database "$APP_DEST"

echo "Installation complete."
echo "You may now double-click .ff and related files to launch zviewer.sh!"
