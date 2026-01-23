#!/bin/bash
set -e

RUNTIME_DIR="/opt/http-server"
SERVER_BIN="server"
SERVICE="http-server"

BIN_UPDATED=false
HTML_UPDATED=false

echo "=== Building HTTP server ==="
g++ Server.cpp -O2 -std=c++20 -o "$SERVER_BIN"

echo "=== Deploying binary ==="
sudo systemctl stop $SERVICE

echo "Copying binary..."
sudo cp "$SERVER_BIN" "$RUNTIME_DIR/$SERVER_BIN"
sudo chown gavincoding:gavincoding "$RUNTIME_DIR/$SERVER_BIN"
sudo chmod 755 "$RUNTIME_DIR/$SERVER_BIN"

echo "Applying port 80 capability..."
sudo setcap cap_net_bind_service=+ep "$RUNTIME_DIR/$SERVER_BIN"

BIN_UPDATED=true

echo "=== Syncing HTML files ==="
for file in *.html; do
    [ -e "$file" ] || continue

    if [ ! -f "$RUNTIME_DIR/$file" ]; then
        echo "Adding new HTML: $file"
        sudo cp "$file" "$RUNTIME_DIR/$file"
        HTML_UPDATED=true
    elif ! diff -q "$file" "$RUNTIME_DIR/$file" >/dev/null; then
        echo "Updating changed HTML: $file"
        sudo cp "$file" "$RUNTIME_DIR/$file"
        HTML_UPDATED=true
    else
        echo "No change: $file"
    fi
done

if [ "$BIN_UPDATED" = true ] || [ "$HTML_UPDATED" = true ]; then
    echo "Restarting server..."
    sudo systemctl start $SERVICE
else
    echo "No changes detected; server not restarted."
fi

echo "=== Deploy complete ==="
