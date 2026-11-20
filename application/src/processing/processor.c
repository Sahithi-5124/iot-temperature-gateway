#include "processor.h"

#include <math.h>

/* Project temperature thresholds in degrees Celsius. */
#define WARNING_THRESHOLD_C  35.0
#define CRITICAL_THRESHOLD_C 40.0

bool processor_process(
    const temperature_sample_t *sample,
    temperature_result_t *out_result
)
{
    if (sample == NULL || out_result == NULL) {
        return false;
    }

    if (!sample->valid || !isfinite(sample->temperature_c)) {
        return false;
    }

    out_result->temperature_c = sample->temperature_c;
    out_result->timestamp_unix_s = sample->timestamp_unix_s;
    out_result->source = sample->source;

    if (sample->temperature_c >= CRITICAL_THRESHOLD_C) {
        out_result->status = TEMPERATURE_STATUS_CRITICAL;
    }
    else if (sample->temperature_c >= WARNING_THRESHOLD_C) {
        out_result->status = TEMPERATURE_STATUS_WARNING;
    }
    else {
        out_result->status = TEMPERATURE_STATUS_NORMAL;
    }

    return true;
}
