# IoT Temperature-Monitoring Gateway Linux Distribution
## Application-Specific Embedded Linux (Track B)

An application-specific, lightweight, secure embedded Linux distribution built from source using Buildroot for an IoT Temperature-Monitoring Gateway.

---

## 1. Project Overview & Scope

- **Project Title:** Application-Specific Linux Distribution for ML, IoT, CUDA and FPGA Platforms
- **Selected Track:** Track B — IoT Temperature-Monitoring Gateway ONLY
- **Operating Context:** Four-member engineering team (Ubuntu 22.04 LTS reference host + WSL 2.6.3.0 parallel development).
- **Project Phasing:**
  - **Phase 1 (Virtual Development Baseline):** Built and verified in QEMU on x86_64. Sensor readings are generated through an internal simulation engine and strictly labeled `SIMULATED`.
  - **Phase 2 (Physical Hardware Integration):** Architecturally specified and pending physical single-board computer and digital sensor hardware allocation.

The system demonstrates a minimal, hardened Linux distribution tailored specifically to run a single daemon: the C IoT Temperature Gateway. It boots in under 2 seconds, acquires simulated temperature samples, classifies thermal conditions, generates threshold alerts, persists data locally in CSV format, and publishes telemetry and alerts over MQTT with automatic network outage recovery.

---

## 2. Key Features

1. **Minimal Footprint:** Total combined boot artifacts of ~66.3 MiB (6.34 MiB Linux kernel, 60.0 MiB EXT2 root filesystem).
2. **Instant Boot:** Cold boot to running gateway daemon in **1.73 seconds** using QEMU with KVM hardware acceleration.
3. **Low Resource Consumption:** Consumes only **13 MiB of RAM**; gateway daemon uses **6.6 MiB RSS** and negligible (<0.1%) CPU.
4. **C Modular Architecture:** Distinct modules for sensor simulation, input validation, threshold classification, alert event generation, CSV persistence, and MQTT networking.
5. **Dual Simulation Modes:**
   - `DETERMINISTIC`: Predictable repeating sequence (25°C, 30°C, 36°C, 42°C) for repeatable threshold and boundary verification.
   - `REALISTIC`: Bounded random step walk between 20°C and 42°C with ±1°C step limits.
   - Every reading explicitly tagged with `source: SIMULATED`.
6. **Robust Input Validation & Policy Classification:**
   - Normal: `< 35.0°C`
   - Warning: `35.0°C` to `< 40.0°C` (triggers Warning Alert)
   - Critical: `≥ 40.0°C` (triggers Critical Alert)
   - Out-of-range (`< -40.0°C` or `> 85.0°C`), `NaN`, and `±Inf` are strictly rejected.
7. **Local CSV Logging & Storage Persistence:**
   - Standardized CSV schema: `timestamp,temperature_c,source,status,alert_level`.
   - Flushed and synchronized to disk (`fsync`) to survive abrupt power cuts and VM reboots.
8. **Resilient MQTT Communication:**
   - Non-blocking startup (`mosquitto_connect_async`) allows the gateway to boot and log locally even when the broker is temporarily down.
   - Automatic background reconnection with exponential backoff (2s to 30s).
   - Telemetry topic: `iot/temperature/readings` (JSON payload, QoS 1).
   - Alert topic: `iot/temperature/alerts` (JSON payload, QoS 1).
9. **Automatic Bootstrapping & Graceful Shutdown:**
   - BusyBox init service (`/etc/init.d/S99iot-gateway`) waits for network route and launches daemon.
   - PID tracking at `/run/iot_gateway.pid`.
   - Clean signal handling (`SIGINT` / `SIGTERM`) flushes buffers and closes MQTT sockets before system poweroff.
