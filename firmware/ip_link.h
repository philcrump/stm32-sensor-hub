#ifndef __IP_LINK_H__
#define __IP_LINK_H__

#define APP_IP_LINK_STATUS_DOWN         0
#define APP_IP_LINK_STATUS_UPBUTNOIP    1
#define APP_IP_LINK_STATUS_BOUND        2

uint32_t app_ip_link_status(void);

#endif /* __IP_LINK_H__ */