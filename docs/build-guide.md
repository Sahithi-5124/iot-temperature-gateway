# Build Guide — IoT Temperature Gateway Linux Distribution

This guide details how to build the custom Linux distribution from source using Buildroot.

---

## 1. Prerequisites & Environment

The build system requires a Linux environment (such as Ubuntu 22.04 / 24.04 or WSL2) with standard cross-compilation tools installed.

Required packages:
```bash
sudo apt update
sudo apt install -y build-essential libncurses-dev rsync cpio unzip bc \
                    python3 python3-pip wget curl git libmosquitto-dev
```

### Disk Space Recommendation
- A full Buildroot compilation with toolchain, kernel, and packages requires approximately **15–20 GB of free disk space**.
- Ensure the build directory resides on an ext4/Linux partition rather than a mounted network share or Windows filesystem to avoid permission and symlink incompatibilities.

---

## 2. Source Tree Layout

- **Buildroot Version:** 2026.08.2
- **External Tree (`BR2_EXTERNAL`):** `buildroot/external/iot_gateway/`
  - Defines the external tree name `IOT_GATEWAY`.
  - Points to custom package `package/iot-gateway/`.
  - Points to rootfs overlay `board/x86_64/rootfs-overlay/`.
  - Points to minimal kernel config `board/x86_64/linux.config`.
  - Points to post-image script `board/x86_64/post-image.sh`.
- **Project Defconfig:** `buildroot/external/iot_gateway/configs/iot_gateway_x86_64_defconfig`.

---

## 3. Automated Build Procedure

The repository provides an automated build script:
```bash
./scripts/build/build-image.sh
```

### What the script executes:
1. Validates that the external tree and required project files exist.
2. Sanitizes the `PATH` environment variable to ensure no spaces from Windows/host paths interfere with Buildroot.
3. Checks available disk space.
4. Loads `iot_gateway_x86_64_defconfig` if `.config` is missing.
5. Invokes Buildroot make with `-j$(nproc)` parallel compilation.
6. Runs `post-image.sh` to generate the QEMU launcher script.
7. Verifies that `bzImage` and `rootfs.ext2` were created successfully.

---

## 4. Manual / Step-by-Step Build Commands

If you prefer to run make commands manually:

```bash
# 1. Set clean PATH
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# 2. Define directory variables
BR_DIR="$(pwd)/buildroot/source/buildroot-2026.08.2"
EXT_DIR="$(pwd)/buildroot/external/iot_gateway"
OUT_DIR="$BR_DIR/output"

# 3. Apply the project defconfig
make -C "$BR_DIR" \
    O="$OUT_DIR" \
    BR2_EXTERNAL="$EXT_DIR" \
    iot_gateway_x86_64_defconfig

# 4. Build the complete system image
make -C "$BR_DIR" \
    O="$OUT_DIR" \
    BR2_EXTERNAL="$EXT_DIR" \
    -j"$(nproc)"
```

---

## 5. Build Artifacts

Upon completion, all target files are placed in:
`buildroot/source/buildroot-2026.08.2/output/images/`

- `bzImage`: Bootable x86_64 Linux kernel 6.18.7 (~6.34 MiB).
- `rootfs.ext2`: 60 MiB EXT2 filesystem image containing BusyBox, libmosquitto, init scripts, and `/usr/bin/iot_gateway`.
- `start-qemu.sh`: Generated helper script for booting the kernel and rootfs in QEMU.
