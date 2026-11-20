#include "mqtt.h"

#include <stdio.h>

int main(void)
{
    temperature_result_t reading = {
        .temperature_c = 30.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .status = TEMPERATURE_STATUS_WARNING
    };

    alert_event_t alert = {
        .temperature_c = 30.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .level = ALERT_LEVEL_WARNING
    };

    printf("=== IoT Gateway MQTT Tests ===\n");

    if (mqtt_publish_temperature(&reading) !=
        MQTT_STATUS_NOT_INITIALIZED) {
        printf("FAIL: Uninitialized client check.\n");
        return 1;
    }

    printf("PASS: Uninitialized client correctly rejected.\n");

    if (mqtt_init("localhost", 1883) != MQTT_STATUS_OK) {
        printf("FAIL: MQTT broker connection.\n");
        return 1;
    }

    printf("PASS: Connected to MQTT broker.\n");

    if (mqtt_publish_temperature(&reading) != MQTT_STATUS_OK) {
        printf("FAIL: Temperature publishing.\n");
        mqtt_cleanup();
        return 1;
    }

    printf("PASS: Temperature reading published.\n");

    if (mqtt_publish_alert(&alert) != MQTT_STATUS_OK) {
        printf("FAIL: Alert publishing.\n");
        mqtt_cleanup();
        return 1;
    }

    printf("PASS: Warning alert published.\n");

    if (mqtt_publish_temperature(NULL) !=
        MQTT_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: Invalid temperature argument check.\n");
        mqtt_cleanup();
        return 1;
    }

    printf("PASS: Invalid temperature argument rejected.\n");

    mqtt_cleanup();

    printf("PASS: MQTT cleanup completed.\n");
    printf("=== MQTT tests completed ===\n");

    return 0;
}
