# Project Overview
## Application-Specific Linux Distribution for an IoT Temperature-Monitoring Gateway

**Course/project family:** Application-Specific Linux Distribution for ML, IoT, CUDA and FPGA Platforms  
**Selected scope:** Track B — IoT Gateway  
**Team:** 4 beginner-level members  
**Available project time:** Approximately 10 hours for the initial demonstrable implementation  
**Hardware status:** Physical board and sensor are not yet confirmed/available  
**Document status:** Initial planning baseline; unverified implementation decisions are marked PROVISIONAL or PENDING.

## 1. Abstract

This project aims to build a lightweight, application-specific Linux distribution for an IoT temperature-monitoring gateway. The gateway will obtain temperature readings from a sensor-input module, validate and process them, classify them against configurable thresholds, generate alerts, maintain logs, and publish readings or alerts to an MQTT broker.

Development is simulation-first. Before physical hardware arrives, a C application will consume explicitly identified simulated readings. The application will separate sensor acquisition from common processing so that a hardware-backed sensor module can later replace the simulator without rewriting threshold detection, logging, or MQTT logic.

A customized Linux image will be built using an embedded Linux build system, provisionally Buildroot, and initially targeted at an x86-64 QEMU virtual machine. This enables software integration and demonstration without modifying the laptops' host operating systems. The eventual physical-board image may require a different architecture, kernel configuration, device tree, bootloader, and image format.

## 2. Problem statement

A small IoT gateway needs to collect environmental measurements and reliably communicate useful information to other systems. A general-purpose desktop distribution may contain unnecessary packages and services for this focused role. The project investigates whether a compact Linux image can provide the required operating-system services and application functionality with a smaller, more controlled userspace.

The team also needs a development path that does not depend on receiving the sensor and target board at the start. A simulation-first design allows application logic, MQTT, logging, boot integration, and much of the Linux image to be developed and tested early.

## 3. Objectives

1. Analyze functional and non-functional requirements for a temperature-monitoring gateway.
2. Compare embedded Linux build approaches and justify a selection.
3. Build a customized Linux kernel and minimal root filesystem for an initial virtual target.
4. Develop a modular C application with interchangeable simulated and hardware sensor inputs.
5. Validate readings and classify them using configurable thresholds.
6. Generate normal, warning, and critical status/alerts.
7. Log readings and application events.
8. Publish readings and alerts through MQTT.
9. Start the application automatically during system boot.
10. Apply at least three practical security-hardening measures.
11. Boot and test the image in QEMU where feasible.
12. Measure image size, boot time, memory, CPU, and application/MQTT performance.
13. Compare results with a clearly defined stock-Linux baseline using a fair methodology.
14. Maintain reproducible configuration, source control, documentation, and a demonstration.
15. Integrate the physical board and sensor after their exact models are confirmed.

## 4. Proposed solution

The proposed system has these layers:

- **Sensor input:** simulation mode initially; hardware mode after board and sensor confirmation.
- **Application logic:** input validation, processing, threshold classification, alerts, and logging.
- **Communication:** MQTT client publishing to a broker.
- **Linux platform:** kernel, required drivers/interfaces, minimal root filesystem, init/startup, and networking.
- **Development/test platform:** native Ubuntu 22.04 for primary integration, Windows + WSL 2.6.3.0 for parallel work, and QEMU for the initial virtual target.

The simulated and hardware sensor modules must implement the same application-facing interface. The common application must not need to know whether a reading came from simulation or a physical sensor, except for explicit source metadata in logs and messages.

## 5. Track selection and justification

**Track B — IoT Gateway** is the selected project scope. It directly supports the temperature sensor, embedded Linux, sensor-facing interface, MQTT communication, application-specific userspace, and resource evaluation.

Track C or other tracks are out of scope for the 10-hour core project. An optional extension may be considered only after the Track B demonstrable system and required report are complete. It must not delay the gateway.

## 6. Technology stack

