#include "main.h"

#include <string.h>

#include "chprintf.h"

extern void watchdog_reconfigSlow(void);
extern void watchdog_reconfigDefault(void);

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

  #define FRAM_OPCODE_WREN  0x06 // Set write enable latch
  #define FRAM_OPCODE_WRDI  0x04 // Reset write enable latch

  #define FRAM_OPCODE_RDSR  0x05 // Read Status Register
  #define FRAM_OPCODE_WRSR  0x01 // Write Status Register

  #define FRAM_OPCODE_READ  0x03 // Read memory data
  #define FRAM_OPCODE_FSTRD 0x0b // Fast read memory data
  #define FRAM_OPCODE_WRITE 0x02 // Write memory data

  #define FRAM_OPCODE_SLEEP 0x99 // Enter sleep mode
  #define FRAM_OPCODE_RDID  0x9F // Read device ID
  #define FRAM_OPCODE_SNR   0xc3 // Read S/N

  static uint8_t fram_deviceid[9] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xC2, 0x24, 0x00 };

  bool FRAM_selfTest(void)
  {
    uint8_t fram_deviceid_read[9];

    SPI_burstread8(SPI_FRAM, FRAM_OPCODE_RDID, fram_deviceid_read, 9);

    return !memcmp(fram_deviceid_read, fram_deviceid, 9);
  }

  void FRAM_write(uint32_t address, uint8_t *data, uint32_t length)
  {
    /* Set Write-Enable Latch (Single byte command) */
    spiSend(&FRAM_SPID, 1, (void *)FRAM_OPCODE_WREN);

    //SPI_burstwrite32(SPI_FRAM, ((FRAM_OPCODE_WRITE << 24) | (address & 0xFFFFFF)), data, length);
    spiSend(&FRAM_SPID, length, (void *)data);
  }

  void FRAM_read(uint32_t address, uint8_t *data, uint32_t length)
  {
    SPI_burstread32(SPI_FRAM, ((FRAM_OPCODE_READ << 24) | (address & 0xFFFFFF)), data, length);
  }

  for(uint32_t i = 0; i < sizeof(app_config_t); i++)
  {
    *((uint32_t *)(FLASH_CONFIG_BASE_ADDR) + i) = *(((uint32_t *)(&app_config)) + i);
    asm volatile("dsb; isb"); // Ensure completion of above write
    while ((FLASH->SR & FLASH_SR_BSY) != 0U) { };
  }

#endif
  palClearLine(LINE_LED3);
}

void config_setdefaults(void)
{
  app_config.network.address = IP4_ADDR_VALUE(192,168,0,95);
  app_config.network.netmask = IP4_ADDR_VALUE(255,255,255,0);
  app_config.network.gateway = IP4_ADDR_VALUE(192,168,0,1);
  //app_config.network.address_mode = NETWORK_ADDRESS_STATIC;
  app_config.network.address_mode = NET_ADDRESS_DHCP;
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
  //memcpy(&app_config, (uint32_t *)FLASH_CONFIG_BASE_ADDR, sizeof(app_config_t));

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