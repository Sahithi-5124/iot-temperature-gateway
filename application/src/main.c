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

#define MQTT_BROKER_HOST "localhost"
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

int main(void)
{
    sensor_device_t *sensor = NULL;

    bool logger_initialized = false;
    bool mqtt_initialized = false;

    struct sigaction action = {0};

    printf("\n====================================\n");
    printf("   IoT Temperature Monitoring Gateway\n");
    printf("====================================\n");

    /* Register Ctrl+C handler. */
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1) {
        fprintf(stderr, "ERROR: Signal handler setup failed.\n");
        return 1;
    }

    /* Initialize simulated sensor. */
    if (sensor_create(SENSOR_SOURCE_SIMULATED, &sensor)
        != SENSOR_STATUS_OK) {
        fprintf(stderr, "ERROR: Sensor initialization failed.\n");
        goto cleanup;
    }

    printf("Sensor initialized successfully.\n");

    /* Initialize local CSV logger. */
    if (logger_init(LOG_FILE_PATH) != LOGGER_STATUS_OK) {
        fprintf(stderr, "ERROR: Logger initialization failed.\n");
        goto cleanup;
    }

    logger_initialized = true;
    printf("Logger initialized successfully.\n");

    /* Connect to MQTT broker. */
    if (mqtt_init(MQTT_BROKER_HOST, MQTT_BROKER_PORT)
        != MQTT_STATUS_OK) {
        fprintf(stderr, "ERROR: MQTT initialization failed.\n");
        goto cleanup;
    }

    mqtt_initialized = true;
    printf("MQTT connected successfully.\n");

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
        if (mqtt_publish_temperature(&result) != MQTT_STATUS_OK) {
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
            if (mqtt_publish_alert(&alert) != MQTT_STATUS_OK) {
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
