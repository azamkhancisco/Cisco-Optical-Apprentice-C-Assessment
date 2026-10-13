#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "telemetry.h"

static int parse_int(const char *text, int *out)
{
    char *end;
    long value;
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < -2147483647L - 1L || value > 2147483647L) return -1;
    *out = (int)value;
    return 0;
}

static int parse_long_value(const char *text, long *out)
{
    char *end;
    errno = 0;
    *out = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') return -1;
    return 0;
}

static int parse_float_value(const char *text, float *out)
{
    char *end;
    errno = 0;
    *out = strtof(text, &end);
    if (errno != 0 || end == text || *end != '\0') return -1;
    return 0;
}

int main(void)
{
    char line[256];
    AlarmList alarms;
    Telemetry telemetry;

    alarm_list_init(&alarms);
    telemetry_init(&telemetry, &alarms);
    puts("STAAM - Sensor Telemetry & Active Alarm Manager");
    puts("Commands: FEED <id> <type> <value> <timestamp>, SHOW ALARMS, CLEAR <id>, STATS, EXIT");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *cmd = strtok(line, " \t\r\n");
        if (cmd == NULL) continue;

        {
            char *p;
            for (p = cmd; *p != '\0'; ++p) *p = (char)toupper((unsigned char)*p);
        }

        if (strcmp(cmd, "FEED") == 0) {
            char *id_s = strtok(NULL, " \t\r\n");
            char *type_s = strtok(NULL, " \t\r\n");
            char *value_s = strtok(NULL, " \t\r\n");
            char *time_s = strtok(NULL, " \t\r\n");
            int id, type;
            float value;
            long timestamp;
            if (id_s == NULL || type_s == NULL || value_s == NULL ||
                time_s == NULL || parse_int(id_s, &id) != 0 ||
                parse_int(type_s, &type) != 0 ||
                parse_float_value(value_s, &value) != 0 ||
                parse_long_value(time_s, &timestamp) != 0 ||
                type < SENSOR_TEMP_CELSIUS || type > SENSOR_FAN_RPM) {
                puts("[Error] Invalid FEED syntax or value.");
                continue;
            }
            if (telemetry_feed(&telemetry, id, (SensorType)type, value, timestamp) != 0)
                puts("[Error] Could not process telemetry sample.");
        } else if (strcmp(cmd, "SHOW") == 0) {
            char *second = strtok(NULL, " \t\r\n");
            if (second != NULL && strcmp(second, "ALARMS") == 0)
                alarm_list_print(&alarms);
            else
                puts("[Error] Use SHOW ALARMS.");
        } else if (strcmp(cmd, "CLEAR") == 0) {
            char *id_s = strtok(NULL, " \t\r\n");
            int id;
            if (id_s == NULL || parse_int(id_s, &id) != 0) {
                puts("[Error] Use CLEAR <id>.");
            } else {
                telemetry_clear_sensor(&telemetry, id);
                printf("[System] Clear requested for sensor %d.\n", id);
            }
        } else if (strcmp(cmd, "STATS") == 0) {
            printf("[Stats] Samples: %lu | Active alarms: %d | Tick span: %ld\n",
                   telemetry.total_samples, alarms.total_alarms,
                   telemetry.total_samples == 0 ? 0 : telemetry.last_tick - telemetry.start_tick);
        } else if (strcmp(cmd, "EXIT") == 0) {
            printf("[System] Freeing all active alarms (%d remaining)...\n", alarms.total_alarms);
            alarm_list_clear_all(&alarms);
            telemetry_destroy(&telemetry);
            puts("[System] Graceful teardown complete.");
            return 0;
        } else {
            puts("[Error] Unknown command.");
        }
    }

    alarm_list_clear_all(&alarms);
    telemetry_destroy(&telemetry);
    return 0;
}
