#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "state_machine.h"
#include "processing.h"

LOG_MODULE_REGISTER(fsm, LOG_LEVEL_INF);

/* Message queue for incoming fusion events */
K_MSGQ_DEFINE(event_msgq, sizeof(fusion_event_t), 10, 4);

static system_state_t current_state = STATE_INIT;
static int recovery_counter = 0;
#define RECOVERY_HYSTERESIS_TICKS 5 // Must remain stable for 5 ticks before returning to NORMAL

system_state_t get_current_system_state(void)
{
    return current_state;
}

const char* state_to_string(system_state_t state)
{
    switch (state) {
        case STATE_INIT:          return "INIT";
        case STATE_NORMAL:        return "NORMAL";
        case STATE_WARNING:       return "WARNING";
        case STATE_ALARM:         return "ALARM";
        case STATE_SENSOR_FAULT:  return "SENSOR_FAULT";
        case STATE_RECOVERY:       return "RECOVERY";
        default:                  return "UNKNOWN";
    }
}

void state_machine_thread_entry(void *p1, void *p2, void *p3)
{
    fusion_event_t event;

    LOG_INF("State Machine FSM Thread Started.");
    current_state = STATE_NORMAL;
    LOG_INF("[FSM State Change] -> NORMAL");

    while (1) {
        if (k_msgq_get(&event_msgq, &event, K_FOREVER) == 0) {
            system_state_t next_state = current_state;

            switch (current_state) {
                case STATE_NORMAL:
                    if (event == EVENT_SENSOR_DISCONNECT) {
                        next_state = STATE_SENSOR_FAULT;
                    } else if (event == EVENT_TACHYCARDIA_ALARM) {
                        next_state = STATE_ALARM;
                    } else if (event == EVENT_TACHYCARDIA_WARNING) {
                        next_state = STATE_WARNING;
                    }
                    break;

                case STATE_WARNING:
                    if (event == EVENT_TACHYCARDIA_ALARM) {
                        next_state = STATE_ALARM;
                    } else if (event == EVENT_NORMAL || event == EVENT_EXERCISE_DETECTED) {
                        next_state = STATE_NORMAL;
                    } else if (event == EVENT_SENSOR_DISCONNECT) {
                        next_state = STATE_SENSOR_FAULT;
                    }
                    break;

                case STATE_ALARM:
                    if (event == EVENT_SENSOR_DISCONNECT) {
                        next_state = STATE_SENSOR_FAULT;
                    } else if (event == EVENT_NORMAL || event == EVENT_EXERCISE_DETECTED) {
                        next_state = STATE_RECOVERY;
                        recovery_counter = 0;
                    }
                    break;

                case STATE_SENSOR_FAULT:
                    if (event != EVENT_SENSOR_DISCONNECT) {
                        next_state = STATE_RECOVERY;
                        recovery_counter = 0;
                    }
                    break;

                case STATE_RECOVERY:
                    if (event == EVENT_SENSOR_DISCONNECT) {
                        next_state = STATE_SENSOR_FAULT;
                    } else if (event == EVENT_TACHYCARDIA_ALARM) {
                        next_state = STATE_ALARM;
                    } else if (event == EVENT_NORMAL || event == EVENT_EXERCISE_DETECTED) {
                        recovery_counter++;
                        if (recovery_counter >= RECOVERY_HYSTERESIS_TICKS) {
                            next_state = STATE_NORMAL;
                        }
                    }
                    break;

                default:
                    next_state = STATE_NORMAL;
                    break;
            }

            if (next_state != current_state) {
                LOG_WRN("[FSM State Change] %s -> %s", state_to_string(current_state), state_to_string(next_state));
                current_state = next_state;
            }
        }
    }
}
