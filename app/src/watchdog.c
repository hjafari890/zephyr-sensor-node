#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "watchdog.h"

LOG_MODULE_REGISTER(watchdog, LOG_LEVEL_INF);

static atomic_t sensor_heartbeat = ATOMIC_INIT(0);

void watchdog_feed_sensor(void)
{
    atomic_set(&sensor_heartbeat, 1);
}

void watchdog_thread_entry(void *p1, void *p2, void *p3)
{
    LOG_INF("Watchdog & Health Monitoring Thread Started.");

    while (1) {
        k_msleep(3000); // Check every 3 seconds

        if (atomic_cas(&sensor_heartbeat, 1, 0) == false) {
            LOG_ERR("[WATCHDOG FAULT] Sensor acquisition thread missed heartbeat!");
        } else {
            // Heartbeat OK
        }
    }
}
