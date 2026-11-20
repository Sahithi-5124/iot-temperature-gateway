#include "logger.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST_LOG_FILE "/tmp/iot_gateway_test.csv"

static void test_file_contains(const char *expected)
{
    FILE *file = fopen(TEST_LOG_FILE, "r");
    assert(file != NULL);

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, expected) != NULL) {
            found = 1;
            break;
        }
    }

    fclose(file);
    assert(found);
}

int main(void)
{
    temperature_result_t result = {
        .temperature_c = 25.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .status = TEMPERATURE_STATUS_NORMAL
    };

    alert_event_t alert = {
        .temperature_c = 42.0,
        .timestamp_unix_s = 1795000002,
        .source = SENSOR_SOURCE_SIMULATED,
        .level = ALERT_LEVEL_CRITICAL
    };

    printf("=== IoT Gateway Logger Tests ===\n\n");

    /* Invalid initialization argument. */
    assert(
        logger_init(NULL) == LOGGER_STATUS_INVALID_ARGUMENT
    );

    assert(
        logger_init("") == LOGGER_STATUS_INVALID_ARGUMENT
    );
    printf("PASS: Invalid initialization arguments.\n");

    /* Initialize logger. */
    assert(logger_init(TEST_LOG_FILE) == LOGGER_STATUS_OK);
    printf("PASS: Logger initialization.\n");

    /* Log a normal temperature reading. */
    assert(
        logger_log_temperature(&result) == LOGGER_STATUS_OK
    );
    printf("PASS: Temperature logging.\n");

    /* Log a critical alert. */
    assert(
        logger_log_alert(&alert) == LOGGER_STATUS_OK
    );
    printf("PASS: Alert logging.\n");

    /* NULL argument tests. */
    assert(
        logger_log_temperature(NULL) ==
        LOGGER_STATUS_INVALID_ARGUMENT
    );

    assert(
        logger_log_alert(NULL) ==
        LOGGER_STATUS_INVALID_ARGUMENT
    );
    printf("PASS: NULL argument validation.\n");

    /* Invalid source and status tests. */
    result.source = (sensor_source_t)99;
    assert(
        logger_log_temperature(&result) ==
        LOGGER_STATUS_INVALID_ARGUMENT
    );

    result.source = SENSOR_SOURCE_SIMULATED;
    result.status = (temperature_status_t)99;
    assert(
        logger_log_temperature(&result) ==
        LOGGER_STATUS_INVALID_ARGUMENT
    );
    printf("PASS: Invalid temperature data rejection.\n");

    /* Invalid alert level test. */
    alert.level = (alert_level_t)99;
    assert(
        logger_log_alert(&alert) ==
        LOGGER_STATUS_INVALID_ARGUMENT
    );
    printf("PASS: Invalid alert data rejection.\n");

    /* Close logger. */
    logger_close();
    printf("PASS: Logger close.\n");

    /* Verify actual CSV contents. */
    test_file_contains(
        "timestamp,temperature_c,source,status,alert_level"
    );

    test_file_contains(
        "1795000000,25.00,SIMULATED,NORMAL"
    );

    test_file_contains(
        "1795000002,42.00,SIMULATED,ALERT,CRITICAL"
    );

    printf("PASS: CSV file content verification.\n");

    printf("\n=== All logger tests passed ===\n");

    return 0;
}
