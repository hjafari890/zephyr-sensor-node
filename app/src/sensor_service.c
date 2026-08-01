#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include "sensor_service.h"
#include "watchdog.h"

LOG_MODULE_REGISTER(sensor_svc, LOG_LEVEL_INF);

/* Define message queue: capacity of 10 samples */
K_MSGQ_DEFINE(sensor_msgq, sizeof(sensor_sample_t), 10, 4);

/* Global copy for telemetry */
sensor_sample_t latest_sample;

#include <zephyr/random/random.h>
#include <zephyr/drivers/uart.h>
#include <stdlib.h>
#include <string.h>

/* Global variables that can be modified by the UART Input Simulator */
static int32_t sim_bpm = 70;
static int32_t sim_temp_c = 365;
static int32_t sim_humidity = 45;
static int32_t sim_pressure = 1013;
static int32_t sim_light_lux = 300;
static int32_t sim_accel_z = 981;

#define RX_BUF_SIZE 128
static char rx_buf[RX_BUF_SIZE];
static int rx_buf_pos = 0;

static void uart_cb(const struct device *dev, void *user_data)
{
    uart_irq_update(dev);
    
    if (uart_irq_rx_ready(dev)) {
        uint8_t c;
        while (uart_fifo_read(dev, &c, 1) == 1) {
            if (c == '\n' || c == '\r') {
                rx_buf[rx_buf_pos] = '\0';
                if (rx_buf_pos > 0) {
                    if (strncmp(rx_buf, "BPM:", 4) == 0) {
                        sim_bpm = atoi(rx_buf + 4);
                    } else if (strncmp(rx_buf, "TEMP:", 5) == 0) {
                        sim_temp_c = atoi(rx_buf + 5);
                    } else if (strncmp(rx_buf, "HUM:", 4) == 0) {
                        sim_humidity = atoi(rx_buf + 4);
                    } else if (strncmp(rx_buf, "PRES:", 5) == 0) {
                        sim_pressure = atoi(rx_buf + 5);
                    } else if (strncmp(rx_buf, "LGT:", 4) == 0) {
                        sim_light_lux = atoi(rx_buf + 4);
                    } else if (strncmp(rx_buf, "ACCZ:", 5) == 0) {
                        sim_accel_z = atoi(rx_buf + 5);
                    }
                }
                rx_buf_pos = 0;
            } else if (rx_buf_pos < RX_BUF_SIZE - 1) {
                rx_buf[rx_buf_pos++] = (char)c;
            }
        }
    }
}

void sensor_thread_entry(void *p1, void *p2, void *p3)
{
    LOG_INF("Starting mocked sensor sampling with Input Simulator support...");

    const struct device *uart1_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));
    if (device_is_ready(uart1_dev)) {
        uart_irq_callback_set(uart1_dev, uart_cb);
        uart_irq_rx_enable(uart1_dev);
        LOG_INF("Input Simulator active on uart1");
    }

    sensor_sample_t sample;
    
    while (1) {
        sample.timestamp = k_uptime_get_32();
        sample.is_valid = true;

        /* Apply random jitter (noise) to the simulated base values to make it look realistic */
        int noise_bpm = (sys_rand32_get() % 5) - 2;
        int noise_temp = (sys_rand32_get() % 5) - 2;
        int noise_accel = (sys_rand32_get() % 5) - 2;
        int noise_light = (sys_rand32_get() % 11) - 5;
        
        sample.ppg_red = (sim_bpm + noise_bpm) * 100; 
        sample.accel_x = 10;
        sample.accel_y = 10;
        sample.accel_z = sim_accel_z + noise_accel;
        
        sample.temp_c = sim_temp_c + noise_temp;
        sample.humidity = sim_humidity;
        sample.pressure = sim_pressure;
        sample.light_lux = sim_light_lux + noise_light;

        latest_sample = sample;

        /* Feed the watchdog timer */
        watchdog_feed_sensor();

        /* Push sample to processing message queue */
        while (k_msgq_put(&sensor_msgq, &sample, K_NO_WAIT) != 0) {
            k_msgq_purge(&sensor_msgq);
        }

        k_msleep(100); // 100ms sampling rate (10Hz)
    }
}
