# PROJECT_STATUS.md
## Authoritative Shared Status — Track B IoT Temperature Gateway

**Last updated:** 2026-11-19  
**Status:** Planning baseline; implementation not yet verified  
**Rule:** This file is the shared source of truth for both ChatGPT chats and all four team members.

## 1. Project goal

Build a lightweight application-specific Linux distribution for an IoT temperature-monitoring gateway. It will collect temperature samples, validate/process them, classify threshold conditions, generate alerts, log readings, and publish readings/alerts through MQTT.

Phase 1 uses explicitly labelled simulated readings and an x86-64 QEMU target. Phase 2 adds the confirmed physical board and sensor.

## 2. Agreed decisions

| Decision | Current value | Status |
|---|---|---|
| Project track | Track B — IoT Gateway | AGREED |
| Team size | 4 | AGREED |
| Initial time budget | Approximately 10 hours | AGREED |
| Primary build environment | Native Ubuntu 22.04 | AGREED |
| Parallel environment | Windows + WSL 2.6.3.0 | AGREED |
| Build system | Buildroot | PROVISIONAL pending final comparison/version pin |
| Initial target | x86-64 QEMU guest | PROVISIONAL Phase 1 target |
| Application language | C | AGREED direction |
| Sensor architecture | Pluggable simulation/hardware backends | AGREED design requirement |
| Initial sensor mode | Simulation, explicitly labelled | AGREED |
| Physical sensor | DHT11 preferred | PENDING confirmation |
| Physical board | Unknown | PENDING |
| MQTT | Required | AGREED |
| MQTT library | libmosquitto candidate | PROVISIONAL |
| MQTT broker | Mosquitto candidate | PROVISIONAL |
| Init | BusyBox init candidate | PROVISIONAL |
| Root filesystem | Minimal Buildroot-generated rootfs | PROVISIONAL |
| QEMU | Initial virtual testing | AGREED |
| Track C | Only optional after Track B complete | AGREED scope rule |

## 3. Pending decisions

- [ ] Final build-system comparison and approval.
- [ ] Exact Buildroot release.
- [ ] Exact Linux kernel version.
- [ ] Exact host dependency versions.
- [ ] QEMU machine type and image format.
- [ ] MQTT topic names, payload schema, QoS, and reconnect policy.
- [ ] Warning/critical threshold values and boundary semantics.
- [ ] Logging backend/path and rotation policy.
- [ ] Final sensor API and data structures.
- [ ] Physical board model/revision/architecture.
- [ ] Physical sensor exact model and electrical details.
- [ ] Board-specific kernel defconfig, device tree, bootloader, GPIO and driver.

## 4. Environments

| Member | Host | Development environment | Role |
|---|---|---|---|
| 1 | Windows + dual boot | Native Ubuntu 22.04 | Primary build/integration |
| 2 | Windows | WSL 2.6.3.0 | Application/simulation |
| 3 | Windows | WSL 2.6.3.0 | MQTT/security/startup contributions |
| 4 | Windows | WSL 2.6.3.0 | Tests/docs/benchmarks |

Project paths:
- Ubuntu: `~/iot-temperature-gateway` (proposed).
- WSL: `/home/<user>/iot-temperature-gateway` (proposed; keep build tree in Linux filesystem).

Actual paths are PENDING until the repository is created.

## 5. Version record

| Component | Version | Status |
|---|---|---|
| Ubuntu | 22.04 | Required host |
| WSL | 2.6.3.0 | Reported; verify per machine |
| Buildroot | PENDING | Not pinned |
| Linux kernel | PENDING | Not pinned |
| QEMU | PENDING | Not inspected |
| Git | PENDING | Not inspected |
| GCC/toolchain | PENDING | Not inspected |
| Make | PENDING | Not inspected |
| MQTT library | PENDING | Not pinned |
| MQTT broker | PENDING | Not pinned |

Do not infer versions from documentation or planned commands. Record actual command output.

## 6. Current milestone

**M0 — Shared project planning and documentation baseline.**

This document and companion planning files are being prepared. No implementation milestone is claimed complete.

## 7. Completed work

- [x] Track B selected as project scope.
- [x] Simulation-first approach agreed.
- [x] Native Ubuntu primary / WSL parallel workflow agreed.
- [x] One shared Git repository and status-file workflow specified.
- [x] Initial architecture and roadmap documented.
- [ ] Repository actually created and files committed.
- [ ] Environment inspection completed.
- [ ] Build dependencies installed.
- [ ] Buildroot configured/built.
- [ ] Application implemented/tested.
- [ ] MQTT integration tested.
- [ ] QEMU boot tested.
- [ ] Benchmarks measured.
- [ ] Physical hardware integrated.

## 8. Files created

Initial documentation set planned:
- `PROJECT_OVERVIEW.md`
- `PROJECT_ARCHITECTURE.md`
- `PROJECT_ROADMAP.md`
- `TEAM_TASKS.md`
- `PROJECT_STATUS.md`
- `BUILD_AND_TEST_PLAN.md`
- `TEAM_SYNC_PROTOCOL.md`

Record actual additions/changes in Git after creation. No source/build files are yet verified.

## 9. Commands tested and results

**None recorded.** This is intentional: no installation, build, boot, or benchmark commands have been executed as part of this planning baseline.

| Date | Environment | Command | Actual output/result | Evidence file |
|---|---|---|---|---|
| | | | | |

## 10. Test results

No application, MQTT, QEMU, security, or benchmark tests have been run yet.

| Test | Status | Evidence |
|---|---|---|
| Simulator scenarios | NOT RUN | — |
| Threshold boundaries | NOT RUN | — |
| Invalid input | NOT RUN | — |
| MQTT publish/subscribe | NOT RUN | — |
| QEMU boot | NOT RUN | — |
| Automatic startup | NOT RUN | — |
| Security controls | NOT RUN | — |
| Image/boot/RAM/CPU measurements | NOT RUN | — |
| Stock Linux comparison | NOT RUN | — |
| Physical sensor | BLOCKED: hardware pending | — |

## 11. Problems and limitations

- Physical board and sensor are not confirmed.
- Exact software versions are not pinned.
- First Buildroot build duration and storage requirement are unknown until environment inspection.
- WSL QEMU display and networking behavior must be verified on the actual machines.
- QEMU does not inherently emulate the physical DHT11 or target-board GPIO.
- QEMU benchmark values cannot be presented as physical-board measurements.

## 12. Next steps

1. Create the Git repository and add the seven shared documents.
2. Start the Ubuntu and WSL ChatGPT branches using their initialization prompts.
3. Perform environment inspection in each environment.
4. Compare results and resolve host-specific issues.
5. Complete build-system comparison and pin Buildroot/kernel versions.
6. Agree the sensor API, threshold semantics, MQTT schema, and configuration format.
7. Begin parallel application/MQTT/docs work while Member 1 prepares the primary image.

## 13. Synchronization notes

- Ubuntu chat owns primary build integration and proposes project-wide technical decisions.
- WSL chat follows approved decisions and reports environment-specific constraints.
- Neither chat may assume the other has executed a task.
- A task is complete only with actual execution/test evidence.
- At each milestone, create a dated SYNC SUMMARY and update this file.
- If a summary conflicts with this file, pause implementation, compare evidence and commit history, then explicitly resolve the discrepancy. Do not silently choose one version.
