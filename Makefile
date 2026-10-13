CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic
TARGET = staam
SOURCES = main.c alarm_list.c telemetry.c

.PHONY: all clean test
all: $(TARGET)

$(TARGET): $(SOURCES) types.h alarm_list.h telemetry.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	./$(TARGET) < sample_input.txt

clean:
	rm -f $(TARGET)
