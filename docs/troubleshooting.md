# Troubleshooting and Problem Resolution Guide

This document catalogs common issues, root causes, and verified solutions across the build system, C gateway application, networking, and virtualization environments.

---

## 1. Build and Environment Issues

### 1.1 Buildroot Error: "Your PATH contains spaces, TABs... Fix your PATH"

- **Symptom:** Running `make` inside Buildroot fails immediately with:
  ```text
  You seem to have the current directory in your PATH variable, or you have
  spaces in your PATH, which is bad for your build system.
  ```
- **Root Cause:** In Windows Subsystem for Linux (WSL2), Windows environment variables are forwarded into the Linux shell by default, injecting entries such as `/mnt/c/Program Files/...`.
- **Solution:** Sanitize the PATH variable at the beginning of build scripts:
  ```bash
  export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
  ```
  This is implemented automatically in [scripts/build/build-image.sh](file:///c:/Users/vivobook/Downloads/iot-2/scripts/build/build-image.sh).

---

### 1.2 Disk Space Exhaustion on Host Drive

- **Symptom:** Build fails with `No space left on device` or host OS alerts.
- **Root Cause:** Storing Buildroot source trees and compilation caches directly on Windows C: drive when available space is low (< 10 GB).
- **Solution:** Place the Buildroot source and output directory on the native WSL2 virtual disk (`/home/<user>/buildroot-iot2-source`), which operates on a dynamic virtual disk with ample storage (e.g. > 900 GB free). Create a symlink from the project directory:
  ```bash
  ln -s /home/anuhya123/buildroot-iot2-source buildroot/source
  ```

---

### 1.3 Buildroot Staging Symlink and Configuration Location

- **Symptom:** `output/.config` not visible, or `output/staging` symlink points to a stale workspace path (`/home/anuhya123/iot-1/...`).
- **Root Cause:**
  1. In in-tree Buildroot builds, `.config` resides at the top level of the Buildroot source tree (`buildroot-2026.08.2/.config`), not inside `output/`.
  2. The initial build tree copied an absolute symlink for `output/staging -> $(STAGING_DIR)`.
- **Solution:**
  1. Add a convenience symlink `output/.config -> ../.config` so both locations can be inspected.
  2. Update `output/staging` to point to the local target sysroot:
     ```bash
     cd buildroot/source/buildroot-2026.08.2/output
     ln -snf host/x86_64-buildroot-linux-gnu/sysroot staging
     ```

---

## 2. MQTT and Networking Issues

### 2.1 Connection Refused: "not authorised"

- **Symptom:** Subscriber or gateway fails with:
  ```text
  Connection error: Connection Refused: not authorised.
  ```
- **Root Cause:** Mosquitto broker enforces `allow_anonymous false` and client connected without credentials, or supplied credentials not matching the ACL for the target topic.
- **Solution:**
  1. For gateway: configure credentials in `/etc/iot-gateway/mqtt.env`:
     ```sh
     MQTT_USERNAME=gateway_pub
     MQTT_PASSWORD=<SECRET_PASSWORD>
     ```
  2. For subscriber: provide authenticated credentials to `mosquitto_sub`:
     ```bash
     mosquitto_sub -h localhost -p 1883 -u monitor_sub -P <PASSWORD> -t 'iot/temperature/#' -v
     ```
  3. Ensure `/etc/mosquitto/acl.conf` grants write access for `gateway_pub` to `iot/temperature/readings` and `iot/temperature/alerts`, and read access for `monitor_sub` to `iot/temperature/#`.

---

### 2.2 Host Subscriber Receives No Messages

- **Symptom:** Running `mosquitto_sub -t 'iot/temperature/#'` produces no output even though the gateway is running.
- **Troubleshooting Steps:**
  1. Check if the gateway is running:
     ```sh
     ps | grep iot_gateway
     ```
  2. Inspect the gateway output log inside the guest:
     ```sh
     cat /var/log/iot_gateway.log
     ```
  3. Verify host network connectivity from inside the guest:
     ```sh
     ping -c 2 10.0.2.2
     ```
  4. Ensure topic pattern matches correctly (`iot/temperature/#` matches all subtopics).

---

## 3. Application and Logging Issues

### 3.1 Gateway Log File Appears Empty During Execution

- **Symptom:** While running in background daemon mode, inspecting `/var/log/iot_gateway.log` shows 0 bytes.
- **Root Cause:** Standard C stdio streams (`stdout`) use full block buffering (4096 bytes) when redirected to a file, delaying visible output until 4 KB of text accumulates.
- **Solution:** Configure line buffering on startup in [application/src/main.c](file:///c:/Users/vivobook/Downloads/iot-2/application/src/main.c):
  ```c
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  ```

---

### 3.2 CSV Log Disappears After VM Reboot

- **Symptom:** Temperature records are lost when QEMU is shut down and restarted.
- **Root Cause:**
  1. Storing logs under `/var/log` or `/tmp`. In Buildroot, `/var/log` is a symlink to `/tmp`, which is mounted as a volatile `tmpfs` in RAM.
  2. Data cached in the kernel page cache without committing to disk blocks.
- **Solution:**
  1. Store the persistent CSV log on a real disk path: `/root/temperature_log.csv` (resides on `/dev/vda`).
  2. Call `fsync(fileno(log_file))` immediately after writing and flushing each CSV row in [application/src/logging/logger.c](file:///c:/Users/vivobook/Downloads/iot-2/application/src/logging/logger.c).

---

## 4. Virtualization Issues

### 4.1 KVM Permission Denied

- **Symptom:** Launching QEMU fails with `Could not access KVM kernel module: Permission denied`.
- **Root Cause:** Current host user account is not a member of the `kvm` group.
- **Solution:**
  1. Add user to the group: `sudo usermod -aG kvm $USER`.
  2. Alternatively, the launch script [scripts/qemu/run-qemu.sh](file:///c:/Users/vivobook/Downloads/iot-2/scripts/qemu/run-qemu.sh) automatically includes fallback flags:
     ```bash
     -accel kvm -accel tcg
     ```
     If KVM is inaccessible, QEMU smoothly falls back to TCG emulation without aborting.

---

### 4.2 Terminal Cluttered or Garbled After QEMU Exit

- **Symptom:** Shell terminal does not echo keystrokes or displays strange characters after exiting QEMU.
- **Root Cause:** QEMU modifies terminal TTY raw settings when running with `-nographic`.
- **Solution:** Run the standard POSIX reset command:
  ```bash
  reset
  ```