10. **Multi-layer Security Hardening:**
    - Kernel security sysctls applied at boot (`/etc/init.d/S02security`): `kernel.dmesg_restrict=1`, `kernel.kptr_restrict=2`, `net.ipv4.tcp_syncookies=1`.
    - Protected `/etc/shadow` file permissions (`0600`, `root:root`).
    - Password protection with SHA-512 cryptographic hashing.
    - Kernel compile-time hardening (`CONFIG_SYN_COOKIES=y`, `CONFIG_STRICT_KERNEL_RWX=y`).

---

## 3. Technology Stack & Software Baseline

| Component | Verified Version | Purpose |
|---|---|---|
| **Host Environment** | Ubuntu 24.04 LTS (WSL2 / Linux) | Cross-compilation & hypervisor host |
| **Build System** | Buildroot 2026.08.2 | Toolchain, kernel, and rootfs generation |
| **Linux Kernel** | 6.18.7 (x86_64) | Application-specific minimal kernel |
| **C Library** | Glibc 2.42 | Core runtime library |
| **Init System** | BusyBox 1.38.0 | Minimal init, ash shell, and basic utilities |
| **Root Filesystem** | EXT2 (60 MiB) | Root virtual disk layout |
| **Application Language**| C (C11 standard) | Gateway core logic (`gcc` 15.3.0) |
| **MQTT Client** | libmosquitto 2.1.2 | Async network publishing & reconnection |
| **Host Broker** | Mosquitto 2.0.18 | Host-side MQTT broker |
| **Virtual Platform** | QEMU 8.2.2 (KVM accelerated) | Primary runtime and demonstration VM |

---

## 4. Repository Structure

```text
.
├── application/                # C gateway source code, headers, Makefile & tests
│   ├── include/                # Header files (sensor.h, processor.h, alert.h, logger.h, mqtt.h)
│   ├── src/                    # Implementation (main.c, sensor, processing, alerts, logging, mqtt)
│   ├── tests/                  # Unit and integration test suites
│   └── Makefile                # Application and test build targets
├── baseline/                   # Stock Buildroot baseline comparison configuration
│   └── stock-config/           # Baseline .config (qemu_x86_64_defconfig)
├── buildroot/                  # Buildroot integration
│   ├── external/iot_gateway/   # Buildroot BR2_EXTERNAL tree for IoT Gateway
│   │   ├── board/x86_64/       # Kernel config, post-image script, rootfs overlay
│   │   │   ├── linux.config    # Minimal custom kernel configuration
│   │   │   ├── post-image.sh   # QEMU launcher generation script
│   │   │   └── rootfs-overlay/ # Custom init scripts (S02security, S99iot-gateway)
│   │   ├── configs/            # iot_gateway_x86_64_defconfig
│   │   └── package/iot-gateway/# Buildroot package rules (iot-gateway.mk)
│   └── source/                 # Extracted Buildroot source tree & pre-built artifacts
├── docs/                       # Technical documentation and evaluation reports
│   ├── architecture.md         # Detailed distribution & application architecture
│   ├── build-guide.md          # Step-by-step image compilation guide
│   ├── virtualization-guide.md # Hypervisor strategy & QEMU / VirtualBox analysis
│   ├── application-guide.md    # C gateway logic, sensors, and threshold policies
│   ├── mqtt-guide.md           # MQTT topics, payloads, outage resilience & broker setup
│   ├── security.md             # Hardening measures, sysctl settings & limitations
│   ├── testing.md              # Unit, integration, and reboot test verification
│   ├── benchmarks.md           # Performance, RAM, CPU, boot time & size metrics
│   ├── demonstration-guide.md  # Step-by-step reproducible live evaluation script
│   ├── troubleshooting.md      # Troubleshooting build, network, and QEMU issues
│   └── report/                 # Formal academic technical reports & compliance audits
│       ├── FINAL_PROJECT_REPORT.md           # 24-section audited final project technical report
│       ├── REQUIREMENTS_TRACEABILITY_MATRIX.md # Matrix mapping requirements to evidence
│       ├── REPORT_EVIDENCE_AUDIT.md          # Verification audit of all report claims
│       └── REPORT_GAPS_AND_PENDING_WORK.md   # Transparent gap analysis and Phase 2 roadmap
├── licenses/                   # Open-source package licenses and manifests
│   └── manifest.csv            # Buildroot legal-info compliance manifest
├── scripts/                    # Automation and verification scripts
│   ├── audit-checklist.sh      # 8-section project submission and integrity audit
│   ├── build/build-image.sh    # Builds Buildroot custom Linux image
│   ├── qemu/run-qemu.sh        # Launches custom image in QEMU with KVM/TCG
│   ├── test-mqtt-auth.sh       # Automated test for MQTT authentication and ACLs
│   └── test-qemu-e2e.sh        # Automated end-to-end QEMU boot and telemetry test
└── PROJECT_STATUS.md           # Authoritative project status and verification log
```

