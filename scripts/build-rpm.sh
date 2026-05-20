#!/usr/bin/env bash
# Genera el tarball fuente y construye el RPM (Fedora).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="$(sed -n 's/^  VERSION //p' "$ROOT/CMakeLists.txt" | head -1)"
if [[ -z "$VERSION" ]]; then
  echo "No se pudo leer VERSION desde CMakeLists.txt" >&2
  exit 1
fi
NAME="obs-background-removal"
ARCH="$(uname -m)"

: "${RPM_TOPDIR:=$HOME/rpmbuild}"

echo "==> Versión: $VERSION"
echo "==> RPM_TOPDIR: $RPM_TOPDIR"

mkdir -p "$RPM_TOPDIR/SOURCES" "$RPM_TOPDIR/SRPMS" "$RPM_TOPDIR/RPMS" \
         "$RPM_TOPDIR/BUILD" "$RPM_TOPDIR/BUILDROOT" "$RPM_TOPDIR/SPECS"

TARBALL="$RPM_TOPDIR/SOURCES/${NAME}-${VERSION}.tar.gz"
STAGING="$(mktemp -d)"
trap 'rm -rf "$STAGING"' EXIT

DIR="$STAGING/BackgroundRemoval-${VERSION}"
mkdir -p "$DIR"

tar -C "$ROOT" \
    --exclude='./build' \
    --exclude='./.git' \
    --exclude='./data/models/*.onnx' \
    -cf - . | tar -C "$DIR" -xf -

tar -C "$STAGING" -czf "$TARBALL" "BackgroundRemoval-${VERSION}"

cp "$ROOT/packaging/${NAME}.spec" "$RPM_TOPDIR/SPECS/"

echo "==> Construyendo RPM..."
rpmbuild -ba "$RPM_TOPDIR/SPECS/${NAME}.spec"

echo ""
echo "RPM generado:"
find "$RPM_TOPDIR/RPMS/$ARCH" -name "${NAME}-${VERSION}*.rpm" -print
