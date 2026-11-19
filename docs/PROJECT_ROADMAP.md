# Project Roadmap
## Approximately 10-hour Track B implementation

**Planning status:** Estimates are provisional. Build downloads and compilation may take longer depending on network, CPU, RAM, and storage.

## Guiding strategy

Deliver a working, honest software demonstration first. Build one x86-64 QEMU target for Phase 1. Keep the application modular so physical hardware can be integrated later. Do not spend the initial 10 hours maintaining multiple board targets or adding optional Track C work.

## Phase 0 — Decisions and preparation (0.5 hour)

**Tasks**
- Confirm Track B scope and functional requirements.
- Record provisional Buildroot and x86-64 QEMU direction.
- Establish Git repository, directory structure, and `PROJECT_STATUS.md`.
- Assign file ownership and module interfaces.
- Check available disk space and network access.

**Dependencies:** None.

**Milestone M0:** Shared decisions and repository plan documented.

**Minimum viable outcome:** One common architecture and no conflicting branch decisions.

**Fallback:** If the board remains unknown, continue with x86-64 QEMU and mark all physical-board details PENDING.

## Phase 1 — Build environment (0.75–1.25 hours)

**Tasks**
- Inspect native Ubuntu 22.04 and WSL environments separately.
- Verify architecture, CPU count, memory, free disk, Git, compiler, Make, and QEMU availability.
- Install required host dependencies only after inspection and approval.
- Pin the same Buildroot release and record host tool versions in both environments.

**Dependencies:** M0.

**Milestone M1/M2:** Both environments inspected; primary environment dependencies verified; Buildroot source/version selected.

**Minimum viable outcome:** Native Ubuntu can configure and start a Buildroot build. WSL can compile/test assigned application tasks.

**Fallback:** If WSL GUI/QEMU display is unavailable, use terminal/serial QEMU where supported or leave final QEMU integration to Ubuntu. WSL can still build and test C modules.

## Phase 2 — Custom Linux image (2.0–3.0 hours)

**Tasks**
- Obtain the pinned Buildroot release.
- Configure x86-64 QEMU target.
- Select kernel, BusyBox, root filesystem, networking, and only required packages.
- Add the application and configuration overlay when ready.
- Build kernel and root filesystem.
- Save the defconfig and build manifest.

**Dependencies:** Phase 1; sufficient disk and network.

**Milestone M3/M4:** Reproducible configuration and first successful bootable image.

**Minimum viable outcome:** A custom Linux image boots in QEMU with a shell and networking if feasible.

**Bottleneck warning:** First builds may take substantial time due to downloads and compilation. Do not promise exact completion time. Keep the build target small and avoid changing configuration during a long build.

**Fallback:** If the full image does not finish, demonstrate Buildroot configuration/build progress plus a separately compiled and tested C application, and clearly report the incomplete image. Do not claim a successful custom boot.

## Phase 3 — Application development (1.5–2.0 hours; parallelizable)

**Tasks**
- Define the shared sensor API and data structures.
- Implement simulation backend.
- Implement validation and threshold classification.
- Implement alerts and logging.
- Build with strict compiler warnings.
- Add unit tests for normal, warning, critical, invalid, and boundary cases.

**Dependencies:** Agreed API and C toolchain. Can start in parallel with Phase 2 after the interface is agreed.

**Milestone M5:** Application runs on host and produces labelled simulated output.

**Minimum viable outcome:** Repeatable test program with correct classification and source metadata.

**Fallback:** Use deterministic scenario input from a file or command-line test mode if a real-time simulator is not ready.

## Phase 4 — Integration (1.0–1.5 hours)

**Tasks**
- Add MQTT client and broker configuration.
- Test publication from host first, then from QEMU guest.
- Add startup script/init integration.
- Add application configuration to rootfs overlay.
- Confirm log location and writable storage.
- Add at least three practical security controls.

**Dependencies:** Working application and initial image.

**Milestone M6/M7/M8:** MQTT test passes; application starts in guest; security controls are documented and verified.

**Minimum viable outcome:** Application runs in guest and publishes a test reading to a reachable broker.

**Fallback:** If guest networking/MQTT integration blocks, demonstrate MQTT from host with the same application and separately show guest boot/application operation. Clearly label the split test and remaining integration.

## Phase 5 — Testing and evaluation (1.0–1.25 hours)

**Tasks**
- Boot and repeat QEMU test.
- Verify automatic startup.
- Test simulation scenarios and invalid input.
- Verify MQTT topics/payloads.
- Measure image size, boot time, guest RAM, CPU, and application timing.
- Select and test a stock Linux baseline with comparable architecture/resources.
- Save raw commands, outputs, and measurement conditions.

**Dependencies:** Working image and application.

**Milestone M9/M10:** Test evidence and measured benchmark table.

**Minimum viable outcome:** Reproducible test record with actual measured values and limitations.

**Fallback:** If the baseline cannot be configured fairly within the time, report the custom system measurements and document the baseline limitation; do not fabricate a comparison.

## Phase 6 — Documentation and demonstration (0.75–1.0 hour)

**Tasks**
- Update overview, architecture, roadmap, status, build/test instructions, and package manifest.
- Capture boot, simulated readings, logs, and MQTT subscriber output.
- Prepare a short demonstration flow.
- Record known limitations and future hardware tasks.
- Verify Git history and final repository contents.

**Dependencies:** Results from prior phases.

**Milestone M11:** Report-ready evidence and repeatable demo.

**Minimum viable outcome:** A truthful demonstration and report with actual outputs.

**Fallback:** Prioritize a concise live demo plus screenshots/logs and a limitations section over polishing optional features.

## Phase 7 — Physical hardware integration (after components arrive; outside initial 10-hour core)

**Tasks**
- Identify exact board, revision, architecture, sensor model, pinout, and voltage requirements.
- Review board and sensor documentation.
- Select board defconfig, kernel version, device tree, bootloader, and image format.
- Select/enable appropriate GPIO and sensor driver/interface.
- Wire safely with power disconnected as appropriate.
- Build board-specific image and boot from supported media.
- Implement hardware sensor backend behind the existing sensor API.
- Verify real readings, alerts, logs, MQTT, and startup.
- Measure physical-device results separately from QEMU.

**Dependencies:** Actual board and sensor.

**Milestone M12:** Physical sensor and gateway validated on the real target.

**Minimum viable outcome:** Real sensor readings are clearly distinguished from earlier simulated results.

**Fallback:** If hardware integration cannot be completed before submission, present the working simulation/QEMU system and document exactly which physical tests remain pending.

## Suggested 10-hour allocation

| Phase | Approx. time |
|---|---:|
| Phase 0 — Decisions/preparation | 0.5 h |
| Phase 1 — Environment | 1.0 h |
| Phase 2 — Image | 2.5 h |
| Phase 3 — Application (parallel) | 1.5 h |
| Phase 4 — Integration | 1.5 h |
| Phase 5 — Testing/evaluation | 1.0 h |
| Phase 6 — Documentation/demo | 1.0 h |
| Contingency | 1.0 h |
| **Total** | **10.0 h** |

The parallel application work overlaps with image building; the table is a coordination budget, not a promise that all tasks execute sequentially.

## Critical path

Environment and downloads → Buildroot configuration/build → first QEMU boot → app/rootfs integration → MQTT/startup → verification and evidence.

Application coding, documentation skeleton, test design, and MQTT protocol decisions can proceed in parallel after interfaces and common decisions are agreed.
