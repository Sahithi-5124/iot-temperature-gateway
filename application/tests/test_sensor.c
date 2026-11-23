#include "sensor.h"

#include <stdio.h>

int main(void)
{
    sensor_device_t *sensor = NULL;
    sensor_device_t *invalid_sensor = NULL;
    temperature_sample_t sample;

    sensor_status_t status;

    printf("=== IoT Gateway Sensor Test ===\n\n");

    /* Test simulator initialization. */
    status = sensor_create(SENSOR_SOURCE_SIMULATED, &sensor);

    if (status != SENSOR_STATUS_OK) {
        printf("FAIL: Could not create simulated sensor.\n");
        return 1;
    }

    printf("PASS: Simulated sensor initialized.\n\n");

    /* Test invalid sensor creation arguments. */

    status = sensor_create(SENSOR_SOURCE_SIMULATED, NULL);

    if (status != SENSOR_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: NULL output pointer was not rejected.\n");
        sensor_destroy(sensor);
        return 1;
    }

    status = sensor_create((sensor_source_t)99, &invalid_sensor);

    if (status != SENSOR_STATUS_INVALID_ARGUMENT ||
        invalid_sensor != NULL) {
        printf("FAIL: Invalid sensor source was not rejected.\n");
        sensor_destroy(invalid_sensor);
        sensor_destroy(sensor);
        return 1;
    }

    printf("PASS: Invalid sensor creation arguments correctly rejected.\n\n");

    /* Test invalid sensor API arguments. */

    status = sensor_read(NULL, &sample);

    if (status != SENSOR_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: NULL sensor was not rejected.\n");
        sensor_destroy(sensor);
        return 1;
    }

    status = sensor_read(sensor, NULL);

    if (status != SENSOR_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: NULL sample pointer was not rejected.\n");
        sensor_destroy(sensor);
        return 1;
    }

    printf("PASS: Invalid read arguments correctly rejected.\n");

    /* Expected repeating temperature sequence. */
    static const double expected_temperatures[] = {
        25.0,
        30.0,
        36.0,
        42.0,
        25.0
    };

    /* Read five samples and verify the repeating sequence. */
    for (int i = 0; i < 5; i++) {

        status = sensor_read(sensor, &sample);

        if (status != SENSOR_STATUS_OK) {
            printf("FAIL: Sensor read failed.\n");
            sensor_destroy(sensor);
            return 1;
        }

        if (!sample.valid ||
            sample.source != SENSOR_SOURCE_SIMULATED) {
            printf("FAIL: Invalid sample or incorrect source.\n");
            sensor_destroy(sensor);
            return 1;
        }

        if (sample.temperature_c != expected_temperatures[i]) {
            printf(
                "FAIL: Expected %.1f C, received %.1f C.\n",
                expected_temperatures[i],
                sample.temperature_c
            );
            sensor_destroy(sensor);
            return 1;
        }

        printf(
            "PASS: Sample %d = %.1f C | Source: SIMULATED\n",
            i + 1,
            sample.temperature_c
        );
    }

    sensor_destroy(sensor);

    /* Test realistic simulation mode. */
    status = sensor_create(SENSOR_SOURCE_SIMULATED, &sensor);

    if (status != SENSOR_STATUS_OK) {
        printf("\nFAIL: Could not create realistic test sensor.\n");
        return 1;
    }

    status = sensor_set_mode(sensor, SENSOR_MODE_REALISTIC);

    if (status != SENSOR_STATUS_OK) {
        printf("\nFAIL: Could not select realistic mode.\n");
        sensor_destroy(sensor);
        return 1;
    }

    printf("\n=== Realistic Simulation Test ===\n");

    for (int i = 0; i < 40; i++) {
        status = sensor_read(sensor, &sample);

        if (status != SENSOR_STATUS_OK ||
            !sample.valid ||
            sample.source != SENSOR_SOURCE_SIMULATED ||
            sample.temperature_c < 20.0 ||
            sample.temperature_c > 42.0) {

            printf("FAIL: Realistic temperature validation failed.\n");
            sensor_destroy(sensor);
            return 1;
        }
    }

    printf("PASS: Realistic readings remain within 20-42 C.\n");
    printf("PASS: Realistic readings retain SIMULATED source.\n");

    sensor_destroy(sensor);
    sensor = NULL;

    /* Test invalid simulation mode. */
    status = sensor_create(SENSOR_SOURCE_SIMULATED, &sensor);

    if (status != SENSOR_STATUS_OK) {
        printf("FAIL: Could not create sensor for invalid mode test.\n");
        return 1;
    }

    status = sensor_set_mode(sensor, (sensor_mode_t)99);

    if (status != SENSOR_STATUS_INVALID_ARGUMENT) {
        printf("FAIL: Invalid simulation mode was accepted.\n");
        sensor_destroy(sensor);
        return 1;
    }

    printf("PASS: Invalid simulation mode correctly rejected.\n");

    sensor_destroy(sensor);
    sensor = NULL;

    /* Test that hardware mode is not falsely supported. */
    sensor = NULL;

    status = sensor_create(SENSOR_SOURCE_HARDWARE, &sensor);

    if (status != SENSOR_STATUS_UNSUPPORTED || sensor != NULL) {
        printf("\nFAIL: Hardware mode should be unsupported.\n");
        sensor_destroy(sensor);
        return 1;
    }

    printf("\nPASS: Hardware mode correctly rejected.\n");
    printf("\n=== All sensor tests passed ===\n");

    return 0;
}