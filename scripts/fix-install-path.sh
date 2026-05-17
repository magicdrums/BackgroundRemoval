#!/usr/bin/env bash
# Mueve el plugin de /usr/local (instalación por defecto de CMake) a /usr (OBS RPM).
set -euo pipefail

LOCAL_SO="/usr/local/lib64/obs-plugins/obs-background-removal.so"
LOCAL_DATA="/usr/local/share/obs/obs-plugins/obs-background-removal"
USR_SO="/usr/lib64/obs-plugins/obs-background-removal.so"
USR_DATA="/usr/share/obs/obs-plugins/obs-background-removal"

if [[ ! -f "$LOCAL_SO" && ! -f "$USR_SO" ]]; then
  echo "No se encontró el plugin. Compila e instala primero: ./scripts/build-fedora.sh" >&2
  exit 1
fi

if [[ -f "$LOCAL_SO" ]]; then
  echo "Copiando $LOCAL_SO -> $USR_SO"
  sudo install -Dm755 "$LOCAL_SO" "$USR_SO"
fi

if [[ -d "$LOCAL_DATA" ]]; then
  echo "Copiando datos $LOCAL_DATA -> $USR_DATA"
  sudo mkdir -p "$USR_DATA"
  sudo cp -a "$LOCAL_DATA/." "$USR_DATA/"
fi

echo ""
echo "Listo. Reinicia OBS y busca el filtro en una fuente de vídeo:"
echo "  Filtros → + → Quitar fondo"
echo ""
ls -la "$USR_SO" 2>/dev/null || true
ls -la "$USR_DATA/models/mediapipe.onnx" 2>/dev/null || echo "AVISO: falta el modelo en $USR_DATA/models/"
