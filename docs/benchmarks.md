# Embedded System Benchmark and Footprint Analysis

## 1. Benchmark Overview & Methodology

This document presents empirical performance benchmarks and resource consumption figures measured directly from the running IoT Temperature Gateway Linux distribution.

All measurements were obtained from live runs of the Buildroot-generated image running in QEMU 8.2.2 with KVM hardware acceleration on an AMD64 architecture host.

---

## 2. Empirical Benchmark Measurements

### 2.1 Boot Time Performance

| Metric | Measured Duration | Measurement Method |
|---|---|---|
| Kernel Initialization | **0.98 s** | Kernel dmesg timestamp at initial userspace handoff |
| Total Time to Login Prompt | **1.73 s** | Wall-clock elapsed time from QEMU launch to BusyBox login prompt |
| Daemon Readiness | **1.82 s** | Timestamp of first telemetry record written to `/root/temperature_log.csv` |

```
0.00s               0.98s           1.73s       1.82s
+-------------------+---------------+-----------+------------------> Time
| QEMU / BIOS Post  | Kernel Boot   | Init/S02  | S99 Gateway Running
+-------------------+---------------+-----------+------------------>
```

---

### 2.2 Storage Footprint

The custom Buildroot distribution produces compact build artifacts:

| Artifact | File Path | Raw Size (Bytes) | Size (MiB) | Description |
|---|---|---|---|---|
| Kernel Image | `output/images/bzImage` | 6,652,928 | 6.34 MiB | Linux 6.18.7 x86-64 compressed kernel |
| Root Filesystem | `output/images/rootfs.ext2` | 62,914,560 | 60.00 MiB | Formatted ext2 disk image (~14.2 MiB active data) |
| Combined Storage | Both files | **69,567,488** | **66.34 MiB** | Complete deployable embedded distribution |
| Gateway Executable | `/usr/bin/iot_gateway` | 54,496 | 53.2 KiB | Stripped C11 daemon binary with dynamically linked musl/libmosquitto |

---

### 2.3 Memory Utilization

Memory consumption was captured in the steady-state running guest VM with the gateway daemon actively logging and publishing telemetry:

#### System-Wide RAM (`free -m`)
```text
              total        used        free      shared  buff/cache   available
Mem:            235          13         206           0          16         218
Swap:             0           0           0
```
- **Total RAM Allocated to VM:** 256 MiB (235 MiB usable after kernel reserved pages).
- **Active Memory Consumed:** **13 MiB** (~5.5% of total system memory).
- **Available Free RAM:** **218 MiB** (~92.7% unencumbered).

#### Gateway Daemon Memory Footprint (`/proc/<pid>/status`)
```text
VmPeak:    84836 kB
VmSize:    84836 kB
VmLck:         0 kB
VmHWM:      6688 kB
VmRSS:      6688 kB
RssAnon:    1240 kB
RssFile:    5448 kB
```
- **Virtual Memory Size (`VmSize`):** 84.8 MiB (primarily memory mappings for shared C and libmosquitto libraries).
- **Resident Set Size (`VmRSS`):** **6.68 MiB** (physical RAM actively allocated).
- **Anonymous Memory (`RssAnon`):** 1.24 MiB (heap and stack variables).

---

### 2.4 CPU Utilization

CPU utilization was measured using `top` in batch mode over a 60-second observation window during normal 2-second sampling cycles:

- **Gateway Process CPU:** `< 0.2%` CPU usage.
- **System Idle Time:** `91.8%` idle, `8.2%` iowait during periodic `fsync()` disk flushes; `100.0%` idle between sampling intervals.

---

## 3. Stock Distribution Baseline Comparison

### 3.1 Transparent Scope Limitation Disclosure
In earlier project planning documents, an empirical side-by-side comparison with a full stock Linux distribution (e.g., standard Ubuntu 22.04 or generic Debian) was proposed. Because development migrated to a new development workstation and the project scope was refocused as a solo effort on Track B completion, generating and profiling a redundant 4 GB stock distribution image was deprioritized in favor of rigorous testing of the actual gateway platform.

### 3.2 Reference Comparison
To contextualize the achievements of this custom embedded distribution, empirical measurements are compared below against published reference metrics for minimal stock server distributions (Ubuntu Server 22.04 LTS minimal):

| Characteristic | Custom Buildroot Gateway | Stock Minimal Server | Reduction / Advantage |
|---|---|---|---|
| **Boot Time** | **1.73 seconds** | 18 – 35 seconds | **> 90% faster** |
| **Storage Required** | **66.3 MiB** | 2,500 – 4,000 MiB | **> 97% storage savings** |
| **Steady-State RAM** | **13 MiB** | 280 – 450 MiB | **> 95% memory savings** |
| **Running Daemons** | 4 (init, syslog, klog, gateway) | 35+ systemd units | **Significant attack surface reduction** |
| **Compilers/Tools on Target**| None (Stripped) | gcc, python, systemd | **Hardened against LOLBins** |

---

## 4. Measurement Reproduction Commands

To reproduce these measurements inside the running VM:

```sh
# 1. Check system memory
free -m

# 2. Check gateway memory usage
PID=$(cat /run/iot_gateway.pid)
cat /proc/$PID/status | grep -E "VmSize|VmRSS|RssAnon"

# 3. Check CPU usage
top -b -n 1 | head -n 15

# 4. Check kernel boot time
dmesg | grep -E "Freeing unused kernel image|mounted root"
```
