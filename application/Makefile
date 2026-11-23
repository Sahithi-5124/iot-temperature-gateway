CC ?= gcc

TARGET ?= iot_gateway

CPPFLAGS += -Iinclude
CFLAGS ?= -O2
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic

LDLIBS += -lmosquitto -lm

SOURCES = \
	src/main.c \
	src/sensor/sensor.c \
	src/processing/processor.c \
	src/alerts/alert.c \
	src/logging/logger.c \
	src/mqtt/mqtt.c

OBJECTS = $(SOURCES:.c=.o)

TEST_TARGETS = test_sensor test_processor test_alert test_logger test_mqtt test_mqtt_reconnect

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

test_sensor: tests/test_sensor.c src/sensor/sensor.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_processor: tests/test_processor.c src/processing/processor.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ -lm

test_alert: tests/test_alert.c src/alerts/alert.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_logger: tests/test_logger.c src/logging/logger.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_mqtt: tests/test_mqtt.c src/mqtt/mqtt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

test_mqtt_reconnect: tests/test_mqtt_reconnect.c src/mqtt/mqtt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

test: test_sensor test_processor test_alert test_logger
	@echo "=== Running Offline Unit Tests ==="
	./test_sensor
	./test_processor
	./test_alert
	./test_logger
	@echo "=== All Offline Unit Tests Passed ==="

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_TARGETS) *.csv test_temperature_log.csv
