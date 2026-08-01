#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "state_machine.h"
#include "sensor_service.h"
#include "processing.h"
#include "telemetry.h"
#include "watchdog.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Thread stack sizes and priorities */
#define SENSOR_STACK_SIZE 1024
#define PROCESSING_STACK_SIZE 1024
#define STATE_MACHINE_STACK_SIZE 1024
#define TELEMETRY_STACK_SIZE 1024
#define WATCHDOG_STACK_SIZE 512

#define SENSOR_PRIORITY 7
#define PROCESSING_PRIORITY 5
#define STATE_MACHINE_PRIORITY 3
#define TELEMETRY_PRIORITY 9
#define WATCHDOG_PRIORITY 1

/* Thread definitions */
K_THREAD_DEFINE(sensor_tid, SENSOR_STACK_SIZE,
                sensor_thread_entry, NULL, NULL, NULL,
                SENSOR_PRIORITY, 0, 0);

K_THREAD_DEFINE(processing_tid, PROCESSING_STACK_SIZE,
                processing_thread_entry, NULL, NULL, NULL,
                PROCESSING_PRIORITY, 0, 0);

K_THREAD_DEFINE(fsm_tid, STATE_MACHINE_STACK_SIZE,
                state_machine_thread_entry, NULL, NULL, NULL,
                STATE_MACHINE_PRIORITY, 0, 0);

K_THREAD_DEFINE(telemetry_tid, TELEMETRY_STACK_SIZE,
                telemetry_thread_entry, NULL, NULL, NULL,
                TELEMETRY_PRIORITY, 0, 0);

K_THREAD_DEFINE(watchdog_tid, WATCHDOG_STACK_SIZE,
                watchdog_thread_entry, NULL, NULL, NULL,
                WATCHDOG_PRIORITY, 0, 0);

int main(void)
{
    LOG_INF("Starting Virtual Biomedical Sensor-Node...");
    LOG_INF("Board: %s", CONFIG_BOARD);
    
    return 0;
}
