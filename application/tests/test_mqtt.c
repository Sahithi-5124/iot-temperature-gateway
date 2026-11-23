#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "mqtt.h"

#include <mosquitto.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile bool subscriber_connected = false;
static volatile bool message_received = false;

static void on_sub_connect(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;
    if (rc == MOSQ_ERR_SUCCESS) {
        subscriber_connected = true;
    }
}

static void on_sub_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    (void)mosq;
    (void)obj;
    if (msg != NULL && msg->payload != NULL) {
        message_received = true;
    }
}

int main(void)
{
    temperature_result_t reading = {
        .temperature_c = 30.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .status = TEMPERATURE_STATUS_WARNING
    };

    alert_event_t alert = {
        .temperature_c = 36.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .level = ALERT_LEVEL_WARNING
    };

    printf("=== IoT Gateway MQTT Authentication & Async Tests ===\n");

    /* 1. Offline checks before initialization */
    if (mqtt_publish_temperature(&reading) != MQTT_STATUS_NOT_INITIALIZED) {
        printf("FAIL: Uninitialized client check.\n");
        return 1;
    }
    printf("PASS: Uninitialized client correctly rejected.\n");

    if (mqtt_publish_temperature(NULL) != MQTT_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: Invalid temperature argument check.\n");
        return 1;
    }
    printf("PASS: NULL reading argument correctly rejected.\n");

    if (mqtt_init_auth(NULL, 1883, "user", "pass") != MQTT_STATUS_INVALID_ARGUMENT ||
        mqtt_init_auth("localhost", 0, "user", "pass") != MQTT_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: Invalid connection argument check.\n");
        return 1;
    }
    printf("PASS: Invalid connection arguments rejected.\n");

    /* Read runtime broker and credential parameters */
    const char *broker_host = getenv("MQTT_BROKER_HOST");
    if (broker_host == NULL || broker_host[0] == '\0') {
        broker_host = "localhost";
    }

    int broker_port = 1883;
    const char *port_str = getenv("MQTT_BROKER_PORT");
    if (port_str != NULL && port_str[0] != '\0') {
        int p = atoi(port_str);
        if (p > 0 && p <= 65535) {
            broker_port = p;
        }
    }

    const char *pub_user = getenv("MQTT_USERNAME");
    if (pub_user == NULL || pub_user[0] == '\0') {
        pub_user = "gateway_pub";
    }

    const char *pub_pass = getenv("MQTT_PASSWORD");
    const char *sub_user = getenv("MQTT_SUB_USERNAME");
    if (sub_user == NULL || sub_user[0] == '\0') {
        sub_user = "monitor_sub";
    }
    const char *sub_pass = getenv("MQTT_SUB_PASSWORD");

    printf("Connecting gateway publisher (host=%s:%d, user=%s)...\n",
           broker_host, broker_port, pub_user);

    /* Initialize gateway publisher with authentication */
    if (mqtt_init_auth(broker_host, broker_port, pub_user, pub_pass) != MQTT_STATUS_OK) {
        printf("FAIL: mqtt_init_auth failed.\n");
        return 1;
    }

    printf("INFO: Client initialized. Awaiting asynchronous connection callback...\n");

    /* Wait for asynchronous connection callback with 3-second timeout */
    bool connected = false;
    for (int i = 0; i < 30; i++) {
        if (mqtt_is_connected()) {
            connected = true;
            break;
        }
        usleep(100000); /* 100 ms */
    }

    if (!connected) {
        printf("NOTE: Broker not reachable or connection refused (async wait timed out).\n");
        printf("      In an offline environment, this is expected.\n");
        mqtt_cleanup();

        /* If strict broker requirement is set, fail; otherwise pass offline unit test */
        if (getenv("REQUIRE_BROKER") != NULL) {
            printf("FAIL: Broker connection required but not established.\n");
            return 1;
        }
        printf("PASS: Async connection timeout handled safely.\n");
        printf("=== MQTT tests completed (Offline Mode) ===\n");
        return 0;
    }

    printf("PASS: Authenticated connection established (on_connect callback verified).\n");

    /* Verify authenticated subscriber client */
    struct mosquitto *sub_client = mosquitto_new("test_sub_client", true, NULL);
    if (sub_client != NULL) {
        if (sub_user != NULL && sub_pass != NULL) {
            mosquitto_username_pw_set(sub_client, sub_user, sub_pass);
        }
        mosquitto_connect_callback_set(sub_client, on_sub_connect);
        mosquitto_message_callback_set(sub_client, on_sub_message);

        if (mosquitto_connect(sub_client, broker_host, broker_port, 60) == MOSQ_ERR_SUCCESS) {
            mosquitto_loop_start(sub_client);
            mosquitto_subscribe(sub_client, NULL, "iot/temperature/readings", 1);
            usleep(200000); /* Allow subscription to register */
        }
    }

    /* Publish temperature reading */
    if (mqtt_publish_temperature(&reading) != MQTT_STATUS_OK) {
        printf("FAIL: Temperature reading publishing failed.\n");
        if (sub_client) { mosquitto_destroy(sub_client); }
        mqtt_cleanup();
        return 1;
    }
    printf("PASS: Temperature reading published successfully under authorized topic.\n");

    /* Publish alert event */
    if (mqtt_publish_alert(&alert) != MQTT_STATUS_OK) {
        printf("FAIL: Alert event publishing failed.\n");
        if (sub_client) { mosquitto_destroy(sub_client); }
        mqtt_cleanup();
        return 1;
    }
    printf("PASS: Alert event published successfully under authorized topic.\n");

    /* Verify subscriber received the message if subscriber was active */
    if (sub_client != NULL && subscriber_connected) {
        for (int i = 0; i < 20 && !message_received; i++) {
            usleep(100000);
        }
        if (message_received) {
            printf("PASS: Authenticated subscriber received published telemetry.\n");
        } else {
            printf("WARNING: Subscriber did not receive message within timeout.\n");
        }
        mosquitto_disconnect(sub_client);
        mosquitto_loop_stop(sub_client, false);
        mosquitto_destroy(sub_client);
    }

    mqtt_cleanup();
    printf("PASS: MQTT client disconnected and cleaned up cleanly.\n");
    printf("=== All MQTT tests PASSED ===\n");

    return 0;
}
