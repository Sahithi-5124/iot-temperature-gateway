# PROJECT STATUS

## Application-Specific Linux Distribution for IoT Temperature-Monitoring Gateway — Track B

**Last Updated:** 2026-11-20

**Project Status:** Buildroot image generation and QEMU boot verification completed. Initial C sensor API, simulated sensor backend, temperature-processing module, alert-generation module, local CSV logging module and MQTT communication module implemented. All five application modules have passed their reported Ubuntu host-side tests. MQTT publishing and actual broker message receipt verified. Main application integration and Buildroot integration remain in progress.

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
| MQTT client library | libmosquitto 2.0.11 | INSTALLED AND VERIFIED ON UBUNTU |
| MQTT broker | Eclipse Mosquitto 2.0.11 | INSTALLED AND VERIFIED ON UBUNTU |
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

### 2.2 Temperature Input Range

The processor currently accepts temperatures in the range:

- Minimum: −40°C.
- Maximum: 85°C.
- Both boundaries are inclusive.

This range is provisional and must be reviewed when the physical sensor model is confirmed.

### 2.3 MQTT Communication Decisions

| Configuration | Selected Value |
|---|---|
| Broker | Eclipse Mosquitto |
| Broker version | 2.0.11 |
| Client library | libmosquitto |
| Client library version | 2.0.11 |
| Reading topic | `iot/temperature/readings` |
| Alert topic | `iot/temperature/alerts` |
| Payload format | JSON |
| QoS | 1 |
| Retain | Disabled |
| Host testing broker address | `localhost` |
| Host testing broker port | 1883 |
| Keepalive | 60 seconds |

**Temperature reading payload fields:**

- `timestamp`: Unix timestamp in seconds.
- `temperature_c`: Temperature in Celsius.
- `source`: `SIMULATED` or `HARDWARE`.
- `status`: `NORMAL`, `WARNING` or `CRITICAL`.

**Alert payload fields:**

- `timestamp`: Unix timestamp in seconds.
- `temperature_c`: Temperature in Celsius.
- `source`: `SIMULATED` or `HARDWARE`.
- `alert_level`: `WARNING` or `CRITICAL`.

### 2.4 MQTT Reconnection Policy

The following policy has been approved:

- Automatic reconnection.
- Retry interval: 5 seconds.
- No busy-wait loops.
- Local CSV logging must continue when MQTT is unavailable.
- MQTT publishing should resume after reconnection.

**Status:** APPROVED — IMPLEMENTATION PENDING.

---

## 3. Pending Decisions and Tasks

The following items remain to be finalized or implemented:

- [ ] Confirm the physical board model, revision and architecture.
- [ ] Confirm the exact temperature sensor model and electrical requirements.
- [ ] Implement the approved MQTT reconnection policy.
- [ ] Select the application logging path for the target gateway.
- [ ] Define log rotation and retention policy.
- [ ] Finalize the init system configuration.
- [ ] Determine physical-board kernel configuration, drivers and device tree.
- [ ] Verify the Buildroot source archive against the official checksum.
- [ ] Review the acceptable temperature input range after sensor confirmation.
- [ ] Define duplicate-alert handling policy.
- [ ] Integrate all application modules into the main application.
- [ ] Integrate the MQTT client library into Buildroot.
- [ ] Integrate the complete application into the Buildroot root filesystem.
- [ ] Configure automatic application startup.
- [ ] Implement and verify at least three security measures.
- [ ] Perform complete functional testing inside QEMU.
- [ ] Perform preliminary performance measurements.
- [ ] Compare against an appropriate stock Linux baseline.
- [ ] Integrate physical hardware when available.

The MQTT topics, JSON payload schema, QoS and retain settings have already been agreed and verified during host-side testing.

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
- Mosquitto is used as the MQTT broker.
- libmosquitto is used as the MQTT client library.

### 4.3 Repository Management

A Git repository was created for the project, and the initial project structure and documentation were committed.

The `dev/build-system` development branch was created and pushed to the GitHub repository.

Previously completed milestones, including the sensor, processor, alert and logging modules, were committed and pushed.

The MQTT implementation has now reached a verified host-side communication milestone.

**Latest confirmed push before the MQTT milestone:** Commit `5464185`.

**MQTT milestone Git commit:** PENDING confirmation.

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
| Mosquitto broker | 2.0.11 | VERIFIED |
| libmosquitto | 2.0.11 | VERIFIED |

