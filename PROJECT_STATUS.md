# Project Status --- Track B: IoT Temperature-Monitoring Gateway

**Last updated:** 21 November 2026\
**Primary development environment:** Native Ubuntu 22.04\
**Git branch:** `dev/build-system`\
**Repository:**
https://github.com/Sahithi-5124/iot-temperature-gateway.git\
**Project scope:** Track B only --- IoT Temperature-Monitoring Gateway

------------------------------------------------------------------------

## 1. Project overview

The project builds a lightweight, application-specific Linux
distribution that collects temperature readings, validates and
classifies them, generates alerts, logs data locally, and publishes
readings and alerts through MQTT.

The team has four beginners and approximately 10 hours for the initial
project.

### Development phases

**Phase 1 --- Before physical hardware arrives** - Develop with
simulated sensor readings on native Ubuntu 22.04. - Configure and build
the Linux system using Buildroot. - Run the x86-64 image in QEMU. -
Develop and test the C gateway, CSV logging, MQTT communication,
startup, shutdown, and security configuration. - Record reproducible
measurements without inventing results.

**Phase 2 --- After hardware arrives** - Confirm the exact board,
revision, sensor model, electrical requirements, and interface. -
Implement and test the hardware sensor backend. - Configure the
appropriate architecture, kernel, drivers, and device tree. - Deploy
only after board compatibility and deployment procedures are confirmed.

All readings verified to date are simulated. They must not be described
as physical measurements.

------------------------------------------------------------------------

## 2. Technical baseline

  Component                     Current decision or verified version
  ----------------------------- --------------------------------------
  Project scope                 Track B --- IoT Gateway
  Primary environment           Native Ubuntu 22.04
  Build system                  Buildroot 2026.08.2
  Phase 1 target architecture   x86-64
  Virtual platform              QEMU
  Linux kernel                  6.18.7
  BusyBox                       1.38.0
  Root filesystem               EXT2
  Init system                   BusyBox init
  Application language          C
  MQTT client library           libmosquitto
  Host MQTT broker              Mosquitto 2.0.11
  Target Mosquitto package      2.1.2
  Version control               Git

The current image is an x86-64 QEMU image. It is not presumed compatible
with the eventual physical board.

### Pending hardware decisions

-   Exact physical board model and revision.
-   Exact temperature sensor model.
-   Electrical compatibility, voltage levels, pin mapping, and
    interface.
-   Final target architecture and kernel/device-tree configuration.

DHT11 was previously noted as a provisional preference only. No
hardware-specific behavior should be assumed before the components are
confirmed.

------------------------------------------------------------------------

## 3. Application behavior and configuration

### Temperature classification

  Temperature          Classification
  -------------------- ----------------
  Below 35°C           NORMAL
  35°C to below 40°C   WARNING
  40°C and above       CRITICAL

The accepted input range is currently −40°C to 85°C inclusive,
provisional until the physical sensor is confirmed. Invalid readings,
NaN, infinity, and out-of-range values are rejected.

### Sensor modes

-   `SIMULATED`: software-generated readings.
-   `HARDWARE`: reserved for a future physical sensor backend; currently
    reports unsupported.

Two simulation modes are implemented:

-   `DETERMINISTIC`: repeats 25°C, 30°C, 36°C, and 42°C.
-   `REALISTIC`: moves in 1°C steps between 20°C and 42°C.

`IOT_SENSOR_MODE` selects the mode: - `deterministic` selects
deterministic simulation. - `realistic` selects realistic simulation. -
Unset defaults to deterministic. - Invalid values are rejected.

The source remains labelled `SIMULATED`.

### MQTT settings

  Setting                  Value
  ------------------------ ----------------------------
  QEMU broker host         `10.0.2.2`
  Host-local broker host   `localhost`
  Port                     `1883`
  Reading topic            `iot/temperature/readings`
  Alert topic              `iot/temperature/alerts`
  QoS                      1
  Retain                   Disabled
  Payload                  JSON

The application accepts `MQTT_BROKER_HOST`; when unset or empty, it
defaults to `localhost`. For the current QEMU user-network setup, the
host broker is reached at `10.0.2.2`.

Reconnect delay configuration uses an initial delay of 2 seconds and
maximum delay of 30 seconds with exponential backoff. The libmosquitto
background network loop is used.

Limitations: - Startup retry when the broker is unavailable before the
initial `mqtt_init()` connection is not implemented. - Durable message
queueing during extended disconnection has not been implemented or
verified. - A successful publish request alone does not prove broker
delivery.

