#include "mqtt.h"

#include <mosquitto.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MQTT_READING_TOPIC "iot/temperature/readings"
#define MQTT_ALERT_TOPIC   "iot/temperature/alerts"
#define MQTT_KEEPALIVE     60
#define MQTT_QOS           1
#define MQTT_RECONNECT_DELAY     2
#define MQTT_RECONNECT_MAX_DELAY 30

static struct mosquitto *mqtt_client = NULL;
static bool mqtt_library_initialized = false;
static volatile bool mqtt_connected = false;

/* Callback when client connects or fails to connect to broker. */
static void on_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;
    if (rc == MOSQ_ERR_SUCCESS) {
        mqtt_connected = true;
        fprintf(stdout, "[MQTT] Connection established to broker.\n");
    } else {
        mqtt_connected = false;
        fprintf(stderr, "[MQTT] Broker connection failed: %s (code %d).\n",
                mosquitto_connack_string(rc), rc);
    }
}

/* Callback when client disconnects from broker. */
static void on_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    (void)mosq;
    (void)obj;
    (void)rc;
    mqtt_connected = false;
}

/* Convert sensor source to its JSON string representation. */
static const char *source_to_string(sensor_source_t source)
{
    switch (source) {
        case SENSOR_SOURCE_SIMULATED:
            return "SIMULATED";

        case SENSOR_SOURCE_HARDWARE:
            return "HARDWARE";

        default:
            return NULL;
    }
}

/* Convert temperature status to its JSON string representation. */
static const char *status_to_string(temperature_status_t status)
{
    switch (status) {
        case TEMPERATURE_STATUS_NORMAL:
            return "NORMAL";

        case TEMPERATURE_STATUS_WARNING:
            return "WARNING";

        case TEMPERATURE_STATUS_CRITICAL:
            return "CRITICAL";

        default:
            return NULL;
    }
}

/* Initialize the MQTT library and connect to the broker with authentication. */
mqtt_status_t mqtt_init_auth(
    const char *broker_host,
    int broker_port,
    const char *username,
    const char *password)
{
    int rc;

    if (broker_host == NULL || broker_host[0] == '\0' ||
        broker_port < 1 || broker_port > 65535 ||
        mqtt_client != NULL) {
        return MQTT_STATUS_INVALID_ARGUMENT;
    }

    rc = mosquitto_lib_init();

    if (rc != MOSQ_ERR_SUCCESS) {
        return MQTT_STATUS_CONNECTION_ERROR;
    }

    mqtt_library_initialized = true;

    mqtt_client = mosquitto_new(NULL, true, NULL);

    if (mqtt_client == NULL) {
        mqtt_cleanup();
        return MQTT_STATUS_CONNECTION_ERROR;
    }

    /* Configure credentials if a username is supplied. */
    if (username != NULL && username[0] != '\0') {
        rc = mosquitto_username_pw_set(mqtt_client, username, password);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "[MQTT] Failed to set credentials: %s\n",
                    mosquitto_strerror(rc));
            mqtt_cleanup();
            return MQTT_STATUS_INVALID_ARGUMENT;
        }
    }

    /* Configure automatic reconnection with exponential backoff. */
    rc = mosquitto_reconnect_delay_set(
        mqtt_client,
        MQTT_RECONNECT_DELAY,
        MQTT_RECONNECT_MAX_DELAY,
        true
    );

    if (rc != MOSQ_ERR_SUCCESS) {
        mqtt_cleanup();
        return MQTT_STATUS_CONNECTION_ERROR;
    }

    /* Configure callbacks for tracking connection state. */
    mosquitto_connect_callback_set(mqtt_client, on_connect_callback);
    mosquitto_disconnect_callback_set(mqtt_client, on_disconnect_callback);

    /* Asynchronous connect allows the gateway to start even if broker is offline,
     * relying on the background loop to establish connection when available. */
    rc = mosquitto_connect_async(
        mqtt_client,
        broker_host,
        broker_port,
        MQTT_KEEPALIVE
    );

    /* If connect_async returns success or an immediate network error (e.g. ECONNREFUSED
     * when broker is not yet running), proceed to start the loop so reconnection will
     * occur automatically once the broker is online. */
    if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_ERRNO && rc != MOSQ_ERR_CONN_REFUSED) {
        mqtt_cleanup();
        return MQTT_STATUS_CONNECTION_ERROR;
    }

    rc = mosquitto_loop_start(mqtt_client);

    if (rc != MOSQ_ERR_SUCCESS) {
        mqtt_cleanup();
        return MQTT_STATUS_CONNECTION_ERROR;
    }

    return MQTT_STATUS_OK;
}