The existing host Linux kernel remains unchanged.

### 5.1 MQTT Development Dependencies

The following Ubuntu packages were installed:

- `mosquitto`
- `mosquitto-clients`
- `libmosquitto-dev`

Development package version:

`2.0.11-1ubuntu1.2`

Verification commands:

```bash
pkg-config --modversion libmosquitto
pkg-config --cflags --libs libmosquitto
test -f /usr/include/mosquitto.h && printf "Mosquitto header found\n"
```

Recorded output:

```text
2.0.11
-lmosquitto
Mosquitto header found
```

The Mosquitto broker service was verified as active.

### 5.2 Buildroot Source Archive

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
- [x] Temperature range validation implemented provisionally.
- [x] Processor compiled with strict GCC warning flags.
- [x] Processor automated tests executed successfully.
- [x] Alert-generation module implemented.
- [x] Alert-generation tests passed.
- [x] Local CSV logging module implemented.
- [x] Logger tests passed, including CSV content verification.
- [x] MQTT development library installed and verified.
- [x] MQTT C client module implemented.
- [x] MQTT module compiled with strict GCC warning flags.
- [x] MQTT test program compiled and linked.
- [x] MQTT broker connection verified.
- [x] Temperature JSON publishing verified.
- [x] Alert JSON publishing verified.
- [x] Actual broker receipt of both JSON messages verified.
- [x] MQTT cleanup tested.

### 7.4 Remaining Development Work

- [ ] Extend sensor tests with timestamp validation and additional edge cases.
- [ ] Review the provisional temperature input range after hardware confirmation.
- [ ] Implement the approved MQTT reconnection policy.
- [ ] Add MQTT connection-state and delivery handling as needed.
- [ ] Integrate all application modules into the main application.
- [ ] Integrate the application into Buildroot.
- [ ] Integrate the required MQTT client library into Buildroot.
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

The sensor source and test program compiled successfully. Runtime tests completed successfully.

**Recorded test result:**

```text
=== All sensor tests passed ===
```

#### Sensor Module Limitations

- Physical sensor backend has not been implemented.
- Application has not yet been integrated into QEMU.
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
- Provisional temperature range validation from −40°C to 85°C inclusive.

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
- Valid temperature range boundaries.
- Out-of-range temperature rejection.

Compilation and execution completed successfully.

**Recorded test output:**

```text
=== IoT Gateway Processor Tests ===

PASS: Normal temperature classification.
PASS: Warning temperature classification.
PASS: Critical temperature classification.
PASS: Valid temperature range boundaries.
PASS: Out-of-range temperature rejection.
PASS: NULL argument validation.
PASS: Invalid sensor sample rejection.
PASS: NaN and infinity rejection.

=== All processor tests passed ===
```

#### Processor Module Limitations

- The accepted temperature range is provisional and must be reviewed after the physical sensor is confirmed.
- Complete application integration has not been performed.
- Testing has been performed on the Ubuntu host.
- Integration with the Buildroot target has not been performed.

### 8.3 Alert Generation Module

**Status: IMPLEMENTED AND TESTED**

Implemented an independent alert-generation module.

**Files created:**

- `application/include/alert.h`
- `application/src/alerts/alert.c`
- `application/tests/test_alert.c`

**Implemented functionality:**

- Normal temperature results generate no alert.
- Warning results generate warning alert events.
- Critical results generate critical alert events.
- Alert events preserve temperature, timestamp and sensor source.
- NULL arguments and invalid temperature status values are rejected.

**Alert generation outcomes:**

- `ALERT_GENERATION_ERROR`
- `ALERT_GENERATION_NONE`
- `ALERT_GENERATION_CREATED`

**Verification:** The alert module compiled successfully using strict C11 warning flags:

`-std=c11 -Wall -Wextra -Wpedantic -Werror`

All five alert test groups passed on the Ubuntu host.

**Recorded test output:**

```text
=== IoT Gateway Alert Tests ===

PASS: Normal temperature generates no alert.
PASS: Warning alert generated correctly.
PASS: Critical alert generated correctly.
PASS: NULL argument validation.
PASS: Invalid status rejected.

=== All alert tests passed ===
```

**Limitation:** Alert events are generated in memory. Console notification and complete main-application integration have not yet been performed.

### 8.4 Local CSV Logging Module

