#ifndef __IP_LINK_H__
#define __IP_LINK_H__

#define APP_IP_LINK_STATUS_DOWN         0
#define APP_IP_LINK_STATUS_UPBUTNOIP    1
#define APP_IP_LINK_STATUS_BOUND        2

extern uint32_t app_ip_link_status;

uint32_t app_ip_link_status_update(void);

#include "lwip/netif.h"

void app_ip_link_netif_get(struct netif** netif_ptr_ptr);

#endif /* __IP_LINK_H__ */