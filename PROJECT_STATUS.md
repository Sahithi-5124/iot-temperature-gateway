# PROJECT STATUS
## Application-Specific Linux Distribution for IoT Temperature-Monitoring Gateway — Track B

**Last Updated:** 2026-11-20

**Project Status:** Buildroot image generation and QEMU boot verification completed. Initial C sensor API, simulated sensor backend and temperature-processing module implemented and tested successfully on the Ubuntu host. Application integration is in progress.

---

## 1. Project Overview

The project focuses on developing a lightweight, application-specific Linux distribution for an IoT temperature-monitoring gateway.

The gateway is designed to:

- Collect temperature readings from a sensor.
- Validate and process the collected readings.
- Classify temperatures based on predefined thresholds.
- Generate alerts when temperature thresholds are exceeded.
- Maintain local temperature logs.
- Publish temperature readings and alerts through MQTT.

### Development Phases

**Phase 1 — Simulation and Virtual Platform**

Development and testing are performed using simulated temperature readings and an x86-64 QEMU environment. The Phase 1 objective is to produce a complete working IoT gateway simulation with a customized Linux image and integrated C application.

**Phase 2 — Physical Hardware Integration**

The application will be adapted to the confirmed physical board and temperature sensor after the hardware components become available.

All simulated readings are identified as `SIMULATED`. Physical sensor readings will be identified as `HARDWARE`.

---

## 2. Project Decisions and Technical Specifications

| Decision | Selected or Current Value | Status |
|---|---|---|
| Project track | Track B — IoT Gateway | AGREED |
| Development arrangement | Individual development | CURRENT |
| Primary development environment | Native Ubuntu 22.04 | AGREED |
| Build system | Buildroot 2026.08.2 | SELECTED |
| Initial target platform | x86-64 QEMU | PROVISIONAL — Phase 1 |
| Linux kernel version | 6.18.7 | SELECTED — Phase 1 |
| Application programming language | C | AGREED |
| Sensor architecture | Interchangeable simulation and hardware backends | AGREED |
| Initial sensor mode | Simulation | AGREED |
| Preferred physical sensor | DHT11 | PENDING hardware confirmation |
| Physical target board | Not yet identified | PENDING |
| Communication protocol | MQTT | REQUIRED |
| MQTT client library | libmosquitto | CANDIDATE |
| MQTT broker | Mosquitto | CANDIDATE |
| Init system | BusyBox init | CANDIDATE |
| Root filesystem | Buildroot-generated EXT2 | CONFIGURED |
| Virtual platform testing | QEMU | AGREED |
| Dashboard visualization | Node-RED | PLANNED |

### 2.1 Temperature Classification Thresholds

The following thresholds have been selected for the initial simulated demonstration.

| Temperature Range | Classification |
|---|---|
| Below 35°C | NORMAL |
| 35°C to below 40°C | WARNING |
| 40°C and above | CRITICAL |

**Boundary conditions:**

- 35.0°C is classified as WARNING.
- 40.0°C is classified as CRITICAL.
- Temperatures below 35.0°C are classified as NORMAL.

These thresholds are intended for project demonstration and are not certified safety limits.

---

## 3. Pending Decisions

The following items remain to be finalized:

- [ ] Confirm the physical board model, revision and architecture.
- [ ] Confirm the exact temperature sensor model and electrical requirements.
- [ ] Finalize MQTT topics and message payload schema.
- [ ] Finalize MQTT Quality of Service (QoS).
- [ ] Finalize MQTT reconnection policy.
- [ ] Select the application logging path.
- [ ] Define log rotation and retention policy.
- [ ] Finalize the init system configuration.
- [ ] Determine physical-board kernel configuration, drivers and device tree.
- [ ] Verify the Buildroot source archive against the official checksum.
- [ ] Define the acceptable temperature input range.
- [ ] Define the alert event structure and duplicate-alert handling policy.

---

## 4. Development Environment and Repository

### 4.1 Development Environment

**Operating System:** Ubuntu 22.04.5 LTS

**Project Repository Location:**

`/home/sahithi/OS_Project`

**GitHub Repository:**

https://github.com/Sahithi-5124/iot-temperature-gateway.git

**Active Development Branch:** `dev/build-system`

### 4.2 Development Tools

