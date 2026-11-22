#!/usr/bin/env bash
set -e

ENV_FILE="/tmp/mqtt_test_env.sh"
if [ ! -f "$ENV_FILE" ]; then
    echo "ERROR: Test environment file $ENV_FILE not found."
    exit 1
fi
source "$ENV_FILE"

rm -f /tmp/live_qemu_telemetry.log

echo "Starting authenticated subscriber on host..."
mosquitto_sub -h localhost -p 1883 -u "$MQTT_SUB_USERNAME" -P "$MQTT_SUB_PASSWORD" -t "iot/temperature/#" -v > /tmp/live_qemu_telemetry.log 2>&1 &
SUB_PID=$!

echo "Launching QEMU virtual machine (test mode with snapshot)..."
cd /mnt/c/Users/vivobook/Downloads/iot-2
./scripts/qemu/run-qemu.sh -snapshot > /tmp/qemu_run.log 2>&1 &
QEMU_PID=$!

echo "Waiting 12 seconds for QEMU boot and telemetry generation..."
sleep 12

echo "Stopping QEMU and subscriber..."
kill -TERM "$QEMU_PID" 2>/dev/null || true
kill -TERM "$SUB_PID" 2>/dev/null || true
wait "$QEMU_PID" 2>/dev/null || true
wait "$SUB_PID" 2>/dev/null || true

echo "=== Telemetry Captured From Booted Guest ==="
cat /tmp/live_qemu_telemetry.log

if grep -q "iot/temperature/readings" /tmp/live_qemu_telemetry.log && \
   grep -q "SIMULATED" /tmp/live_qemu_telemetry.log; then
    echo "SUCCESS: Authenticated QEMU guest published telemetry received on host!"
else
    echo "FAIL: Expected telemetry not captured from guest."
    exit 1
fi
