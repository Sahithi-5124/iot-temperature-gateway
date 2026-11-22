#!/usr/bin/env bash
set -e

REPO_DIR="/mnt/c/Users/vivobook/Downloads/iot-2"
cd "$REPO_DIR"

echo "=========================================================="
echo "    TRACK B IoT TEMPERATURE GATEWAY - FINAL AUDIT SCRIPT   "
echo "=========================================================="
echo

echo "==================== 1. GIT INTEGRITY ===================="
echo "Project Root: $(pwd)"
echo "Current Branch: $(git branch --show-current)"
echo "Latest Commits:"
git log -3 --oneline
echo
echo "Tracked Files Summary:"
git ls-files | wc -l | awk '{print "Total tracked files:", $1}'
echo

echo "================ 2. STRUCTURE & DELIVERABLES ============="
echo "Top-level directories:"
find . -maxdepth 2 -type d -not -path './.git*' -not -path '*/output*' | sort
echo
echo "Core Status & Readme:"
ls -lh README.md PROJECT_STATUS.md
echo
echo "Documentation files in docs/:"
find docs -maxdepth 2 -type f | sort
echo

echo "================ 3. BUILDROOT & IMAGE ARTIFACTS =========="
BR="$REPO_DIR/buildroot/source/buildroot-2026.08.2"
OUT="$BR/output"

echo "Builder Version:"
make -C "$BR" --version | head -n 1
echo
echo "Generated Images in output/images/:"
ls -lh "$OUT/images/"
echo
echo "Kernel Image File Type:"
file "$OUT/images/bzImage"
echo
echo "Rootfs Image File Type:"
file "$OUT/images/rootfs.ext2"
echo
echo "Image Sizes:"
du -h "$OUT/images/bzImage" "$OUT/images/rootfs.ext2"
echo
echo "Target Kernel Version in .config:"
grep -E "BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE" "$OUT/.config"
echo
echo "Target Architecture in .config:"
grep -E "^BR2_x86_64=y" "$OUT/.config"
echo
echo "Application Binary Architecture on Target:"
file "$OUT/target/usr/bin/iot_gateway"
echo

echo "================ 4. APPLICATION TESTS ===================="
cd "$REPO_DIR/application"
make clean >/dev/null 2>&1
echo "Running offline C unit tests:"
make test
echo "TEST EXIT CODE: $?"
cd "$REPO_DIR"
echo

echo "================ 5. MQTT SECURITY & ACL AUDIT ============="
if [ -f "/tmp/mqtt_test_env.sh" ]; then
    bash "$REPO_DIR/scripts/test-mqtt-auth.sh"
else
    echo "WARNING: /tmp/mqtt_test_env.sh not found. Skipping live broker test."
fi
echo

echo "================ 6. SECURITY CONFIGURATION ================"
echo "Mosquitto Password Database Permissions:"
stat -c "%A %U:%G %n" /etc/mosquitto/passwd
echo
echo "Mosquitto Broker Security Configuration:"
grep -nE '^(listener|allow_anonymous|password_file|acl_file)' /etc/mosquitto/mosquitto.conf
echo
echo "Gateway Runtime Secret Permissions (Target Overlay):"
ls -la "$REPO_DIR/buildroot/external/iot_gateway/board/x86_64/rootfs-overlay/etc/iot-gateway/"
echo

echo "================ 7. SECRETS IN GIT AUDIT ================="
echo "Checking for accidental secrets in tracked files:"
TRACKED_SECRETS=$(git ls-files | grep -E '(^|/)\.env($|\.)|\.pem$|\.key$|id_rsa$' || true)
if [ -z "$TRACKED_SECRETS" ]; then
    echo "PASS: Zero private credentials or .env files tracked in Git."
else
    echo "FAIL: Tracked secrets detected: $TRACKED_SECRETS"
fi
echo
echo "Verifying .gitignore rules for mqtt.env:"
git check-ignore -v mqtt.env buildroot/external/iot_gateway/board/x86_64/rootfs-overlay/etc/iot-gateway/mqtt.env
echo

echo "================ 8. IMAGE CHECKSUMS (SHA-256) ============"
sha256sum \
  "$OUT/images/bzImage" \
  "$OUT/images/rootfs.ext2"
echo

echo "=========================================================="
echo "                  AUDIT EXECUTION COMPLETE                "
echo "=========================================================="
