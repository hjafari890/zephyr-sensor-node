#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "telemetry.h"
#include "state_machine.h"
#include "sensor_service.h"

LOG_MODULE_REGISTER(telemetry, LOG_LEVEL_INF);

void telemetry_thread_entry(void *p1, void *p2, void *p3)
{
    LOG_INF("Telemetry Thread Started.");

    while (1) {
        system_state_t state = get_current_system_state();
        
        /* Structured UART Log Output for Renode / Robot Framework Assertions */
        LOG_INF("[TELEMETRY] {\"status\":\"%s\", \"uptime_ms\":%lld, \"bpm\":%d, \"temp_c\":%d, \"humidity\":%d, \"pressure\":%d, \"light\":%d, \"accel_mag\":%d}",
                state_to_string(state),
                k_uptime_get(),
                latest_sample.ppg_red / 100,
                latest_sample.temp_c,
                latest_sample.humidity,
                latest_sample.pressure,
                latest_sample.light_lux,
                latest_sample.accel_z); // Mocking mag with Z for now

        k_msleep(1000); // 1-second telemetry interval
    }
}
