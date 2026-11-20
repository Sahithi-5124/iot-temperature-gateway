#!/usr/bin/env bash

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    cat <<'HELP'
Usage: ./scripts/qemu/run-qemu.sh [QEMU_OPTIONS...]

Starts the IoT Temperature Gateway image in QEMU.

Options:
  -h, --help    Display this help and exit

Additional arguments are passed to QEMU.
Exit the guest with Ctrl+A followed by X.
HELP
    exit 0
fi

set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMAGE_DIR="$ROOT_DIR/buildroot/source/buildroot-2026.08.2/output/images"

KERNEL="$IMAGE_DIR/bzImage"
ROOTFS="$IMAGE_DIR/rootfs.ext2"

if [[ ! -s "$KERNEL" || ! -s "$ROOTFS" ]]; then
    echo "ERROR: Required QEMU images are missing."
    echo "Run scripts/build/build-image.sh first."
    exit 1
fi

if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "ERROR: qemu-system-x86_64 is not installed."
    exit 1
fi

echo "Starting IoT Gateway in QEMU..."
echo "Exit QEMU with Ctrl+A followed by X."

exec qemu-system-x86_64 \
    -M pc \
    -kernel "$KERNEL" \
    -drive "file=$ROOTFS,if=virtio,format=raw" \
    -append "rootwait root=/dev/vda console=ttyS0" \
    -nographic \
    -netdev user,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    "$@"
