
#ifndef IOT_GATEWAY_SENSOR_H
#define IOT_GATEWAY_SENSOR_H

#include <stdbool.h>
#include <time.h>

/* Identifies where a temperature reading originates. */
typedef enum {
    SENSOR_SOURCE_SIMULATED = 0,
    SENSOR_SOURCE_HARDWARE
} sensor_source_t;

/* Result codes returned by sensor operations. */
typedef enum {
    SENSOR_STATUS_OK = 0,
    SENSOR_STATUS_INVALID_ARGUMENT,
    SENSOR_STATUS_OUT_OF_MEMORY,
    SENSOR_STATUS_NOT_INITIALIZED,
    SENSOR_STATUS_READ_ERROR,
    SENSOR_STATUS_UNSUPPORTED
} sensor_status_t;

/* Common representation of a temperature reading. */
typedef struct {
    double temperature_c;
    time_t timestamp_unix_s;
    sensor_source_t source;
    bool valid;
} temperature_sample_t;

/* Selectable simulation behaviour. */
typedef enum {
    SENSOR_MODE_DETERMINISTIC = 0,
    SENSOR_MODE_REALISTIC
} sensor_mode_t;

/* Backend implementation is hidden from other modules. */
typedef struct sensor_device sensor_device_t;

/* Create and initialize a selected sensor backend. */
sensor_status_t sensor_create(
    sensor_source_t source,
    sensor_device_t **out_sensor
);

/* Configure the behaviour of a simulated sensor. */
sensor_status_t sensor_set_mode(
    sensor_device_t *sensor,
    sensor_mode_t mode
);

/* Obtain one temperature sample. */
sensor_status_t sensor_read(
    sensor_device_t *sensor,
    temperature_sample_t *out_sample
);

/* Release sensor resources. */
void sensor_destroy(sensor_device_t *sensor);

#endif /* IOT_GATEWAY_SENSOR_H */
