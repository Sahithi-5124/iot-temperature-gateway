# System Architecture — IoT Temperature Gateway Linux Distribution

## 1. Distribution Architectural Philosophy

The Track B IoT Temperature Gateway is an **application-specific minimal Linux distribution**. Unlike general-purpose distributions (such as Ubuntu, Debian, or Alpine) that contain broad package ecosystems, multi-user utilities, and dynamic service managers, this distribution is constructed from the ground up to fulfill a single purpose: run the IoT Temperature-Monitoring Gateway securely and reliably.

### Design Principles:
1. **Minimal Attack Surface:** Eliminates unneeded daemons, listening sockets, interpreters, compilers, and user accounts.
2. **Deterministic Bootstrapping:** Boots straight to the application in under 2 seconds using lightweight BusyBox init without systemd overhead.
3. **Strict Separation of Concerns:** Modular C gateway architecture where sensor acquisition, validation, classification, alert dispatching, CSV persistence, and network telemetry operate through clearly defined interfaces.
4. **Resilience to Failure:** Safe handling of offline brokers, line-buffered output redirection, and data synchronization (`fsync`) before system poweroff.

---

## 2. Block Diagram

```text
+-------------------------------------------------------------------------------+
|                             QEMU Virtual Machine                              |
|                                                                               |
|  +-------------------------------------------------------------------------+  |
|  |                     Linux Kernel 6.18.7 (x86_64)                        |  |
|  |       (STRICT_KERNEL_RWX, SYN_COOKIES, VirtIO Network & Block)          |  |
|  +-------------------------------------------------------------------------+  |
|                                       |                                       |
|  +-------------------------------------------------------------------------+  |
|  |                         BusyBox Init (1.38.0)                           |  |
|  |        +-------------------------------------------------------+        |  |
|  |        | S02security: Kernel sysctls (dmesg, kptr, syncookies) |        |  |
|  |        +-------------------------------------------------------+        |  |
|  |        | S40network:  DHCP eth0 setup (10.0.2.15 via 10.0.2.2) |        |  |
|  |        +-------------------------------------------------------+        |  |
|  |        | S99iot-gateway: Process tracking, daemon startup      |        |  |
|  |        +-------------------------------------------------------+        |  |
|  +-------------------------------------------------------------------------+  |
|                                       |                                       |
|  +------------------------------------+------------------------------------+  |
|  |                     C IoT Gateway Daemon (/usr/bin/iot_gateway)         |  |
|  |                                                                         |  |
|  |    +-----------------------+           +---------------------------+    |  |
|  |    | Simulated Sensor      |           | Input Validation Engine   |    |  |
|  |    | (Deterministic / Real)| --------> | (Rejects out-of-range,    |    |  |
|  |    | Tag: "SIMULATED"      |           |  NaN, Infinity, malformed)|    |  |
|  |    +-----------------------+           +---------------------------+    |  |
|  |                                                      |                  |  |
|  |                                                      v                  |  |
|  |    +-----------------------+           +---------------------------+    |  |
|  |    | Alert Event Generator |           | Threshold Classifier      |    |  |
|  |    | (Warning / Critical)  | <-------- | (<35 Normal, 35-39 Warn,  |    |  |
|  |    +-----------------------+           |  >=40 Critical)           |    |  |
|  |                |                       +---------------------------+    |  |
|  |                +-------------------+                 |                  |  |
|  |                                    |                 |                  |  |
|  |                                    v                 v                  |  |
|  |                       +-----------------------------------+             |  |
|  |                       | Local CSV Logger (/root/*.csv)    |             |  |
|  |                       | (Appends & fsync to /dev/vda)     |             |  |
|  |                       +-----------------------------------+             |  |
|  |                                    |                                    |  |
|  |                                    v                                    |  |
|  |                       +-----------------------------------+             |  |
|  |                       | MQTT Telemetry Client (libmosq)   |             |  |
|  |                       | (Async connect, background retry) |             |  |
|  |                       +-----------------------------------+             |  |
|  +------------------------------------+------------------------------------+  |
+---------------------------------------|---------------------------------------+
                                        | VirtIO Network (10.0.2.2:1883)
                                        v
+-------------------------------------------------------------------------------+
|                            Host Machine (Ubuntu WSL2)                         |
|                                                                               |
|  +-------------------------------------------------------------------------+  |
|  |                       Mosquitto MQTT Broker (2.0.18)                    |  |
|  |       Topics: iot/temperature/readings  |  iot/temperature/alerts       |  |
|  +-------------------------------------------------------------------------+  |
|                                       |                                       |
|  +-------------------------------------------------------------------------+  |
|  |                   Host Subscriber (mosquitto_sub CLI)                   |  |
|  +-------------------------------------------------------------------------+  |
+-------------------------------------------------------------------------------+
```

---

## 3. Data Processing Pipeline

1. **Acquisition:** The sensor module (`sensor.c`) generates temperature samples. In `DETERMINISTIC` mode, it iterates through `25.0°C`, `30.0°C`, `36.0°C`, and `42.0°C`. In `REALISTIC` mode, it performs a bounded random walk between `20.0°C` and `42.0°C`. All samples set `source = SENSOR_SOURCE_SIMULATED`.
2. **Validation:** The processor module (`processor.c`) inspects the sample. It verifies that `sample->valid == true`, `isfinite(temperature)`, and `-40.0°C <= temperature <= 85.0°C`. Invalid inputs are rejected.
3. **Classification:**
   - `< 35.0°C`: `TEMPERATURE_STATUS_NORMAL`
   - `35.0°C` to `< 40.0°C`: `TEMPERATURE_STATUS_WARNING`
   - `≥ 40.0°C`: `TEMPERATURE_STATUS_CRITICAL`
4. **Alert Generation:** If status is `WARNING` or `CRITICAL`, `alert_generate()` constructs an alert event with the matching severity level. Normal readings produce `ALERT_GENERATION_NONE`.
5. **Local Persistence:** `logger_log_temperature()` and `logger_log_alert()` format records into CSV format (`timestamp,temperature_c,source,status,alert_level`), flush the stdio stream, and issue `fsync()` to ensure writes hit the persistent virtual disk block layer.
6. **Network Dispatch:** `mqtt_publish_temperature()` and `mqtt_publish_alert()` format JSON strings and publish via `libmosquitto` at QoS 1. If the broker is unreachable, background threads manage exponential reconnection backoff without stalling local gateway processing.
