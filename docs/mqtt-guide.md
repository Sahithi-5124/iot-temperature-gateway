# MQTT Communication and Telemetry Guide

## 1. Overview

The IoT Temperature Gateway uses MQTT (Message Queuing Telemetry Transport) v3.1.1 to publish simulated sensor telemetry and alerts to an external MQTT broker. The implementation utilizes `libmosquitto` and operates seamlessly across the virtual network boundary between the QEMU virtual machine and the development host.

```
+-----------------------------------+             +----------------------------------+
|      QEMU Guest (Linux 6.18.7)    |             |        Host / Development PC     |
|                                   |             |                                  |
|   +--------------------------+    |             |    +-----------------------+     |
|   |   iot_gateway (C app)    |    |             |    |   Mosquitto Broker    |     |
|   |  - libmosquitto client   |    |    SLIRP    |    |   Listening 0.0.0.0   |     |
|   |  - Async non-blocking    | ---+------------->    |   Port 1883           |     |
|   |  - Publishes QoS 1       |    |  (10.0.2.2) |    +-----------------------+     |
|   +--------------------------+    |             |               |                  |
|                                   |             |               v                  |
|                                   |             |    +-----------------------+     |
|                                   |             |    | mosquitto_sub client  |     |
|                                   |             |    | (Verification/Monitor)|     |
|                                   |             |    +-----------------------+     |
+-----------------------------------+             +----------------------------------+
```

---

## 2. Network Topology & Addressing

Under QEMU SLIRP user-mode networking:
- The QEMU guest is assigned a private IP via internal DHCP (typically `10.0.2.15`).
- The development host machine is accessible from inside the guest at fixed IP **`10.0.2.2`**.
- The gateway daemon connects to `10.0.2.2:1883` by default when running in the virtual machine.

### Host Broker Configuration
The host MQTT broker is configured with authentication enabled, anonymous access disabled, and strict topic ACLs:

```conf
# /etc/mosquitto/mosquitto.conf
listener 1883 0.0.0.0
allow_anonymous false
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl.conf

# Logging configuration
log_dest file /var/log/mosquitto/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information

persistence false
```

### Access Control List (ACL) Configuration
Located at `/etc/mosquitto/acl.conf`:
```conf
# Gateway Publisher Account (Write access only to readings and alerts)
user gateway_pub
topic write iot/temperature/readings
topic write iot/temperature/alerts

# Monitoring Subscriber Account (Read access to temperature tree)
user monitor_sub
topic read iot/temperature/#
```

To verify the broker is active on the host:
```bash
sudo systemctl status mosquitto
# or verify port listening:
ss -tulpn | grep 1883
```

---

## 3. Topics and Message Payloads

The gateway publishes to two dedicated topics. All messages are JSON-encoded and published with **QoS 1** (At least once) and **Retain = false**.

### 3.1 Telemetry Reading Topic: `iot/temperature/readings`

Published periodically every sampling interval (default: 2 seconds in VM).

#### Schema
```json
{
  "timestamp": "string (ISO-8601 UTC)",
  "temperature_c": "number (float, 2 decimals)",
  "source": "SIMULATED",
  "status": "VALID",
  "alert_level": "NORMAL | WARNING | CRITICAL"
}
```

#### Example Payloads
```json
{"timestamp":"2026-11-22T09:47:32Z","temperature_c":25.00,"source":"SIMULATED","status":"VALID","alert_level":"NORMAL"}
{"timestamp":"2026-11-22T09:47:34Z","temperature_c":30.00,"source":"SIMULATED","status":"VALID","alert_level":"NORMAL"}
{"timestamp":"2026-11-22T09:47:36Z","temperature_c":36.00,"source":"SIMULATED","status":"VALID","alert_level":"WARNING"}
{"timestamp":"2026-11-22T09:47:38Z","temperature_c":42.00,"source":"SIMULATED","status":"VALID","alert_level":"CRITICAL"}
```

---

### 3.2 Alert Topic: `iot/temperature/alerts`

