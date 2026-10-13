#include <stdio.h>
#include <stdlib.h>
#include "alarm_list.h"

void alarm_list_init(AlarmList *list)
{
    if (list == NULL) return;
    list->head = NULL;
    list->tail = NULL;
    list->total_alarms = 0;
}

int alarm_list_insert_sorted(AlarmList *list, AlarmNode *node)
{
    AlarmNode *cur;

    if (list == NULL || node == NULL) return -1;
    node->prev = NULL;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        list->total_alarms++;
        return 0;
    }

    cur = list->head;
    while (cur != NULL) {
        if (node->severity > cur->severity ||
            (node->severity == cur->severity &&
             node->timestamp < cur->timestamp)) {
            node->next = cur;
            node->prev = cur->prev;
            if (cur->prev != NULL) cur->prev->next = node;
            else list->head = node;
            cur->prev = node;
            list->total_alarms++;
            return 0;
        }
        cur = cur->next;
    }

    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
    list->total_alarms++;
    return 0;
}

AlarmNode *alarm_list_find(const AlarmList *list, int sensor_id)
{
    AlarmNode *cur;
    if (list == NULL) return NULL;
    cur = list->head;
    while (cur != NULL) {
        if (cur->sensor_id == sensor_id) return cur;
        cur = cur->next;
    }
    return NULL;
}

int alarm_list_delete(AlarmList *list, int sensor_id)
{
    AlarmNode *node = alarm_list_find(list, sensor_id);
    if (node == NULL) return -1;

    if (node->prev != NULL) node->prev->next = node->next;
    else list->head = node->next;

    if (node->next != NULL) node->next->prev = node->prev;
    else list->tail = node->prev;

    free(node);
    list->total_alarms--;
    return 0;
}

static const char *severity_name(SeverityLevel severity)
{
    switch (severity) {
        case SEVERITY_CRITICAL: return "CRITICAL";
        case SEVERITY_MAJOR: return "MAJOR";
        case SEVERITY_MINOR: return "MINOR";
        case SEVERITY_WARNING: return "WARNING";
        default: return "UNKNOWN";
    }
}

static const char *sensor_name(SensorType type)
{
    switch (type) {
        case SENSOR_TEMP_CELSIUS: return "TEMP_C";
        case SENSOR_OPTICAL_RX_DBM: return "OPTICAL_RX";
        case SENSOR_VOLTAGE_VOLTS: return "VOLTAGE";
        case SENSOR_FAN_RPM: return "FAN_RPM";
        default: return "UNKNOWN";
    }
}

void alarm_list_print(const AlarmList *list)
{
    AlarmNode *cur;
    if (list == NULL) return;

    printf("\n================ ACTIVE ALARM REGISTER (%d) ================\n",
           list->total_alarms);
    if (list->head == NULL) printf("(No active alarms)\n");

    for (cur = list->head; cur != NULL; cur = cur->next) {
        printf("[%-8s] Sensor ID: %d | Type: %-10s | Value: %.2f | Time: %ld\n",
               severity_name(cur->severity), cur->sensor_id,
               sensor_name(cur->sensor_type), cur->trigger_value,
               cur->timestamp);
    }
    printf("============================================================\n");
}

void alarm_list_clear_all(AlarmList *list)
{
    AlarmNode *cur;
    if (list == NULL) return;
    cur = list->head;
    while (cur != NULL) {
        AlarmNode *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->total_alarms = 0;
}
