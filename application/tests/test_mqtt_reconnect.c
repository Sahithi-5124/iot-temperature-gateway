#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "mqtt.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int max_iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (max_iterations <= 0) {
        max_iterations = 5;
    }

    temperature_result_t reading = {
        .temperature_c = 28.5,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .status = TEMPERATURE_STATUS_NORMAL
    };

    printf("=== MQTT Asynchronous Reconnection & Reliability Test ===\n");

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
    const char *pub_pass = getenv("MQTT_PASSWORD");

    printf("Initializing client (host=%s:%d, iterations=%d)...\n",
           broker_host, broker_port, max_iterations);

    if (mqtt_init_auth(broker_host, broker_port, pub_user, pub_pass) != MQTT_STATUS_OK) {
        printf("FAIL: Initial MQTT initialization failed.\n");
        return 1;
    }

    printf("PASS: MQTT client initialized (async non-blocking mode).\n");
    printf("Monitoring connection callback state and publish requests:\n");

    for (int i = 1; i <= max_iterations; i++) {
        bool connected = mqtt_is_connected();
        mqtt_status_t pub_status = mqtt_publish_temperature(&reading);

        printf("  [Tick %2d s] Connected Callback: %s | Publish Result: %s\n",
               i,
               connected ? "YES (Active)" : "NO (Waiting/Reconnecting)",
               pub_status == MQTT_STATUS_OK ? "ACCEPTED" : "QUEUED/DROPPED");

        sleep(1);
    }

    printf("Shutting down MQTT client...\n");
    mqtt_cleanup();

    printf("PASS: Reconnection test loop concluded cleanly.\n");
    printf("=== MQTT Reconnection Test Completed ===\n");

    return 0;
}
