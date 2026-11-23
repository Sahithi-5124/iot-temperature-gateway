# Project Status — Track B: IoT Temperature-Monitoring Gateway

**Last updated:** 23 November 2026  
**Operating Context:** Four-Member Engineering Team (Ubuntu 22.04 LTS & WSL 2.6.3.0 dual workflow)  
**Repository Branch:** `dev/build-system`  
**Repository Remote:** `https://github.com/Sahithi-5124/iot-temperature-gateway.git`  
**Target Architecture:** x86_64  
**Runtime Platform:** QEMU 8.2.2 (with KVM Hardware Acceleration)  
**Project Scope:** Track B — Application-Specific Embedded Linux IoT Gateway (Phase 1 Virtual Simulation Baseline Verified; Phase 2 Physical Hardware Integration Pending)

---

## 1. Project Status Summary

The Track B IoT Temperature-Monitoring Gateway Linux distribution has completed **Phase 1 (Virtual Development & Simulation Baseline)** with all milestones verified:
1. Custom Buildroot Linux image builds deterministically.
2. Kernel 6.18.7 boots into BusyBox init in **1.73 seconds**.
3. C IoT Gateway application starts automatically on boot via `/etc/init.d/S99iot-gateway`.
4. Sensor engine operates in deterministic and realistic modes; all readings are tagged `SIMULATED`.
5. Temperature classification boundaries (Normal `<35°C`, Warning `35–39.9°C`, Critical `≥40°C`) and input rejection (`[-40°C, 85°C]`, NaN, Inf) operate accurately.
6. Local CSV logging flushes and synchronizes records to persistent virtual disk storage at `/root/temperature_log.csv` via `fsync()`.
7. MQTT publisher streams telemetry to `iot/temperature/readings` and alerts to `iot/temperature/alerts` over QoS 1 JSON payloads to host broker.
8. Background asynchronous MQTT reconnection recovers seamlessly across broker outages.
9. Five distinct security-hardening measures are active and verified inside the guest and broker.
10. Comprehensive benchmark metrics have been collected and documented.
11. Phase 2 physical hardware roadmap is fully specified and pending hardware allocation.

### 1.1 Team Ownership & Collaboration Matrix
- **Member 1 (Native Ubuntu 22.04):** Primary Buildroot configuration, minimal Linux 6.18.7 kernel, image build scripts, QEMU launcher.
- **Member 2 (WSL 2.6.3.0):** C gateway daemon, sensor abstraction, dual simulation modes, input validation, threshold classification, unit tests.
- **Member 3 (WSL 2.6.3.0):** MQTT networking module, durable CSV logger, SysV init scripts, broker security, ACLs, authentication tests.
- **Member 4 (WSL 2.6.3.0):** Requirements traceability, empirical benchmarks, documentation suite, gap analysis, evaluation demo script.

---

## 2. Technical Baseline & Verified Versions

| Subsystem | Verified Version | Configuration / Path |
|---|---|---|
| **Build System** | Buildroot 2026.08.2 | `buildroot/source/buildroot-2026.08.2` |
| **Defconfig** | `iot_gateway_x86_64_defconfig` | `buildroot/external/iot_gateway/configs/` |
| **Linux Kernel** | 6.18.7 (x86_64) | `buildroot/external/iot_gateway/board/x86_64/linux.config` |
| **C Library** | Glibc 2.42 | Host cross-compiler `gcc` 15.3.0 |
| **Init System** | BusyBox 1.38.0 | `etc/init.d/rcS`, `S02security`, `S99iot-gateway` |
| **Root Filesystem** | EXT2 (60.00 MiB) | `output/images/rootfs.ext2` |
| **Kernel Image** | bzImage (6.34 MiB) | `output/images/bzImage` |
| **MQTT Client** | libmosquitto 2.1.2 | Linked against `libmosquitto.so` |
| **Host Broker** | Mosquitto 2.0.18 | Host listener on port 1883 |
| **Hypervisor** | QEMU 8.2.2 (`-accel kvm -accel tcg`) | `scripts/qemu/run-qemu.sh` |

---

