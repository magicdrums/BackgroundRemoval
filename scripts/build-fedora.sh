#!/usr/bin/env bash
# Compila e instala el plugin para OBS 32.x instalado vía RPM en Fedora.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"

echo "==> Comprobando dependencias..."
missing=()
for pkg in obs-studio-devel opencv-devel onnxruntime-devel cmake gcc-c++ gtest-devel; do
  rpm -q "$pkg" &>/dev/null || missing+=("$pkg")
done

if ((${#missing[@]})); then
  echo "Faltan paquetes. Instálalos con:"
  echo "  sudo dnf install ${missing[*]}"
  exit 1
fi

echo "==> Descargando modelo ONNX (si falta)..."
bash "$ROOT/scripts/download-model.sh"

echo "==> Configurando CMake..."
cmake -S "$ROOT" -B "$BUILD" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DLINUX_PORTABLE=OFF \
  -DBUILD_TESTING=ON

echo "==> Compilando..."
cmake --build "$BUILD" -j"$(nproc)"

echo "==> Pruebas unitarias..."
export OBS_BGREMOVAL_MODEL_PATH="$ROOT/data/models/mediapipe.onnx"
ctest --test-dir "$BUILD" --output-on-failure

echo "==> Instalando en el sistema (requiere sudo)..."
sudo cmake --install "$BUILD"

echo ""
echo "Plugin instalado. Reinicia OBS y añade el filtro:"
echo "  Clic derecho en fuente de vídeo → Filtros → Quitar fondo"
echo ""
echo "Rutas típicas:"
echo "  /usr/lib64/obs-plugins/obs-background-removal.so"
echo "  /usr/share/obs/obs-plugins/obs-background-removal/"