- Visual Studio Code is used for creating and editing source code and project files.
- Ubuntu Terminal is used for compilation, testing, Git operations, Buildroot configuration and QEMU execution.
- GCC is used for host-side C application compilation and testing.
- Git is used for source-code version control.

### 4.3 Repository Management

A Git repository was created for the project, and the initial project structure and documentation were committed.

The `dev/build-system` development branch was created and pushed to the GitHub repository.

The initial sensor module and related documentation changes were also committed and pushed.

Generated Buildroot output directories, downloaded source archives, caches and large generated images are excluded from ordinary Git commits.

---

## 5. Software Versions and Environment Information

| Component | Version | Verification Status |
|---|---|---|
| Ubuntu | 22.04.5 LTS | VERIFIED |
| Host architecture | x86_64 | VERIFIED |
| Host kernel | 6.8.0-87-generic | VERIFIED |
| Buildroot | 2026.08.2 | SELECTED |
| Target Linux kernel | 6.18.7 | BUILT AND BOOTED |
| QEMU | 6.2.0 | VERIFIED |
| Git | 2.34.1 | VERIFIED |
| GCC | 11.4.0 | VERIFIED |
| GNU Make | 4.3 | VERIFIED |
| Python | 3.10.12 | VERIFIED |

The existing host Linux kernel remains unchanged.

### 5.1 Buildroot Source Archive

Source archive:

`buildroot/source/buildroot-2026.08.2.tar.xz`

Recorded SHA-256:

`bc9497c8e3a734c277e541e2f7fcf9a6275dc92f75bd2f753b05981c54be8705`

The archive integrity check was performed using `xz -t` and completed successfully.

Independent comparison of the SHA-256 value against the official Buildroot checksum remains pending.

---

## 6. M3 — Buildroot Image Generation and QEMU Boot Verification

**Status: COMPLETED**

The Buildroot configuration, Linux kernel image and EXT2 root filesystem were generated. The resulting system successfully booted in QEMU and passed the initial guest-system verification checks.

The final exit status of the resumed Buildroot `make` command was not separately captured. Successful image generation and QEMU boot were independently verified.

### 6.1 Build Configuration

| Configuration | Selected Value |
|---|---|
| Build system | Buildroot 2026.08.2 |
| Target architecture | x86-64 |
| Toolchain | Buildroot internal toolchain |
| C library | glibc |
| Linux kernel | 6.18.7 |
| Root filesystem | EXT2 |
| Configured root filesystem size | 60 MB |
| Buildroot host QEMU package | Disabled |
| Host QEMU | Ubuntu-installed QEMU 6.2.0 |
| Kernel configuration | Project-owned external-tree configuration |
| Post-image script | Project-owned script |
| Download checksum enforcement | Enabled |
| Global patch directory | Project-owned external-tree directory |

### 6.2 Buildroot External Tree

Location:

`buildroot/external/iot_gateway/`

Directory structure:

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

The external tree was successfully registered with Buildroot.

### 6.3 Linux Kernel Checksum Verification

The project-owned checksum files contain the SHA-256 checksum for `linux-6.18.7.tar.xz`:

`b726a4d15cf9ae06219b56d87820776e34d89fbc137e55fb54a9b9c3015b8f1e`

The kernel source archive checksum was independently checked using Buildroot's `support/download/check-hash` utility. The utility returned `OK`.

### 6.4 Generated Image Artifacts

Generated artifacts are located in:

`buildroot/source/buildroot-2026.08.2/output/images/`

| Artifact | Observed Result |
|---|---|
| `bzImage` | Generated, approximately 6.4 MB |
| `rootfs.ext2` | Generated, approximately 60 MB |
| `start-qemu.sh` | Generated and executable |

The custom post-image script generated the QEMU launcher.

### 6.5 QEMU Boot Verification

The generated system was launched using:

`./output/images/start-qemu.sh`

The following observations were recorded during guest-system inspection:

- Linux kernel 6.18.7 booted successfully.
- Buildroot welcome message appeared.
- Root login succeeded.
- Root filesystem mounted as EXT2.
- Network interface `eth0` became active.
- DHCP assigned IP address `10.0.2.15/24`.
- Default QEMU user-network gateway was `10.0.2.2`.
- BusyBox version 1.38.0 was verified.
- Init process PID 1 was observed.
- Syslog, kernel logging and cron services started.
- A shell was available for guest inspection.

