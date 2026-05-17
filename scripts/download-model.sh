#!/usr/bin/env bash
# Descarga el modelo MediaPipe ONNX usado por el plugin.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/data/models"
# URL raw del archivo en el repo (la URL "blob" del navegador no sirve para curl/wget)
URL="https://raw.githubusercontent.com/obs-backgroundremoval/obs-backgroundremoval/main/data/models/mediapipe.onnx"

mkdir -p "$DEST"
OUT="$DEST/mediapipe.onnx"

if [[ -f "$OUT" ]]; then
  echo "Modelo ya existe: $OUT"
  exit 0
fi

echo "Descargando modelo a $OUT ..."
if command -v curl >/dev/null 2>&1; then
  curl -fL "$URL" -o "$OUT"
elif command -v wget >/dev/null 2>&1; then
  wget -O "$OUT" "$URL"
else
  echo "Instala curl o wget para descargar el modelo." >&2
  exit 1
fi

echo "Listo: $OUT"