## 3. Verified Application Behavior & Policies

### 3.1 Temperature Classification Policy
- **Normal Range:** `< 35.00°C` → Logged as `NORMAL`, no alert generated.
- **Warning Threshold:** `35.00°C` to `< 40.00°C` → Logged as `WARNING`, Warning Alert generated.
- **Critical Threshold:** `≥ 40.00°C` → Logged as `CRITICAL`, Critical Alert generated.
- **Input Validation Range:** `[-40.00°C, 85.00°C]` inclusive. Values outside this range, non-finite values (`NaN`, `+Infinity`, `-Infinity`), or invalid sample flags are strictly rejected.

### 3.2 Sensor Simulation Engine
- Source identifier: Always explicitly stamped as `SIMULATED`.
- **Deterministic Mode:** Repeating 4-sample test cycle: `25.0°C` (Normal), `30.0°C` (Normal), `36.0°C` (Warning), `42.0°C` (Critical).
- **Realistic Mode:** Bounded random walk constrained between `20.0°C` and `42.0°C` with ±1.0°C maximum step deltas.
- Mode selection: Controlled via environment variable `IOT_SENSOR_MODE` (`deterministic` or `realistic`).

### 3.3 CSV Logging & Disk Persistence
- Target File: `/root/temperature_log.csv` (on `/dev/vda` root filesystem).
- Header format: `timestamp,temperature_c,source,status,alert_level`.
- Synchronization: Uses `fflush()` followed by kernel block commit via `fsync()`.
- Persistence test: Verified that records survive guest power-off and can be inspected in the backing `rootfs.ext2` image via `debugfs`.

### 3.4 MQTT Telemetry, Authentication & Outage Recovery
- Reading Topic: `iot/temperature/readings` (Write-only for `gateway_pub`, read for `monitor_sub`)
- Alert Topic: `iot/temperature/alerts` (Write-only for `gateway_pub`, read for `monitor_sub`)
- Authentication: Enforced (`allow_anonymous false`). Credentials configured via `mosquitto_username_pw_set()` from protected runtime configuration `/etc/iot-gateway/mqtt.env` (mode `0600`).
- Quality of Service: QoS 1 (At least once).
- Retain Flag: Disabled (`false`).
- Target Host in VM: `10.0.2.2:1883` (QEMU user-networking host gateway).
- Outage Resilience: Implemented via non-blocking `mosquitto_connect_async()` and background thread loop. The gateway starts normally when the broker is offline, continues local processing and CSV logging, and establishes/resumes MQTT publishing the moment the broker becomes available.

---

## 4. Test Evidence & Results

### 4.1 Unit Test Suite (`application/tests/`)
All tests run with `-Wall -Wextra -Wpedantic` with zero warnings:

| Test Binary | Target Module | Scope | Result |
|---|---|---|---|
| `test_sensor` | `sensor.c` | Deterministic cycle, realistic bounds, source tag, invalid inputs | **PASS** |
| `test_processor` | `processor.c` | Normal, Warning, Critical boundaries, range limits, NaN/Inf | **PASS** |
| `test_alert` | `alert.c` | Suppression of Normal, Warning/Critical event generation | **PASS** |
| `test_logger` | `logger.c` | Header creation, append mode, fsync, field parsing | **PASS** |
| `test_mqtt` | `mqtt.c` | Telemetry & alert payload schema, topic separation, error checks | **PASS** |
| `test_mqtt_reconnect`| `mqtt.c` | Async non-blocking startup & reconnection monitoring | **PASS** |

### 4.2 Guest Integration & Boot Verification
Verified via automated serial console interaction inside the booted QEMU guest:
- Kernel Boot: **PASS** (Linux 6.18.7 loaded, devices enumerated).
- Sysctl Hardening: **PASS** (`dmesg_restrict=1`, `kptr_restrict=2`, `tcp_syncookies=1`).
- Shadow Permissions: **PASS** (`ls -l /etc/shadow` = `-rw------- 1 root root`).
- Network Setup: **PASS** (Default route established via DHCP on `eth0`).
- Process Startup: **PASS** (`iot_gateway` daemon running as PID 125).
- Output Redirection: **PASS** (`/var/log/iot_gateway.log` actively populated).
- CSV File Persistence: **PASS** (`/root/temperature_log.csv` verified on virtual disk).
- Daemon Shutdown: **PASS** (`/etc/init.d/S99iot-gateway stop` cleanly stops PID 125).