The guest reported approximately 109.1 MB of total memory. This is a QEMU guest measurement and does not represent physical target hardware memory.

---

## 7. Completed Project Milestones

### 7.1 Project Initialization

- [x] Track B selected.
- [x] Simulation-first development approach established.
- [x] Git repository created.
- [x] Initial project structure created.
- [x] Initial documentation created and committed.
- [x] Development branch `dev/build-system` created.
- [x] Ubuntu development environment inspected.
- [x] Required Buildroot host dependencies inspected.

### 7.2 Buildroot and QEMU

- [x] Buildroot 2026.08.2 source downloaded and extracted.
- [x] Buildroot external tree created.
- [x] External tree registered successfully.
- [x] Project x86-64 defconfig created.
- [x] Project defconfig successfully loaded.
- [x] Project-owned Linux kernel configuration prepared.
- [x] Project-owned QEMU post-image script created.
- [x] Unnecessary upstream QEMU script references removed from project configuration.
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

### 7.3 C Application Development

- [x] Initial C sensor API implemented.
- [x] Initial simulated sensor backend implemented.
- [x] Sensor module compiled with strict GCC warning flags.
- [x] Initial sensor runtime tests executed successfully.
- [x] NULL-pointer error-handling tests executed successfully.
- [x] Automated expected-value assertions implemented for the repeating temperature sequence.
- [x] Invalid sensor creation argument tests completed.
- [x] Initial temperature-processing API implemented.
- [x] Temperature classification implemented.
- [x] Warning and critical thresholds configured.
- [x] Temperature boundary semantics implemented and tested.
- [x] Invalid processor arguments rejected.
- [x] Invalid sensor samples rejected.
- [x] NaN and infinity inputs rejected.
- [x] Processor compiled with strict GCC warning flags.
- [x] Processor automated tests executed successfully.

### 7.4 Remaining Development Work

- [ ] Extend sensor tests with timestamp validation and additional edge cases.
- [ ] Implement comprehensive temperature input and range validation.
- [ ] Implement alert generation.
- [ ] Implement local logging.
- [ ] Implement MQTT communication.
- [ ] Integrate all application modules.
- [ ] Integrate the application into Buildroot.
- [ ] Configure automatic application startup.
- [ ] Implement and verify at least three security measures.
- [ ] Perform functional and integration testing in QEMU.
- [ ] Perform preliminary performance measurements.
- [ ] Compare against an appropriate stock Linux baseline.
- [ ] Integrate physical hardware when available.

---

## 8. M4 — C Application Implementation

### 8.1 Sensor API and Simulated Backend

**Status: IMPLEMENTED AND TESTED**

#### Sensor API

File:

`application/include/sensor.h`

The sensor interface defines:

- `sensor_source_t`: identifies SIMULATED and HARDWARE sensor sources.
- `sensor_status_t`: defines sensor operation result codes.
- `temperature_sample_t`: stores temperature in Celsius, Unix timestamp, source and validity.
- `sensor_device_t`: opaque sensor device structure.
- `sensor_create()`: creates and initializes the selected sensor backend.
- `sensor_read()`: obtains a temperature sample.
- `sensor_destroy()`: releases sensor resources.

#### Simulated Sensor Backend

File:

`application/src/sensor/sensor.c`

Implemented functionality:

- Deterministic repeating temperature sequence: 25.0, 30.0, 36.0 and 42.0°C.
- Successful readings are labelled `SIMULATED`.
- Unix timestamps are attached to successful readings.
- Sample validity is set to `true` for successful simulated readings.
- Hardware mode returns `SENSOR_STATUS_UNSUPPORTED`.
- Invalid pointer arguments are rejected.
- Dynamic sensor allocation and resource cleanup are implemented.

The simulated values are illustrative inputs for application testing.

#### Sensor Automated Tests

File:

`application/tests/test_sensor.c`

**Compiler:** GCC 11.4.0 on Ubuntu 22.04.

**Compilation flags:**

`-std=c11 -Wall -Wextra -Wpedantic -Werror`

Tests performed:

- Sensor initialization.
- NULL creation output pointer rejection.
- Invalid sensor source rejection.
- NULL sensor read argument rejection.
- NULL sample output argument rejection.
- Automated verification of five expected temperature values.
- Repeating sequence verification: 25.0, 30.0, 36.0, 42.0 and 25.0°C.
- Sample validity checks.
- SIMULATED source checks.
- Hardware backend rejection.

