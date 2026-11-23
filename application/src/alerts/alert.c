#include "alert.h"

alert_generation_status_t alert_generate(
    const temperature_result_t *result,
    alert_event_t *out_alert
)
{
    if (result == NULL || out_alert == NULL) {
        return ALERT_GENERATION_ERROR;
    }

    if (result->status == TEMPERATURE_STATUS_NORMAL) {
        return ALERT_GENERATION_NONE;
    }

    if (result->status != TEMPERATURE_STATUS_WARNING &&
        result->status != TEMPERATURE_STATUS_CRITICAL) {
        return ALERT_GENERATION_ERROR;
    }

    out_alert->temperature_c = result->temperature_c;
    out_alert->timestamp_unix_s = result->timestamp_unix_s;
    out_alert->source = result->source;

    if (result->status == TEMPERATURE_STATUS_WARNING) {
        out_alert->level = ALERT_LEVEL_WARNING;
    } else {
        out_alert->level = ALERT_LEVEL_CRITICAL;
    }

    return ALERT_GENERATION_CREATED;
}
