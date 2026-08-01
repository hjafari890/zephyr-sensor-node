#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <stdint.h>
#include <zephyr/kernel.h>

typedef struct {
    uint32_t timestamp;
    int32_t ppg_red;
    int32_t ppg_ir;
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int32_t temp_c;
    int32_t humidity;
    int32_t pressure;
    int32_t light_lux;
    bool is_valid;
} sensor_sample_t;

/* Global access to latest sample for telemetry */
extern sensor_sample_t latest_sample;

/* Message queue for passing sensor samples to processing thread */
extern struct k_msgq sensor_msgq;

void sensor_thread_entry(void *p1, void *p2, void *p3);

#endif