The sensor source and test program compiled successfully. Runtime tests completed successfully and printed:

```text
=== All sensor tests passed ===
```

#### Sensor Module Limitations

- Physical sensor backend has not been implemented.
- Application has not yet been integrated into QEMU.
- Comprehensive temperature range validation remains pending.
- Additional timestamp edge-case testing remains pending.

### 8.2 Temperature Processing Module

**Status: IMPLEMENTED AND TESTED**

#### Source Files

- `application/include/processor.h`
- `application/src/processing/processor.c`
- `application/tests/test_processor.c`

#### Implemented Functionality

- Normal, warning and critical temperature classification.
- Warning threshold configured at 35°C.
- Critical threshold configured at 40°C.
- Temperature value preserved in the processing result.
- Unix timestamp preserved in the processing result.
- Sensor source preserved in the processing result.
- NULL argument rejection.
- Invalid sensor sample rejection.
- NaN and infinity rejection.

#### Temperature Classification

| Temperature | Expected Classification |
|---|---|
| 25.0°C | NORMAL |
| 34.9°C | NORMAL |
| 35.0°C | WARNING |
| 39.9°C | WARNING |
| 40.0°C | CRITICAL |
| 42.0°C | CRITICAL |

#### Compiler Verification

**Compiler:** GCC 11.4.0 on Ubuntu 22.04.

**Compilation flags:**

`-std=c11 -Wall -Wextra -Wpedantic -Werror`

The processor source compiled successfully.

The generated object file was inspected:

`/tmp/processor.o`

Observed file type:

`ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped`

#### Automated Tests

File:

`application/tests/test_processor.c`

The following tests were executed:

- Normal temperature classification.
- Warning temperature classification.
- Critical temperature classification.
- 35°C warning boundary.
- 40°C critical boundary.
- NULL input argument rejection.
- NULL output argument rejection.
- Invalid sensor sample rejection.
- NaN rejection.
- Positive infinity rejection.
- Negative infinity rejection.
- Temperature value preservation.
- Timestamp preservation.
- Sensor source preservation.

Compilation and execution completed successfully.

**Recorded Test Output:**

```text
=== IoT Gateway Processor Tests ===

PASS: Normal temperature classification.
PASS: Warning temperature classification.
PASS: Critical temperature classification.
PASS: NULL argument validation.
PASS: Invalid sensor sample rejection.
PASS: NaN and infinity rejection.

=== All processor tests passed ===
```

### Temperature Input Range Validation

The temperature-processing module was extended with input range validation.

- Accepted temperature range: −40°C to 85°C, inclusive (provisional).
- Readings below −40°C or above 85°C are rejected.
- NaN and positive/negative infinity are rejected.
- Invalid sensor samples and NULL arguments are rejected.
- Existing normal, warning and critical classification thresholds remain unchanged.

**Verification:** The processor test program compiled with strict C11 warning flags and all eight test groups passed, including valid range boundaries, out-of-range values, invalid samples, NULL arguments and non-finite values.

**Limitation:** The accepted input range is provisional and must be reviewed when the physical sensor model is confirmed. Testing was performed on the Ubuntu host; QEMU integration remains pending.

#### Processor Module Limitations

- Comprehensive acceptable temperature range validation has not been implemented.
- Alert generation has not been implemented.
- Local logging has not been implemented.
- MQTT publishing has not been implemented.
- Testing has been performed on the Ubuntu host.
- Integration with the Buildroot target has not been performed.

---

## 9. Project Directory Structure

The current application directory structure is:

```text
application/
├── include/
│   ├── sensor.h
│   └── processor.h
├── src/
│   ├── alerts/
│   ├── logging/
│   ├── mqtt/
│   ├── processing/
│   │   └── processor.c
│   └── sensor/
│       └── sensor.c
└── tests/
    ├── test_sensor.c
    └── test_processor.c
```

The `alerts`, `logging` and `mqtt` directories have been created as part of the application structure. Their implementations remain pending.

### 9.1 Important Documentation Files

