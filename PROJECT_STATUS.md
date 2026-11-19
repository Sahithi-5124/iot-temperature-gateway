# PROJECT_STATUS.md
## Authoritative Project Status — Track B IoT Temperature Gateway

**Last updated:** 2026-11-20
**Status:** Buildroot configuration prepared; first build not started.

## 1. Project goal

Build a lightweight, application-specific Linux distribution for an IoT temperature-monitoring gateway.

The system will:
- Collect temperature readings.
- Validate and process readings.
- Detect temperature threshold violations.
- Generate alerts.
- Log readings.
- Publish readings and alerts through MQTT.

**Phase 1:** Use simulated sensor readings and an x86-64 QEMU target.

**Phase 2:** Integrate the confirmed physical board and sensor.

Simulated readings must always be labelled SIMULATED. Physical readings must be labelled HARDWARE.

## 2. Project decisions

| Decision | Current value | Status |
|---|---|---|
| Project track | Track B — IoT Gateway | AGREED |
| Development arrangement | Individual development | CURRENT |
| Primary environment | Native Ubuntu 22.04 | AGREED |
| Build system | Buildroot 2026.08.2 | SELECTED |
| Initial target | x86-64 QEMU | PROVISIONAL — Phase 1 |
| Kernel version | 6.18.7 | SELECTED — Phase 1 |
| Application language | C | AGREED |
| Sensor architecture | Interchangeable simulation/hardware backends | AGREED |
| Initial sensor mode | Simulation | AGREED |
| Physical sensor | DHT11 preferred | PENDING confirmation |
| Physical board | Unknown | PENDING |
| Communication | MQTT | REQUIRED |
| MQTT library | libmosquitto | CANDIDATE |
| MQTT broker | Mosquitto | CANDIDATE |
| Init system | BusyBox init | CANDIDATE |
| Root filesystem | Buildroot-generated EXT2 | CONFIGURED |
| QEMU testing | Required for Phase 1 | AGREED |

The x86-64 QEMU target is a provisional development target. It must not be assumed compatible with the eventual physical board.

## 3. Pending decisions

- [ ] Confirm physical board model, revision and architecture.
- [ ] Confirm exact sensor model and electrical requirements.
- [ ] Finalize MQTT topics and payload schema.
- [ ] Finalize MQTT QoS and reconnect policy.
- [ ] Define warning and critical temperature thresholds.
- [ ] Define threshold boundary semantics.
- [ ] Finalize sensor API and application data structures.
- [ ] Select logging path and rotation policy.
- [ ] Select and configure the final init system.
- [ ] Determine physical-board kernel configuration, drivers and device tree.
- [ ] Verify the Buildroot source archive against the official checksum.

## 4. Development environment

**Primary development environment:** Native Ubuntu 22.04.

**Repository location:**
`/home/sahithi/OS_Project`

**GitHub repository:**
`https://github.com/Sahithi-5124/iot-temperature-gateway.git`

**Active development branch:** `dev/build-system`

The project is kept in a separate repository. Build outputs, downloads and generated images must not be committed as ordinary source files.

## 5. Version and environment record

| Component | Version | Status |
|---|---|---|
| Ubuntu | 22.04.5 LTS | VERIFIED |
| Host architecture | x86_64 | VERIFIED |
| Host kernel | 6.8.0-87-generic | VERIFIED |
| Buildroot | 2026.08.2 | SELECTED |
| Linux kernel target | 6.18.7 | CONFIGURED |
| QEMU | 6.2.0 | VERIFIED on host |
| Git | 2.34.1 | VERIFIED |
| GCC | 11.4.0 | VERIFIED |
| GNU Make | 4.3 | VERIFIED |
| Python | 3.10.12 | VERIFIED |

The host kernel is not being replaced or modified.

### Buildroot source

Source archive:

`buildroot/source/buildroot-2026.08.2.tar.xz`

Recorded SHA-256:

`bc9497c8e3a734c277e541e2f7fcf9a6275dc92f75bd2f753b05981c54be8705`

Archive integrity was checked with `xz -t`. The SHA-256 has not yet been independently verified against the official checksum.

## 6. Current milestone

**M2 — Buildroot configuration and external tree preparation.**

Status: Configuration successfully loaded and inspected. First compilation has not started.

### Build configuration

- Target architecture: x86-64.
- Build system: Buildroot 2026.08.2.
- Toolchain: Buildroot internal toolchain.
- C library: glibc.
- Kernel: Linux 6.18.7.
- Root filesystem: EXT2.
- Configured root filesystem size: 60 MB.
- Buildroot host QEMU package: Disabled.
- Host QEMU: Ubuntu-installed QEMU 6.2.0.
- Kernel configuration: Project-owned external-tree file.
- Post-image script: Project-owned external-tree script.

### External tree

Location:

`buildroot/external/iot_gateway/`

Structure:

```text
iot_gateway/
├── board/
│   └── x86_64/
│       ├── linux.config
│       └── post-image.sh
├── configs/
│   └── iot_gateway_x86_64_defconfig
├── Config.in
├── external.desc
└── external.mk
```