### Payload examples

Reading:

``` json
{
  "timestamp": 1795000000,
  "temperature_c": 30.00,
  "source": "SIMULATED",
  "status": "NORMAL"
}
```

Alert:

``` json
{
  "timestamp": 1795000000,
  "temperature_c": 36.00,
  "source": "SIMULATED",
  "alert_level": "WARNING"
}
```

Timestamps are Unix time in seconds.

------------------------------------------------------------------------

## 4. Buildroot, kernel, and QEMU

**Status: Completed and verified for the Phase 1 x86-64 virtual
target.**

Completed: - Buildroot 2026.08.2 downloaded and configured. - Custom
Buildroot external tree created. - Linux kernel 6.18.7 configured and
built. - EXT2 root filesystem generated. - Custom `iot-gateway`
Buildroot package integrated. - Target executable cross-compiled and
installed. - QEMU launcher created and used successfully.

### Verified QEMU environment

-   Linux kernel: 6.18.7.
-   BusyBox: 1.38.0.
-   Root filesystem: EXT2.
-   Network interface: `eth0`.
-   Guest address: `10.0.2.15/24`.
-   Default gateway: `10.0.2.2`.
-   Root login successful.
-   Init process is PID 1.

### Image measurements

  Item                                     Observed value
  ----------------------------- -------------------------
  `output/images/rootfs.ext2`            62,914,560 bytes
  Root filesystem size                             60 MiB
  Kernel image                                   6.35 MiB
  Combined kernel and rootfs      Approximately 66.35 MiB

The combined figure is the arithmetic sum of the kernel image and root
filesystem file sizes; it is not a complete disk-image measurement.

### Important paths

-   Buildroot source: `buildroot/source/buildroot-2026.08.2/`
-   Buildroot output: `buildroot/source/buildroot-2026.08.2/output/`
-   External tree: `buildroot/external/iot_gateway/`
-   Defconfig:
    `buildroot/external/iot_gateway/configs/iot_gateway_x86_64_defconfig`
-   QEMU launcher: `scripts/qemu/run-qemu.sh`
-   Build script: `scripts/build/build-image.sh`
-   Generated rootfs:
    `buildroot/source/buildroot-2026.08.2/output/images/rootfs.ext2`

Both automation scripts previously passed `bash -n`; help options and
executable permissions were checked. The QEMU launcher successfully
booted the existing image.

------------------------------------------------------------------------

## 5. C application modules

The integrated application consists of sensor acquisition, temperature
processing, alert generation, CSV logging, and MQTT communication.

### Source tree

``` text
application/
├── Makefile
├── include/
│   ├── sensor.h
│   ├── processor.h
│   ├── alert.h
│   ├── logger.h
│   └── mqtt.h
├── src/
│   ├── main.c
│   ├── sensor/sensor.c
│   ├── processing/processor.c
│   ├── alerts/alert.c
│   ├── logging/logger.c
│   └── mqtt/mqtt.c
└── tests/
    ├── test_sensor.c
    ├── test_processor.c
    ├── test_alert.c
    ├── test_logger.c
    ├── test_mqtt.c
    └── test_mqtt_reconnect.c
```

### Module implementation and test evidence

  -------------------------------------------------------------------------
  Module                  Implemented behavior      Recorded result
  ----------------------- ------------------------- -----------------------
  Sensor                  Simulation, timestamps,   PASS
                          source identification,    
                          argument validation,      
                          unsupported hardware      
                          status, deterministic and 
                          realistic modes           

  Processor               Range validation,         PASS
                          NORMAL/WARNING/CRITICAL   
                          classification,           
                          NaN/infinity rejection,   
                          invalid sample handling   

  Alerts                  No alert for NORMAL;      PASS
                          WARNING and CRITICAL      
                          alert generation;         
                          input/status validation   

  Logger                  CSV header, append mode,  PASS
                          readings and alerts,      
                          flushing and cleanup      

  MQTT                    Broker connection, JSON   PASS
                          publication, QoS 1,       
                          reading and alert topics, 
                          cleanup                   

  MQTT reconnect          Reconnect delay/backoff   PASS on host
                          configuration and broker  
                          interruption test         
  -------------------------------------------------------------------------

The integrated pipeline is:

`Sensor → Processor → Alert Generation → CSV Logging → MQTT Publishing`

The main application runs continuously until interrupted. Signal
handling initiates graceful shutdown and resource cleanup.

