# Virtualization Guide — QEMU & Hypervisor Evaluation

This document outlines the virtualization strategy, hypervisor requirements, networking configuration, and evaluation of Oracle VirtualBox versus QEMU.

---

## 1. Virtualization Strategy Overview

The Track B IoT Gateway is designed to run in a virtual machine acting as an embedded hardware platform. The runtime environment must provide:
- Deterministic x86_64 CPU emulation or hardware acceleration.
- VirtIO block device emulation for the root filesystem (`/dev/vda`).
- VirtIO network interface (`eth0`) with DHCP and IP routing to the host.
- Serial console redirection (`ttyS0`) for headless execution and logging.

---

## 2. Hypervisor Comparison & Evaluation

| Evaluation Criteria | QEMU (Retained Runtime) | Oracle VirtualBox (Evaluated) |
|---|---|---|
| **Host Availability** | Installed & verified (`qemu-system-x86_64` 8.2.2) | Not installed on host or subsystem |
| **KVM Acceleration** | Supported directly via `/dev/kvm` | Not supported within WSL2 environment |
| **Direct Kernel Boot** | Native (`-kernel bzImage -drive rootfs.ext2`) | Requires GRUB bootloader or complex VDI conversion |
| **Cold Boot Duration** | **1.73 seconds** | 10–20 seconds (BIOS/EFI + bootloader stage) |
| **Headless Operation** | Native (`-nographic` over `ttyS0`) | Requires `VBoxHeadless` / GUI window |
| **Reproducibility** | Single shell script (`run-qemu.sh`) | Requires VM creation commands, XML configs |
| **Host Networking** | Built-in user net (`10.0.2.2` reaches host) | Requires NAT with port forwarding or Host-Only adapter |

### Architectural Decision
As directed in Master Prompt Sections 3 and 12, **QEMU is retained as the primary, authoritative demonstration platform**. VirtualBox is not present in the development environment, and nested virtualization inside WSL2 cannot run VirtualBox VMs without major structural destabilization. QEMU with KVM provides a clean, fast (1.73s boot), and fully reproducible runtime.

---

## 3. QEMU VM Configuration Specifications

- **Hypervisor:** QEMU emulator version 8.2.2 (Debian package).
- **Target Architecture:** x86_64.
- **Machine Model:** Standard PC (`i440FX + PIIX, 1996`).
- **Acceleration:** `-accel kvm -accel tcg` (Hardware KVM preferred, automatic TCG fallback).
- **RAM Allocation:** 256 MiB (Guest uses only 13 MiB; remainder remains free cache).
- **CPU Allocation:** 1 virtual core (Guest CPU utilization <0.1%).
- **Virtual Disk Layout:** VirtIO block driver (`if=virtio,format=raw`) attached to `rootfs.ext2`.
- **Boot Configuration:** Direct kernel boot via `-kernel bzImage -append "rootwait root=/dev/vda console=ttyS0 quiet"`.
- **Network Adapter:** VirtIO network PCI device (`virtio-net-pci`) attached to `netdev user,id=net0`.

---

## 4. Guest-to-Host Networking & MQTT Routing

QEMU's User-Mode Networking (SLIRP) creates a private virtual network for the guest:
- **Guest IP Address:** `10.0.2.15` (assigned automatically by Buildroot's `udhcpc` via `S40network`).
- **Default Gateway:** `10.0.2.2`.
- **Host Loopback Alias:** `10.0.2.2` transparently routes to the host machine's `127.0.0.1`.

### MQTT Port Access
- When the host runs Mosquitto on `0.0.0.0:1883`, the guest connects directly to `10.0.2.2:1883`.
- No manual port forwarding, bridges, or iptables rules are required.
- The init script `/etc/init.d/S99iot-gateway` exports `MQTT_BROKER_HOST=10.0.2.2` automatically.

---

## 5. Startup & Shutdown Operations

### Starting the VM:
```bash
./scripts/qemu/run-qemu.sh
```

### Exiting / Shutting Down:
1. **Graceful in-guest shutdown:**
   ```sh
   /etc/init.d/S99iot-gateway stop
   poweroff
   ```
2. **QEMU escape sequence:**
   Press `Ctrl+A`, release, then press `X`.
