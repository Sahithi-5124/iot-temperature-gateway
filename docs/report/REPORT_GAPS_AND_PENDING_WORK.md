# Project Gaps and Pending Work: Phase 1 to Phase 2 Transition Roadmap

**Project:** Application-Specific Linux Distribution for an IoT Temperature Gateway (Track B)  
**Document Purpose:** Transparent Engineering Gap Analysis & Actionable Phase 2 Execution Plan  
**Project Phase Transition:** Phase 1 (Virtual Development Baseline) $\rightarrow$ Phase 2 (Physical Hardware Integration)  
**Date of Audit:** 23 November 2026  
**Document Location:** `docs/report/REPORT_GAPS_AND_PENDING_WORK.md`  

---

## 1. Executive Context & Scope Disclosure

In accordance with professional embedded engineering standards and the strict assignment rules governing technical integrity:
- **Phase 1 (Virtual Machine Baseline Development):** Core operating system milestones (Buildroot compilation, minimal Linux 6.18.7 kernel, SysV init orchestration, C gateway daemon, mathematical validation, threshold classification, durable CSV logging with `fsync()`, authenticated MQTT with QoS 1, and 5 security hardening controls) have been **built, verified, and tested in QEMU**.
- **Partially Implemented Systems:** The **Read-Only Root plus OverlayFS** filesystem layout is partially implemented (volatile directories use `tmpfs`, but the root partition is currently mounted read-write for Phase 1 debugging).
- **Phase 2 (Physical Hardware Integration):** Remains **PENDING HARDWARE CONFIRMATION**. Physical sensor deployment, board-specific Device Tree authoring, electrical bench power measurements, and hardware secure boot cannot be fabricated in the absence of physical silicon.

This document details every requirement that remains pending or partially implemented, explains why it could not be completed in Phase 1, and provides an actionable engineering roadmap to close each gap upon hardware receipt.

---

## 2. In-Depth Gap Analysis & Resolution Roadmap

### Gap 1: Physical Sensor Acquisition & Driver Integration

#### 1. Current Repository State
- The sensor subsystem (`application/src/sensor/sensor.c`) implements an abstraction interface (`application/include/sensor.h`).
- Two software simulation modes are active: `SENSOR_MODE_DETERMINISTIC` (fixed test cycle) and `SENSOR_MODE_REALISTIC` (bounded thermodynamic random walk).
- All generated telemetry and alert records are explicitly stamped with `"source": "SIMULATED"`.
- The hardware source branch explicitly returns `SENSOR_STATUS_UNSUPPORTED`:
  ```c
  if (source == SENSOR_SOURCE_HARDWARE) {
      return SENSOR_STATUS_UNSUPPORTED;
  }
  ```

#### 2. Why Incomplete in Phase 1
- Development occurred on laptops using native Ubuntu 22.04 LTS and Windows WSL2. Physical single-board computers (SBCs) and physical sensors were not allocated during initial virtual development.
- The provisional virtual platform (x86_64 in QEMU) does not feature physical GPIO pins.

#### 3. Action Plan & Required Implementation for Phase 2
1. **Sensor Selection & Protocol Understanding:**
   - The selected physical sensor is the **DHT11 digital temperature and humidity sensor**.
   - *Protocol Note:* The DHT11 uses a **proprietary single-wire bidirectional digital protocol** (not I2C or SPI). It requires microsecond-level timing pulses (18 ms start pulse followed by 40 bits of pulse-width modulated data).
2. **Electrical Wiring:**
   - Pin 1 (`VCC`): Connect to 3.3V or 5.0V DC.
   - Pin 2 (`DATA`): Connect to an interrupt-capable SoC GPIO pin (e.g., GPIO 4) with a $4.7\,\text{k}\Omega - 10\,\text{k}\Omega$ pull-up resistor to `VCC`.
   - Pin 3 (`NC`): Not connected.
   - Pin 4 (`GND`): Connect to common ground (`GND`).
3. **Linux Kernel Driver Selection:**
   - Enable the Linux kernel Industrial I/O (IIO) DHT11 driver:
     ```conf
     CONFIG_IIO=y
     CONFIG_DHT11=y
     ```
   - This driver uses kernel high-resolution timers (`hrtimer`) and interrupt handlers to decode the 40-bit pulse stream cleanly in kernel space.