**Status: IMPLEMENTED AND TESTED**

Implemented a local CSV logging module for recording processed temperature readings and generated alerts.

**Files created:**

- `application/include/logger.h`
- `application/src/logging/logger.c`
- `application/tests/test_logger.c`

**Implemented functionality:**

- Initializes a CSV log file using a configurable file path.
- Creates the CSV header when the file is empty.
- Appends temperature readings without overwriting existing records.
- Records timestamps, temperature values, sensor source and temperature status.
- Records warning and critical alert events.
- Flushes log records after each write.
- Validates NULL arguments, sensor sources, temperature statuses and alert levels.
- Closes the log file safely.

**CSV header:**

```text
timestamp,temperature_c,source,status,alert_level
```

**Verification:** The logger test program compiled successfully using strict C11 warning flags:

`-std=c11 -Wall -Wextra -Wpedantic -Werror`

All logger tests passed on the Ubuntu host, including CSV content verification.

**Test log location:**

`/tmp/iot_gateway_test.csv`

**Limitation:**

- The current test uses temporary host storage.
- Persistent logging within the Buildroot gateway has not yet been integrated or verified.
- Log rotation and retention policy remain pending.
- The final target-device log path has not been selected.

### 8.5 MQTT Communication Module

**Status: IMPLEMENTED AND HOST-SIDE COMMUNICATION VERIFIED**

#### Files Created

- `application/include/mqtt.h`
- `application/src/mqtt/mqtt.c`
- `application/tests/test_mqtt.c`

#### Development Dependencies

Installed Ubuntu packages:

- `mosquitto`
- `mosquitto-clients`
- `libmosquitto-dev`

Verified versions:

- Mosquitto broker: 2.0.11.
- libmosquitto: 2.0.11.
- `libmosquitto-dev`: 2.0.11-1ubuntu1.2.

The broker service was verified as active.

The library was verified through `pkg-config`, and the header `/usr/include/mosquitto.h` was found.

#### Implemented Functionality

The MQTT module currently provides:

- `mqtt_init()`: initializes the Mosquitto library, creates the client and connects to the broker.
- `mqtt_publish_temperature()`: publishes processed temperature readings as JSON.
- `mqtt_publish_alert()`: publishes warning and critical alert events as JSON.
- `mqtt_cleanup()`: disconnects and releases MQTT resources.

The module uses:

- Reading topic: `iot/temperature/readings`.
- Alert topic: `iot/temperature/alerts`.
- QoS: 1.
- Retain: disabled.
- Keepalive: 60 seconds.

The implementation validates arguments, checks enum values, rejects non-finite temperatures, checks JSON formatting results and checks MQTT API return codes.

#### MQTT Compilation Verification

The MQTT source compiled independently using:

```bash
cd /home/sahithi/OS_Project

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
-Iapplication/include \
$(pkg-config --cflags libmosquitto) \
-c application/src/mqtt/mqtt.c \
-o /tmp/mqtt.o
```

Compilation completed without errors or warnings.

The generated object file was inspected:

`/tmp/mqtt.o`

Observed file size: approximately 5.2 KB.

#### MQTT Test Compilation