The external tree has been successfully registered with Buildroot.

## 7. Completed work

- [x] Track B selected.
- [x] Simulation-first architecture established.
- [x] Git repository created.
- [x] Initial project structure and documentation committed.
- [x] Development branch `dev/build-system` created.
- [x] Ubuntu environment inspected.
- [x] Required Buildroot host dependencies inspected.
- [x] Buildroot 2026.08.2 source downloaded and extracted.
- [x] Buildroot external tree created.
- [x] External tree registered successfully.
- [x] Project x86-64 defconfig created.
- [x] Project defconfig successfully loaded.
- [x] Project-owned Linux kernel configuration prepared.
- [x] Project-owned QEMU post-image script created.
- [x] Unnecessary upstream QEMU script references removed from the project configuration.
- [x] Buildroot host QEMU package disabled.
- [x] Kernel and root filesystem configuration verified.
- [ ] First Buildroot compilation.
- [ ] Boot generated image in QEMU.
- [ ] Implement C temperature-monitoring application.
- [ ] Implement sensor simulation.
- [ ] Implement threshold detection and alerts.
- [ ] Implement local logging.
- [ ] Integrate MQTT.
- [ ] Configure automatic application startup.
- [ ] Implement and verify at least three security measures.
- [ ] Perform functional and integration tests.
- [ ] Measure image size, boot time, RAM and CPU usage.
- [ ] Compare against an appropriate stock Linux baseline.
- [ ] Integrate physical hardware when available.

## 8. Important project files

- `README.md`
- `PROJECT_STATUS.md`
- `docs/PROJECT_OVERVIEW.md`
- `docs/PROJECT_ARCHITECTURE.md`
- `docs/PROJECT_ROADMAP.md`
- `docs/BUILD_AND_TEST_PLAN.md`
- `docs/TEAM_SYNC_PROTOCOL.md`

Buildroot-specific files:

- `buildroot/external/iot_gateway/configs/iot_gateway_x86_64_defconfig`
- `buildroot/external/iot_gateway/board/x86_64/linux.config`
- `buildroot/external/iot_gateway/board/x86_64/post-image.sh`

## 9. Commands executed and verified

| Activity | Result |
|---|---|
| Ubuntu version inspection | Successful |
| Git version inspection | Successful |
| GCC and Make inspection | Successful |
| QEMU version inspection | Successful |
| Buildroot archive integrity check | Successful |
| Buildroot source extraction | Successful |
| Reference QEMU configuration inspection | Successful |
| External tree registration | Successful |
| Project defconfig loading | Successful |
| Kernel configuration path verification | Successful |
| Post-image script path verification | Successful |
| Git branch and working-tree inspection | Successful |

These results establish configuration progress only. They do not establish successful compilation, image generation or QEMU boot.

## 10. Test results

| Test | Status |
|---|---|
| Buildroot compilation | NOT STARTED |
| Kernel compilation | NOT STARTED |
| Root filesystem generation | NOT STARTED |
| QEMU boot | NOT RUN |
| Sensor simulation | NOT RUN |
| Threshold boundary tests | NOT RUN |
| Invalid input tests | NOT RUN |
| MQTT publish/subscribe | NOT RUN |
| Automatic startup | NOT RUN |
| Security verification | NOT RUN |
| Benchmark measurements | NOT RUN |
| Physical sensor testing | BLOCKED — hardware pending |

## 11. Resource constraints and limitations

Last inspected host resources:

| Resource | Available/observed |
|---|---|
| Disk space | Approximately 13 GB free |
| RAM | 14 GB total |
| Available RAM | Approximately 10 GB |
| Swap | 0 GB |
| Logical CPUs | 12 |

The first build must be performed cautiously, initially using two parallel jobs and monitoring disk usage.

Additional limitations:

- Physical board and sensor are not confirmed.
- QEMU does not inherently emulate the physical DHT11 sensor or target-board GPIO.
- Host QEMU is version 6.2.0; compatibility must be tested with the generated image.
- Build duration and actual storage requirements are not yet known.
- QEMU measurements must not be represented as physical hardware measurements.

## 12. Next steps

1. Update and verify this status document.
2. Review and commit the Buildroot configuration milestone.
3. Check available disk space before compilation.
4. Perform the first Buildroot build with two parallel jobs.
5. Inspect generated kernel and root filesystem images.
6. Test the generated QEMU launcher.
7. Implement the C application and simulated sensor backend.
8. Integrate threshold detection, alerts, logging and MQTT.
9. Configure automatic startup and security measures.
10. Perform functional testing and benchmarks.
11. Update documentation with actual results.

## 13. Project record-keeping rules

- This document is the authoritative record of project decisions and progress.
- Record exact versions only when verified.
- Mark undecided items as PENDING.
- Mark unapproved proposals as PROVISIONAL.
- Do not mark a build, test, boot or benchmark successful without actual evidence.
- Keep generated build outputs and download caches out of ordinary Git commits.
- Record significant configuration changes and their reasons.
- Update this document whenever a milestone is completed.
- Preserve the distinction between simulated and physical sensor results.