| Area | Proposed technology | Status |
|---|---|---|
| Build system | Buildroot | PROVISIONAL; justify and pin exact release |
| Initial virtual target | x86-64 QEMU | PROVISIONAL Phase 1 target |
| Primary build host | Native Ubuntu 22.04 | Agreed |
| Parallel development host | Windows + WSL 2.6.3.0 | Agreed |
| Application language | C | Preferred/agreed direction |
| Sensor | DHT11 preferred | PENDING exact model and availability |
| Initial sensor source | Deterministic or bounded simulated readings | Agreed Phase 1 approach |
| MQTT client | libmosquitto or another small supported client | PROVISIONAL |
| MQTT broker | Mosquitto, initially on host or reachable machine | PROVISIONAL |
| Init | BusyBox init | PROVISIONAL |
| Root filesystem | Buildroot-generated minimal root filesystem | PROVISIONAL |
| Virtual testing | QEMU system emulator | Planned |
| Version control | Git | Agreed |

Exact Buildroot, kernel, QEMU, compiler, MQTT library, and broker versions must be recorded after environment inspection. Do not silently use different versions in the two branches.

## 7. Expected inputs and outputs

### Inputs

- Temperature values from simulation in Phase 1.
- Temperature values from a physical sensor in Phase 2.
- Configuration for thresholds, sampling interval, log destination, MQTT broker, topic names, and simulation/hardware mode.
- Runtime events such as sensor-read errors or MQTT connection failures.

### Outputs

- Validated temperature samples with timestamps and source labels.
- Status classification: Normal, Warning, or Critical.
- Alert events when thresholds are crossed.
- Local logs.
- MQTT messages containing temperature, timestamp, status, source mode, and relevant alert information.
- Build artifacts: kernel, root filesystem, QEMU disk/image artifact, configuration, package manifest, scripts, and test evidence.
- Report, benchmark tables, architecture diagram, and demonstration recording.

## 8. Proposed functional features

- Selectable simulation/hardware sensor backend.
- Configurable sample interval.
- Input validation and invalid-reading handling.
- Configurable warning and critical thresholds.
- Status classification and alert generation.
- Local logging with explicit `SIMULATED` or `HARDWARE` source metadata.
- MQTT publication and reconnect/error handling.
- Automatic startup through the selected init mechanism.
- Graceful shutdown and signal handling where feasible.
- Unit/integration tests for boundary values and error cases.

Illustrative simulated values (not physical measurements):

| Simulated value | Illustrative classification |
|---:|---|
| 25°C | Normal |
| 30°C | Normal |
| 36°C | Warning |
| 42°C | Critical |

Threshold values are examples only. The team must define and record actual configured boundaries and boundary semantics before testing.

## 9. Non-functional requirements

- Small image and minimal package set.
- Reliable boot and automatic application startup.
- Clear failure behavior if the sensor or broker is unavailable.
- Reproducible build configuration.
- Simple, documented module interfaces.
- Basic security hardening.
- Measurable performance and footprint.
- Portability of application logic across simulation and hardware backends.

## 10. Constraints and assumptions

- Approximately 10 hours for the initial project implementation.
- Team members are beginners.
- Physical board and sensor are pending.
- Exact board architecture and boot chain are unknown.
- Native Ubuntu 22.04 is the primary integration environment.
- WSL 2.6.3.0 is a parallel development environment, not a separate product target.
- QEMU measurements are virtual-machine measurements and must not be presented as physical-device results.
- Hardware-specific GPIO, timing, electrical, device-tree, bootloader, and driver behavior cannot be fully verified without the actual board and sensor.
- Buildroot image compilation and downloads may be the schedule's largest bottleneck.

## 11. Safety and host protection

The custom Linux system is a guest image/artifact. Do not repartition laptops, replace host operating systems, install the custom kernel as the host kernel, or flash hardware until the exact target and procedure are verified. Keep project files in a dedicated repository directory. Explain destructive commands before execution.

## 12. Expected demonstration

Minimum viable demonstration:

1. Show the customized Linux image booting in QEMU.
2. Show the application starting automatically.
3. Show logs explicitly marked as simulated input.
4. Demonstrate normal, warning, and critical classifications.
5. Demonstrate invalid-input handling or a sensor error path.
6. Show MQTT messages arriving at the broker/subscriber.
7. Show the relevant image-size, boot-time, RAM, CPU, and application measurements.
8. Explain the stock Linux baseline and the limits of the comparison.
9. Clearly identify physical hardware integration as pending if it has not been completed.

A later hardware demonstration should additionally show the exact board/sensor, safe wiring, real readings, and board-specific test evidence.