/* Backward-compatible unauthenticated init. */
mqtt_status_t mqtt_init(const char *broker_host, int broker_port)
{
    return mqtt_init_auth(broker_host, broker_port, NULL, NULL);
}

/* Publish a processed temperature reading as JSON. */
mqtt_status_t mqtt_publish_temperature(
    const temperature_result_t *result)
{
    char payload[256];
    const char *source;
    const char *status;
    int length;
    int rc;

    if (result == NULL ||
        !isfinite(result->temperature_c)) {
        return MQTT_STATUS_INVALID_ARGUMENT;
    }

    if (mqtt_client == NULL) {
        return MQTT_STATUS_NOT_INITIALIZED;
    }

    source = source_to_string(result->source);
    status = status_to_string(result->status);

    if (source == NULL || status == NULL) {
        return MQTT_STATUS_INVALID_ARGUMENT;
    }

    length = snprintf(
        payload,
        sizeof(payload),
        "{\"timestamp\":%lld,\"temperature_c\":%.2f,"
        "\"source\":\"%s\",\"status\":\"%s\"}",
        (long long)result->timestamp_unix_s,
        result->temperature_c,
        source,
        status
    );

    if (length < 0 || (size_t)length >= sizeof(payload)) {
        return MQTT_STATUS_PUBLISH_ERROR;
    }

    if (!mqtt_connected) {
        mosquitto_reconnect_async(mqtt_client);
    }

    rc = mosquitto_publish(
        mqtt_client,
        NULL,
        MQTT_READING_TOPIC,
        length,
        payload,
        MQTT_QOS,
        false
    );

    return (rc == MOSQ_ERR_SUCCESS)
        ? MQTT_STATUS_OK
        : MQTT_STATUS_PUBLISH_ERROR;
}

/* Publish a generated alert as JSON. */
mqtt_status_t mqtt_publish_alert(const alert_event_t *alert)
{
    char payload[256];
    const char *source;
    const char *alert_level;
    int length;
    int rc;

    if (alert == NULL || !isfinite(alert->temperature_c)) {
        return MQTT_STATUS_INVALID_ARGUMENT;
    }

    if (mqtt_client == NULL) {
        return MQTT_STATUS_NOT_INITIALIZED;
    }

    source = source_to_string(alert->source);

    if (alert->level == ALERT_LEVEL_WARNING) {
        alert_level = "WARNING";
    } else if (alert->level == ALERT_LEVEL_CRITICAL) {
        alert_level = "CRITICAL";
    } else {
        return MQTT_STATUS_INVALID_ARGUMENT;
    }

    if (source == NULL) {
        return MQTT_STATUS_INVALID_ARGUMENT;
    }

    length = snprintf(
        payload,
        sizeof(payload),
        "{\"timestamp\":%lld,\"temperature_c\":%.2f,"
        "\"source\":\"%s\",\"alert_level\":\"%s\"}",
        (long long)alert->timestamp_unix_s,
        alert->temperature_c,
        source,
        alert_level
    );

    if (length < 0 || (size_t)length >= sizeof(payload)) {
        return MQTT_STATUS_PUBLISH_ERROR;
    }

    if (!mqtt_connected) {
        mosquitto_reconnect_async(mqtt_client);
    }

    rc = mosquitto_publish(
        mqtt_client,
        NULL,
        MQTT_ALERT_TOPIC,
        length,
        payload,
        MQTT_QOS,
        false
    );

    return (rc == MOSQ_ERR_SUCCESS)
        ? MQTT_STATUS_OK
        : MQTT_STATUS_PUBLISH_ERROR;
}

/* Check whether the MQTT client is currently connected. */
bool mqtt_is_connected(void)
{
    return mqtt_connected;
}

/* Disconnect and release MQTT resources. */
void mqtt_cleanup(void)
{
    mqtt_connected = false;

    if (mqtt_client != NULL) {
        mosquitto_disconnect(mqtt_client);
        mosquitto_loop_stop(mqtt_client, true);
        mosquitto_destroy(mqtt_client);
        mqtt_client = NULL;
    }

    if (mqtt_library_initialized) {
        mosquitto_lib_cleanup();
        mqtt_library_initialized = false;
    }
}
