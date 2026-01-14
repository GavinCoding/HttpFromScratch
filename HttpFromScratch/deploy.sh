#!/bin/bash
set -e

PROJECT_DIR="$(pwd)"
RUNTIME_DIR="/opt/http-server"
SERVER_BIN="server"

echo "=== Building HTTP server ==="

g++ Server.cpp -O2 -std=c++20 -o "$SERVER_BIN"

echo "=== Deploying binary ==="
echo "Stopping Server..."
sudo systemctl stop http-server

echo "Deploying binary..."
echo cp "$SERVER_BIN" "$RUNTIME_DIR/$SERVER_BIN"
sudo chown gavincoding:gavincoding "$RUNTIME_DIR/$SERVER_BIN"

echo "Starting Server..."
sudo systemctl start http-server

BIN_UPDATED=true

echo "=== Syncing HTML files ==="

HTML_UPDATED=false

for file in *.html; do
    [ -e "$file" ] || continue

    if [ ! -f "$RUNTIME_DIR/$file" ]; then
        echo "Adding new HTML: $file"
        sudo cp "$file" "$RUNTIME_DIR/$file"
        HTML_UPDATED=true
    else
        if ! diff -q "$file" "$RUNTIME_DIR/$file" >/dev/null; then
            echo "Updating changed HTML: $file"
            sudo cp "$file" "$RUNTIME_DIR/$file"
            HTML_UPDATED=true
        else
            echo "No change: $file"
        fi
    fi
done

if [ "$BIN_UPDATED" = true ] || [ "$HTML_UPDATED" = true ]; then
    echo "=== Restarting server ==="
    sudo systemctl restart http-server
else
    echo "=== No changes detected ==="
fi

echo "=== Deploy complete ==="
