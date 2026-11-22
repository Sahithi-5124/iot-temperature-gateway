#!/usr/bin/env bash
set -e

ENV_FILE="/tmp/mqtt_test_env.sh"
if [ ! -f "$ENV_FILE" ]; then
    echo "ERROR: Test environment file $ENV_FILE not found."
    exit 1
fi
source "$ENV_FILE"

echo "=== TEST 1: Anonymous Access Rejection ==="
if mosquitto_sub -h localhost -p 1883 -t "iot/temperature/#" -v -W 2 2>&1 | grep -q "not authorised"; then
    echo "PASS: Anonymous subscription rejected with 'not authorised'."
else
    echo "FAIL: Anonymous subscription was not rejected."
    exit 1
fi

if mosquitto_pub -h localhost -p 1883 -t "iot/temperature/readings" -m "test" 2>&1 | grep -q "not authorised"; then
    echo "PASS: Anonymous publish rejected with 'not authorised'."
else
    echo "FAIL: Anonymous publish was not rejected."
    exit 1
fi

echo "=== TEST 2: Authenticated Subscriber & Publisher ==="
mosquitto_sub -h localhost -p 1883 -u "$MQTT_SUB_USERNAME" -P "$MQTT_SUB_PASSWORD" -t "iot/temperature/#" -C 2 -W 5 > /tmp/test_received.txt 2>/dev/null &
SUB_PID=$!
sleep 1

# Publish reading as gateway_pub
mosquitto_pub -h localhost -p 1883 -u "$MQTT_USERNAME" -P "$MQTT_PASSWORD" -t "iot/temperature/readings" -m '{"temperature_c":25.00,"source":"SIMULATED","status":"NORMAL"}'
# Publish alert as gateway_pub
mosquitto_pub -h localhost -p 1883 -u "$MQTT_USERNAME" -P "$MQTT_PASSWORD" -t "iot/temperature/alerts" -m '{"temperature_c":42.00,"source":"SIMULATED","alert_level":"CRITICAL"}'

wait $SUB_PID 2>/dev/null || true
echo "Received messages by monitor_sub:"
cat /tmp/test_received.txt

if grep -q "NORMAL" /tmp/test_received.txt && grep -q "CRITICAL" /tmp/test_received.txt; then
    echo "PASS: Authenticated publisher delivered and authenticated subscriber received both topics."
else
    echo "FAIL: Expected messages not received."
    exit 1
fi

echo "=== TEST 3: Unauthorized Account Cannot Read Temperature Topics ==="
# unauth_user attempts to subscribe; stdout captures any messages, stderr captures timeouts
mosquitto_sub -h localhost -p 1883 -u "$MQTT_UNAUTH_USER" -P "$MQTT_UNAUTH_PASS" -t "iot/temperature/#" -C 1 -W 2 > /tmp/unauth_received.txt 2>/dev/null || true
if [ ! -s /tmp/unauth_received.txt ]; then
    echo "PASS: Unauthorized account denied access to temperature topics (zero messages received)."
else
    echo "FAIL: Unauthorized account received messages:"
    cat /tmp/unauth_received.txt
    exit 1
fi

echo "=== TEST 4: Unauthorized Account Cannot Publish to Temperature Topics ==="
# Attempt to publish as unauth_user
mosquitto_sub -h localhost -p 1883 -u "$MQTT_SUB_USERNAME" -P "$MQTT_SUB_PASSWORD" -t "iot/temperature/#" -C 1 -W 2 > /tmp/tamper_received.txt 2>/dev/null &
TAMPER_SUB_PID=$!
sleep 0.5
mosquitto_pub -h localhost -p 1883 -u "$MQTT_UNAUTH_USER" -P "$MQTT_UNAUTH_PASS" -t "iot/temperature/readings" -m '{"unauthorized":"injection"}' 2>/dev/null || true
wait $TAMPER_SUB_PID 2>/dev/null || true
if [ ! -s /tmp/tamper_received.txt ]; then
    echo "PASS: Unauthorized account publish blocked by broker ACL (zero messages delivered)."
else
    echo "FAIL: Unauthorized publish was delivered!"
    exit 1
fi

echo "=== TEST 5: Password Database Permissions ==="
PERMS=$(stat -c "%a %U:%G" /etc/mosquitto/passwd)
echo "Password file stat: $PERMS"
if [ "$PERMS" = "600 mosquitto:mosquitto" ]; then
    echo "PASS: Password file has restrictive 0600 ownership and permissions."
else
    echo "FAIL: Password file permissions are insecure: $PERMS"
    exit 1
fi

rm -f /tmp/test_received.txt /tmp/unauth_received.txt /tmp/tamper_received.txt
echo "=== ALL MQTT AUTHENTICATION & ACL TESTS PASSED ==="