The integrated application was compiled on Ubuntu with strict GCC
warning flags. A POSIX feature-test macro, `_POSIX_C_SOURCE 200809L`,
was added before includes in `main.c` after strict C11 compilation
initially hid POSIX declarations. The subsequent compilation completed
without warnings or errors.

### Host runtime evidence

The host Mosquitto service was observed active. The integrated
application: - Initialized the sensor and logger. - Connected to MQTT. -
Repeated the deterministic simulated sequence. - Classified 25°C and
30°C as NORMAL, 36°C as WARNING, and 42°C as CRITICAL. - Generated
warning and critical alerts. - Logged CSV records. - Published readings
and alerts received by a `mosquitto_sub` subscriber.

Graceful Ctrl+C shutdown output included: - Shutdown requested. - MQTT
disconnected. - Logger closed. - Sensor released. - Gateway stopped.

CSV fields:

`timestamp,temperature_c,source,status,alert_level`

Observed CSV entries included separate reading and alert records. The
logger opens the file in append mode, so recorded line counts may
include multiple application runs.

------------------------------------------------------------------------

## 6. MQTT integration and reconnection

### Host broker test

The host subscriber used:

``` bash
mosquitto_sub -h localhost -p 1883 -t 'iot/temperature/#' -v
```

It received JSON messages on both: - `iot/temperature/readings` -
`iot/temperature/alerts`

Observed messages contained timestamp, temperature, `SIMULATED` source,
status, and alert level as appropriate.

### Broker interruption test

The host reconnect test ran for 90 seconds: 1. Connected to the local
broker. 2. Published readings. 3. Broker was stopped. 4. Publish
requests failed while disconnected. 5. Broker was restarted. 6. Publish
requests were accepted again.

Recorded observations showed initial connection, publish errors during
interruption, and resumed accepted publish requests after broker
recovery. The test was marked PASS for automatic reconnection and
resumed publishing on native Ubuntu.

Limitations: - Accepted publish requests are not independently proof of
delivery. - Startup retry before the first successful connection is not
implemented. - Durable offline buffering is not implemented or verified.

### QEMU-to-host MQTT test

The QEMU guest connected to the Ubuntu host broker through
`10.0.2.2:1883`. A host subscriber received QEMU-generated messages on
the reading and alert topics.

A realistic simulation test recorded: - 17 reading messages covering
25--41°C. - 5 WARNING alerts covering 35--39°C. - 2 CRITICAL alerts
covering 40--41°C.

All reported sources were `SIMULATED`. This verifies virtual
guest-to-host MQTT communication, not physical hardware operation.

------------------------------------------------------------------------

## 7. Automatic startup and shutdown

**Status: Completed and verified in QEMU.**

The root filesystem overlay contains: -
`buildroot/external/iot_gateway/board/x86_64/rootfs-overlay/etc/init.d/S02security` -
`buildroot/external/iot_gateway/board/x86_64/rootfs-overlay/etc/init.d/S99iot-gateway`

The `S99iot-gateway` BusyBox init service: - Waits for a default network
route. - Starts `/usr/bin/iot_gateway` in the background. - Uses
`/run/iot_gateway.pid` for process tracking. - Sets
`MQTT_BROKER_HOST=10.0.2.2` for the current QEMU user network. -
Supports stop/restart actions. - Sends SIGINT during shutdown to allow
application cleanup.

Verification: - Startup script was copied into the target filesystem and
shell syntax checked. - Fresh rootfs image generated. - QEMU booted and
the gateway started without manual launch. - Gateway process and PID
file were observed. - Host subscriber received automatically generated
MQTT readings and alerts. - `/etc/init.d/S99iot-gateway stop` stopped
the process cleanly. - A guest `poweroff` invoked the stop action; the
gateway stopped before QEMU powered down.

Limitations: - `10.0.2.2` is specific to the current QEMU user-network
arrangement. - Startup retry if the MQTT broker is unavailable before
the initial connection is not implemented. - The daemon log file was
empty during one guest inspection; log redirection needs separate
investigation. - Persistence across QEMU reboot has not been verified.

------------------------------------------------------------------------

## 8. Security hardening

**Status: Implemented and verified in the QEMU guest.**

The `S02security` startup script applies: -
`kernel.dmesg_restrict = 1` - `kernel.kptr_restrict = 2` -
`net.ipv4.tcp_syncookies = 1`

The script passed shell syntax checking and is integrated into the root
filesystem overlay.