- `README.md`
- `PROJECT_STATUS.md`
- `docs/PROJECT_OVERVIEW.md`
- `docs/PROJECT_ARCHITECTURE.md`
- `docs/PROJECT_ROADMAP.md`
- `docs/BUILD_AND_TEST_PLAN.md`
- `docs/TEAM_SYNC_PROTOCOL.md`

### 9.2 Important Buildroot Files

- `buildroot/external/iot_gateway/configs/iot_gateway_x86_64_defconfig`
- `buildroot/external/iot_gateway/board/x86_64/linux.config`
- `buildroot/external/iot_gateway/board/x86_64/post-image.sh`
- `buildroot/external/iot_gateway/patches/linux/linux.hash`
- `buildroot/external/iot_gateway/patches/linux-headers/linux-headers.hash`

Generated Buildroot artifacts remain under the ignored output directory.

---

## 10. Commands Executed and Verification Record

| Activity | Result |
|---|---|
| Ubuntu version inspection | SUCCESSFUL |
| Git version inspection | SUCCESSFUL |
| GCC and Make version inspection | SUCCESSFUL |
| QEMU version inspection | SUCCESSFUL |
| Buildroot archive integrity check | SUCCESSFUL |
| Buildroot source extraction | SUCCESSFUL |
| Reference QEMU configuration inspection | SUCCESSFUL |
| External tree registration | SUCCESSFUL |
| Project defconfig loading | SUCCESSFUL |
| Kernel configuration path verification | SUCCESSFUL |
| Post-image script path verification | SUCCESSFUL |
| Linux source archive checksum verification | SUCCESSFUL |
| Buildroot image artifact inspection | SUCCESSFUL |
| Custom QEMU launcher execution | SUCCESSFUL |
| QEMU kernel boot | SUCCESSFUL |
| Guest root login | SUCCESSFUL |
| Guest kernel and architecture inspection | SUCCESSFUL |
| Guest root filesystem inspection | SUCCESSFUL |
| Guest network inspection | SUCCESSFUL |
| BusyBox version inspection | SUCCESSFUL |
| Sensor API header creation and inspection | SUCCESSFUL |
| Sensor source compilation with strict GCC flags | SUCCESSFUL |
| Sensor test program compilation | SUCCESSFUL |
| Sensor runtime test execution | SUCCESSFUL |
| Sensor invalid-pointer tests | SUCCESSFUL |
| Processor header creation and inspection | SUCCESSFUL |
| Processor source compilation with strict GCC flags | SUCCESSFUL |
| Processor object-file inspection | SUCCESSFUL |
| Processor test program compilation | SUCCESSFUL |
| Processor automated test execution | SUCCESSFUL |
| Processor boundary tests | SUCCESSFUL |
| Processor invalid-input tests | SUCCESSFUL |
| Git whitespace validation (`git diff --check`) | SUCCESSFUL in previously reported verification |

The final exit status of the resumed Buildroot `make` command was not separately captured. The generated image artifacts and successful QEMU boot provide independent evidence of a working kernel and root filesystem.

Sensor and processor tests were executed on the Ubuntu host. They have not yet been executed inside QEMU.

---

## 11. Consolidated Test Results

| Test | Status |
|---|---|
| Buildroot image artifacts generated | VERIFIED |
| Linux kernel image generated | VERIFIED |
| Root filesystem generated | VERIFIED |
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
| Processor compilation | PASS — strict GCC flags |
| Processor object-file verification | PASS |
| Normal temperature classification | PASS |
| Warning temperature classification | PASS |
| Critical temperature classification | PASS |
| 35°C boundary test | PASS |
| 40°C boundary test | PASS |
| NULL processor argument tests | PASS |
| Invalid sensor sample rejection | PASS |
| NaN rejection | PASS |
| Infinity rejection | PASS |
| Temperature preservation | PASS |
| Timestamp preservation | PASS |
| Sensor source preservation | PASS |
| Comprehensive temperature range validation | NOT IMPLEMENTED |
| Sensor timestamp edge-case tests | NOT COMPLETED |
| Alert generation tests | NOT RUN |
| Local logging tests | NOT RUN |
| MQTT publish/subscribe | NOT RUN |
| Automatic application startup | NOT RUN |
| Security verification | NOT RUN |
| Benchmark measurements | NOT RUN |
| Physical sensor testing | BLOCKED — hardware pending |

The sensor and processor modules have passed host-side tests. Complete application integration and end-to-end gateway testing remain pending.

