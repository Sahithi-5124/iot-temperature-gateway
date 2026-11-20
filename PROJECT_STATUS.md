# PROJECT_STATUS.md

## Authoritative Project Status — Track B IoT Temperature Gateway

**Last updated:** 2026-11-20

**Status:** Buildroot image generation and QEMU boot verified. Initial C sensor API and simulated sensor backend implemented and host-side tests passed. Application integration remains in progress.

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

Simulated readings must always be labelled `SIMULATED`. Physical readings must always be labelled `HARDWARE`.

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
- [ ] Review and synchronize the initial sensor API with the parallel development team.
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

**Development workflow:**

- VS Code is used for writing and editing source code and project files.
- Ubuntu Terminal (`Ctrl + Alt + T`) is used for commands, compilation, testing and Git operations.

## 5. Version and environment record

| Component | Version | Status |
|---|---|---|
| Ubuntu | 22.04.5 LTS | VERIFIED |
| Host architecture | x86_64 | VERIFIED |
| Host kernel | 6.8.0-87-generic | VERIFIED |
| Buildroot | 2026.08.2 | SELECTED |
| Linux kernel target | 6.18.7 | BUILT AND BOOTED |
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

## 6. M3 — Buildroot image generation and QEMU boot verification

**Status:** COMPLETED — Kernel and root filesystem image artifacts generated. The generated system successfully booted in QEMU, accepted a root login, and passed initial environment checks.

The final exit status of the resumed Buildroot `make` command was not separately captured. Image generation and boot are recorded based on the actual generated files and QEMU test results.

### Build configuration