4. **Application Backend Update:**
   - Update `application/src/sensor/sensor.c` under `SENSOR_SOURCE_HARDWARE` to read ambient temperature from the sysfs interface:
     ```c
     FILE *fp = fopen("/sys/bus/iio/devices/iio:device0/in_temp_input", "r");
     // Read millidegrees Celsius, divide by 1000.0, and mark source as HARDWARE
     ```
   - Set sample metadata to `"source": "HARDWARE"`.

---

### Gap 2: Device Tree Configuration for Physical Target Board

#### 1. Current Repository State
- No Device Tree Source (`.dts`) or Device Tree Blob (`.dtb`) exists in the repository.
- The Linux kernel is compiled via `buildroot/external/iot_gateway/board/x86_64/linux.config`.

#### 2. Why Incomplete in Phase 1
- The provisional development hypervisor is **x86_64 PC architecture** (`qemu-system-x86_64 -M pc`).
- On x86_64 systems, hardware topology is not described via Device Trees. The BIOS/UEFI firmware provides **ACPI tables**, and peripheral buses (PCI, SATA, VirtIO) are enumerated dynamically via **PCI configuration space scanning**.
- Authoring a Device Tree for an unconfirmed ARM/RISC-V board would be fictitious and untestable.

#### 3. Action Plan & Required Implementation for Phase 2
1. **Board Confirmation:** Confirm the target embedded architecture (e.g., Raspberry Pi Zero 2 W with Broadcom BCM2837, BeagleBone Black with TI AM335x, or Allwinner H3).
2. **Device Tree Overlay (`.dtso`) Authoring:**
   Author a device tree overlay binding the DHT11 IIO driver to the designated GPIO pin:
   ```dts
   /dts-v1/;
   /plugin/;

   / {
       compatible = "brcm,bcm2835", "ti,am33xx";
       fragment@0 {
           target = <&gpio>;
           __overlay__ {
               dht11_pins: dht11_pins {
                   brcm,pins = <4>;
                   brcm,function = <0>; /* Input */
                   brcm,pull = <2>;     /* Pull-up */
               };
           };
       };
       fragment@1 {
           target-path = "/";
           __overlay__ {
               dht11: dht11@4 {
                   compatible = "dht11";
                   pinctrl-names = "default";
                   pinctrl-0 = <&dht11_pins>;
                   gpios = <&gpio 4 0>;
                   status = "okay";
               };
           };
       };
   };
   ```
3. **Compilation:** Compile the overlay using the Device Tree Compiler (`dtc -I dts -O dtb -o dht11.dtbo dht11.dtso`) and install it into `/boot/overlays/`.

---

### Gap 3: Filesystem Layout — Read-Only Root plus OverlayFS

#### 1. Current Repository State
- In Phase 1 QEMU testing, the root filesystem on `/dev/vda` is mounted **read-write (`rw`)** (`rootwait root=/dev/vda console=ttyS0`).
- Volatile directories `/tmp`, `/var/run`, and `/var/log` are RAM-backed on `tmpfs` (`/var/run -> ../tmp/run`, `/var/log -> ../tmp/log`).
- Persistent telemetry records are written directly to `/root/temperature_log.csv` on the read-write `/dev/vda` root filesystem with `fsync()`.
- Runtime configuration `/etc/iot-gateway/mqtt.env` is stored directly on `/dev/vda`.
- On reboot, ephemeral logs in `/var/log` are wiped, while `/root/temperature_log.csv` and `mqtt.env` survive on the ext2 virtual disk.

#### 2. Why Incomplete in Phase 1
- Mounting root read-write during Phase 1 virtual development was a deliberate engineering decision: it facilitated rapid iteration on runtime credentials, interactive debugging, and direct verification that local CSV records survived reboot cycles without requiring complex multi-partition initramfs scripts before physical storage geometries were known.

