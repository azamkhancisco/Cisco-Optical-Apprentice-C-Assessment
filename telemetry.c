#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "telemetry.h"

typedef struct {
    int abnormal;
    SeverityLevel severity;
    const char *description;
} Evaluation;

static Evaluation evaluate(SensorType type, float value)
{
    Evaluation e = {0, SEVERITY_WARNING, "Out of normal range"};

    switch (type) {
        case SENSOR_TEMP_CELSIUS:
            if (value < 20.0f || value > 65.0f) e.abnormal = 1;
            if (value > 85.0f) e.severity = SEVERITY_CRITICAL;
            else if (value > 65.0f) e.severity = SEVERITY_MINOR;
            break;
        case SENSOR_OPTICAL_RX_DBM:
            if (value < -18.0f || value > -3.0f) e.abnormal = 1;
            if (value < -25.0f) e.severity = SEVERITY_CRITICAL;
            else if (value < -18.0f || value > -3.0f) e.severity = SEVERITY_MINOR;
            break;
        case SENSOR_VOLTAGE_VOLTS:
            if (value < 11.4f || value > 12.6f) e.abnormal = 1;
            if (value < 10.5f || value > 13.5f) e.severity = SEVERITY_CRITICAL;
            else if (e.abnormal) e.severity = SEVERITY_MINOR;
            break;
        case SENSOR_FAN_RPM:
            if (value < 3000.0f || value > 8000.0f) e.abnormal = 1;
            if (value < 1500.0f) e.severity = SEVERITY_CRITICAL;
            else if (value < 3000.0f) e.severity = SEVERITY_MINOR;
            break;
        default:
            e.abnormal = -1;
            break;
    }
    return e;
}

static SensorState *find_state(Telemetry *t, int id)
{
    SensorState *s;
    for (s = t->states; s != NULL; s = s->next)
        if (s->sensor_id == id) return s;
    return NULL;
}

static SensorState *get_state(Telemetry *t, int id, SensorType type)
{
    SensorState *s = find_state(t, id);
    if (s != NULL) {
        if (s->type != type) {
            s->type = type;
            s->out_count = 0;
            s->normal_count = 0;
        }
        return s;
    }
    s = malloc(sizeof(*s));
    if (s == NULL) return NULL;
    s->sensor_id = id;
    s->type = type;
    s->out_count = 0;
    s->normal_count = 0;
    s->last_value = 0.0f;
    s->last_timestamp = 0;
    s->next = t->states;
    t->states = s;
    return s;
}

void telemetry_init(Telemetry *t, AlarmList *alarms)
{
    if (t == NULL) return;
    t->alarms = alarms;
    t->states = NULL;
    t->total_samples = 0;
    t->start_tick = 0;
    t->last_tick = 0;
}

int telemetry_feed(Telemetry *t, int id, SensorType type, float value, long timestamp)
{
    SensorState *s;
    AlarmNode *alarm;
    Evaluation e;

    if (t == NULL || t->alarms == NULL || id < 0) return -1;
    e = evaluate(type, value);
    if (e.abnormal < 0) return -1;

    s = get_state(t, id, type);
    if (s == NULL) return -1;

    t->total_samples++;
    if (t->total_samples == 1) t->start_tick = timestamp;
    t->last_tick = timestamp;
    s->last_value = value;
    s->last_timestamp = timestamp;

    alarm = alarm_list_find(t->alarms, id);

    if (e.abnormal) {
        s->normal_count = 0;
        if (s->out_count < 3) s->out_count++;
        printf("[Telemetry] Sensor %d: Out-of-spec sample count = %d",
               id, s->out_count);
        if (s->out_count >= 3) {
            if (alarm == NULL) {
                alarm = malloc(sizeof(*alarm));
                if (alarm == NULL) return -1;
                alarm->sensor_id = id;
                alarm->sensor_type = type;
                alarm->severity = e.severity;
                alarm->trigger_value = value;
                alarm->timestamp = timestamp;
                (void)snprintf(alarm->description, sizeof(alarm->description),
                               "%s", e.description);
                alarm->prev = NULL;
                alarm->next = NULL;
                if (alarm_list_insert_sorted(t->alarms, alarm) != 0) {
                    free(alarm);
                    return -1;
                }
                printf(" -> ALARM RAISED [%s]\n",
                       e.severity == SEVERITY_CRITICAL ? "CRITICAL" :
                       e.severity == SEVERITY_MAJOR ? "MAJOR" :
                       e.severity == SEVERITY_MINOR ? "MINOR" : "WARNING");
            } else {
                if (alarm->severity != e.severity) {
                    (void)alarm_list_delete(t->alarms, id);
                    alarm = malloc(sizeof(*alarm));
                    if (alarm == NULL) return -1;
                    alarm->sensor_id = id;
                    alarm->sensor_type = type;
                    alarm->severity = e.severity;
                    alarm->trigger_value = value;
                    alarm->timestamp = timestamp;
                    (void)snprintf(alarm->description, sizeof(alarm->description),
                                   "%s", e.description);
                    alarm->prev = NULL;
                    alarm->next = NULL;
                    if (alarm_list_insert_sorted(t->alarms, alarm) != 0) {
                        free(alarm);
                        return -1;
                    }
                } else {
                    alarm->trigger_value = value;
                    alarm->timestamp = timestamp;
                }
                printf(" -> ALARM UPDATED [%s]\n",
                       e.severity == SEVERITY_CRITICAL ? "CRITICAL" :
                       e.severity == SEVERITY_MAJOR ? "MAJOR" :
                       e.severity == SEVERITY_MINOR ? "MINOR" : "WARNING");
            }
        } else {
            printf(".\n");
        }
    } else {
        s->out_count = 0;
        if (alarm != NULL) {
            if (s->normal_count < 2) s->normal_count++;
            printf("[Telemetry] Sensor %d: Normal sample count = %d",
                   id, s->normal_count);
            if (s->normal_count >= 2) {
                (void)alarm_list_delete(t->alarms, id);
                s->normal_count = 0;
                printf(" -> ALARM CLEARED\n");
            } else {
                printf(".\n");
            }
        } else {
            s->normal_count = 0;
            printf("[Telemetry] Sensor %d: Sample %.2f ingested (Normal).\n",
                   id, value);
        }
    }
    return 0;
}

void telemetry_clear_sensor(Telemetry *t, int id)
{
    SensorState *s;
    if (t == NULL) return;
    (void)alarm_list_delete(t->alarms, id);
    s = find_state(t, id);
    if (s != NULL) {
        s->out_count = 0;
        s->normal_count = 0;
    }
}

void telemetry_destroy(Telemetry *t)
{
    SensorState *s;
    if (t == NULL) return;
    s = t->states;
    while (s != NULL) {
        SensorState *next = s->next;
        free(s);
        s = next;
    }
    t->states = NULL;
}
