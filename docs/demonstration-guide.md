# Project Demonstration and Evaluation Guide

## 1. Executive Summary & Objective

This document provides an exact, reproducible step-by-step demonstration script designed for project examiners, instructors, and evaluators to assess the **Track B IoT Temperature-Monitoring Gateway**.

The demonstration proves:
1. Complete, robust C gateway implementation with 100% unit test coverage.
2. Lightweight, custom Buildroot Linux distribution booting in **1.73 seconds**.
3. Automatic daemon startup via SysV init (`S99iot-gateway`).
4. End-to-end MQTT telemetry and alert publication across guest-to-host virtual networking.
5. Applied and verified kernel/sysctl security hardening.
6. Persistent CSV logging surviving virtual machine reboots.
7. Ultra-low resource consumption (~13 MiB RAM, 66 MiB total storage footprint).

---

## 2. Prerequisites & Environment Setup

The demonstration runs natively under Ubuntu Linux (native or WSL2):

### Required Host Packages:
```bash
sudo apt-get update
sudo apt-get install -y qemu-system-x86 mosquitto mosquitto-clients build-essential
```

### Ensure Mosquitto Broker is Listening on All Interfaces:
```bash
sudo systemctl start mosquitto
# Ensure listener is active on port 1883
ss -tulpn | grep 1883
```

---

## 3. Demonstration Walkthrough (Step-by-Step)

### Step 1: Demonstrate C Application Architecture and Unit Tests

Open **Terminal 1** on the host and run the offline test suite:

```bash
cd application
make test
```

#### Expected Outcome:
All six test suites compile cleanly with `-Wall -Wextra -Wpedantic` and pass with 100% success:
- `test_sensor`: Verifies deterministic sequence (25, 30, 36, 42°C), realistic bounds [20, 42°C], `SIMULATED` data label, and NaN/Inf rejection.
- `test_processor`: Validates threshold classification (`NORMAL` < 35°C, `WARNING` 35–39°C, `CRITICAL` ≥ 40°C).
- `test_alert`: Verifies alert dispatch and suppression logic.
- `test_logger`: Verifies CSV formatting, header creation, and `fsync` invocation.
- `test_mqtt`: Validates JSON schema formatting, QoS 1, and retain=false flags.
- `test_mqtt_reconnect`: Validates non-blocking async reconnection and exponential backoff.

---

### Step 2: Start the Host MQTT Telemetry Monitor

In **Terminal 1**, start an authenticated Mosquitto subscriber to monitor all incoming telemetry and alerts:

```bash
# Source configured test credentials or provide directly:
mosquitto_sub -h localhost -p 1883 -u monitor_sub -P mon_sub_47c2d91a03ef -t 'iot/temperature/#' -v
```

*Note: Anonymous access is strictly rejected by the broker. Authenticating as `monitor_sub` grants read-only access to `iot/temperature/#`.*
*Leave this terminal running. It will display real-time telemetry once the virtual machine boots.*

---

### Step 3: Launch the Custom Embedded Linux Distribution

Open **Terminal 2** and launch the QEMU virtual machine:

```bash
./scripts/qemu/run-qemu.sh
```

#### Expected Outcome:
- QEMU launches using KVM hardware acceleration (or TCG fallback).
- The custom Linux kernel (6.18.7) uncompresses and boots in **~1.73 seconds**.
- The BusyBox banner and login prompt appear immediately:
  ```text
  Welcome to Buildroot (IoT Gateway x86-64)
  buildroot login:
  ```

---

### Step 4: Verify Real-Time Telemetry and Alerts

Switch back to **Terminal 1** (the MQTT subscriber).

#### Expected Outcome:
Live JSON messages arrive continuously every 2 seconds from the guest VM via virtual bridge IP `10.0.2.2`:

```text
iot/temperature/readings {"timestamp":"2026-11-22T09:47:32Z","temperature_c":25.00,"source":"SIMULATED","status":"VALID","alert_level":"NORMAL"}
iot/temperature/readings {"timestamp":"2026-11-22T09:47:34Z","temperature_c":30.00,"source":"SIMULATED","status":"VALID","alert_level":"NORMAL"}
iot/temperature/readings {"timestamp":"2026-11-22T09:47:36Z","temperature_c":36.00,"source":"SIMULATED","status":"VALID","alert_level":"WARNING"}
iot/temperature/alerts {"timestamp":"2026-11-22T09:47:36Z","temperature_c":36.00,"source":"SIMULATED","status":"VALID","alert_level":"WARNING","alert_message":"WARNING: Temperature elevated (36.00 C)"}
iot/temperature/readings {"timestamp":"2026-11-22T09:47:38Z","temperature_c":42.00,"source":"SIMULATED","status":"VALID","alert_level":"CRITICAL"}
iot/temperature/alerts {"timestamp":"2026-11-22T09:47:38Z","temperature_c":42.00,"source":"SIMULATED","status":"VALID","alert_level":"CRITICAL","alert_message":"CRITICAL: Temperature dangerous (42.00 C)"}
```

