#ifndef IOT_GATEWAY_LOGGER_H
#define IOT_GATEWAY_LOGGER_H

#include "alert.h"

/* Logging operation results. */
typedef enum {
    LOGGER_STATUS_OK = 0,
    LOGGER_STATUS_INVALID_ARGUMENT,
    LOGGER_STATUS_FILE_ERROR
} logger_status_t;

/* Initialize the logger and prepare the log file. */
logger_status_t logger_init(const char *file_path);

/* Record a processed temperature reading. */
logger_status_t logger_log_temperature(
    const temperature_result_t *result
);

/* Record a generated alert event. */
logger_status_t logger_log_alert(
    const alert_event_t *alert
);

/* Close the logger and release resources. */
void logger_close(void);

#endif /* IOT_GATEWAY_LOGGER_H */
