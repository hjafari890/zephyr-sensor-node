#ifndef WATCHDOG_H
#define WATCHDOG_H

void watchdog_feed_sensor(void);
void watchdog_thread_entry(void *p1, void *p2, void *p3);

#endif
