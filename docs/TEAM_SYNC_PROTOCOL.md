# Team Synchronization Protocol
## Ubuntu Primary Chat + WSL Parallel Chat

## 1. Purpose

The team uses two ChatGPT chats in one Project, plus one Git repository. The chats do not automatically share their complete conversation history, terminal outputs, or files. Therefore, synchronization must use committed files, `PROJECT_STATUS.md`, and explicit SYNC SUMMARIES.

## 2. Authority and decision ownership

**Ubuntu Primary Build chat** coordinates project-wide technical decisions and primary integration because Member 1 maintains the reference build.

It proposes and records decisions for:
- Build system and exact release.
- Target architecture and initial QEMU platform.
- Linux kernel version and configuration.
- Buildroot configuration and package set.
- Root filesystem and init/startup design.
- Shared application interfaces when they affect image integration.
- Integration acceptance and benchmark conditions.

**WSL Parallel Development chat**:
- Follows approved decisions.
- Performs parallel implementation/testing.
- Reports WSL-specific constraints and evidence.
- May propose changes, but must not independently switch architecture, build system, kernel version, target platform, MQTT schema, or shared APIs.

Authority does not mean Ubuntu may silently change an interface. All affected owners must be notified, and the change must be documented and reviewed.

## 3. Decisions that cannot change independently

Neither chat/member may unilaterally change:
- Track B project objective or scope.
- Build system or pinned release.
- Initial target architecture.
- Kernel version.
- Physical target assumptions.
- Sensor API and data structure.
- Temperature units and threshold boundary semantics.
- MQTT topic/payload schema.
- Directory structure and build integration contract.
- Security requirements and benchmark methodology.

A proposal must state:
1. Current decision.
2. Proposed change.
3. Reason and evidence.
4. Files affected.
5. Compatibility impact.
6. Build/test work required.
7. Approval from the Ubuntu integration owner and affected module owners.

Until approved, mark the proposal PENDING and continue with the existing agreed interface where possible.

## 4. Git workflow

Suggested branches:
- `main`
- `member1/build-system`
- `member2/application`
- `member3/mqtt-security`
- `member4/documentation`

Recommended sequence:
1. Pull the latest `main` before starting work.
2. Create or switch to your own feature branch.
3. Work only in the files you own or have coordinated to edit.
4. Run relevant tests locally.
5. Commit small, coherent changes with descriptive messages.
6. Push the feature branch.
7. Request review from the relevant owner.
8. Merge only after conflicts and tests are resolved.
9. Pull the updated `main` before starting the next task.

Do not force-push shared branches. Do not commit generated Buildroot output, caches, temporary files, secrets, or large images to normal Git history.

## 5. Environment-specific repository rules

- Ubuntu project/build path: a directory under the Linux home directory, e.g. `~/iot-temperature-gateway`.
- WSL project/build path: inside the WSL Linux filesystem, e.g. `/home/<user>/iot-temperature-gateway`.
- Avoid `/mnt/c` for intensive WSL build trees.
- Both environments use the same repository content and pinned project versions.
- Host dependencies may differ; record them separately without changing target output configuration.
- Do not install the custom kernel as a host kernel.

## 6. When to pull, commit, and sync

**Pull**
- Before starting a task.
- Before merging.
- After the other branch reports a shared-interface/configuration change.
- Before generating a final image.

**Commit**
- After a coherent unit of work passes its local test.
- Before requesting review.
- After updating project status and documentation for an approved decision.

**Sync summary**
- At every major milestone.
- Whenever a shared interface/version/configuration changes.
- Whenever a blocker affects the other branch.
- Before handing work to another member/chat.

## 7. PROJECT_STATUS.md update rules

`PROJECT_STATUS.md` is authoritative for:
- Agreed and pending decisions.
- Exact software versions.
- Milestone progress.
- Files changed.
- Commands actually executed.
- Test outputs and benchmark evidence.
- Errors and limitations.
- Next actions and task ownership.

Rules:
- Mark a task complete only with evidence.
- Record exact versions from command output.
- Distinguish PLANNED, NOT RUN, PASSED, FAILED, and BLOCKED.
- Record whether a result came from host, WSL, QEMU guest, or physical hardware.
- Include date, member, environment, commit hash, and evidence path when practical.
- Never replace an observed failure with a guessed success.

## 8. SYNC SUMMARY template

Copy the following structure into the other chat after each milestone:

### SYNC SUMMARY — [date] — [milestone]

**Branch/environment:**  
**Member(s):**  
**Git branch and commit:**  

1. **Current agreed decisions**
   - Build system/release:
   - Kernel version:
   - Architecture/target:
   - Application interface:
   - MQTT schema:
   - Other relevant settings:

2. **Versions used**
   - Host OS:
   - WSL (if applicable):
   - Compiler/toolchain:
   - Buildroot:
   - Kernel:
   - QEMU:
   - MQTT client/broker:

3. **Completed work**
   - List only work actually completed and verified.

4. **Files created/modified/deleted**
   - Exact repository paths and purpose.

5. **Commands actually tested**
   - Exact commands and working directory.

6. **Actual results**
   - Include observed outputs, test counts, logs, or artifact paths.
   - Distinguish host, WSL, QEMU, and physical results.

7. **Errors and limitations**
   - Exact error and what remains unresolved.

8. **Current milestone/status**
   - Passed, failed, blocked, or in progress.

9. **Next steps**
   - Ordered actions and owner.

10. **Parallel tasks for the other environment**
    - Tasks that can safely proceed without changing shared decisions.

11. **Required changes/alerts for the other chat**
    - Interface changes, version changes, compatibility warnings, or blockers.

12. **PROJECT_STATUS.md**
    - Confirm whether updated and identify commit.

## 9. Conflict resolution

If the chats recommend conflicting decisions:
1. Stop work that depends on the disputed decision.
2. Compare the latest committed `PROJECT_STATUS.md`, relevant config files, Git commits, and actual test evidence.
3. Identify whether the difference is a proposal, an untested assumption, or a verified result.
4. Ask the Ubuntu integration owner and affected module owners to review.
5. Choose one compatible decision based on assignment requirements, evidence, and time.
6. Update `PROJECT_STATUS.md` and relevant configuration.
7. Send a SYNC SUMMARY to both chats.
8. Resume only after both branches acknowledge the new baseline.

Never resolve a conflict by silently using different kernel versions, MQTT schemas, sensor APIs, or target assumptions.

## 10. Parallel task boundaries

Safe parallel tasks after initial agreements:
- Member 1: Buildroot/virtual image preparation.
- Member 2: C simulator, validation, threshold tests.
- Member 3: MQTT client/broker test, logging and security proposals.
- Member 4: requirements, test matrix, benchmark methodology, report structure.

Tasks requiring coordination:
- Sensor API changes.
- MQTT payload/topic changes.
- Buildroot package additions.
- Rootfs paths/startup integration.
- Kernel and architecture changes.
- Physical-board assumptions.

## 11. Chat initialization

At the beginning of each chat:
1. State whether it is Ubuntu Primary or WSL Parallel.
2. Provide the latest `PROJECT_STATUS.md` and most recent SYNC SUMMARY when available.
3. Ask the assistant to acknowledge current decisions and identify pending items.
4. Request only the next milestone step.
5. Do not assume tasks from the other chat are complete unless the status file or summary contains evidence.
