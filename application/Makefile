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

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
