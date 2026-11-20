#include "logger.h"
#include <stdio.h>
#include <string.h>

static FILE *log_file = NULL;

logger_status_t logger_init(const char *file_path)
{
    if (file_path == NULL || file_path[0] == '\0') {
        return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    if (log_file != NULL) {
        return LOGGER_STATUS_FILE_ERROR;
    }

    log_file = fopen(file_path, "a");

    if (log_file == NULL) {
        return LOGGER_STATUS_FILE_ERROR;
    }

    /* Write the CSV header only when the file is empty. */
    if (ftell(log_file) == 0) {
        if (fprintf(
                log_file,
                "timestamp,temperature_c,source,status,alert_level\n"
            ) < 0) {
            logger_close();
            return LOGGER_STATUS_FILE_ERROR;
        }
    }

    if (fflush(log_file) != 0) {
        logger_close();
        return LOGGER_STATUS_FILE_ERROR;
    }

    return LOGGER_STATUS_OK;
}

logger_status_t logger_log_temperature(
    const temperature_result_t *result
)
{
    const char *source_name;
    const char *status_name;

    if (result == NULL || log_file == NULL) {
        return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    switch (result->source) {
        case SENSOR_SOURCE_SIMULATED:
            source_name = "SIMULATED";
            break;
        case SENSOR_SOURCE_HARDWARE:
            source_name = "HARDWARE";
            break;
        default:
            return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    switch (result->status) {
        case TEMPERATURE_STATUS_NORMAL:
            status_name = "NORMAL";
            break;
        case TEMPERATURE_STATUS_WARNING:
            status_name = "WARNING";
            break;
        case TEMPERATURE_STATUS_CRITICAL:
            status_name = "CRITICAL";
            break;
        default:
            return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    if (fprintf(
            log_file,
            "%lld,%.2f,%s,%s,\n",
            (long long)result->timestamp_unix_s,
            result->temperature_c,
            source_name,
            status_name
        ) < 0) {
        return LOGGER_STATUS_FILE_ERROR;
    }

    return fflush(log_file) == 0
        ? LOGGER_STATUS_OK
        : LOGGER_STATUS_FILE_ERROR;
}

logger_status_t logger_log_alert(const alert_event_t *alert)
{
    const char *source_name;
    const char *alert_name;

    if (alert == NULL || log_file == NULL) {
        return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    switch (alert->source) {
        case SENSOR_SOURCE_SIMULATED:
            source_name = "SIMULATED";
            break;
        case SENSOR_SOURCE_HARDWARE:
            source_name = "HARDWARE";
            break;
        default:
            return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    switch (alert->level) {
        case ALERT_LEVEL_WARNING:
            alert_name = "WARNING";
            break;
        case ALERT_LEVEL_CRITICAL:
            alert_name = "CRITICAL";
            break;
        default:
            return LOGGER_STATUS_INVALID_ARGUMENT;
    }

    if (fprintf(
            log_file,
            "%lld,%.2f,%s,ALERT,%s\n",
            (long long)alert->timestamp_unix_s,
            alert->temperature_c,
            source_name,
            alert_name
        ) < 0) {
        return LOGGER_STATUS_FILE_ERROR;
    }

    return fflush(log_file) == 0
        ? LOGGER_STATUS_OK
        : LOGGER_STATUS_FILE_ERROR;
}

void logger_close(void)
{
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}
