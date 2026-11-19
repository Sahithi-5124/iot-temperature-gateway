# Build and Test Plan
## Track B — IoT Temperature-Monitoring Gateway

**Status:** Initial plan. Versions and hardware-specific settings remain PENDING until verified.

## 1. Build-system comparison

| Build approach | Strengths | Trade-offs for this project |
|---|---|---|
| Buildroot | Focused embedded Linux workflow; builds toolchain, kernel, rootfs and selected packages; relatively direct configuration | Less flexible than Yocto for complex product layers; configuration and package customization still require learning |
| Yocto Project | Highly customizable metadata/layers, recipes, reproducible product-oriented builds | More concepts and setup; likely higher learning/time cost for a beginner 10-hour project |
| Debian-based minimal/custom image | Familiar package ecosystem and quick access to packages | Less focused on producing a tightly controlled embedded image from a unified build configuration |

**Proposed selection: Buildroot — PROVISIONAL.** Confirm after checking assignment requirements, host dependencies, package availability, and available time. Once approved, pin one exact release for both branches. Do not mix Buildroot releases across environments.

## 2. Host/build environment

Primary build host: native Ubuntu 22.04. Parallel development: WSL 2.6.3.0.

Before installation, inspect:
- OS release and kernel/architecture.
- CPU count and memory.
- Free disk space.
- Git, GCC, Make, QEMU versions.
- WSL version and distribution details on Windows machines.

Keep WSL source/build trees inside the WSL Linux filesystem (for example `/home/<user>/iot-temperature-gateway`) for Linux build workloads. QEMU terminal/serial mode is the first option if GUI support is unavailable.

Do not install the custom kernel as the host kernel. Buildroot outputs remain project artifacts.

## 3. Initial target and kernel configuration

**Phase 1 target:** x86-64 QEMU virtual machine (PROVISIONAL).

Kernel configuration should include only what the virtual target requires:
- x86-64 architecture and boot support.
- Console/serial console suitable for QEMU testing.
- Virtual block device support required by selected QEMU storage configuration.
- Selected root filesystem support (for example ext4 only if that is the chosen image/rootfs format).
- Networking and the virtual NIC driver required by QEMU networking mode.
- Essential kernel facilities required by selected userspace/init.

Do not enable board-specific GPIO/device-tree/sensor options in the x86-64 configuration merely because a future board may need them.

### Physical target later

After board confirmation, revisit:
- Architecture and cross toolchain.
- Board defconfig and kernel version compatibility.
- Bootloader and boot medium.
- Device tree and overlays.
- GPIO controller and pin mapping.
- Sensor driver and kernel/user interface.
- Board-specific storage/network/console support.

A configured driver is not physically verified until tested on the real target.

## 4. Userland/package selection

Candidate minimal components:
- BusyBox for core utilities and possibly init.
- C monitoring application.
- MQTT client library (libmosquitto is a candidate).
- CA certificates if TLS is enabled.
- Minimal network configuration/diagnostic tools.
- Required runtime libraries only.
- A writable data/log location.

Avoid desktop packages, large runtimes, containers, and unrelated services unless a requirement justifies them. Final package list must come from the actual Buildroot configuration and generated manifest.

## 5. Application build plan

Proposed modules:
- Sensor abstraction and source selection.
- Simulation backend.
- Future hardware backend.
- Validation/normalization.
- Threshold classification.
- Alert generation.
- Logging.
- MQTT publication.
- Main loop/configuration.

Build on the host first using the agreed compiler and warning flags. Then cross-compile or integrate into Buildroot for the target. Record compiler, flags, library dependencies, and exact commands.

Tests:
1. Normal values.
2. Warning values.
3. Critical values.
4. Exactly-at-threshold values.
5. Values just below/above thresholds.
6. Invalid/out-of-range/non-finite input.
7. Sensor read error.
8. MQTT broker unavailable/reconnect.
9. Logging failure or unwritable path.
10. Simulation source label present in all relevant output.

Threshold values and inclusive/exclusive boundary rules must be defined in configuration/documentation before tests are called pass/fail.

## 6. Sensor simulation test plan

Use deterministic scenarios for repeatability. Example values 25°C, 30°C, 36°C, and 42°C are illustrative only. The simulator must mark every sample as `SIMULATED`.

A second optional mode may produce bounded varying values, but deterministic tests remain the basis for threshold verification.

Do not claim that QEMU reads the physical sensor. QEMU tests the software path unless an actual sensor/interface model has been added and verified.

## 7. MQTT test plan

Provisional broker: Mosquitto. Provisional client library: libmosquitto.

Test sequence:
1. Start broker on a reachable host or machine.
2. Subscribe to the proposed reading and alert topics.
3. Run application in simulation mode.
4. Verify payload schema, source field, timestamp, temperature, and status.
5. Test broker unavailable and reconnect behavior.
6. Test authentication if configured.
7. Test TLS only if certificates and time permit.
8. Repeat from QEMU guest to validate guest networking.

Do not commit credentials. Record broker version, client version, address/network mode, topic, QoS, and exact test command. A host-only MQTT test and a guest MQTT test are distinct results.

## 8. Startup and root filesystem test plan

