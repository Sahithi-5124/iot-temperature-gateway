#ifndef IOT_GATEWAY_MQTT_H
#define IOT_GATEWAY_MQTT_H

#include "alert.h"
#include "processor.h"

/* MQTT operation results. */
typedef enum {
    MQTT_STATUS_OK = 0,
    MQTT_STATUS_INVALID_ARGUMENT,
    MQTT_STATUS_NOT_INITIALIZED,
    MQTT_STATUS_CONNECTION_ERROR,
    MQTT_STATUS_PUBLISH_ERROR
} mqtt_status_t;

/* Initialize the MQTT client and connect to the broker. */
mqtt_status_t mqtt_init(
    const char *broker_host,
    int broker_port
);

/* Publish a processed temperature reading. */
mqtt_status_t mqtt_publish_temperature(
    const temperature_result_t *result
);

/* Publish a generated temperature alert. */
mqtt_status_t mqtt_publish_alert(
    const alert_event_t *alert
);

/* Disconnect and clean up the MQTT client. */
void mqtt_cleanup(void);

#endif /* IOT_GATEWAY_MQTT_H */