#### 3. Action Plan & Required Implementation for Phase 2
1. **Kernel Configuration:** Enable `CONFIG_OVERLAY_FS=y` in `linux.config`.
2. **Buildroot Setting:** Set `BR2_TARGET_GENERIC_REMOUNT_ROOTFS_RW=n` in the defconfig.
3. **Boot Arguments:** Change kernel boot parameters to mount root read-only (`root=/dev/mmcblk0p2 ro`).
4. **Init Script / Overlay Setup:**
   Mount a `tmpfs` RAM disk at `/run/overlay` with `upperdir=/run/overlay/upper` and `workdir=/run/overlay/work`. Execute an OverlayFS mount:
   ```sh
   mount -t overlay overlay -o lowerdir=/etc,upperdir=/run/overlay/upper,workdir=/run/overlay/work /etc
   ```
   This ensures that any runtime changes to `/etc` exist only in volatile RAM, while the physical rootfs remains immutable and protected against corruption.
5. **Persistent Partition:**
   Format a dedicated persistent partition (`/dev/mmcblk0p4`, EXT4) mounted at `/data`. Reconfigure the gateway daemon via `IOT_LOG_FILE=/data/temperature_log.csv`, completely decoupling persistent telemetry logs from the read-only operating system partition.

---

### Gap 4: Physical Power Consumption Benchmarking

#### 1. Current Repository State
- Power consumption is cataloged in `docs/benchmarks.md` and `FINAL_PROJECT_REPORT.md` as `NOT MEASURED (PENDING PHYSICAL HARDWARE)`.

#### 2. Why Incomplete in Phase 1
- QEMU is a virtualization process running on a host CPU.
- Virtual machine profiling tools (`top`, `/proc/stat`) measure CPU time slices and virtual memory allocations, but cannot quantify physical electrical current ($I$), voltage ($V$), or wattage ($P = V \times I$).
- Any numerical power metric reported inside a virtual machine would be entirely fabricated.

#### 3. Action Plan & Required Implementation for Phase 2
1. **Measurement Apparatus:**
   - Inline USB Power Profiler (e.g., Nordic Power Profiler Kit II or MakerHawk UM34C USB Tester) or digital bench multimeter measuring current draw in series on the 5V DC supply rail.
2. **Test Protocol:**
   - **Baseline Idle State:** System booted, gateway daemon idle between 2-second sampling ticks. Measure average current ($I_{\text{idle}}$).
   - **Active Acquisition & Logging State:** Sensor read active, `fsync()` flushing dirty blocks to eMMC/micro-SD. Measure peak current ($I_{\text{active}}$).
   - **Network Transmission State:** Wi-Fi / Ethernet interface active transmitting QoS 1 MQTT packets. Measure peak transmission current ($I_{\text{tx}}$).
3. **Power Optimization:**
   - If power exceeds target (< 2.5W), configure Linux kernel CPU frequency governors (`CONFIG_CPU_FREQ=y`, `powersave` or `ondemand`) and disable unused onboard USB hub ICs.

---

### Gap 5: Hardware Secure Boot & Signed Dual-Partition OTA Updates

#### 1. Current Repository State
- Software-level security hardening (kernel sysctls, credential file permissions 0600, compiler protections, and authenticated MQTT with ACLs) is **100% complete and verified**.
- Hardware secure boot and cryptographic Over-The-Air (OTA) updates are documented in Section 13.5 of the Technical Report as **PROPOSED / FUTURE WORK**.

#### 2. Why Incomplete in Phase 1
- Hardware Secure Boot requires a physical hardware Root of Trust: on-chip One-Time-Programmable (OTP) eFuses, a Trusted Platform Module (TPM 2.0), or a Cryptographic Secure Element (e.g., ATECC608A) containing burned public key hashes.
- Standard QEMU PC virtual hardware does not implement secure eFuse burning.
- The virtual disk currently uses a single root filesystem partition (`rootfs.ext2`) rather than a production dual-partition A/B flash geometry.

