#ifndef ALARM_LIST_H
#define ALARM_LIST_H

#include "types.h"

void alarm_list_init(AlarmList *list);
int alarm_list_insert_sorted(AlarmList *list, AlarmNode *node);
AlarmNode *alarm_list_find(const AlarmList *list, int sensor_id);
int alarm_list_delete(AlarmList *list, int sensor_id);
void alarm_list_print(const AlarmList *list);
void alarm_list_clear_all(AlarmList *list);

#endif
