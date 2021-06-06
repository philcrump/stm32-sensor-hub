#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct {
  uint32_t        address;
  uint32_t        netmask;
  uint32_t        gateway;
  /**
   * @brief   Startup network addressing mode - static, DHCP, auto.
   */
  net_addr_mode_t address_mode; // static, DHCP, auto.
  char hostname[64];
} app_config_network_t;

typedef struct {
  uint32_t _start; /* Waste of space, but it's cheap here */
  app_config_network_t network;
  uint32_t _padding;
  uint32_t crc;
} app_config_t;

extern app_config_t app_config;

void config_load(void);

void config_setdefaults(void);

void config_setnetwork(app_config_network_t *new_config_network_ptr);

void config_network_load_and_start(void);
void config_network_reload(void);

#endif /* __CONFIG_H__ */