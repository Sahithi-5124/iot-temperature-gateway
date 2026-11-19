#!/bin/sh

set -eu

LAUNCHER="${BINARIES_DIR}/start-qemu.sh"

cat > "${LAUNCHER}" <<'LAUNCHER_EOF'
#!/bin/sh

set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

exec qemu-system-x86_64 \
    -M pc \
    -kernel "${SCRIPT_DIR}/bzImage" \
    -drive "file=${SCRIPT_DIR}/rootfs.ext2,if=virtio,format=raw" \
    -append "rootwait root=/dev/vda console=ttyS0" \
    -nographic \
    -netdev user,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    "$@"
LAUNCHER_EOF

chmod +x "${LAUNCHER}"
