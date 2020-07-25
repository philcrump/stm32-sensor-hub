#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#define WATCHDOG_DOG_MAIN        0
#define WATCHDOG_DOG_WEB_TLS     1

#define WATCHDOG_MASK       ((1 << WATCHDOG_DOG_MAIN) \
							| (0 << WATCHDOG_DOG_WEB_TLS))

void watchdog_init(void);

THD_FUNCTION(watchdog_service_thread, arg);

void watchdog_feed(uint32_t dog);

#endif /* __WATCHDOG_H__ */