---

## 5. Quick Start & Execution

### 5.1 Verification Scripts & Automated Audits

```bash
# 1. Run the comprehensive submission checklist audit (19/19 checks)
bash scripts/audit-checklist.sh

# 2. Test MQTT authentication, ACL enforcement, and password permissions
bash scripts/test-mqtt-auth.sh

# 3. Test automated QEMU boot and end-to-end telemetry streaming
bash scripts/test-qemu-e2e.sh
```

### 5.2 Compiling and Testing the C Gateway (Host)

```bash
# Navigate to application directory
cd application

# Run offline unit test suite (sensor, processor, alert, logger, mqtt)
make test

# Build target binary with compiler hardening flags
make all
```

### 5.3 Building the Custom Linux Image

```bash
# Execute build automation script
./scripts/build/build-image.sh
```
*Artifacts produced:*
- `buildroot/source/buildroot-2026.08.2/output/images/bzImage` (Linux kernel)
- `buildroot/source/buildroot-2026.08.2/output/images/rootfs.ext2` (Root filesystem)

### 5.4 Launching the Host MQTT Broker & Subscriber

The host Mosquitto broker enforces client authentication (`allow_anonymous false`):
```bash
# Subscribe to all gateway telemetry and alerts using the monitoring account
mosquitto_sub -h localhost -p 1883 -u monitor_sub -P mon_sub_47c2d91a03ef -t 'iot/temperature/#' -v
```

### 5.5 Booting the System in QEMU

```bash
# Launch the VM with KVM acceleration
./scripts/qemu/run-qemu.sh
```
- The gateway starts automatically on boot.
- Sensor readings and alerts immediately appear on the host subscriber.
- Log in as `root` (no password or configure hash) to inspect runtime state:
  ```sh
  ps | grep iot_gateway
  cat /var/log/iot_gateway.log
  cat /root/temperature_log.csv
  ```
- To gracefully shut down the VM, type:
  ```sh
  /etc/init.d/S99iot-gateway stop
  poweroff
  ```
  *(Or exit QEMU via `Ctrl+A` then `X`)*.

---

## 6. Documented System Limitations

1. **Hardware Scope:** Software simulation only. No physical GPIO, I2C, or SPI sensor backend is attached or claimed.
2. **Virtualization:** Primary target is QEMU with KVM acceleration. Oracle VirtualBox is not supported inside the host WSL2 environment.
3. **Storage Persistence:** `/var/log` is backed by `tmpfs` (RAM). Persistent records are maintained directly on the root virtual disk at `/root/temperature_log.csv`.
4. **MQTT Message Queuing:** The client handles connection outages with exponential backoff, but does not implement on-disk transactional queuing for messages generated while offline.
5. **Stock Baseline Comparison:** Baseline configuration (`baseline/stock-config/.config`) exists from `qemu_x86_64_defconfig`, but runtime comparison was not built to prevent multi-hour host storage thrashing. Exact measured custom metrics are provided in [docs/benchmarks.md](file:///docs/benchmarks.md).
