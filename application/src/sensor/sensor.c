
#include "sensor.h"

#include <stdlib.h>
#include <time.h>

/* Private implementation of the sensor device. */
struct sensor_device {
    sensor_source_t source;
    unsigned int next_sample_index;
};

/* Deterministic sample values for the simulator. */
static const double simulated_temperatures[] = {
    25.0,
    30.0,
    36.0,
    42.0
};

#define SIMULATED_SAMPLE_COUNT \
    (sizeof(simulated_temperatures) / sizeof(simulated_temperatures[0]))

sensor_status_t sensor_create(
    sensor_source_t source,
    sensor_device_t **out_sensor
)
{
    sensor_device_t *sensor;

    if (out_sensor == NULL) {
        return SENSOR_STATUS_INVALID_ARGUMENT;
    }

    *out_sensor = NULL;

    if (source == SENSOR_SOURCE_HARDWARE) {
        return SENSOR_STATUS_UNSUPPORTED;
    }

    if (source != SENSOR_SOURCE_SIMULATED) {
        return SENSOR_STATUS_INVALID_ARGUMENT;
    }

    sensor = malloc(sizeof(*sensor));

    if (sensor == NULL) {
        return SENSOR_STATUS_OUT_OF_MEMORY;
    }

    sensor->source = source;
    sensor->next_sample_index = 0;

    *out_sensor = sensor;

    return SENSOR_STATUS_OK;
}

sensor_status_t sensor_read(
    sensor_device_t *sensor,
    temperature_sample_t *out_sample
)
{
    time_t current_time;

    if (sensor == NULL || out_sample == NULL) {
        return SENSOR_STATUS_INVALID_ARGUMENT;
    }

    if (sensor->source != SENSOR_SOURCE_SIMULATED) {
        return SENSOR_STATUS_UNSUPPORTED;
    }

    current_time = time(NULL);

    if (current_time == (time_t)-1) {
        return SENSOR_STATUS_READ_ERROR;
    }

    out_sample->temperature_c =
        simulated_temperatures[sensor->next_sample_index];

    out_sample->timestamp_unix_s = current_time;
    out_sample->source = SENSOR_SOURCE_SIMULATED;
    out_sample->valid = true;

    sensor->next_sample_index =
        (sensor->next_sample_index + 1) % SIMULATED_SAMPLE_COUNT;

    return SENSOR_STATUS_OK;
}

void sensor_destroy(sensor_device_t *sensor)
{
    free(sensor);
}
