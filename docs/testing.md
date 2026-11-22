# Testing and Verification Report

## 1. Testing Philosophy and Strategy

Verification of the IoT Temperature Gateway follows a multi-tiered test strategy covering:
1. **Unit Testing:** Isolated functional verification of each C module with edge-case validation and error rejection.
2. **Integration Testing:** Network protocol testing against live MQTT brokers.
3. **Virtual Machine Boot Testing:** Automated kernel and userland boot verification under QEMU.
4. **End-to-End Telemetry Testing:** Full pipeline verification from simulated sensor in guest VM to host MQTT subscribers.
5. **Durability and Outage Testing:** Broker downtime handling and filesystem persistence across guest shutdowns.

```
+------------------------------------------------------------------------+
|                          Verification Hierarchy                        |
|                                                                        |
|  [ Level 5: Persistence ]  - Ext2 block inspection after VM shutdown   |
|  [ Level 4: E2E Pipeline ] - Host mosquitto_sub receiving guest JSON   |
|  [ Level 3: VM Runtime ]   - BusyBox init, PID files, sysctl settings  |
|  [ Level 2: Integration ]  - Real TCP/MQTT handshake and reconnection  |
|  [ Level 1: Unit Tests ]   - Sensor, processor, alert, logger modules  |
+------------------------------------------------------------------------+
```

---

## 2. Unit Testing Suite

