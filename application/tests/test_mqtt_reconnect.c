#include "mqtt.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(void)
{
    temperature_result_t reading = {
        .temperature_c = 30.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .status = TEMPERATURE_STATUS_NORMAL
    };

    printf("=== MQTT Reconnection Test ===\n");

    printf("Initializing MQTT client...\n");

    if (mqtt_init("localhost", 1883) != MQTT_STATUS_OK) {
        printf("FAIL: Initial MQTT connection failed.\n");
        return 1;
    }

    printf("PASS: Initially connected to broker.\n");
    printf("Waiting for broker interruption...\n");

    for (int i = 1; i <= 90; i++) {
        mqtt_status_t result = mqtt_publish_temperature(&reading);

        if (result == MQTT_STATUS_OK) {
            printf("[%d s] Publish request accepted.\n", i);
        } else {
            printf("[%d s] Publish failed: status %d\n", i, result);
        }

        sleep(1);
    }

    printf("Cleaning up MQTT client...\n");
    mqtt_cleanup();

    printf("=== Test completed ===\n");

    return 0;
}
