# Security Architecture and Hardening

## 1. Security Overview and Threat Model

The IoT Temperature Gateway operates as a dedicated, single-purpose embedded device. In connected IoT environments, edge gateways are subject to network probing, denial-of-service attempts, local privilege escalation, and side-channel inspection.

To secure this platform, hardening measures were implemented across the kernel configuration, sysctl runtime parameters, authentication subsystems, and filesystem permissions.

```
+------------------------------------------------------------------------+
|                     Layered Security Defenses                          |
|                                                                        |
|  [ Network Layer ]    - net.ipv4.tcp_syncookies=1                      |
|                       - Minimal open ports (No SSH/Telnet/HTTP daemon) |
|                                                                        |
|  [ Kernel Layer ]     - CONFIG_STRICT_KERNEL_RWX=y                     |
|                       - CONFIG_STRICT_MODULE_RWX=y                     |
|                       - kernel.kptr_restrict=2 (Hides all %pK addresses|
|                       - kernel.dmesg_restrict=1 (Restricts log buffer) |
|                                                                        |
|  [ OS / FS Layer ]    - Root password encrypted with SHA-512           |
|                       - /etc/shadow permissions locked to 0600         |
|                       - Stripped minimal rootfs (no compilers, GDB)    |
+------------------------------------------------------------------------+
```

---

## 2. Kernel Hardening Configurations

The Linux kernel (version 6.18.7) was compiled with specific hardening flags enabled in the Buildroot defconfig:

1. **`CONFIG_STRICT_KERNEL_RWX=y` (Write-xor-Execute Kernel Memory):**
   - Strictly enforces memory protections such that kernel code pages are mapped read-only and executable, while data pages are readable/writable but non-executable (`NX`).
   - Prevents memory-corruption vulnerabilities from executing shellcode directly in kernel space.

2. **`CONFIG_STRICT_MODULE_RWX=y`:**
   - Applies the same `W^X` memory protection principles to dynamically loaded kernel modules.

3. **`CONFIG_SYN_COOKIES=y`:**
   - Compiles kernel support for cryptographic TCP SYN cookies to mitigate SYN-flood Denial of Service (DoS) attacks on listening sockets.

---

## 3. Sysctl Runtime Parameter Hardening

Runtime security policies are automatically applied during system boot via `/etc/init.d/S02security` before any network daemons initialize:

### 3.1 Kernel Pointer Restriction (`kernel.kptr_restrict=2`)
- **Setting:** `sysctl -w kernel.kptr_restrict=2`
- **Security Objective:** Prevents information disclosure attacks. In mode `2`, all kernel pointers printed using `%pK` are replaced with zeroes (`0000000000000000`) for all users, including root, unless specifically requested by an authorized kernel trace. This prevents attackers from extracting kernel address locations to bypass Kernel Address Space Layout Randomization (KASLR).

### 3.2 Kernel Ring Buffer Restriction (`kernel.dmesg_restrict=1`)
- **Setting:** `sysctl -w kernel.dmesg_restrict=1`
- **Security Objective:** Restricts access to the `dmesg` kernel ring buffer to users with `CAP_SYSLOG`. Unprivileged processes cannot inspect kernel messages, preventing them from gleaning hardware identifiers, kernel backtraces, or memory layout details.

### 3.3 TCP SYN Flood Protection (`net.ipv4.tcp_syncookies=1`)
- **Setting:** `sysctl -w net.ipv4.tcp_syncookies=1`
- **Security Objective:** Activates SYN cookies when the TCP socket listen backlog queue overflows. The kernel generates a cryptographic challenge cookie encoded in the sequence number rather than dropping incoming connections or exhausting RAM with half-open connection state tables.

---

## 4. Authentication & Credential Storage

### 4.1 SHA-512 Password Hashing
- The root account password is cryptographically protected using salted SHA-512 hashes (`$6$...`).
- Buildroot configuration: `BR2_TARGET_GENERIC_ROOT_PASSWD_SHA512=y`.
- Passwords and private credentials are never committed to the source control repository in plaintext.

### 4.2 File Permissions
- `/etc/shadow` ownership is set to `root:root` with strict permissions:
  ```text
  -rw-------    1 root     root           343 Nov 22 15:00 /etc/shadow
  ```
  Mode `0600` ensures that only the `root` superuser can read encrypted password hashes, mitigating local offline dictionary and brute-force attacks.

