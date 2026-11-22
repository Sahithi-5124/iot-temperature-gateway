# Requirements Traceability Matrix (RTM)

**Course / Project Title:** Application-Specific Linux Distribution for ML, IoT, CUDA and FPGA Platforms  
**Track:** Track B — IoT Temperature-Monitoring Gateway  
**Project Phase:** Phase 1 (Virtual Development Baseline) Verified; Phase 2 (Physical Hardware Integration) Pending  
**Last Updated:** 23 November 2026  
**Document Location:** `docs/report/REQUIREMENTS_TRACEABILITY_MATRIX.md`  

---

## 1. Traceability Matrix Structure
This matrix provides bidirectional traceability between every formal assignment requirement, the corresponding section in the Technical Report (`FINAL_PROJECT_REPORT.md`), the concrete implementation files in the repository, the verification test evidence, and the current compliance status.

---

## 2. Requirements Compliance and Verification Matrix

| Req ID | Assignment Requirement | Report Section | Implementation Evidence (Repository Path) | Verification Evidence & Method | Completion Status |
|---|---|---|---|---|---|
| **REQ-01** | **Target Scope & Track Selection:** Focus strictly on Track B (IoT Temperature Gateway). Exclude unrelated ML, CUDA, and FPGA stacks. | Sections 1.1, 5.1 | `buildroot/external/iot_gateway/configs/iot_gateway_x86_64_defconfig`<br>`application/Makefile` | Defconfig audit confirms zero CUDA, FPGA, or ML packages enabled. | **COMPLETE AND VERIFIED** |
| **REQ-02** | **Requirements Analysis:** Document CPU, memory, storage, latency, power, and environmental requirements. | Section 6.1 | `docs/report/FINAL_PROJECT_REPORT.md` (Table 6.1: Hardware Specification Matrix) | Empirical metrics collected from QEMU target (`free -m`, `stat`, `top`). | **COMPLETE AND VERIFIED** |
| **REQ-03** | **Stock Distro Unsuitability:** Justify why a general-purpose distribution (e.g. Ubuntu Server) is unsuitable. | Sections 4, 6.1, 16.2 | `docs/benchmarks.md`<br>`docs/report/FINAL_PROJECT_REPORT.md` (Section 16.2) | Quantitative comparison against stock Ubuntu 24.04 baseline (>2.8 GB vs 66.3 MB). | **COMPLETE AND VERIFIED** |
| **REQ-04** | **Build System Selection:** Compare at least two build systems (Buildroot, Yocto, Debian live-build, JetPack/PetaLinux). Justify choice. | Section 7.1, 7.2 | `docs/build-guide.md`<br>`docs/report/FINAL_PROJECT_REPORT.md` (Table 7.1) | Architectural comparison matrix; in-tree Buildroot 2026.08.2 implementation verified. | **COMPLETE AND VERIFIED** |
| **REQ-05** | **Custom Kernel Configuration:** Build a custom minimal kernel stripped of non-essential drivers. | Section 9.1, 9.2 | `buildroot/external/iot_gateway/board/x86_64/linux.config`<br>`iot_gateway_x86_64_defconfig` | Generated `bzImage` (6.34 MiB); graphics, sound, wireless verified disabled. | **COMPLETE AND VERIFIED** |
| **REQ-06** | **Sensor Interface & Device Tree Clarification:** Explain physical sensor interface (DHT11 single-wire protocol) and explain x86 ACPI vs ARM Device Tree. | Sections 6.2, 9.3 | `docs/report/FINAL_PROJECT_REPORT.md` (Sections 6.2 & 9.3)<br>`application/src/sensor/sensor.c` | Architectural justification: x86 uses ACPI/PCI; device tree is PENDING Phase 2 SBC confirmation. | **COMPLETE AND VERIFIED (PHASE 1)** |
| **REQ-07** | **Kernel Size & Boot Time Comparison:** Report kernel size and cold boot time before and after optimization. | Sections 9.4, 16.1, 16.2 | `docs/benchmarks.md`<br>`output/images/bzImage` | Measured 1.73s cold boot in QEMU; bzImage is 6.34 MiB vs stock vmlinuz 14.8 MiB. | **COMPLETE AND VERIFIED** |
| **REQ-08** | **Minimal Userland Selection:** Select minimal package set and justify all major inclusions and exclusions. | Section 10.1, 10.2 | `licenses/manifest.csv`<br>`buildroot/external/iot_gateway/configs/` | Buildroot legal manifest catalogs exactly 8 core packages; Python/systemd excluded. | **COMPLETE AND VERIFIED** |
| **REQ-09** | **Legal Compliance & Manifest:** Provide an open-source package manifest and license list. | Section 18 | `licenses/manifest.csv`<br>`docs/report/FINAL_PROJECT_REPORT.md` (Section 18) | Generated via `make legal-info`. Full CSV with versions, source sites, and licenses. | **COMPLETE AND VERIFIED** |
| **REQ-10** | **Sensor Data Acquisition & Tagging:** Acquire temperature telemetry; ensure all simulated readings are explicitly tagged. | Section 11.2 | `application/src/sensor/sensor.c`<br>`application/include/sensor.h` | `test_sensor` passes; JSON & CSV records explicitly output `"source": "SIMULATED"`. | **COMPLETE AND VERIFIED** |
| **REQ-11** | **Input Validation & Sanity Checks:** Validate readings against physical bounds ([-40°C, 85°C]) and reject NaN/Inf. | Section 11.3 | `application/src/processing/processor.c`<br>`application/tests/test_processor.c` | `test_processor` verifies boundary values and IEEE-754 NaN/Inf rejection. | **COMPLETE AND VERIFIED** |
| **REQ-12** | **Temperature Classification & Alerts:** Classify temperatures (Normal, Warning, Critical) and generate threshold alerts. | Section 11.3 | `application/src/processing/processor.c`<br>`application/src/alerts/alert.c` | `test_alert` verifies Warning at ≥35°C and Critical at ≥40°C. | **COMPLETE AND VERIFIED** |
| **REQ-13** | **Local Durable Disk Logging:** Store telemetry in CSV format locally with power-loss durability guarantees. | Section 11.4 | `application/src/logging/logger.c`<br>`application/tests/test_logger.c` | `test_logger` verifies `fflush()` and `fsync()`; persistent file on virtual disk. | **COMPLETE AND VERIFIED** |
| **REQ-14** | **MQTT Publishing & QoS 1:** Publish telemetry and alerts via MQTT over QoS 1 JSON payloads. | Section 12.1 | `application/src/mqtt/mqtt.c`<br>`application/tests/test_mqtt.c` | Live test captures telemetry on `iot/temperature/readings` and alerts on `alerts`. | **COMPLETE AND VERIFIED** |
| **REQ-15** | **MQTT Outage Resilience:** Handle network and broker outages with non-blocking reconnection. | Section 12.4 | `application/src/mqtt/mqtt.c`<br>`application/tests/test_mqtt_reconnect.c` | `test_mqtt_reconnect` verifies exponential backoff and continuous local logging. | **COMPLETE AND VERIFIED** |
| **REQ-16** | **Service Auto-Start on Boot:** Automatically initialize the gateway daemon during system boot. | Section 13.1 | `rootfs-overlay/etc/init.d/S99iot-gateway`<br>`application/src/main.c` | Confirmed `iot_gateway` daemon starts as PID 125 via SysV init during boot. | **COMPLETE AND VERIFIED** |
| **REQ-17** | **Security Hardening (At least 3 measures):** Implement and verify system-level hardening measures. | Section 13.2 | `rootfs-overlay/etc/init.d/S02security`<br>`application/Makefile`<br>`scripts/test-mqtt-auth.sh` | **5 measures verified:** `kptr_restrict=2`, `dmesg_restrict=1`, `tcp_syncookies=1`, permissions 0600, authenticated MQTT with ACLs. | **COMPLETE AND VERIFIED** |
| **REQ-18** | **Secure Credential Handling:** Protect runtime secrets; keep passwords out of Git and logs. | Section 12.2, 12.3 | `application/src/main.c`<br>`.gitignore`<br>`rootfs-overlay/etc/iot-gateway/mqtt.env.example` | Dynamic env loader; permissions 0600; `mqtt.env` ignored by Git. | **COMPLETE AND VERIFIED** |
| **REQ-19** | **Virtual Machine Execution:** Run the customized distribution reliably in a virtual machine (QEMU). | Section 14.3 | `scripts/qemu/run-qemu.sh`<br>`board/x86_64/post-image.sh` | QEMU 8.2.2 boots with KVM hardware acceleration in 1.73s. | **COMPLETE AND VERIFIED** |
| **REQ-20** | **40% Image-Size Reduction Target:** Achieve at least 40% image-size reduction vs stock distribution. | Section 16.2 | `docs/benchmarks.md`<br>`output/images/rootfs.ext2` | **97.8% rootfs reduction** vs stock Ubuntu Server minimal (60 MB vs 2,800 MB). | **COMPLETE AND VERIFIED** |
| **REQ-21** | **Reproducibility & Git History:** Clean repository, no secrets, clear documentation, four-member team structure. | Sections 1.2, 17 | `docs/TEAM_TASKS.md`<br>`docs/TEAM_SYNC_PROTOCOL.md`<br>`PROJECT_STATUS.md` | Clean Git working tree on `dev/build-system`; 4-member ownership matrix documented. | **COMPLETE AND VERIFIED** |
| **REQ-22** | **Physical Hardware Integration:** Confirm board, GPIO wiring, physical DHT11 driver, bench power measurement. | Section 6.1, 20.2 | `docs/report/FINAL_PROJECT_REPORT.md` (Section 20.2: Phase 2 Roadmap) | Hardware backend returns `SENSOR_STATUS_UNSUPPORTED` in Phase 1; physical board pending. | **PENDING (PHASE 2)** |
| **REQ-23** | **Hardware Secure Boot & Signed OTA:** Hardware root of trust and cryptographic dual-partition A/B updates. | Section 13.4, 21 | `docs/report/FINAL_PROJECT_REPORT.md` (Section 13.4) | Architectural specification documented; physical TPM/ROM signature chain pending. | **PENDING (PHASE 2)** |

---

## 3. Compliance Summary Statistics

- **Total Requirements Tracked:** 23
- **Fully Implemented & Verified in Phase 1:** 21 (91.3%)
- **Architecturally Specified & Pending Phase 2 Physical Hardware:** 2 (8.7%)
  - *REQ-22: Physical Sensor & SBC Integration*
  - *REQ-23: Hardware Secure Boot & Signed OTA Firmware Update Chain*
- **Requirements Unaccounted For:** 0 (0.0%)
