#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <math.h>
#include "processing.h"
#include "sensor_service.h"
#include "state_machine.h"

LOG_MODULE_REGISTER(processing, LOG_LEVEL_INF);

/* External event queue defined in state_machine.c */
extern struct k_msgq event_msgq;

/* Sensor Fusion thresholds */
#define HR_WARNING_THRESHOLD  120 // BPM
#define HR_ALARM_THRESHOLD    150 // BPM
#define MOTION_EXERCISE_LIMIT 250 // g-force magnitude scaled

void processing_thread_entry(void *p1, void *p2, void *p3)
{
    sensor_sample_t sample;
    fusion_event_t event;

    LOG_INF("Processing & Sensor Fusion thread started.");

    while (1) {
        if (k_msgq_get(&sensor_msgq, &sample, K_FOREVER) == 0) {
            if (!sample.is_valid) {
                event = EVENT_SENSOR_DISCONNECT;
            } else {
                /* Calculate magnitude of 3-axis motion acceleration */
                int32_t motion_mag = (int32_t)sqrt((double)(sample.accel_x * sample.accel_x +
                                                    sample.accel_y * sample.accel_y +
                                                    sample.accel_z * sample.accel_z));

                /* Simulate Heart Rate estimation from PPG Red channel (for demo windowing) */
                int32_t estimated_hr = sample.ppg_red / 100; // Scaled mock HR

                /* Sensor Fusion Decision Matrix */
                if (estimated_hr >= HR_ALARM_THRESHOLD) {
                    if (motion_mag > MOTION_EXERCISE_LIMIT) {
                        /* High HR + High Motion = Patient is exercising (Normal context) */
                        event = EVENT_EXERCISE_DETECTED;
                    } else {
                        /* High HR + Low Motion = Medical anomaly (Tachycardia Alarm) */
                        event = EVENT_TACHYCARDIA_ALARM;
                    }
                } else if (estimated_hr >= HR_WARNING_THRESHOLD) {
                    event = EVENT_TACHYCARDIA_WARNING;
                } else {
                    event = EVENT_NORMAL;
                }
            }

            /* Push fusion event to State Machine queue */
            k_msgq_put(&event_msgq, &event, K_NO_WAIT);
        }
    }
}
