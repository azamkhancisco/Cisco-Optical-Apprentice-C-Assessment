#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "alarm_list.h"

typedef struct SensorState {
    int sensor_id;
    SensorType type;
    int out_count;
    int normal_count;
    float last_value;
    long last_timestamp;
    struct SensorState *next;
} SensorState;

typedef struct {
    AlarmList *alarms;
    SensorState *states;
    unsigned long total_samples;
    long start_tick;
    long last_tick;
} Telemetry;

void telemetry_init(Telemetry *telemetry, AlarmList *alarms);
int telemetry_feed(Telemetry *telemetry, int sensor_id, SensorType type,
                   float value, long timestamp);
void telemetry_clear_sensor(Telemetry *telemetry, int sensor_id);
void telemetry_destroy(Telemetry *telemetry);

#endif