The test program was compiled and linked using:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
-Iapplication/include \
application/tests/test_mqtt.c \
application/src/mqtt/mqtt.c \
-o /tmp/test_mqtt \
$(pkg-config --cflags --libs libmosquitto) \
-pthread -lm
```

#### MQTT Test Results

The test program was executed:

```bash
/tmp/test_mqtt
```

Recorded output:

```text
=== IoT Gateway MQTT Tests ===
PASS: Uninitialized client correctly rejected.
PASS: Connected to MQTT broker.
PASS: Temperature reading published.
PASS: Warning alert published.
PASS: Invalid temperature argument rejected.
PASS: MQTT cleanup completed.
=== MQTT tests completed ===
```

#### Actual Broker Message Verification

A Mosquitto subscriber listened to:

- `iot/temperature/readings`
- `iot/temperature/alerts`

Both JSON messages were received.

**Temperature reading received:**

```json
{
  "timestamp": 1795000000,
  "temperature_c": 30.00,
  "source": "SIMULATED",
  "status": "WARNING"
}
```

**Alert received:**

```json
{
  "timestamp": 1795000000,
  "temperature_c": 30.00,
  "source": "SIMULATED",
  "alert_level": "WARNING"
}
```

The actual subscriber output was:

```text
iot/temperature/readings {"timestamp":1795000000,"temperature_c":30.00,"source":"SIMULATED","status":"WARNING"}
iot/temperature/alerts {"timestamp":1795000000,"temperature_c":30.00,"source":"SIMULATED","alert_level":"WARNING"}
```

**Verification result:** Both messages were successfully received by the local Mosquitto broker subscriber.

#### MQTT Module Limitations

- Automatic reconnection is not implemented yet.
- The approved five-second retry policy remains to be implemented.
- Connection-state handling and delivery confirmation handling need further work.
- Host-side testing has been completed; QEMU MQTT testing remains pending.
- MQTT library integration into Buildroot remains pending.
- The broker address `localhost` is for host-side testing. Inside QEMU, `localhost` refers to the guest itself, not the Ubuntu host.
- QEMU-to-host broker networking must be configured and verified separately.
- MQTT credentials and transport security have not yet been configured.
- Main-application integration has not yet been performed.

---

## 9. Project Directory Structure

The current application directory structure is:

```text
application/
├── include/
│   ├── sensor.h
│   ├── processor.h
│   ├── alert.h
│   ├── logger.h
│   └── mqtt.h
├── src/
│   ├── alerts/
│   │   └── alert.c
│   ├── logging/
│   │   └── logger.c
│   ├── mqtt/
│   │   └── mqtt.c
│   ├── processing/
│   │   └── processor.c
│   └── sensor/
│       └── sensor.c
└── tests/
    ├── test_sensor.c
    ├── test_processor.c
    ├── test_alert.c
    ├── test_logger.c
    └── test_mqtt.c