---

## 5. Measured Benchmark Results

Collected from the booted distribution in QEMU (AMD64, 256M VM RAM, KVM acceleration):

| Metric | Measured Value | Methodology |
|---|---|---|
| **Cold Boot Duration** | **1.73 seconds** | Wall-clock timer from QEMU invocation to login prompt |
| **Kernel Size (`bzImage`)** | **6,652,928 bytes** (6.34 MiB) | `stat -c %s` on build artifact |
| **Rootfs Size (`rootfs.ext2`)**| **62,914,560 bytes** (60.00 MiB) | `stat -c %s` on build artifact |
| **Combined Image Artifacts** | **69,567,488 bytes** (66.34 MiB) | Sum of kernel and rootfs |
| **Guest Total RAM** | **235 MiB** (240,556 kB) | `free -m` inside guest |
| **Guest Used RAM** | **13 MiB** | `free -m` inside guest |
| **Guest Available RAM** | **216 MiB** (220,764 kB) | `/proc/meminfo` |
| **Gateway Process RSS** | **6,688 kB** (~6.5 MiB) | `VmRSS` in `/proc/<pid>/status` |
| **Gateway Process VmSize** | **84,836 kB** | `VmSize` in `/proc/<pid>/status` |
| **CPU (Gateway Operating)** | **0.0% CPU** (91% idle, 8% io) | 10s top sample (5 iterations x 2s) |
| **CPU (Idle / Gateway Stopped)**| **0.0% CPU** (100% idle, 0% io) | 10s top sample (5 iterations x 2s) |

---

## 6. Hypervisor & Virtualization Strategy

1. **Primary Platform: QEMU (Retained and Fully Verified)**
   - Installed Version: QEMU emulator 8.2.2.
   - Hardware Acceleration: KVM enabled (`/dev/kvm`).
   - Launch Command: `scripts/qemu/run-qemu.sh` (or `start-qemu.sh` in images directory).
   - Performance: Sub-2-second boot time, direct serial console passthrough, flawless host network integration via user networking (`10.0.2.2`).

2. **Secondary Platform: Oracle VirtualBox (Evaluated)**
   - VirtualBox is not installed on the development host or within the WSL2 environment.
   - Nested virtualization of VirtualBox inside WSL2 is not supported.
   - Technical decision: In accordance with Project Instructions (Sections 3 & 12), QEMU is retained as the primary runtime environment to preserve stability and reproducibility without introducing fragile hypervisor conversions.

---

## 7. Security Hardening Verification

1. **Kernel Address Space Leak Protection:**
   - Setting: `kernel.kptr_restrict = 2`
   - Purpose: Hides kernel symbol addresses from unprivileged users, defeating kernel exploit payload alignment.
   - Verification: Confirmed `2` via guest `sysctl kernel.kptr_restrict`.
2. **Kernel Ring Buffer Access Control:**
   - Setting: `kernel.dmesg_restrict = 1`
   - Purpose: Prevents non-root users from reading `dmesg`, shielding device details and kernel addresses.
   - Verification: Confirmed `1` via guest `sysctl kernel.dmesg_restrict`.
3. **TCP SYN Flood Defense:**
   - Setting: `net.ipv4.tcp_syncookies = 1` & `CONFIG_SYN_COOKIES=y`
   - Purpose: Protects the gateway against TCP connection exhaustion DoS attacks.
   - Verification: Confirmed `1` via guest `sysctl net.ipv4.tcp_syncookies`.
4. **Credential Storage Hardening:**
   - Setting: Mode `0600` on `/etc/shadow`, root password hashed with SHA-512.
   - Verification: Confirmed `-rw------- 1 root root` via guest `ls -l /etc/shadow`.