Published only when the temperature condition is classified as `WARNING` (\(35.0^\circ\text{C} \le T < 40.0^\circ\text{C}\)) or `CRITICAL` (\(T \ge 40.0^\circ\text{C}\)). Normal readings do not emit alert messages.

#### Schema
```json
{
  "timestamp": "string (ISO-8601 UTC)",
  "temperature_c": "number (float, 2 decimals)",
  "source": "SIMULATED",
  "status": "VALID",
  "alert_level": "WARNING | CRITICAL",
  "alert_message": "string"
}
```

#### Example Payloads
```json
{"timestamp":"2026-11-22T09:47:36Z","temperature_c":36.00,"source":"SIMULATED","status":"VALID","alert_level":"WARNING","alert_message":"WARNING: Temperature elevated (36.00 C)"}
{"timestamp":"2026-11-22T09:47:38Z","temperature_c":42.00,"source":"SIMULATED","status":"VALID","alert_level":"CRITICAL","alert_message":"CRITICAL: Temperature dangerous (42.00 C)"}
```

---

## 4. Quality of Service (QoS) & Reliability

- **QoS Level 1:** Every published packet requires a `PUBACK` from the broker. This ensures packets are delivered through the network stack without silent drops.
- **Retain = false:** Telemetry and alert messages are real-time events. Retain is explicitly disabled to prevent stale temperature values or old critical alerts from being dispatched to newly connected subscribers.
- **Non-blocking Network Loop:** Network packet exchanges are managed on a background POSIX thread spawned by `mosquitto_loop_start()`. The main sensor loop remains responsive regardless of network latency.

### Network Outages and Disconnections
- When the connection to the MQTT broker drops or fails to initialize:
  1. The `on_disconnect` callback transitions internal state to `disconnected`.
  2. The gateway logs the disconnection to stderr/syslog.
  3. The gateway daemon continues sensor acquisition and local CSV logging without disruption.
  4. An automatic reconnection loop is scheduled with exponential backoff:
     $$\text{delay}_{i} = \min(2^i, 30)\text{ seconds}$$
     (Sequence: 2s, 4s, 8s, 16s, 30s).
  5. Once the broker becomes reachable, the connection is re-established automatically without requiring a daemon or VM restart.
- **Delivery Guarantees During Outages:** The client does not maintain an unbounded on-disk message queue during network partitions. In-flight messages dropped during an active disconnection are discarded in RAM to protect system memory. The complete historical audit trail is preserved locally in `/root/temperature_log.csv`.

---

## 5. Verification & Monitoring Commands

To monitor the gateway's MQTT output from the host machine using the authenticated monitoring account:

### Subscribe to All Telemetry and Alerts (Authenticated)
```bash
mosquitto_sub -h localhost -p 1883 -u monitor_sub -P <MONITOR_PASSWORD> -t 'iot/temperature/#' -v
```

### Subscribe Exclusively to Alerts
```bash
mosquitto_sub -h localhost -p 1883 -u monitor_sub -P <MONITOR_PASSWORD> -t 'iot/temperature/alerts' -v
```

### Format Output with `jq`
```bash
mosquitto_sub -h localhost -p 1883 -u monitor_sub -P <MONITOR_PASSWORD> -t 'iot/temperature/readings' | jq .
```

### Verify Anonymous Access is Rejected
```bash
# This must fail with "Connection Refused: not authorised"
mosquitto_sub -h localhost -p 1883 -t 'iot/temperature/#' -v
```

---

## 6. Account Creation and Password Database Administration

To create or update the authenticated accounts:

```bash
# 1. Create or update user password (batch mode)
sudo mosquitto_passwd -b /etc/mosquitto/passwd gateway_pub <NEW_PASSWORD>
sudo mosquitto_passwd -b /etc/mosquitto/passwd monitor_sub <NEW_PASSWORD>

# 2. Lock password file permissions
sudo chown mosquitto:mosquitto /etc/mosquitto/passwd
sudo chmod 0600 /etc/mosquitto/passwd

# 3. Reload Mosquitto configuration
sudo systemctl restart mosquitto
```