```

The individual modules have been implemented and tested on the Ubuntu host. They have not yet been integrated into a complete main application.

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
| Sensor source compilation with strict GCC flags | SUCCESSFUL |
| Sensor test program compilation | SUCCESSFUL |
| Sensor runtime test execution | SUCCESSFUL |
| Sensor invalid-pointer tests | SUCCESSFUL |
| Processor source compilation with strict GCC flags | SUCCESSFUL |
| Processor object-file inspection | SUCCESSFUL |
| Processor test program compilation | SUCCESSFUL |
| Processor automated test execution | SUCCESSFUL |
| Processor boundary tests | SUCCESSFUL |
| Processor invalid-input tests | SUCCESSFUL |
| Alert module compilation and tests | SUCCESSFUL |
| Logger module compilation and tests | SUCCESSFUL |
| Logger CSV content verification | SUCCESSFUL |
| Mosquitto installation | SUCCESSFUL |
| Mosquitto service activation | SUCCESSFUL |
| Mosquitto local publish/subscribe test | SUCCESSFUL |
| libmosquitto development library installation | SUCCESSFUL |
| libmosquitto pkg-config verification | SUCCESSFUL |
| Mosquitto header verification | SUCCESSFUL |
| MQTT source compilation | SUCCESSFUL |
| MQTT object-file inspection | SUCCESSFUL |
| MQTT test program compilation and linking | SUCCESSFUL |
| MQTT client connection | SUCCESSFUL |
| MQTT temperature publishing | SUCCESSFUL |
| MQTT alert publishing | SUCCESSFUL |
| Actual MQTT broker subscriber receipt | SUCCESSFUL |
| MQTT cleanup test | SUCCESSFUL |
| Git whitespace validation (`git diff --check`) | SUCCESSFUL in previously reported verification |

The final exit status of the resumed Buildroot `make` command was not separately captured. The generated image artifacts and successful QEMU boot provide independent evidence of a working kernel and root filesystem.

Sensor, processor, alert, logger and MQTT tests were executed on the Ubuntu host. They have not yet been executed inside QEMU as an integrated application.

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
| Temperature range boundary tests | PASS |
| Out-of-range temperature rejection | PASS |
| NULL processor argument tests | PASS |
| Invalid sensor sample rejection | PASS |
| NaN rejection | PASS |
| Infinity rejection | PASS |
| Temperature preservation | PASS |
| Timestamp preservation | PASS |
| Sensor source preservation | PASS |
| Alert generation tests | PASS — five test groups |
| Local logging tests | PASS — CSV content verified |
| Mosquitto broker service | PASS |
| libmosquitto development dependency | PASS |
| MQTT source compilation | PASS |
| MQTT test program | PASS |
| MQTT broker connection | PASS |
| Temperature JSON publishing | PASS |
| Alert JSON publishing | PASS |
| Actual broker message receipt | PASS |
| MQTT cleanup | PASS |
| Automatic MQTT reconnection | NOT IMPLEMENTED |
| Main application integration | PENDING |
| Buildroot application integration | PENDING |
| QEMU application integration testing | PENDING |
| QEMU MQTT testing | PENDING |
| Automatic application startup | NOT RUN |
| Security verification | NOT RUN |
| Benchmark measurements | NOT RUN |
| Physical sensor testing | BLOCKED — hardware pending |

The sensor, processor, alert, logger and MQTT modules have passed their reported Ubuntu host-side tests.

Complete application integration and end-to-end gateway testing remain pending.

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
- MQTT has been verified on the Ubuntu host but not inside QEMU.
- The approved MQTT reconnection policy has not been implemented.
- MQTT authentication and transport security have not been configured.
- Node-RED has not been installed; dashboard development is deferred until the core gateway and MQTT communication are integrated.
- End-to-end testing of the complete gateway has not yet been performed.

---

## 13. Remaining Work and Development Roadmap

### 13.1 Sensor and Processing

1. Extend sensor tests with timestamp validation and additional edge cases.
2. Review the acceptable temperature range after the physical sensor is confirmed.
3. Preserve the interchangeable sensor backend architecture.

### 13.2 Alert Generation

4. Alert-generation module implemented.
5. Warning and critical alert event generation verified.
6. Alert module tests completed.
7. Decide and implement duplicate-alert handling policy if required.

### 13.3 Local Logging

8. Local CSV logging module implemented.
9. Timestamp, temperature, classification and sensor source recording implemented.
10. File-access and write error handling implemented.
11. Logger tests and CSV content verification completed.
12. Determine persistent target-device logging path.
13. Define log rotation and retention policy.
14. Integrate logging with the main application.

### 13.4 MQTT Communication

15. MQTT topics and JSON payload schema finalized.
16. QoS 1 and retain-disabled configuration finalized.
17. Mosquitto broker and libmosquitto development environment installed and verified on Ubuntu.
18. MQTT publishing module implemented.
19. Host-side MQTT publishing and broker receipt verified.
20. Implement approved automatic reconnection policy with five-second retry interval.
21. Ensure local CSV logging continues while the broker is unavailable.
22. Investigate libmosquitto integration with Buildroot.
23. Integrate MQTT publishing into the main application.
24. Test communication between the QEMU gateway and broker.
25. Review MQTT authentication and transport security.

### 13.5 Application Integration

26. Integrate sensor acquisition, processing, alerts, logging and MQTT into the main application.
27. Compile and test the complete application on Ubuntu.
28. Cross-compile the application for the Buildroot target.
29. Integrate the application and required libraries into the root filesystem.
30. Configure automatic application startup.

### 13.6 Security and Testing

31. Implement at least three security-hardening measures.
32. Verify each security measure with recorded evidence.
33. Perform complete functional testing inside QEMU.
34. Test MQTT communication between the QEMU gateway and broker.
35. Test application error handling and reconnection behaviour.
36. Verify that local logging continues during broker outages.

### 13.7 Evaluation and Demonstration

37. Measure image size, boot time, RAM and CPU usage.
38. Establish a suitable stock Linux baseline.
39. Compare the customized distribution against the baseline.
40. Document the build process and testing methodology.
41. Prepare the final project demonstration.

### 13.8 Visualization

42. Install and configure Node-RED after MQTT communication is integrated.
43. Create a temperature-monitoring dashboard.
44. Display live readings, temperature classifications, alerts and historical graphs.

### 13.9 Physical Hardware Integration

45. Identify the exact physical board and sensor.
46. Confirm electrical compatibility and supported interfaces.
47. Configure the required kernel drivers and device tree.
48. Implement the hardware sensor backend.
49. Deploy the appropriate image and perform physical sensor testing.

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
- Host-side tests, QEMU tests and physical hardware tests are recorded separately.
- MQTT host-side broker verification is not treated as QEMU MQTT verification.
- Git commits and pushes are recorded only after their successful execution has been confirmed.

**Current milestone:** Individual C modules and host-side MQTT communication verified.

**Next major task:** Implement and test the approved MQTT reconnection policy, then integrate the modules into the main C application.