Additional verified configuration: - Root account password protection
uses a SHA-512 password hash. - `/etc/shadow` was observed owned by root
with mode `0600`. - Kernel configuration includes
`CONFIG_SYN_COOKIES=y`. - Kernel configuration includes
`CONFIG_STRICT_KERNEL_RWX=y`. - Kernel configuration includes
`CONFIG_STRICT_MODULE_RWX=y`.

After QEMU boot, the three sysctl values were checked and reported as
`1`, `2`, and `1`.

Do not include passwords or password hashes in project documentation.

------------------------------------------------------------------------

## 9. Benchmarking and baseline comparison

### Current custom-image measurements

  ------------------------------------------------------------------------
  Metric                              Observed value Qualification
  --------------------- ---------------------------- ---------------------
  Root filesystem                             60 MiB `rootfs.ext2`,
                                                     62,914,560 bytes

  Kernel image                              6.35 MiB Separate kernel file

  Combined kernel +          Approximately 66.35 MiB Arithmetic sum
  rootfs                                             

  Guest RAM                     Approximately 109 MB QEMU guest
                                                     observation

  Gateway RSS                 2,452--2,548 KB (about Process samples
                                     2.39--2.49 MiB) 

  Gateway virtual        77,568 KB (about 75.75 MiB) Process sample
  memory                                             

  Idle CPU                             99--100% idle Short idle samples

  Gateway CPU, short                    0% displayed Five-sample
  samples                                            observation

  Gateway CPU,            0% in most samples; one 1% One-second interval
  30-second observation               gateway sample sampling

  Load average                                  0.00 Observed during idle
                                                     sampling
  ------------------------------------------------------------------------

The 30-second observation used:

``` sh
top -d 1 -b -n 30 | grep -E 'CPU:|iot_gateway'
```

The gateway process remained present (PID 119) with RSS 2,452 KB.
Overall CPU idle ranged from 98% to 100%. The gateway showed 1% CPU in
one displayed sample and 0% in the other displayed samples.

These are sampled observations under the tested simulated workload, not
peak CPU usage or a comprehensive performance benchmark. Brief activity
between samples may not be captured.

### Boot-time limitation

`/proc/uptime` was observed as `10.23 7.34` shortly after a fresh boot.
This is kernel uptime at the time of inspection, not an exact
QEMU-launch-to-login measurement or gateway-startup duration. Exact boot
time remains unmeasured.

### Stock Buildroot baseline

A stock configuration was generated using `qemu_x86_64_defconfig` and
saved at:

`baseline/stock-config/.config`

Configuration-selection comparison: - Stock target package selections,
excluding host packages: 109. - Custom target package selections,
excluding host packages: 113. - Custom-only selections included cJSON,
the IoT gateway package, and Mosquitto-related packages.

These counts describe configuration selections, not installed runtime
package counts.

A separate stock image was not built. No stock image size, boot time,
RAM, CPU, or runtime comparison is available. No quantitative
performance or image-size improvement over stock Linux is claimed.

### Resource constraint

The Buildroot output directory previously occupied approximately 12 GB,
with limited host disk space remaining (approximately 3.7--3.9 GB in
earlier checks). Avoid a second full Buildroot build or destructive
cleanup until disk space and backups are reviewed. Preserve the existing
working output.

------------------------------------------------------------------------

## 10. Git and repository state

Repository:

`https://github.com/Sahithi-5124/iot-temperature-gateway.git`

Active branch:

`dev/build-system`

Recent confirmed history: - `d6b14fa` --- Add realistic simulation and
security hardening. - `846d0db897fec113ef963af2316cc202c90e158a` ---
previously recorded commit before that update.

The `d6b14fa` commit was pushed successfully to
`origin/dev/build-system`. A subsequent `git status --short` returned no
output, indicating a clean working tree at that check.

Files included in the recent implementation update included: -
`.gitignore` - `PROJECT_STATUS.md` - `application/include/sensor.h` -
`application/src/main.c` - `application/src/sensor/sensor.c` -
`application/tests/test_sensor.c` - `baseline/stock-config/.config` -
`buildroot/external/iot_gateway/board/x86_64/linux.config` -
`buildroot/external/iot_gateway/board/x86_64/rootfs-overlay/etc/init.d/S02security`

Generated Buildroot output, binaries, runtime logs, and generated
baseline metadata should remain excluded from Git. The stock `.config`
is tracked; generated metadata under `baseline/stock-config/` is
ignored.

Do not rely on older historical Git sections in this document for
current staged/commit status; the latest confirmed state above takes
precedence.

------------------------------------------------------------------------

## 11. Project structure

