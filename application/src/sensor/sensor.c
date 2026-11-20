
#include "sensor.h"

#include <stdlib.h>
#include <time.h>

/* Private implementation of the sensor device. */
struct sensor_device {
    sensor_source_t source;
    sensor_mode_t mode;
    unsigned int next_sample_index;
    double current_temperature;
    int temperature_direction;
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
    sensor->mode = SENSOR_MODE_DETERMINISTIC;
    sensor->next_sample_index = 0;
    sensor->current_temperature = 25.0;
    sensor->temperature_direction = 1;

    *out_sensor = sensor;

    return SENSOR_STATUS_OK;
}

sensor_status_t sensor_set_mode(
    sensor_device_t *sensor,
    sensor_mode_t mode
)
{
    if (sensor == NULL) {
        return SENSOR_STATUS_INVALID_ARGUMENT;
    }

    if (sensor->source != SENSOR_SOURCE_SIMULATED) {
        return SENSOR_STATUS_UNSUPPORTED;
    }

    if (mode != SENSOR_MODE_DETERMINISTIC &&
        mode != SENSOR_MODE_REALISTIC) {
        return SENSOR_STATUS_INVALID_ARGUMENT;
    }

    sensor->mode = mode;
    sensor->next_sample_index = 0;
    sensor->current_temperature = 25.0;
    sensor->temperature_direction = 1;

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

    if (sensor->mode == SENSOR_MODE_DETERMINISTIC) {
        out_sample->temperature_c =
            simulated_temperatures[sensor->next_sample_index];

        sensor->next_sample_index =
            (sensor->next_sample_index + 1) % SIMULATED_SAMPLE_COUNT;

    } else if (sensor->mode == SENSOR_MODE_REALISTIC) {
        out_sample->temperature_c = sensor->current_temperature;

        sensor->current_temperature +=
            sensor->temperature_direction * 1.0;

        if (sensor->current_temperature >= 42.0) {
            sensor->current_temperature = 42.0;
            sensor->temperature_direction = -1;

        } else if (sensor->current_temperature <= 20.0) {
            sensor->current_temperature = 20.0;
            sensor->temperature_direction = 1;
        }

    } else {
        return SENSOR_STATUS_READ_ERROR;
    }

    out_sample->timestamp_unix_s = current_time;
    out_sample->source = SENSOR_SOURCE_SIMULATED;
    out_sample->valid = true;

    return SENSOR_STATUS_OK;
}

void sensor_destroy(sensor_device_t *sensor)
{
    free(sensor);
}
