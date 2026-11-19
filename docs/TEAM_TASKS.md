# Team Tasks and Ownership

**Team:** Four beginners  
**Project:** Track B IoT Temperature-Monitoring Gateway  
**Coordination rule:** One shared repository, one common architecture, and one authoritative `PROJECT_STATUS.md`.

## 1. Responsibility overview

| Member | Environment | Primary responsibility | Main deliverables |
|---|---|---|---|
| Member 1 | Native Ubuntu 22.04 | Primary Buildroot, kernel, QEMU build and integration | Defconfig, kernel config, rootfs integration, boot evidence |
| Member 2 | WSL 2.6.3.0 | C application and simulation | Sensor API, simulator, validation, thresholds, tests |
| Member 3 | WSL 2.6.3.0 | MQTT, logging/startup support, security test contributions | MQTT module/config, broker test procedure, security evidence |
| Member 4 | WSL 2.6.3.0 | Requirements, test plan, benchmarks, documentation/demo | Report files, test matrix, benchmark records, demo script |

Member 1 owns primary image integration, but all members contribute to the single product. WSL is not a separate implementation.

## 2. Member 1 — Primary build and integration

**Owns**
- `buildroot/configs/`
- `kernel/config/`
- Buildroot integration scripts and primary QEMU launch script.
- Rootfs overlay integration, coordinating with Member 3.
- Integration decisions recorded in `PROJECT_STATUS.md`.

**Tasks**
1. Inspect native Ubuntu host and record tool versions/resources.
2. Compare build-system options and confirm/pin Buildroot release with the team.
3. Configure x86-64 QEMU target.
4. Build minimal kernel and root filesystem.
5. Record selected kernel options and package manifest.
6. Integrate application and MQTT dependencies into the image.
7. Configure startup and test boot in QEMU.
8. Measure guest/image metrics and preserve exact commands/logs.
9. Review changes affecting shared architecture or build configuration.

**Do not**
- Assume the physical board is x86-64.
- commit generated output/cache directories.
- change the sensor API or MQTT schema without agreement.
- flash physical hardware before its exact model and procedure are verified.

## 3. Member 2 — C application and sensor simulation

**Owns**
- `application/src/`
- `application/include/`
- `application/Makefile`
- Application unit tests under `tests/application/` (or another agreed path).

**Tasks**
1. Propose a small sensor abstraction API.
2. Implement deterministic simulation mode and optionally bounded varying readings.
3. Ensure all samples carry a source label.
4. Implement input validation and error handling.
5. Implement threshold classification using configuration.
6. Add tests for normal, warning, critical, invalid, and boundary values.
7. Build with warnings enabled and document compiler commands.
8. Keep hardware backend as a stub/interface until the actual board/sensor is known.

**Integration contract**
- Publish the proposed header/API early for review.
- Do not change data structures after MQTT/logging integration without notifying Members 1 and 3.
- The app must not describe simulator values as physical readings.

## 4. Member 3 — MQTT, logging, startup and security contributions

**Owns**
- `application/src/mqtt_client.c` and corresponding header/interface, coordinated with Member 2.
- `configs/mqtt.example.conf` or equivalent non-secret example configuration.
- `rootfs-overlay/etc/init.d/` startup script contribution, coordinated with Member 1.
- MQTT/security test notes.

**Tasks**
1. Review MQTT client library availability in the pinned Buildroot release.
2. Propose topic names and payload schema.
3. Set up broker/client test procedure on a reachable machine.
4. Implement or integrate MQTT publication and error handling.
5. Ensure logs and payloads include source mode.
6. Avoid committing credentials; use placeholders/environment/config with restrictive permissions.
7. Help implement security measures: least privilege, minimal services/packages, permissions, broker authentication/TLS where feasible.
8. Test startup and broker-unavailable behavior.

**Integration contract**
- Coordinate payload fields and sensor sample API with Member 2.
- Coordinate rootfs paths and init mechanism with Member 1.
- Do not edit Buildroot defconfig without agreement.

## 5. Member 4 — Requirements, testing, benchmarks and documentation

**Owns**
- `docs/`
- `benchmarks/` (raw evidence and tables)
- Project planning documents and demo script.
- Documentation sections of `PROJECT_STATUS.md`, coordinated with Member 1.

**Tasks**
1. Maintain requirements traceability.
2. Write overview, architecture, roadmap, build/test plan, and limitations.
3. Create test matrix covering simulation, thresholds, invalid readings, MQTT, startup, and security.
4. Record exact benchmark conditions and raw outputs.
5. Define stock Linux comparison methodology.
6. Prepare architecture diagram and demonstration sequence.
7. Maintain license/package manifest documentation with inputs from Member 1.
8. Check that all report claims have evidence.

**Integration contract**
- Never invent benchmark values or mark unexecuted tests as passed.
- Ask the responsible member for actual outputs before writing results.
- Do not alter application or Buildroot source files without coordinating with their owners.

## 6. Shared interfaces and ownership boundaries

| Shared item | Owner/coordinator | Required reviewers |
|---|---|---|
| Buildroot release and defconfig | Member 1 | All members |
| Kernel version/config | Member 1 | All members; hardware owner later |
| Sensor API/data structures | Member 2 | Members 1 and 3 |
| Threshold semantics/config | Member 2 | Members 3 and 4 |
| MQTT topics/payload | Member 3 | Members 2 and 4 |
| Startup script/rootfs path | Member 1 + Member 3 | Member 2 |
| Benchmark methodology | Member 4 | Member 1 |
| Project status and sync | Member 1 coordinates; all contribute | All members |

## 7. Parallel work plan

Can start in parallel after the common interface decisions:
- Member 1: host inspection, Buildroot preparation, QEMU target planning.
- Member 2: application skeleton, simulator, validation and threshold tests.
- Member 3: MQTT broker/client experiment and payload proposal.
- Member 4: requirements, test matrix, documentation skeleton, benchmark methodology.

Must wait for a shared decision:
- Exact Buildroot/kernel versions before final integration.
- Final sensor API before connecting modules.
- Final MQTT schema before claiming end-to-end compatibility.
- Physical board confirmation before board-specific kernel/device-tree/wiring work.

## 8. Conflict avoidance

- One owner edits each primary file group.
- Use feature branches named `memberN/short-task`.
- Pull/rebase from `main` before starting a new task.
- Commit small coherent changes.
- Do not force-push shared branches.
- Open a pull request or request review before merging shared interfaces/configurations.
- If a conflict affects architecture, kernel, Buildroot, MQTT schema, or sensor API, stop and resolve it through the Ubuntu primary decision process.
- Update `PROJECT_STATUS.md` with the decision and evidence.

## 9. Definition of done

A task is complete only when:
1. Its source/configuration is committed.
2. The responsible member records the exact command/test.
3. Actual output or evidence is saved.
4. Known limitations are documented.
5. Shared status is updated.
6. The other branch receives a SYNC SUMMARY if the change affects its work.