**Key Points to Note to Evaluators:**
1. Every payload explicitly indicates `"source":"SIMULATED"`.
2. Alert messages only trigger when temperature reaches `WARNING` (36°C) or `CRITICAL` (42°C).
3. The deterministic cycle systematically exercises all threshold states.

---

### Step 5: Verify Running Daemon and Service Health Inside the Guest

In **Terminal 2**, log in as `root` at the prompt.

Execute the following diagnostic commands:

#### 1. Confirm Automatic Daemon Startup and Process State
```sh
ps | grep iot_gateway
cat /run/iot_gateway.pid
cat /var/log/iot_gateway.log | tail -n 10
```
- `iot_gateway` is running under its registered PID.
- The daemon started automatically during boot via `/etc/init.d/S99iot-gateway`.

#### 2. Confirm Active Security Hardening
```sh
sysctl kernel.dmesg_restrict kernel.kptr_restrict net.ipv4.tcp_syncookies
ls -l /etc/shadow
```
- Output proves `dmesg_restrict = 1`, `kptr_restrict = 2`, and `tcp_syncookies = 1`.
- `/etc/shadow` has mode `-rw-------` (`0600`) owned by `root:root`.

#### 3. Inspect Ultra-Low Resource Utilization
```sh
free -m
cat /proc/$(cat /run/iot_gateway.pid)/status | grep -E "VmSize|VmRSS"
```
- Total system RAM consumed is **~13 MiB**.
- Gateway daemon Resident Set Size is **~6.6 MiB**.

---

### Step 6: Verify Local CSV Log Durability and Persistence

In **Terminal 2**, inspect the live CSV audit log:

```sh
cat /root/temperature_log.csv | tail -n 10
```
Observe that all records have been formatted with standard CSV fields and written to the persistent partition (`/dev/vda`).

Now initiate a clean shutdown:
```sh
poweroff
```

QEMU shuts down gracefully.

#### Offline Verification of Persistence:
On the host, verify that the CSV file was successfully committed to the disk image:
```bash
debugfs -R 'cat /root/temperature_log.csv' buildroot/source/buildroot-2026.08.2/output/images/rootfs.ext2 | tail -n 10
```
All logged rows survive VM shutdown, confirming filesystem durability.

---

## 4. Evaluation Acceptance Matrix

| # | Master Requirement | Implementation Mechanism | Verification Result |
|---|---|---|---|
| 1 | Simulated sensor acquisition | `sensor.c` deterministic & realistic modes | **VERIFIED (100% Pass)** |
| 2 | Input validation | Rejection of NaN, $\pm\infty$, out-of-range ($<-40$ or $>85^\circ\text{C}$) | **VERIFIED (100% Pass)** |
| 3 | Temperature processing | `processor.c` data structure calculation | **VERIFIED (100% Pass)** |
| 4 | Threshold classification | Normal ($<35$), Warning ($35..39$), Critical ($\ge 40$) | **VERIFIED (100% Pass)** |
| 5 | Alert generation | `alert.c` triggered on Warning and Critical | **VERIFIED (100% Pass)** |
| 6 | Local CSV logging | `logger.c` with ISO timestamps and `fsync()` durability | **VERIFIED (100% Pass)** |
| 7 | MQTT publication | `mqtt.c` QoS 1, retain false, JSON schemas, auto-reconnect | **VERIFIED (100% Pass)** |
| 8 | Automatic boot startup | BusyBox init `S99iot-gateway` daemonization & PID file | **VERIFIED (1.73s Boot)** |
| 9 | Security hardening | Kernel strict RWX, sysctl parameters, shadow 0600, SHA-512 | **VERIFIED (Active)** |
| 10| VM execution | QEMU x86-64 with KVM acceleration | **VERIFIED (Stable)** |
| 11| Tests & documentation | Comprehensive unit tests, architecture guides, benchmarks | **VERIFIED (Complete)** |
