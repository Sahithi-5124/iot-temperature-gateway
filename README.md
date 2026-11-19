# IoT Temperature-Monitoring Gateway
## Application-Specific Linux Distribution — Track B

## 1. Project Overview

This project aims to build a lightweight, application-specific Linux
distribution for an IoT temperature-monitoring gateway.

The system will collect temperature readings, validate and process them,
detect threshold violations, generate alerts, log readings, and support
MQTT communication.

## 2. Development Strategy

### Phase 1 — Before Physical Hardware Arrives

Development will use laptops, Linux development environments, QEMU, and
simulated sensor readings.

All simulated readings must be clearly identified as SIMULATED.

### Phase 2 — After Physical Hardware Arrives

The exact board and sensor will be identified before implementing
hardware-specific configuration, wiring, drivers, and deployment.

Physical measurements must be identified as HARDWARE.

## 3. Current Technical Direction

| Component | Current direction | Status |
|---|---|---|
| Build system | Buildroot | Provisional |
| Primary environment | Native Ubuntu 22.04 | Agreed |
| Parallel environment | Windows + WSL 2.6.3.0 | Agreed |
| Virtual target | x86-64 QEMU | Provisional |
| Physical board | To be identified | Pending |
| Application language | C | Agreed |
| Sensor | DHT11 preferred | Pending confirmation |
| Communication | MQTT | Agreed |
| MQTT client library | To be selected | Pending |
| Init system | BusyBox init candidate | Provisional |

Exact software versions will be recorded after inspection and selection.

## 4. Repository Structure

- `buildroot/` — Buildroot configuration and customizations.
- `kernel/` — Kernel configuration and patches.
- `application/` — C application source code and headers.
- `rootfs-overlay/` — Custom root filesystem files.
- `configs/` — Application configuration.
- `scripts/` — Build, QEMU, test, and benchmark scripts.
- `tests/` — Unit and integration tests.
- `benchmarks/` — Benchmark data and results.
- `docs/` — Project documentation and report.
- `licenses/` — Third-party license information.
- `PROJECT_STATUS.md` — Authoritative shared project status.

## 5. Development and Safety Rules

- Do not confuse simulated readings with physical measurements.
- Do not assume hardware-specific details before board identification.
- Do not commit generated build output, caches, secrets, or large images.
- Use Git branches and review changes before integration.
- Keep the host operating system safe; test custom images in QEMU.

## 6. Project Status

Refer to [PROJECT_STATUS.md](PROJECT_STATUS.md) for current decisions,
milestones, verified versions, test evidence, and pending work.