#### 3. Action Plan & Required Implementation for Phase 2
1. **Storage Partitioning Layout:**
   Format target eMMC / SD storage with an A/B recovery layout:
   - `Partition 1` (`/dev/mmcblk0p1`): `boot` (FAT32, U-Boot, Device Tree, boot counter).
   - `Partition 2` (`/dev/mmcblk0p2`): `rootfs_A` (EXT4, Active Read-Only Root Filesystem Slot).
   - `Partition 3` (`/dev/mmcblk0p3`): `rootfs_B` (EXT4, Standby Read-Only Root Filesystem Slot).
   - `Partition 4` (`/dev/mmcblk0p4`): `data` (EXT4, Persistent read/write telemetry & logs).
2. **Asymmetric Signature Verification:**
   - Generate an asymmetric Ed25519 or RSA-2048 key pair.
   - Embed the public verification certificate into `/etc/ssl/certs/gateway-update.crt` inside the base rootfs image.
   - Private signing keys remain securely held on a restricted build server.
3. **OTA Update Agent:**
   - Integrate SWUpdate or RAUC into the Buildroot defconfig.
   - When an update bundle (`.swu` / `.raucb`) is downloaded, verify its digital signature against the public certificate.
   - Stream the verified image into the inactive slot (`rootfs_B`).
   - Configure U-Boot environment variables (`bootcount=0`, `bootlimit=3`) and set the next boot slot.
4. **Watchdog Rollback:**
   - Enable hardware watchdog (`/dev/watchdog`).
   - If the new kernel fails to mount or the gateway daemon does not issue a heartbeat ping within 60 seconds, the hardware watchdog reboots the board, and U-Boot automatically reverts to `rootfs_A`.

---

### Gap 6: Persistent On-Disk MQTT Transaction Queuing

#### 1. Current Repository State
- The gateway daemon implements non-blocking asynchronous reconnection via `mosquitto_connect_async()` and `mosquitto_loop_start()`.
- During an MQTT broker outage, the gateway does not crash or block; it continues local sensor acquisition and writes all records to `/root/temperature_log.csv`.
- However, telemetry messages generated while disconnected are not buffered in an on-disk transactional spool; libmosquitto's internal in-memory queue is bounded and volatile.

#### 2. Why Incomplete in Phase 1
- Phase 1 prioritized zero-dependency, ultra-low-footprint C11 userspace. Integrating an embedded database engine (e.g., SQLite) would increase the rootfs footprint and RAM usage.
- The assignment specifies local CSV storage as the primary durable repository.

#### 3. Action Plan & Required Implementation for Phase 2
1. **Transactional Spooling Engine:**
   - Implement an on-disk circular queue or embed a minimal SQLite database with Write-Ahead Logging (WAL) in `application/src/logging/`.
2. **Outage Drain Routine:**
   - When the MQTT connection state transitions from disconnected to connected, query unacknowledged records from the local database and publish them in burst mode until the queue is synchronized with the cloud broker.

---

## 3. Summary of Gaps, Status, and Viva Defense Strategy

| Project Subsystem | Phase 1 Status (Current) | Phase 2 Status (Roadmap) | Defensible Viva Position |
|---|---|---|---|
| **OS & Kernel Build** | **Complete & Verified** | Architecture Retained | Full custom Buildroot and Linux 6.18.7 kernel verified in QEMU in 1.73s to prompt, 1.82s to readiness. |
| **Gateway Application** | **Complete & Verified** | Logic Retained | C11 architecture, validation, classification, CSV logging, and MQTT QoS 1 pass all unit and integration tests. |
| **Sensor Telemetry** | **Simulated & Verified** | Pending Physical Sensor | All telemetry is honestly tagged `SIMULATED`; abstraction layer is fully prepared for Phase 2 IIO driver integration. |
| **Filesystem Layout** | **Partially Implemented** | Full Read-Only + OverlayFS | Volatile directories use `tmpfs` in RAM; rootfs mounted rw for Phase 1 debugging; full OverlayFS roadmapped for Phase 2. |
| **Security Hardening** | **Complete & Verified** | Expanded to Hardware | 5 software/kernel security controls verified; hardware secure boot and signed updates specified as Phase 2 future work. |
| **Power & Device Tree** | **Disclosed as Pending** | Bench Measurement Plan | Device trees are N/A on x86 ACPI systems; physical power cannot be measured in virtual QEMU. Explicit disclosures protect academic integrity. |

All gaps are formally documented, technically justified, and accompanied by concrete implementation plans.
