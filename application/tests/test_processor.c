#include "processor.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_temperature(
    double temperature,
    temperature_status_t expected_status
)
{
    temperature_sample_t sample = {
        .temperature_c = temperature,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .valid = true
    };

    temperature_result_t result;

    assert(processor_process(&sample, &result));

    assert(result.temperature_c == temperature);
    assert(result.timestamp_unix_s == sample.timestamp_unix_s);
    assert(result.source == SENSOR_SOURCE_SIMULATED);
    assert(result.status == expected_status);
}

static void test_invalid_temperature(double temperature)
{
    temperature_sample_t sample = {
        .temperature_c = temperature,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .valid = true
    };

    temperature_result_t result;

    assert(!processor_process(&sample, &result));
}

int main(void)
{
    temperature_sample_t sample = {
        .temperature_c = 25.0,
        .timestamp_unix_s = 1795000000,
        .source = SENSOR_SOURCE_SIMULATED,
        .valid = true
    };

    temperature_result_t result;

    printf("=== IoT Gateway Processor Tests ===\n\n");

    /* Normal temperature tests. */
    test_temperature(25.0, TEMPERATURE_STATUS_NORMAL);
    test_temperature(34.9, TEMPERATURE_STATUS_NORMAL);
    printf("PASS: Normal temperature classification.\n");

    /* Warning boundary tests. */
    test_temperature(35.0, TEMPERATURE_STATUS_WARNING);
    test_temperature(39.9, TEMPERATURE_STATUS_WARNING);
    printf("PASS: Warning temperature classification.\n");

    /* Critical boundary tests. */
    test_temperature(40.0, TEMPERATURE_STATUS_CRITICAL);
    test_temperature(42.0, TEMPERATURE_STATUS_CRITICAL);
    printf("PASS: Critical temperature classification.\n");

    /* Valid input range boundary tests. */
    test_temperature(-40.0, TEMPERATURE_STATUS_NORMAL);
    test_temperature(85.0, TEMPERATURE_STATUS_CRITICAL);
    printf("PASS: Valid temperature range boundaries.\n");

    /* Out-of-range temperature tests. */
    test_invalid_temperature(-40.1);
    test_invalid_temperature(85.1);
    printf("PASS: Out-of-range temperature rejection.\n");

    /* Invalid argument tests. */
    assert(!processor_process(NULL, &result));
    assert(!processor_process(&sample, NULL));
    printf("PASS: NULL argument validation.\n");

    /* Invalid sensor sample test. */
    sample.valid = false;
    assert(!processor_process(&sample, &result));
    printf("PASS: Invalid sensor sample rejection.\n");

    /* Non-finite temperature tests. */
    test_invalid_temperature(NAN);
    test_invalid_temperature(INFINITY);
    test_invalid_temperature(-INFINITY);
    printf("PASS: NaN and infinity rejection.\n");

    printf("\n=== All processor tests passed ===\n");

    return 0;
}