---

## 12. Resource Constraints and Project Limitations

The last recorded host resource inspection after image generation showed:

| Resource | Observed Value |
|---|---|
| Disk space available | Approximately 4.7 GB |
| Root filesystem usage | Approximately 90% |
| Buildroot output directory | Approximately 8.5 GB |
| Total RAM | 14 GB |
| Available RAM | Approximately 10 GB |
| Swap | 0 GB |
| Logical CPUs | 12 |

Disk space is a significant development constraint.

Unnecessary full rebuilds should be avoided. Existing Buildroot output directories and download caches should not be deleted without checking their dependencies and available alternatives.

### Additional Limitations

- Physical board and sensor have not yet been confirmed.
- QEMU does not inherently emulate the physical DHT11 sensor or target-board GPIO.
- Host QEMU version 6.2.0 successfully booted the generated image.
- Build duration and actual storage requirements have not been benchmarked.
- QEMU measurements must not be represented as physical hardware measurements.
- Independent verification of the Buildroot source archive SHA-256 remains pending.
- The complete C application has not been integrated into the generated Linux image.
- MQTT communication and security configuration remain unimplemented.
- Node-RED has not been installed; dashboard development is deferred until the core gateway and MQTT communication are functional.
- End-to-end testing of the complete gateway has not yet been performed.

---

## 13. Remaining Work and Development Roadmap

The following tasks are planned for completion.

### 13.1 Sensor and Processing

1. Extend sensor tests with timestamp validation and additional edge cases.
2. Implement comprehensive temperature input validation.
3. Define and enforce the acceptable temperature range.

### 13.2 Alert Generation

4. Implement the alert-generation module.
5. Generate warning and critical alert events.
6. Test alert classification, boundaries and invalid inputs.

### 13.3 Local Logging

7. Implement local temperature logging.
8. Include timestamps, temperature values, classification and sensor source.
9. Handle file-access and write errors.
10. Test logging functionality and determine the log rotation policy.

### 13.4 MQTT Communication

11. Finalize MQTT topics and JSON payload schema.
12. Finalize QoS and reconnection policy.
13. Investigate libmosquitto integration with Buildroot.
14. Implement MQTT publishing for temperature readings and alerts.
15. Test communication with a Mosquitto broker.

### 13.5 Application Integration

16. Integrate sensor acquisition, processing, alerts, logging and MQTT into the main application.
17. Compile and test the complete application on Ubuntu.
18. Cross-compile the application for the Buildroot target.
19. Integrate the application into the root filesystem.
20. Configure automatic application startup.

### 13.6 Security and Testing

21. Implement at least three security-hardening measures.
22. Verify each security measure with recorded evidence.
23. Perform complete functional testing inside QEMU.
24. Test MQTT communication between the QEMU gateway and broker.
25. Test application error handling and reconnection behaviour.

### 13.7 Evaluation and Demonstration

26. Measure image size, boot time, RAM and CPU usage.
27. Establish a suitable stock Linux baseline.
28. Compare the customized distribution against the baseline.
29. Document the build process and testing methodology.
30. Prepare the final project demonstration.

### 13.8 Visualization

31. Install and configure Node-RED after MQTT communication is functional.
32. Create a temperature-monitoring dashboard.
33. Display live readings, temperature classifications, alerts and historical graphs.

### 13.9 Physical Hardware Integration

34. Identify the exact physical board and sensor.
35. Confirm electrical compatibility and supported interfaces.
36. Configure the required kernel drivers and device tree.
37. Implement the hardware sensor backend.
38. Deploy the appropriate image and perform physical sensor testing.

---

## 14. Documentation and Verification Policy

The project status document serves as the authoritative record of implementation progress, technical decisions and verification results.

The following practices are maintained:

- Exact software versions are recorded after verification.
- Undecided technical choices are marked as PENDING.
- Unapproved proposals are marked as PROVISIONAL.
- Build and test results are recorded only after actual execution.
- Generated build outputs, downloaded archives and caches are excluded from ordinary Git commits.
- Significant configuration changes are documented.
- Simulated and physical sensor results are clearly distinguished.
- The modular sensor architecture is maintained to support future hardware integration.
- Actual commands and test outputs are recorded when milestones are completed.
- End-to-end functionality is not claimed until the complete application has been integrated and tested.