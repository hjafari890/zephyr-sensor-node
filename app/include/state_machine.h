#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <zephyr/kernel.h>

typedef enum {
    STATE_INIT,
    STATE_NORMAL,
    STATE_WARNING,
    STATE_ALARM,
    STATE_SENSOR_FAULT,
    STATE_RECOVERY
} system_state_t;

extern struct k_msgq event_msgq;
system_state_t get_current_system_state(void);
const char* state_to_string(system_state_t state);

void state_machine_thread_entry(void *p1, void *p2, void *p3);

#endif