5. **Memory Protection:**
   - Kernel compile options: `CONFIG_STRICT_KERNEL_RWX=y` and `CONFIG_STRICT_MODULE_RWX=y` enforce memory permissions so code is non-writable and data is non-executable.
6. **Gateway Credential Storage Hardening:**
   - Setting: Protected configuration `/etc/iot-gateway/mqtt.env` with mode `0600` (`-rw------- 1 root root`).
   - Purpose: Prevents hardcoded credentials in source code and unauthorized reading of gateway secrets.
7. **Broker Password Database Security:**
   - Setting: `/etc/mosquitto/passwd` owned by `mosquitto:mosquitto` with mode `0600` (`-rw-------`).
   - Purpose: Restricts password database access exclusively to the Mosquitto broker daemon and system root.
8. **Topic ACL Least Privilege:**
   - Setting: Strict ACLs in `/etc/mosquitto/acl.conf`.
   - Purpose: Gateway publisher account (`gateway_pub`) is write-only for `iot/temperature/readings` and `iot/temperature/alerts`; monitoring subscriber (`monitor_sub`) is read-only for `iot/temperature/#`; anonymous access is rejected.

---

## 8. Known System Limitations

1. **No Physical Hardware:** All telemetry is software-simulated. There is no physical board or sensor integration.
2. **Volatile `/var/log`:** Standard Buildroot symlinks `/var/log` to `/tmp` (`tmpfs`). Persistent telemetry is saved at `/root/temperature_log.csv`.
3. **Stock Comparison:** Stock Buildroot configuration (`qemu_x86_64_defconfig`) was cataloged, but a separate stock image was not built to prevent disk exhaustion. Custom metrics are measured and documented directly.
4. **Offline Message Spooling:** libmosquitto automatically reconnects with backoff, but does not provide an on-disk transaction queue for telemetry generated while the broker is offline.

---

## 9. Documentation Index

The complete documentation suite for Track B is located in the [`docs/`](docs/) directory:

### Core Technical Reports (`docs/report/`)
- [docs/report/FINAL_PROJECT_REPORT.md](docs/report/FINAL_PROJECT_REPORT.md) — Comprehensive 24-section audited academic final project report
- [docs/report/REQUIREMENTS_TRACEABILITY_MATRIX.md](docs/report/REQUIREMENTS_TRACEABILITY_MATRIX.md) — Traceability matrix mapping requirements to evidence and status
- [docs/report/REPORT_EVIDENCE_AUDIT.md](docs/report/REPORT_EVIDENCE_AUDIT.md) — Systematic audit verifying all report claims against repository files
- [docs/report/REPORT_GAPS_AND_PENDING_WORK.md](docs/report/REPORT_GAPS_AND_PENDING_WORK.md) — Transparent gap analysis and Phase 2 physical hardware roadmap

### Engineering Guides (`docs/`)
- [docs/architecture.md](docs/architecture.md) — System architecture, module boundaries, data pipeline, and system diagram
- [docs/build-guide.md](docs/build-guide.md) — Environment prerequisites, Buildroot configuration, and image generation
- [docs/virtualization-guide.md](docs/virtualization-guide.md) — Hypervisor evaluation, QEMU configuration, networking, and VirtualBox analysis
- [docs/application-guide.md](docs/application-guide.md) — C gateway implementation, sensor simulation modes, validation, and threshold policies
- [docs/mqtt-guide.md](docs/mqtt-guide.md) — MQTT topics, JSON schemas, QoS, reconnection backoff, and host subscriber commands
- [docs/security.md](docs/security.md) — Kernel memory protections, sysctl runtime hardening, credential security, and live verification
- [docs/testing.md](docs/testing.md) — Multi-tier test suite results, unit tests, integration tests, and persistence tests
- [docs/benchmarks.md](docs/benchmarks.md) — Measured performance metrics (boot time, RAM, storage footprint, CPU utilization)
- [docs/troubleshooting.md](docs/troubleshooting.md) — Common build, networking, buffering, and virtualization pitfalls and resolutions
- [docs/demonstration-guide.md](docs/demonstration-guide.md) — Step-by-step reproducible evaluation walkthrough for examiners

