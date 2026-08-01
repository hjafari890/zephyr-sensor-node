#ifndef PROCESSING_H
#define PROCESSING_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    EVENT_NONE,
    EVENT_NORMAL,
    EVENT_TACHYCARDIA_WARNING,
    EVENT_TACHYCARDIA_ALARM,
    EVENT_EXERCISE_DETECTED,
    EVENT_SENSOR_DISCONNECT
} fusion_event_t;

void processing_thread_entry(void *p1, void *p2, void *p3);

#endif
