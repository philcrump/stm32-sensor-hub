#include "main.h"

#include <string.h>

#include "chprintf.h"
#include "lwipthread.h"

extern void watchdog_reconfigSlow(void);
extern void watchdog_reconfigDefault(void);

#define FLASH_CONFIG_BASE_ADDR  0x08008000

#define FLASH_KEY1   0x45670123
#define FLASH_KEY2   0xCDEF89AB

#define APP_CONFIG_CRC_PAYLOAD_WSIZE (((sizeof(uint32_t) + sizeof(app_config_network_t)) >> 2) + 1)

app_config_t app_config;

static uint8_t _macAddress[] = {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x47};
static lwipthread_opts_t lwip_config = {
  .macaddress = _macAddress,
  .address = 0,
  .netmask = 0,
  .gateway = 0,
  .addrMode = NET_ADDRESS_DHCP,
  .ourHostName = "sensor-hub-h7",
  .link_up_cb = NULL,
  .link_down_cb = NULL
};

static uint32_t crc32_stm32f7(uint32_t *p, uint32_t plen)
{
  CRC->CR = CRC_CR_RESET; // Set defaults and reset
  while(plen--)
  {
    CRC->DR = *p++;
  }
  return CRC->DR;
}

static uint32_t config_crc32(void)
{
  return crc32_stm32f7((uint32_t *)&(app_config._start), APP_CONFIG_CRC_PAYLOAD_WSIZE);
}

static void config_write(void)
{
  palSetLine(LINE_LED3);
#if 0
  watchdog_reconfigSlow();

  debugWriteStr("Writing config to flash..\r\n");

  app_config.crc = config_crc32();

  /* Unlock Flash if not already unlocked */
  if((FLASH->CR & FLASH_CR_LOCK) != 0U)
  {
    FLASH->KEYR |= FLASH_KEY1;
    FLASH->KEYR |= FLASH_KEY2;
  }

  /* Erase Sector */
  while ((FLASH->SR & FLASH_SR_BSY) != 0U) { };
  FLASH->CR &= ~(FLASH_CR_MER1 | FLASH_CR_MER2); // Clear Mass Erase bits
  FLASH->CR |= FLASH_CR_PSIZE_1; // x32 (32b word)
  FLASH->CR |= FLASH_CR_SER; // Sector Erase
  FLASH->CR |= FLASH_CR_SNB_0; // Sector 1
  asm volatile("dsb; isb"); // Ensure completion of above writes
  FLASH->CR |= FLASH_CR_STRT; // Start sector erase
  while ((FLASH->SR & FLASH_SR_BSY) != 0U) { };
  FLASH->CR &= ~(FLASH_CR_SER | FLASH_CR_SNB_0); // Clear

  /* Program sector */
  FLASH->CR |= FLASH_CR_PSIZE_1; // x32 (32b word)
  FLASH->CR |= FLASH_CR_PG; // Enable Programming
  asm volatile("dsb; isb"); // Ensure completion of above writes

  while ((FLASH->SR & FLASH_SR_BSY) != 0U) { };
  for(uint32_t i = 0; i < sizeof(app_config_t); i++)
  {
    *((uint32_t *)(FLASH_CONFIG_BASE_ADDR) + i) = *(((uint32_t *)(&app_config)) + i);
    asm volatile("dsb; isb"); // Ensure completion of above write
    while ((FLASH->SR & FLASH_SR_BSY) != 0U) { };
  }
  FLASH->CR &= ~(FLASH_CR_PG); // Clear

  /* Relock Flash */
  FLASH->CR |= FLASH_CR_LOCK;

  debugWriteStr("Flushing DCache..\r\n");
  SCB_CleanDCache();

  debugWriteStr("Config write complete\r\n");

  watchdog_reconfigDefault();
#endif
  palClearLine(LINE_LED3);
}

void config_setdefaults(void)
{
  app_config.network.address = IP4_ADDR_VALUE(192,168,0,95);
  app_config.network.netmask = IP4_ADDR_VALUE(255,255,255,0);
  app_config.network.gateway = IP4_ADDR_VALUE(192,168,0,1);
  //app_config.network.address_mode = NETWORK_ADDRESS_STATIC;
  app_config.network.address_mode = NETWORK_ADDRESS_DHCP;
  strncpy(app_config.network.hostname, "sensor-hub-h7", 63);

  config_write();
}

void config_setnetwork(app_config_network_t *new_config_network_ptr)
{
  app_config.network.address = new_config_network_ptr->address;
  app_config.network.netmask = new_config_network_ptr->netmask;
  app_config.network.gateway = new_config_network_ptr->gateway;
  app_config.network.address_mode = new_config_network_ptr->address_mode;
  strncpy(app_config.network.hostname, new_config_network_ptr->hostname, 63);

  config_write();
}

void config_load(void)
{
  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)FLASH_CONFIG_BASE_ADDR, 0x8000);

  memcpy(&app_config, (uint32_t *)FLASH_CONFIG_BASE_ADDR, sizeof(app_config_t));

  uint32_t calc_crc = config_crc32();

  if(calc_crc != app_config.crc)
  {
    debugWriteStr("Config CRC fail - loading defaults..\r\n");

    config_setdefaults();
  }
  else
  {
    debugWriteStr("Config CRC ok\r\n");
  }
}

void config_network_load_and_start(void)
{
  lwip_config.address = app_config.network.address;
  lwip_config.netmask = app_config.network.netmask;
  lwip_config.gateway = app_config.network.gateway;
  lwip_config.addrMode = app_config.network.address_mode;

  lwipInit(&lwip_config);
}

void config_network_reload(void)
{
  lwipreconf_opts_t lwip_newconfig;

  lwip_newconfig.address = app_config.network.address;
  lwip_newconfig.netmask = app_config.network.netmask;
  lwip_newconfig.gateway = app_config.network.gateway;
  lwip_newconfig.addrMode = app_config.network.address_mode;

  lwipReconfigure(&lwip_newconfig);
}