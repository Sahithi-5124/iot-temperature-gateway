
#ifndef IOT_GATEWAY_PROCESSOR_H
#define IOT_GATEWAY_PROCESSOR_H

#include "sensor.h"

/* Temperature classification levels. */
typedef enum {
    TEMPERATURE_STATUS_NORMAL = 0,
    TEMPERATURE_STATUS_WARNING,
    TEMPERATURE_STATUS_CRITICAL
} temperature_status_t;

/* Result of processing a temperature sample. */
typedef struct {
    double temperature_c;
    time_t timestamp_unix_s;
    sensor_source_t source;
    temperature_status_t status;
} temperature_result_t;

/* Classify a valid temperature sample. */
bool processor_process(
    const temperature_sample_t *sample,
    temperature_result_t *out_result
);

#endif /* IOT_GATEWAY_PROCESSOR_H */