- Target architecture: x86-64.
- Build system: Buildroot 2026.08.2.
- Toolchain: Buildroot internal toolchain.
- C library: glibc.
- Kernel: Linux 6.18.7.
- Root filesystem: EXT2.
- Configured root filesystem image size: 60 MB.
- Buildroot host QEMU package: Disabled.
- Host QEMU: Ubuntu-installed QEMU 6.2.0.
- Kernel configuration: Project-owned external-tree file.
- Post-image script: Project-owned external-tree script.
- Download checksum enforcement: Enabled.
- Global patch directory: Project-owned external-tree `patches/` directory.

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
├── patches/
│   ├── linux/
│   │   └── linux.hash
│   └── linux-headers/
│       └── linux-headers.hash
├── Config.in
├── external.desc
└── external.mk
```

The external tree has been successfully registered with Buildroot.

The two project-owned checksum files contain the SHA-256 checksum for `linux-6.18.7.tar.xz`:

`b726a4d15cf9ae06219b56d87820776e34d89fbc137e55fb54a9b9c3015b8f1e`

The kernel archive checksum was independently checked using Buildroot's `support/download/check-hash` utility and returned `OK`.

### Generated image artifacts

Generated under:

`buildroot/source/buildroot-2026.08.2/output/images/`

| Artifact | Observed result |
|---|---|
| `bzImage` | Present, approximately 6.4 MB |
| `rootfs.ext2` | Present, approximately 60 MB |
| `start-qemu.sh` | Present and executable |

The custom post-image script generated the QEMU launcher.

### QEMU boot verification

The generated system was started using:

`./output/images/start-qemu.sh`

The following were observed:

- Linux kernel 6.18.7 booted.
- Buildroot welcome message appeared.
- Root login succeeded.
- Root filesystem mounted as EXT2.
- Network interface `eth0` became active.
- DHCP assigned `10.0.2.15/24`.
- Default QEMU user-network gateway was `10.0.2.2`.
- BusyBox version 1.38.0 was verified.
- Init process PID 1 was observed.
- Syslog, kernel logging and cron services started.
- A shell was available for guest inspection.

The guest's observed memory was approximately 109.1 MB total. This is a guest measurement, not a measurement of physical target hardware.

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
- [x] Project-owned Linux and Linux-header checksum files created.
- [x] Linux 6.18.7 source archive checksum verified.
- [x] Kernel image artifact generated.
- [x] EXT2 root filesystem image generated.
- [x] Custom QEMU launcher generated.
- [x] Generated image successfully booted in QEMU.
- [x] Root login and initial guest inspection completed.
- [x] Guest kernel version and architecture verified.
- [x] Guest root filesystem type verified.
- [x] Guest network interface and DHCP configuration verified.
- [x] BusyBox version verified.
- [x] Initial C sensor API implemented.
- [x] Initial simulated sensor backend implemented.
- [x] Sensor module compiled with strict GCC warning flags.
- [x] Initial sensor runtime tests executed successfully.
- [x] Null-pointer error-handling tests executed successfully.
- [x] Automated expected-value assertions for the repeating temperature sequence.
- [x] Invalid sensor creation argument tests.
- [ ] Integrate sensor module into the complete C application.
- [ ] Implement comprehensive input validation.
- [ ] Implement threshold detection and alerts.
- [ ] Implement local logging.
- [ ] Integrate MQTT.
- [ ] Configure automatic application startup.
- [ ] Implement and verify at least three security measures.
- [ ] Perform functional and integration tests.
- [ ] Measure image size, boot time, RAM and CPU usage.
- [ ] Compare against an appropriate stock Linux baseline.
- [ ] Integrate physical hardware when available.

## 8. M4 — Initial C sensor module

**Status:** Initial sensor API and simulated backend implemented. Host-side compilation and runtime tests passed.

### Sensor API

File:

`application/include/sensor.h`

Implemented:

- `sensor_source_t`: identifies SIMULATED and HARDWARE sources.
- `sensor_status_t`: operation result codes.
- `temperature_sample_t`: temperature in Celsius, Unix timestamp, source and validity.
- `sensor_device_t`: opaque sensor device structure.
- `sensor_create()`: creates and initializes a selected sensor backend.
- `sensor_read()`: obtains one temperature sample.
- `sensor_destroy()`: releases sensor resources.

The initial API is implemented but remains subject to review and synchronization with the parallel development team.

### Simulation backend

File:

`application/src/sensor/sensor.c`

Implemented:

- Deterministic repeating sample sequence: 25.0, 30.0, 36.0 and 42.0 °C.
- All successful samples labelled `SIMULATED`.
- Unix timestamp attached to each sample.
- Sample validity set to `true` for successful simulated readings.
- Hardware mode returns `SENSOR_STATUS_UNSUPPORTED`.
- Invalid pointer arguments are rejected.
- Dynamic sensor allocation and cleanup implemented.

The sample temperatures are illustrative simulation values, not finalized warning or critical thresholds.

### Sensor tests

File:

`application/tests/test_sensor.c`

Host compiler: GCC 11.4.0 on Ubuntu 22.04.

Compilation flags:

`-std=c11 -Wall -Wextra -Wpedantic -Werror`

Verified tests:

- Sensor initialization.
- NULL creation output pointer rejection.
- Invalid sensor source rejection.
- NULL sensor read argument rejection.
- NULL sample output argument rejection.
- Five expected temperature values automatically asserted.
- Repeating sequence verified: 25.0, 30.0, 36.0, 42.0 and 25.0 °C.
- Sample validity checks.
- SIMULATED source checks.
- Hardware backend correctly rejected.

Compilation and execution both succeeded. The test program printed `All sensor tests passed`.

**Limitations:**

- No physical sensor backend exists.
- No QEMU application integration has been performed.
- Threshold detection, alerts, logging and MQTT are not implemented yet.
- Comprehensive temperature validity and range checking has not yet been implemented.
- The initial sensor API has not yet been synchronized with the parallel development team.

## 9. Important project files

- `README.md`
- `PROJECT_STATUS.md`
- `docs/PROJECT_OVERVIEW.md`
- `docs/PROJECT_ARCHITECTURE.md`
- `docs/PROJECT_ROADMAP.md`
- `docs/BUILD_AND_TEST_PLAN.md`
- `docs/TEAM_SYNC_PROTOCOL.md`

Application files:

- `application/include/sensor.h`
- `application/src/sensor/sensor.c`
- `application/tests/test_sensor.c`

Buildroot-specific files:

- `buildroot/external/iot_gateway/configs/iot_gateway_x86_64_defconfig`
- `buildroot/external/iot_gateway/board/x86_64/linux.config`
- `buildroot/external/iot_gateway/board/x86_64/post-image.sh`
- `buildroot/external/iot_gateway/patches/linux/linux.hash`
- `buildroot/external/iot_gateway/patches/linux-headers/linux-headers.hash`

Generated artifacts are kept under the ignored Buildroot output directory and must not be committed.

## 10. Commands executed and verified

| Activity | Result |
|---|---|
| Ubuntu version inspection | Successful |
| Git version inspection | Successful |
| GCC and Make version inspection | Successful |
| QEMU version inspection | Successful |
| Buildroot archive integrity check | Successful |
| Buildroot source extraction | Successful |
| Reference QEMU configuration inspection | Successful |
| External tree registration | Successful |
| Project defconfig loading | Successful |
| Kernel configuration path verification | Successful |
| Post-image script path verification | Successful |
| Linux source archive checksum verification | Successful |
| Buildroot image artifact inspection | Successful |
| Custom QEMU launcher execution | Successful |
| QEMU kernel boot | Successful |
| Guest root login | Successful |
| Guest kernel and architecture inspection | Successful |
| Guest root filesystem inspection | Successful |
| Guest network inspection | Successful |
| BusyBox version inspection | Successful |
| Sensor API header creation and inspection | Successful |
| Sensor source compilation with strict GCC flags | Successful |
| Sensor test program compilation | Successful |
| Sensor runtime test execution | Successful |
| Sensor invalid-pointer tests | Successful |
| Git whitespace validation (`git diff --check`) | Successful |

The final exit status of the resumed Buildroot `make` command was not captured separately. The generated image artifacts and successful QEMU boot provide independent evidence of a working kernel and root filesystem.

Sensor tests were executed on the Ubuntu host, not inside QEMU.

## 11. Test results

| Test | Status |
|---|---|
| Buildroot image artifacts generated | VERIFIED |
| Linux kernel compilation | VERIFIED through generated and booted kernel |
| Root filesystem generation | VERIFIED |
| QEMU boot | PASS |
| Root login | PASS |
| Kernel version | PASS — 6.18.7 |
| Architecture | PASS — x86_64 |
| Root filesystem mount | PASS — EXT2 |
| DHCP networking | PASS — `eth0` received `10.0.2.15` |
| BusyBox version | VERIFIED — 1.38.0 |
| Sensor API compilation | PASS — host GCC |
| Sensor simulation | PASS — host-side test |
| Sensor sequence and repetition | PASS |
| Sample validity and source checks | PASS |
| NULL sensor argument handling | PASS |
| NULL output argument handling | PASS |
| Hardware backend rejection | PASS |
| Automated expected-value assertions | PASS |
| Invalid sensor creation argument tests | PASS |
| Comprehensive temperature input validation | NOT IMPLEMENTED |
| Threshold boundary tests | NOT RUN |
| Alert generation tests | NOT RUN |
| Local logging tests | NOT RUN |
| MQTT publish/subscribe | NOT RUN |
| Automatic application startup | NOT RUN |
| Security verification | NOT RUN |
| Benchmark measurements | NOT RUN |
| Physical sensor testing | BLOCKED — hardware pending |

The initial sensor module has passed host-side tests. The complete temperature-monitoring application has not yet been integrated or tested.

## 12. Resource constraints and limitations

Last inspected host resources after image generation:

| Resource | Available/observed |
|---|---|
| Disk space | Approximately 4.7 GB free |
| Root filesystem usage | Approximately 90% |
| Buildroot output directory | Approximately 8.5 GB |
| RAM | 14 GB total |
| Available RAM | Approximately 10 GB at last inspection |
| Swap | 0 GB |
| Logical CPUs | 12 |

Disk space is a significant constraint. Avoid unnecessary full rebuilds and do not delete the existing Buildroot output or download cache without first checking dependencies and available alternatives.

Additional limitations:

- Physical board and sensor are not confirmed.
- QEMU does not inherently emulate the physical DHT11 sensor or target-board GPIO.
- Host QEMU is version 6.2.0; compatibility with the current generated image has been demonstrated by successful boot.
- Build duration and actual storage requirements have not been benchmarked.
- QEMU measurements must not be represented as physical hardware measurements.
- The Buildroot source archive SHA-256 still needs independent comparison against the official checksum.
- The final application, MQTT integration and security configuration are not yet implemented.

## 13. Next steps

1. Review and synchronize the initial sensor API with the parallel development team.
2. Extend sensor tests with timestamp validation and additional edge cases.
3. Implement temperature input validation and processing.
4. Define warning and critical temperature thresholds.
5. Define threshold boundary semantics and test them.
6. Implement alert generation.
7. Implement local logging.
8. Finalize MQTT topics and payload schema.
9. Integrate MQTT publishing.
10. Configure automatic application startup.
11. Implement and verify at least three security measures.
12. Perform functional and integration tests inside QEMU.
13. Measure image size, boot time, RAM and CPU usage.
14. Compare against an appropriate stock Linux baseline.
15. Integrate the confirmed physical board and sensor when available.

## 14. Project record-keeping rules

- This document is the authoritative record of project decisions and progress.
- Record exact versions only when verified.
- Mark undecided items as PENDING.
- Mark unapproved proposals as PROVISIONAL.
- Do not mark a build, test, boot or benchmark successful without actual evidence.
- Keep generated build outputs and download caches out of ordinary Git commits.
- Record significant configuration changes and their reasons.
- Update this document whenever a milestone is completed.
- Preserve the distinction between simulated and physical sensor results.
- Synchronize shared interfaces and decisions with the parallel development team.
