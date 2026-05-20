# OBS Background Removal (Fedora / OBS 32)

Plugin de filtro para [OBS Studio](https://obsproject.com/) 28+ que quita el fondo de una fuente de vídeo usando segmentación neuronal (modelo MediaPipe vía ONNX Runtime).

**v2:** permite elegir una **imagen de fondo** directamente en el filtro, sin añadir otra fuente en la escena.

Probado con **OBS 32.1.1** instalado desde RPM en Fedora 43.

![CI](https://github.com/magicdrums/BackgroundRemoval/actions/workflows/ci.yml/badge.svg?branch=main)

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
2. Clic derecho en la fuente → **Filtros** → **+** → **Quitar fondo**.
3. En **Fondo**, elige:
   - **Transparente** — solo se quita el fondo (alpha en la silueta).
   - **Imagen personalizada** — selecciona un PNG/JPG/WebP; se estira al tamaño del vídeo.
4. Ajusta umbral, suavizado y hilos de CPU según tu equipo.

## Compilación manual

```bash
./scripts/download-model.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLINUX_PORTABLE=OFF
cmake --build build -j$(nproc)
sudo cmake --install build
```

## Pruebas unitarias

```bash
./scripts/download-model.sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
export OBS_BGREMOVAL_MODEL_PATH="$PWD/data/models/mediapipe.onnx"
ctest --test-dir build --output-on-failure
```

## Empaquetado RPM (Fedora)

```bash
sudo dnf install rpm-build rpmdevtools obs-studio-devel opencv-devel onnxruntime-devel gtest-devel
./scripts/build-rpm.sh
# RPM en ~/rpmbuild/RPMS/x86_64/
```

## CI (GitHub Actions)

El workflow [`.github/workflows/ci.yml`](.github/workflows/ci.yml) en Fedora 43:

1. Compila el plugin
2. Ejecuta pruebas unitarias (GTest + inferencia ONNX opcional)
3. Genera el RPM y lo publica como artefacto del workflow

## Créditos

Basado en la arquitectura del plugin [obs-backgroundremoval](https://github.com/obs-backgroundremoval/obs-backgroundremoval) (GPL-2.0). Modelo MediaPipe de su distribución de releases.

Parte del código y la documentación de este repositorio se desarrolló con ayuda de **Cursor Agent** (asistente IA). Si quieres reflejarlo en GitHub, puedes añadir al final del mensaje de commit (tras una línea en blanco):

```
Co-authored-by: Cursor <cursoragent@cursor.com>
```

## Licencia

GPL-2.0 — ver [LICENSE](LICENSE).