``` text
OS_Project/
├── application/
│   ├── Makefile
│   ├── include/
│   ├── src/
│   └── tests/
├── baseline/
│   └── stock-config/
│       └── .config
├── buildroot/
│   ├── source/
│   │   └── buildroot-2026.08.2/
│   └── external/
│       └── iot_gateway/
│           ├── Config.in
│           ├── external.desc
│           ├── external.mk
│           ├── board/x86_64/
│           ├── configs/
│           ├── package/iot-gateway/
│           └── patches/
├── docs/
├── scripts/
│   ├── build/
│   │   └── build-image.sh
│   └── qemu/
│       └── run-qemu.sh
├── PROJECT_STATUS.md
├── README.md
└── .gitignore
```

Generated files such as `application/iot_gateway`,
`temperature_log.csv`, and Buildroot `output/` are build/runtime
artifacts and should not be committed.

------------------------------------------------------------------------

## 12. Verification summary

  Feature                                Environment         Result
  -------------------------------------- ------------------- ------------------
  Sensor module and simulation modes     Ubuntu host         PASS
  Sensor regression tests                Ubuntu host         PASS
  Processor and threshold tests          Ubuntu host         PASS
  Alert tests                            Ubuntu host         PASS
  Logger tests                           Ubuntu host         PASS
  MQTT initialization/publication        Ubuntu host         PASS
  MQTT subscriber reception              Ubuntu host         PASS
  Broker interruption/reconnection       Ubuntu host         PASS
  Integrated application compilation     Ubuntu host         PASS
  Integrated CSV logging                 Ubuntu host         PASS
  Integrated MQTT readings and alerts    Ubuntu host         PASS
  Graceful application shutdown          Ubuntu host         PASS
  Buildroot package cross-compilation    Buildroot           PASS
  Target application installation        Buildroot           PASS
  EXT2 rootfs generation                 Buildroot           PASS
  QEMU boot and root login               QEMU                PASS
  Application execution and simulation   QEMU                PASS
  Threshold classification and alerts    QEMU                PASS
  QEMU-to-host MQTT delivery             QEMU + Ubuntu       PASS
  Automatic gateway startup              QEMU                PASS
  Init-managed gateway shutdown          QEMU                PASS
  Security sysctl settings               QEMU                PASS
  Root shadow-file permissions           QEMU                PASS
  Short CPU and memory observations      QEMU                RECORDED
  Exact boot-time measurement            ---                 NOT YET MEASURED
  Stock image build and benchmark        ---                 NOT DONE
  Physical sensor operation              Physical hardware   PENDING

------------------------------------------------------------------------

## 13. Remaining work

### Documentation and evaluation

-   Keep this status document current.
-   Improve README build and QEMU execution instructions.
-   Record a reproducible exact boot-time measurement.
-   Continue benchmark documentation with clear methodology.
-   Build and compare a separate stock image only if sufficient storage
    is available and the current working output can be preserved.
-   Do not claim a 40% image-size reduction unless a valid stock-image
    comparison demonstrates it; otherwise document the result and
    limitations.

### MQTT

-   Decide whether startup retry is needed when the broker is
    unavailable at application launch.
-   If tested, verify actual post-recovery message delivery with a
    subscriber and distinguish it from a publish request being accepted.
-   Evaluate whether durable queueing is required.

### Dashboard

-   Node-RED is optional after the core gateway is stable.
-   If time permits, subscribe to the reading and alert topics and build
    temperature visualization and alert indicators.
-   Do not let the dashboard delay the core demonstration.

### Physical hardware

-   Confirm board and sensor model/revision.
-   Verify electrical compatibility and interface.
-   Implement the hardware backend.
-   Configure the actual target architecture, kernel, drivers, and
    device tree.
-   Deploy and test only after the exact hardware and safe procedure are
    confirmed.

------------------------------------------------------------------------

## 14. Current milestone assessment

The Phase 1 virtual Track B gateway is integrated and demonstrated in
QEMU:

-   Custom Buildroot Linux image boots.
-   Gateway starts automatically through BusyBox init.
-   Deterministic and realistic simulated temperature readings work.
-   Temperature classification and WARNING/CRITICAL alerts work.
-   CSV logging is implemented and tested.
-   MQTT readings and alerts reach the Ubuntu host broker.
-   Graceful application and init-managed shutdown are verified.
-   Multiple security-hardening settings are implemented and checked.
-   Preliminary image, memory, and CPU observations are recorded.

Physical sensor functionality, physical-board compatibility, exact boot
time, and a fair stock-image benchmark remain unverified.