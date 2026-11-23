#define _POSIX_C_SOURCE 200809L

#include "sensor.h"
#include "processor.h"
#include "alert.h"
#include "logger.h"
#include "mqtt.h"

#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MQTT_BROKER_PORT 1883

#define LOG_FILE_PATH "temperature_log.csv"
#define READING_INTERVAL_SECONDS 2

/* Set to true when Ctrl+C is pressed. */
static volatile sig_atomic_t running = 1;

/* Handle graceful shutdown request. */
static void handle_signal(int signal_number)
{
    (void)signal_number;
    running = 0;
}

/* Helper to safely load KEY=VALUE environment variables from a protected file. */
static void load_env_file(const char *file_path)
{
    FILE *fp;
    char line[512];

    if (file_path == NULL || file_path[0] == '\0') {
        return;
    }

    fp = fopen(file_path, "r");
    if (fp == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *start = line;
        char *eq;
        char *key;
        char *val;
        size_t len;

        while (*start == ' ' || *start == '\t') {
            start++;
        }

        if (*start == '#' || *start == '\r' || *start == '\n' || *start == '\0') {
            continue;
        }

        eq = strchr(start, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        key = start;
        val = eq + 1;

        len = strlen(key);
        while (len > 0 && (key[len - 1] == ' ' || key[len - 1] == '\t')) {
            key[--len] = '\0';
        }

        while (*val == ' ' || *val == '\t') {
            val++;
        }

        len = strlen(val);
        while (len > 0 && (val[len - 1] == '\r' || val[len - 1] == '\n' ||
                           val[len - 1] == ' '  || val[len - 1] == '\t')) {
            val[--len] = '\0';
        }

        if (len >= 2 && ((val[0] == '"' && val[len - 1] == '"') ||
                         (val[0] == '\'' && val[len - 1] == '\''))) {
            val[len - 1] = '\0';
            val++;
        }

        if (key[0] != '\0') {
            setenv(key, val, 0);
        }
    }

    fclose(fp);
}

int main(void)
{
    /* Ensure stdout and stderr flush immediately when redirected to log files. */
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Load runtime configuration from protected config file if available. */
    const char *config_file = getenv("IOT_CONFIG_FILE");
    if (config_file == NULL || config_file[0] == '\0') {
        config_file = "/etc/iot-gateway/mqtt.env";
    }
    load_env_file(config_file);
    if (access(config_file, R_OK) != 0) {
        load_env_file("mqtt.env");
    }

    const char *mqtt_broker_host = getenv("MQTT_BROKER_HOST");
    if (mqtt_broker_host == NULL || mqtt_broker_host[0] == '\0') {
        mqtt_broker_host = "localhost";
    }

    int mqtt_broker_port = MQTT_BROKER_PORT;
    const char *port_env = getenv("MQTT_BROKER_PORT");
    if (port_env != NULL && port_env[0] != '\0') {
        int p = atoi(port_env);
        if (p > 0 && p <= 65535) {
            mqtt_broker_port = p;
        }
    }

    const char *mqtt_user = getenv("MQTT_USERNAME");
    const char *mqtt_pass = getenv("MQTT_PASSWORD");

    const char *log_file_path = getenv("IOT_LOG_FILE");
    if (log_file_path == NULL || log_file_path[0] == '\0') {
        log_file_path = LOG_FILE_PATH;
    }

    sensor_device_t *sensor = NULL;

    bool logger_initialized = false;
    bool mqtt_initialized = false;

    struct sigaction action = {0};

    printf("\n====================================\n");
    printf("   IoT Temperature Monitoring Gateway\n");
    printf("====================================\n");

    /* Register Ctrl+C (SIGINT) and termination (SIGTERM) handlers. */
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1) {
        fprintf(stderr, "ERROR: Signal handler setup failed.\n");
        return 1;
    }

    /* Initialize simulated sensor. */
    if (sensor_create(SENSOR_SOURCE_SIMULATED, &sensor)
        != SENSOR_STATUS_OK) {
        fprintf(stderr, "ERROR: Sensor initialization failed.\n");
        goto cleanup;
    }

    /* Select simulation mode from environment. */
    const char *sensor_mode_env = getenv("IOT_SENSOR_MODE");
    sensor_mode_t sensor_mode = SENSOR_MODE_DETERMINISTIC;

    if (sensor_mode_env != NULL) {
        if (strcmp(sensor_mode_env, "realistic") == 0) {
            sensor_mode = SENSOR_MODE_REALISTIC;
        } else if (strcmp(sensor_mode_env, "deterministic") != 0) {
            fprintf(stderr, "ERROR: Invalid IOT_SENSOR_MODE.\n");
            goto cleanup;
        }
    }

    if (sensor_set_mode(sensor, sensor_mode) != SENSOR_STATUS_OK) {
        fprintf(stderr, "ERROR: Sensor mode configuration failed.\n");
        goto cleanup;
    }

    printf("Sensor initialized successfully. Mode: %s\n",
           sensor_mode == SENSOR_MODE_REALISTIC
               ? "REALISTIC" : "DETERMINISTIC");

    /* Initialize local CSV logger. */
    if (logger_init(log_file_path) != LOGGER_STATUS_OK) {
        fprintf(stderr, "ERROR: Logger initialization failed for: %s\n", log_file_path);
        goto cleanup;
    }

    logger_initialized = true;
    printf("Logger initialized successfully. Path: %s\n", log_file_path);

    /* Initialize MQTT client. Async mode connects in background and
     * retries automatically if the broker is initially offline. */
    if (mqtt_init_auth(mqtt_broker_host, mqtt_broker_port, mqtt_user, mqtt_pass) != MQTT_STATUS_OK) {
        fprintf(stderr, "WARNING: MQTT setup failed; continuing local monitoring.\n");
    } else {
        mqtt_initialized = true;
        if (mqtt_user != NULL && mqtt_user[0] != '\0') {
            printf("MQTT client initialized (target broker: %s:%d, authenticated user: %s).\n",
                   mqtt_broker_host, mqtt_broker_port, mqtt_user);
        } else {
            printf("MQTT client initialized (target broker: %s:%d, unauthenticated).\n",
                   mqtt_broker_host, mqtt_broker_port);
        }
    }

    printf("\nMonitoring started. Press Ctrl+C to stop.\n\n");

    /* Main monitoring loop. */
    while (running) {
        temperature_sample_t sample;
        temperature_result_t result;
        alert_event_t alert;

        sensor_status_t sensor_status;
        alert_generation_status_t alert_status;

        /* Read temperature. */
        sensor_status = sensor_read(sensor, &sample);

        if (sensor_status != SENSOR_STATUS_OK) {
            fprintf(stderr, "ERROR: Sensor reading failed.\n");
            sleep(READING_INTERVAL_SECONDS);
            continue;
        }

        /* Validate and classify temperature. */
        if (!processor_process(&sample, &result)) {
            fprintf(stderr, "ERROR: Invalid temperature reading.\n");
            sleep(READING_INTERVAL_SECONDS);
            continue;
        }

        printf("Temperature: %.2f C | Source: SIMULATED | ",
               result.temperature_c);

        switch (result.status) {
            case TEMPERATURE_STATUS_NORMAL:
                printf("Status: NORMAL\n");
                break;

            case TEMPERATURE_STATUS_WARNING:
                printf("Status: WARNING\n");
                break;

            case TEMPERATURE_STATUS_CRITICAL:
                printf("Status: CRITICAL\n");
                break;
        }

        /* Save processed reading to CSV. */
        if (logger_log_temperature(&result) != LOGGER_STATUS_OK) {
            fprintf(stderr, "ERROR: Temperature logging failed.\n");
        }

        /* Publish processed reading through MQTT. */
        if (mqtt_initialized && mqtt_publish_temperature(&result) != MQTT_STATUS_OK) {
            fprintf(stderr, "WARNING: MQTT reading publish failed.\n");
        }

        /* Generate an alert when necessary. */
        alert_status = alert_generate(&result, &alert);

        if (alert_status == ALERT_GENERATION_CREATED) {
            printf("ALERT: %s temperature detected!\n",
                   alert.level == ALERT_LEVEL_WARNING
                       ? "WARNING" : "CRITICAL");

            /* Save alert to CSV. */
            if (logger_log_alert(&alert) != LOGGER_STATUS_OK) {
                fprintf(stderr, "ERROR: Alert logging failed.\n");
            }

            /* Publish alert through MQTT. */
            if (mqtt_initialized && mqtt_publish_alert(&alert) != MQTT_STATUS_OK) {
                fprintf(stderr, "WARNING: MQTT alert publish failed.\n");
            }
        }
        else if (alert_status == ALERT_GENERATION_ERROR) {
            fprintf(stderr, "ERROR: Alert generation failed.\n");
        }

        printf("------------------------------------\n");

        sleep(READING_INTERVAL_SECONDS);
    }

    printf("\nShutdown requested.\n");

cleanup:

    if (mqtt_initialized) {
        mqtt_cleanup();
        printf("MQTT disconnected.\n");
    }

    if (logger_initialized) {
        logger_close();
        printf("Logger closed.\n");
    }

    if (sensor != NULL) {
        sensor_destroy(sensor);
        printf("Sensor released.\n");
    }

    printf("Gateway stopped.\n");

    return 0;
}
