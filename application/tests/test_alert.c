#include "alert.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    temperature_result_t result = {
        .temperature_c = 25.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .status = TEMPERATURE_STATUS_NORMAL
    };

    alert_event_t alert;

    printf("=== IoT Gateway Alert Tests ===\n\n");

    /* Normal temperature: no alert expected. */
    assert(
        alert_generate(&result, &alert) ==
        ALERT_GENERATION_NONE
    );
    printf("PASS: Normal temperature generates no alert.\n");

    /* Warning temperature: warning alert expected. */
    result.temperature_c = 36.0;
    result.status = TEMPERATURE_STATUS_WARNING;

    assert(
        alert_generate(&result, &alert) ==
        ALERT_GENERATION_CREATED
    );

    assert(alert.level == ALERT_LEVEL_WARNING);
    assert(alert.temperature_c == 36.0);
    assert(alert.timestamp_unix_s == result.timestamp_unix_s);
    assert(alert.source == SENSOR_SOURCE_SIMULATED);

    printf("PASS: Warning alert generated correctly.\n");

    /* Critical temperature: critical alert expected. */
    result.temperature_c = 42.0;
    result.status = TEMPERATURE_STATUS_CRITICAL;

    assert(
        alert_generate(&result, &alert) ==
        ALERT_GENERATION_CREATED
    );

    assert(alert.level == ALERT_LEVEL_CRITICAL);
    assert(alert.temperature_c == 42.0);

    printf("PASS: Critical alert generated correctly.\n");

    /* NULL argument validation. */
    assert(
        alert_generate(NULL, &alert) ==
        ALERT_GENERATION_ERROR
    );

    assert(
        alert_generate(&result, NULL) ==
        ALERT_GENERATION_ERROR
    );

    printf("PASS: NULL argument validation.\n");

    /* Invalid status validation. */
    result.status = (temperature_status_t)99;

    assert(
        alert_generate(&result, &alert) ==
        ALERT_GENERATION_ERROR
    );

    printf("PASS: Invalid status rejected.\n");

    printf("\n=== All alert tests passed ===\n");

    return 0;
}
