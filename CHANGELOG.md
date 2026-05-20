# Changelog

## [2.0.0] — 2026-05-20

### Añadido

- Filtro **Quitar fondo** para OBS Studio 28+ (probado en OBS 32.1.1, Fedora 43).
- Segmentación de persona con modelo **MediaPipe** (ONNX Runtime, CPU).
- Modo de fondo **transparente** (alpha en la silueta).
- Modo de fondo con **imagen personalizada** (PNG, JPG, WebP, BMP, TGA).
- Ajustes de umbral, contorno, suavizado de silueta y feather.
- Interfaz en español e inglés.
- Pruebas unitarias (GTest) e inferencia ONNX en CI.
- Empaquetado **RPM** para Fedora 43.
- CI en GitHub Actions (compilar, tests, RPM).

### Instalación

- **Desde RPM (Fedora):** descarga `obs-background-removal-2.0.0-*.fc43.x86_64.rpm` del release e instala con `sudo dnf install ./obs-background-removal-*.rpm`.
- **Desde fuente:** ver [README.md](README.md).

### Requisitos

- OBS Studio ≥ 28
- `onnxruntime`, OpenCV (incluidos como dependencias del RPM en Fedora)