The C application provides a comprehensive unit test suite located in [application/tests/](file:///c:/Users/vivobook/Downloads/iot-2/application/tests/).

All tests compile with `-std=c11 -Wall -Wextra -Wpedantic` and execute without memory faults:

```bash
cd application
make test
```

### 2.1 Test Suite Breakdown

| Suite | File | Scope / Assertions | Result |
|---|---|---|---|
| **Sensor Test** | `test_sensor.c` | Verifies deterministic sequence (25, 30, 36, 42°C); realistic random-walk bounds ([20, 42°C], step $\le 1^\circ\text{C}$); source tag matches `"SIMULATED"`; rejects NaN, $+\infty$, $-\infty$, $<-40^\circ\text{C}$, $>85^\circ\text{C}$. | **PASS (100%)** |
| **Processor Test** | `test_processor.c` | Validates threshold classifications (`NORMAL` for $<35$, `WARNING` for $35..39.99$, `CRITICAL` for $\ge 40$); tests exact boundary conditions ($34.99$, $35.00$, $39.99$, $40.00^\circ\text{C}$). | **PASS (100%)** |
| **Alert Test** | `test_alert.c` | Verifies alert formatting; ensures alerts are emitted only on `WARNING` and `CRITICAL`; confirms suppression on `NORMAL`. | **PASS (100%)** |
| **Logger Test** | `test_logger.c` | Validates initial CSV header creation; verifies record formatting; checks append behavior; confirms `fflush` and `fsync` invocation. | **PASS (100%)** |
| **MQTT Test** | `test_mqtt.c` | Verifies topic strings; validates JSON schema serialization; confirms QoS 1 and retain=false; validates asynchronous connect logic with `on_connect` callback waiting; tests authenticated publish and subscriber verification. | **PASS (100%)** |
| **MQTT Reconnect Test** | `test_mqtt_reconnect.c` | Validates non-blocking reconnection behavior; tests state transitions during broker outage; monitors periodic tick state without false connection reporting. | **PASS (100%)** |

**Summary:** 6/6 test suites passed (100% pass rate).

---

## 3. Integration, Security & Outage Testing

### 3.1 Network Handshake & Security Authentication Testing
Automated script: `scripts/test-mqtt-auth.sh`
- **Anonymous Rejection:** Verified that unauthenticated client connections are rejected with `Connection Refused: not authorised` (`PASS`).
- **Authenticated Publisher/Subscriber:** Verified `gateway_pub` successfully publishes to `iot/temperature/readings` and `iot/temperature/alerts`, and `monitor_sub` receives both streams (`PASS`).
- **ACL Enforcement:** Verified that an unauthorized user account cannot read or publish to temperature topics (`PASS`).
- **Password File Security:** Verified `/etc/mosquitto/passwd` ownership is `mosquitto:mosquitto` and permissions are strictly `0600` (`PASS`).

### 3.2 Broker Outage Simulation & Local Logging Durability
- **Scenario:** The gateway was launched while the broker was deliberately offline (service stopped).
- **Observed Behavior:**
  - The daemon did not crash or block.
  - Startup proceeded immediately into the acquisition and logging loop.
  - CSV records were continuously written, flushed, and committed via `fsync()` to the local CSV log file.
  - Reconnect attempts progressed in the background.
  - When the broker was started, the daemon automatically re-established the authenticated connection and resumed MQTT publishing without restarting the gateway process.

---

## 4. Virtual Machine Runtime Verification

The built image was booted inside QEMU 8.2.2 with KVM hardware acceleration:

```bash
./scripts/qemu/run-qemu.sh -d -s
```

### Verified Runtime Milestones:
1. **Boot Time:** Linux 6.18.7 kernel initialized in 0.98s; total userspace ready in **1.73 seconds**.
2. **Security Init:** `/etc/init.d/S02security` applied sysctl hardening rules before daemon startup:
   - `kernel.dmesg_restrict = 1`
   - `kernel.kptr_restrict = 2`
   - `net.ipv4.tcp_syncookies = 1`
3. **Daemon Initialization:** `/etc/init.d/S99iot-gateway` started the gateway in background mode:
   - Process running under PID 125 (`iot_gateway`).
   - PID file recorded at `/run/iot_gateway.pid`.
   - Log output redirected to `/var/log/iot_gateway.log`.

---

## 5. End-to-End Telemetry Verification

With the host Mosquitto broker listening and QEMU SLIRP networking mapping the host to `10.0.2.2`, a host subscriber was executed:

```bash
mosquitto_sub -h localhost -p 1883 -t 'iot/temperature/#' -v
```

### Empirical Telemetry Stream Captured on Host:
```text
iot/temperature/readings {"timestamp":"2026-11-22T09:47:32Z","temperature_c":25.00,"source":"SIMULATED","status":"VALID","alert_level":"NORMAL"}
iot/temperature/readings {"timestamp":"2026-11-22T09:47:34Z","temperature_c":30.00,"source":"SIMULATED","status":"VALID","alert_level":"NORMAL"}
iot/temperature/readings {"timestamp":"2026-11-22T09:47:36Z","temperature_c":36.00,"source":"SIMULATED","status":"VALID","alert_level":"WARNING"}
iot/temperature/alerts {"timestamp":"2026-11-22T09:47:36Z","temperature_c":36.00,"source":"SIMULATED","status":"VALID","alert_level":"WARNING","alert_message":"WARNING: Temperature elevated (36.00 C)"}
iot/temperature/readings {"timestamp":"2026-11-22T09:47:38Z","temperature_c":42.00,"source":"SIMULATED","status":"VALID","alert_level":"CRITICAL"}
iot/temperature/alerts {"timestamp":"2026-11-22T09:47:38Z","temperature_c":42.00,"source":"SIMULATED","status":"VALID","alert_level":"CRITICAL","alert_message":"CRITICAL: Temperature dangerous (42.00 C)"}
```

**Verification:** Telemetry readings arrived every 2 seconds. Alerts triggered only on `WARNING` and `CRITICAL` conditions. All readings bore the source tag `SIMULATED`.

---

## 6. Durability & Disk Persistence Verification

To verify that logged data survives guest shutdown and is written to persistent disk blocks:
1. The gateway was allowed to run in QEMU, writing records to `/root/temperature_log.csv` on `/dev/vda`.
2. A clean shutdown was initiated via `poweroff`.
3. The guest disk image (`rootfs.ext2`) was inspected directly on the host using `debugfs`:
   ```bash
   debugfs -R 'dump /root/temperature_log.csv /tmp/recovered_log.csv' rootfs.ext2
   cat /tmp/recovered_log.csv
   ```
4. **Result:** The CSV header and all recorded data entries were present, intact, and undamaged, confirming that `fsync()` committed records to the virtual disk blocks.