### 4.3 Gateway Runtime Credential Security (`/etc/iot-gateway/mqtt.env`)
- The gateway daemon does not contain hardcoded credentials in C source code.
- Credentials are read at runtime from `/etc/iot-gateway/mqtt.env` (or environment variables).
- On the running target, permissions are locked to `0600` (`-rw-------`) owned by `root:root` by `S02security` and `S99iot-gateway`.
- Passwords are never committed to Git; `mqtt.env` is excluded via `.gitignore`, while `mqtt.env.example` provides the sanitized template.

### 4.4 Mosquitto Broker Password Database Hardening (`/etc/mosquitto/passwd`)
- The password file is owned by `mosquitto:mosquitto` with mode `0600` (`-rw-------`):
  ```text
  -rw------- 1 mosquitto mosquitto 375 Nov 22 11:18 /etc/mosquitto/passwd
  ```
- This ensures only the Mosquitto broker daemon process and the system root user can read password hashes.
- World and group read permissions are completely disabled.

### 4.5 Strict Topic-Level Access Control Lists (ACLs)
The broker configuration enforces the Principle of Least Privilege in `/etc/mosquitto/acl.conf`:
- **Anonymous Access:** Explicitly disabled (`allow_anonymous false`). Unauthenticated clients are rejected with `Connection Refused: not authorised`.
- **Gateway Publisher Account (`gateway_pub`):** Write-only permissions strictly restricted to:
  - `topic write iot/temperature/readings`
  - `topic write iot/temperature/alerts`
  No read permissions and no wildcard rights are granted.
- **Monitoring Subscriber Account (`monitor_sub`):** Read-only permissions:
  - `topic read iot/temperature/#`
  No write or publish permissions are granted.
- **Unauthorized Accounts:** Any account without matching ACL entries is blocked from reading or writing temperature data.

---

## 5. Attack Surface Minimization

1. **Elimination of Superfluous Daemons:**
   - The custom Buildroot distribution contains **no** remote shell servers (no SSH daemon, no Telnet, no rlogin), no web servers (no Apache/Nginx), and no remote debugging utilities.
   - The only active userspace processes are:
     - `init` (BusyBox PID 1)
     - `syslogd` and `klogd` (system logging)
     - `iot_gateway` (gateway daemon)
2. **Minimal Target Utilities:**
   - Compilers (`gcc`), interpreters (`python`), debuggers (`gdb`), and package managers (`apt`/`apk`) are excluded from the root filesystem image. Attackers gaining execution capability cannot compile payloads or leverage common LOLBins (Living Off the Land Binaries).

---

## 6. Live Verification in Running System

Execute the following commands in the running QEMU guest to verify active security policies:

```sh
# 1. Verify sysctl hardening parameters
sysctl kernel.dmesg_restrict kernel.kptr_restrict net.ipv4.tcp_syncookies

# Expected Output:
# kernel.dmesg_restrict = 1
# kernel.kptr_restrict = 2
# net.ipv4.tcp_syncookies = 1

# 2. Verify shadow file permissions
ls -l /etc/shadow

# Expected Output:
# -rw-------    1 root     root           ... /etc/shadow

# 3. Verify running network listeners
netstat -tulpn

# Expected: No unexpected listening TCP/UDP ports
```

---

## 7. Limitations & Production Recommendations

While the current security posture satisfies the requirements for the isolated Track B demonstration, the following enhancements are recommended for commercial deployment:

| Area | Current Lab State | Production Recommendation |
|---|---|---|
| Gateway Execution User | Runs as `root` (UID 0) | Create dedicated `iotgw` system user (UID 1001) with dropped capabilities |
| Root Filesystem | Read-Write (`ext2`) | Mount root filesystem read-only (`ro`), with a separate encrypted `dm-crypt` partition for persistent logs |
| Network Encryption | Plaintext MQTT (TCP 1883) | MQTT over TLS v1.3 (TCP 8883) with mutual x509 certificate authentication (mTLS) |
| Boot Integrity | Standard direct kernel boot | UEFI Secure Boot with signed `shim` / `grub` and `dm-verity` rootfs verification |
