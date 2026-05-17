# OBS Background Removal (Fedora / OBS 32)

Plugin de filtro para [OBS Studio](https://obsproject.com/) 28+ que quita el fondo de una fuente de vídeo usando segmentación neuronal (modelo MediaPipe vía ONNX Runtime).

Probado con **OBS 32.1.1** instalado desde RPM en Fedora 43.

## Requisitos

- OBS Studio ≥ 28 (RPM: `obs-studio`)
- Paquetes de desarrollo en Fedora:

```bash
sudo dnf install obs-studio-devel opencv-devel onnxruntime-devel cmake gcc-c++ curl
```

## Compilar e instalar

```bash
chmod +x scripts/*.sh
./scripts/build-fedora.sh
```

El script descarga el modelo `mediapipe.onnx`, compila el plugin e instala en:

- `/usr/lib64/obs-plugins/obs-background-removal.so`
- `/usr/share/obs/obs-plugins/obs-background-removal/` (efectos, locale, modelo)

**Importante:** OBS en Fedora (RPM) solo carga plugins desde `/usr/lib64/obs-plugins/`, no desde `/usr/local`. Si ya compilaste antes y el filtro no aparece, ejecuta:

```bash
./scripts/fix-install-path.sh
```

Reinicia OBS después de instalar.

## Uso en OBS

1. Añade una fuente de vídeo (cámara, captura, etc.).
2. Clic derecho en la fuente → **Filtros** → **+** → **Quitar fondo** (o *Background Removal* en inglés).
3. Ajusta umbral, suavizado y hilos de CPU según tu equipo.

Para fondo transparente, coloca debajo una fuente o escena con transparencia; el filtro deja el canal alpha en la silueta detectada.

## Compilación manual

```bash
./scripts/download-model.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLINUX_PORTABLE=OFF
cmake --build build -j$(nproc)
sudo cmake --install build
```

## Créditos

Basado en la arquitectura del plugin [obs-backgroundremoval](https://github.com/obs-backgroundremoval/obs-backgroundremoval) (GPL-2.0). Modelo MediaPipe de su distribución de releases.

## Licencia

GPL-2.0 — ver [LICENSE](LICENSE).