- Confirm the selected init system and its startup sequence.
- Add the application and config to the rootfs overlay.
- Add an init script or equivalent service start mechanism.
- Confirm executable permissions and required directories.
- Boot QEMU and verify the app starts without manual intervention.
- Verify logs and writable data location.
- Test behavior when the broker is unavailable.

Read-only root filesystem and writable overlay/data partition are stretch goals. First ensure a reliable writable logging path and successful boot.

## 9. Security test plan

Implement and verify at least three measures, selected from:
1. Minimal package/service set; confirm unnecessary services are absent/disabled.
2. Dedicated unprivileged application account; confirm process UID and file permissions.
3. Restrictive permissions for configuration and logs; verify with filesystem inspection.
4. MQTT authentication and TLS where feasible; verify broker rejects unauthenticated access if enabled.
5. Restrict network exposure; inspect listening sockets and firewall/network configuration.
6. Read-only root filesystem with separate writable data/log area, if stable.
7. No secrets in Git, image defaults, or logs.

Document the threat each measure addresses and the actual verification evidence. Do not count a planned measure as implemented.

## 10. QEMU test plan

Record:
- QEMU version.
- Machine type.
- vCPU count.
- RAM allocation.
- Image format and size.
- Disk/controller configuration.
- Networking mode and host forwarding, if used.
- Console mode.
- Exact launch command.

Verify:
- Guest boots to a usable console.
- Root filesystem mounts.
- Application starts automatically.
- Simulated readings and classifications appear.
- Logs are written.
- MQTT publication reaches broker if networking is configured.
- Guest survives repeated boot.

QEMU is a virtual platform. Its boot, memory, and CPU results are not equivalent to physical-board results.

## 11. Performance and footprint evaluation

Measure actual values; never invent or estimate a result and label it measured.

| Metric | Suggested method | Record conditions |
|---|---|---|
| Image size | `stat` or `du` on final image/artifact | File type, sparse/compressed status |
| Kernel size | `stat` on kernel artifact | Exact artifact |
| Rootfs size | Buildroot output and filesystem size tools | Used vs allocated size |
| Boot time | Timestamped serial/console capture or repeatable boot measurement | Start/end event definitions; number of runs |
| RAM | Guest `/proc/meminfo`, QEMU allocation, and tool used | Guest vs host memory clearly distinguished |
| CPU | Guest process/system tools or host QEMU process tools | Guest/host distinction and interval |
| Application processing time | Monotonic clock around defined processing path | Sample count, warm-up, compiler flags |
| MQTT latency | Timestamp publish/receive or request/response method | Network path, QoS, broker location |
| Package count | Buildroot configuration/package manifest | Define what is counted |

Report median/min/max or average and spread when repeated measurements are feasible. Preserve raw outputs and commands.

## 12. Stock Linux baseline methodology

Select a stock Linux baseline with the same architecture and, as far as practical, comparable QEMU machine type, vCPU count, RAM, storage, and measurement method.

Possible baseline: a minimal stock distribution or a standard distribution image suitable for the same x86-64 QEMU setup. The exact distribution/image and version are PENDING.

Compare:
- Disk/image size, while distinguishing compressed download size from expanded filesystem size.
- Kernel and root filesystem size where comparable.
- Idle memory under clearly defined conditions.
- Boot time using the same start/end definitions.
- CPU under the same workload.
- Application workload performance where the same binary/dependencies can be used.

Avoid misleading comparisons:
- Do not compare a compressed installer download with an uncompressed Buildroot image.
- Do not compare different architectures as if equivalent.
- Do not compare host RAM with guest RAM.
- Do not claim a percentage reduction unless numerator, denominator, and measurement method are explicit.
- If a fair baseline cannot be obtained within the deadline, document the limitation rather than fabricate a number.

## 13. Reproducibility

Preserve:
- Exact Buildroot release and source checksum/commit.
- Defconfig and kernel configuration.
- Rootfs overlay files.
- Package manifest.
- Compiler/toolchain versions.
- Build scripts and environment notes.
- QEMU command line.
- Test commands and raw outputs.
- Benchmark conditions.
- Git commit hashes.

Do not commit large generated output trees or caches by default. Use a release artifact/shared storage if the team needs to exchange images.

## 14. Physical hardware test plan

After the board and sensor arrive:
1. Record exact board model, revision, processor, RAM, boot medium, and supported OS/build documentation.
2. Record exact sensor model and datasheet.
3. Confirm supply voltage, logic levels, pinout, and current requirements.
4. Confirm GPIO pin mapping and whether pins are 5-V tolerant; never assume.
5. Confirm the Linux kernel driver/interface and device-tree configuration.
6. Build a board-specific image; do not reuse x86-64 QEMU image as if it were board-compatible.
7. Wire with power disconnected where appropriate and follow board/sensor documentation.
8. Verify sensor interface separately before integrating the full application.
9. Implement hardware backend behind the existing sensor interface.
10. Test real readings and explicitly mark source `HARDWARE`.
11. Test thresholds, logs, MQTT, and boot startup with real data.
12. Measure physical boot, RAM, CPU, image size, and application performance separately.

## 15. Reporting rule

Every result must be labelled one of:
- PLANNED
- NOT RUN
- PASSED (with evidence)
- FAILED (with evidence)
- BLOCKED
- NOT APPLICABLE

Simulation, QEMU, and physical hardware results must be kept distinct.
