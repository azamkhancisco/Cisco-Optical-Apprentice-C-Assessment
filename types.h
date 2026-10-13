#ifndef TYPES_H
#define TYPES_H

typedef enum {
    SEVERITY_WARNING = 1,
    SEVERITY_MINOR = 2,
    SEVERITY_MAJOR = 3,
    SEVERITY_CRITICAL = 4
} SeverityLevel;

typedef enum {
    SENSOR_TEMP_CELSIUS = 1,
    SENSOR_OPTICAL_RX_DBM = 2,
    SENSOR_VOLTAGE_VOLTS = 3,
    SENSOR_FAN_RPM = 4
} SensorType;

typedef struct AlarmNode {
    int sensor_id;
    SensorType sensor_type;
    SeverityLevel severity;
    float trigger_value;
    long timestamp;
    char description[64];
    struct AlarmNode *prev;
    struct AlarmNode *next;
} AlarmNode;

typedef struct {
    AlarmNode *head;
    AlarmNode *tail;
    int total_alarms;
} AlarmList;

#endif
