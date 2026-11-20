#!/usr/bin/env bash

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    cat <<'HELP'
Usage: ./scripts/build/build-image.sh

Builds the IoT Temperature Gateway image using the existing
Buildroot configuration and output directory.

Options:
  -h, --help    Display this help and exit

Note: Building can require substantial disk space.
HELP
    exit 0
fi

set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

BR_VERSION="2026.08.2"
BR_DIR="$ROOT_DIR/buildroot/source/buildroot-$BR_VERSION"
SOURCE_DIR="$ROOT_DIR/buildroot/source"
ARCHIVE="$SOURCE_DIR/buildroot-$BR_VERSION.tar.xz"
OUTPUT_DIR="$BR_DIR/output"
EXTERNAL_DIR="$ROOT_DIR/buildroot/external/iot_gateway"

EXPECTED_SHA256="bc9497c8e3a734c277e541e2f7fcf9a6275dc92f75bd2f753b05981c54be8705"

echo "========================================"
echo " IoT Temperature Gateway Build"
echo " Buildroot: $BR_VERSION"
echo "========================================"

# Check required project files.
if [[ ! -d "$EXTERNAL_DIR" ]]; then
    echo "ERROR: Buildroot external tree not found:"
    echo "$EXTERNAL_DIR"
    exit 1
fi

mkdir -p "$SOURCE_DIR"

# Obtain Buildroot source if it is not already extracted.
if [[ ! -f "$BR_DIR/Makefile" ]]; then
    echo "Buildroot source is not extracted."

    if [[ ! -f "$ARCHIVE" ]]; then
        echo "Downloading Buildroot $BR_VERSION..."
        curl -fL --retry 5 --retry-all-errors \
            "https://buildroot.org/downloads/buildroot-$BR_VERSION.tar.xz" \
            -o "$ARCHIVE"
    fi

    echo "Verifying Buildroot archive..."
    echo "$EXPECTED_SHA256  $ARCHIVE" | sha256sum -c -
    xz -t "$ARCHIVE"

    echo "Extracting Buildroot source..."
    tar -xf "$ARCHIVE" -C "$SOURCE_DIR"
fi

if [[ ! -f "$BR_DIR/Makefile" ]]; then
    echo "ERROR: Buildroot source extraction failed."
    exit 1
fi

# Preserve an existing configuration.
if [[ ! -f "$BR_DIR/.config" ]]; then
    echo "No Buildroot configuration found."
    echo "Loading the project defconfig..."

    make -C "$BR_DIR" \
        O="$OUTPUT_DIR" \
        BR2_EXTERNAL="$EXTERNAL_DIR" \
        iot_gateway_x86_64_defconfig
else
    echo "Existing Buildroot .config found."
    echo "Preserving the current configuration."
fi

# Avoid unexpectedly starting a large build with insufficient disk space.
if [[ ! -f "$OUTPUT_DIR/images/bzImage" ||
      ! -f "$OUTPUT_DIR/images/rootfs.ext2" ]]; then

    AVAILABLE_KB="$(df -Pk "$ROOT_DIR" | awk 'NR==2 {print $4}')"
    REQUIRED_KB=$((25 * 1024 * 1024))

    if (( AVAILABLE_KB < REQUIRED_KB )); then
        echo "ERROR: Less than 25 GB of free space."
        echo "Free space before starting a full image build."
        exit 1
    fi
fi

echo "Starting Buildroot build..."
make -C "$BR_DIR" \
    O="$OUTPUT_DIR" \
    BR2_EXTERNAL="$EXTERNAL_DIR" \
    -j"$(nproc)"

echo
echo "Checking generated images..."

test -s "$OUTPUT_DIR/images/bzImage"
test -s "$OUTPUT_DIR/images/rootfs.ext2"

echo "SUCCESS: Build completed."
ls -lh "$OUTPUT_DIR/images/bzImage" \
       "$OUTPUT_DIR/images/rootfs.ext2"
