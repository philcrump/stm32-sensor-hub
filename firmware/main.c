#include "main.h"

#include <math.h>

#include "lwipthread.h"

#include <string.h>
#include "chprintf.h"

static THD_WORKING_AREA(watchdog_service_wa, 128);
static THD_WORKING_AREA(contacts_service_wa, 128);
static THD_WORKING_AREA(environmentals_service_wa, 128);
static THD_WORKING_AREA(usbshell_service_wa, 128);

static uint8_t _macAddress[] = {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x47};
static lwipthread_opts_t lwip_opts = {
  .macaddress = _macAddress,
  .address = 0,
  .netmask = 0,
  .gateway = 0,
  .addrMode = NET_ADDRESS_DHCP,
  .ourHostName = "sensor-hub",
  .link_up_cb = ip_link_up_cb,
  .link_down_cb = ip_link_down_cb
};

#define debugWriteStr(s)  sdWrite(&SD3, (uint8_t *)s, sizeof(s))
#define debugPrintf(...) chprintf((BaseSequentialStream*)&SD3, __VA_ARGS__)

#define FLASH_CONFIG_BASE_ADDR  0x08008000

#define FLASH_KEY1   0x45670123
#define FLASH_KEY2   0xCDEF89AB

static THD_WORKING_AREA(blinker_wa, 128);
static THD_FUNCTION(blinker_thread, arg)
{
  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetLine(LINE_LED1);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_LED2);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_LED3);
    chThdSleepMilliseconds(100);
    palClearLine(LINE_LED1);
    chThdSleepMilliseconds(50);
    palClearLine(LINE_LED2);
    chThdSleepMilliseconds(50);
    palClearLine(LINE_LED3);
    chThdSleepMilliseconds(100);
  }
}

typedef struct {
  uint32_t        address;
  uint32_t        netmask;
  uint32_t        gateway;
  /**
   * @brief   Startup network addressing mode - static, DHCP, auto.
   */
  net_addr_mode_t addrMode; // static, DHCP, auto.
  char hostname[64];
} app_config_inner_t;

typedef struct {
  app_config_inner_t params;
  uint32_t _padding;
  uint32_t crc;
} app_config_t;

app_config_t app_config;

uint32_t hw_crc32(uint32_t *p, uint32_t wlen)
{
  CRC->CR = CRC_CR_RESET; // Set defaults and reset
  while(wlen--)
  {
    CRC->DR = *p++;
  }
  return CRC->DR;
}

int main(void)
{
  halInit();
  chSysInit();

  /* For some really weird reason the ethernet peripheral requires the PLL SAI clock to be on (both STM32F4 & STM32F7) */
  // Enable PLLSAI
  RCC->CR |= RCC_CR_PLLSAION;
  // Wait for PLLSAI to lock
  while(!(RCC->CR & RCC_CR_PLLSAIRDY));

  /* Enable CRC clock */
  rccEnableCRC(0);

  chThdCreateStatic(blinker_wa, sizeof(blinker_wa), NORMALPRIO + 1, blinker_thread, NULL);

  /* Set up debug serial (115200 on ttyACM0 [ST-Link]) */
  sdStart(&SD3, NULL);

  /* Load config from flash */
  memcpy(&app_config, (uint32_t *)FLASH_CONFIG_BASE_ADDR, sizeof(app_config_t));
  debugWriteStr("Config load:\r\n");
  debugPrintf("a = 0x%08X\r\n", app_config.params.address);
  debugPrintf("b = 0x%08X\r\n", app_config.params.netmask);
  debugPrintf("c = 0x%08X\r\n", app_config.params.gateway);
  debugPrintf("mode = %s\r\n", app_config.params.addrMode == NET_ADDRESS_DHCP ? "DHCP" : "Static");
  debugPrintf("CRC = 0x%08X\r\n", app_config.crc);

  uint32_t calc_crc = hw_crc32((uint32_t *)&(app_config.params), (sizeof(app_config_inner_t) >> 2) + 1);
  debugPrintf("Calc CRC = 0x%08X\r\n", calc_crc);

  if(calc_crc == app_config.crc)
  {
    debugWriteStr("CRC FAIL - loading defaults..\r\n");

    app_config.params.address = IP4_ADDR_VALUE(192,168,100,51);
    app_config.params.netmask = IP4_ADDR_VALUE(255,255,255,0);
    app_config.params.gateway = IP4_ADDR_VALUE(192,168,100,1);
    app_config.params.addrMode = NET_ADDRESS_STATIC;
    app_config.crc = hw_crc32((uint32_t *)&(app_config.params), (sizeof(app_config_inner_t) >> 2) + 1);

    debugWriteStr("Writing defaults to flash..\r\n");
  }
  else
  {
    debugWriteStr("CRC Pass\r\n");
  }

  /* Unlock Flash if not already unlocked */
  if((FLASH->CR & FLASH_CR_LOCK) != 0U)
  {
    FLASH->KEYR |= FLASH_KEY1;
    FLASH->KEYR |= FLASH_KEY2;

    if((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
      debugWriteStr(" - ERROR: Flash is still locked!\r\n");
     }
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
  if((FLASH->CR & FLASH_CR_LOCK) == 0U)
  {
    debugWriteStr(" - ERROR: Flash is still unlocked!\r\n");
  }

  lwip_opts.address = app_config.params.address;
  lwip_opts.netmask = app_config.params.netmask;
  lwip_opts.gateway = app_config.params.gateway;

  /* Set up watchdog */
  watchdog_init();
  chThdCreateStatic(watchdog_service_wa, sizeof(watchdog_service_wa), HIGHPRIO, watchdog_service_thread, NULL);

  /* Set up IP stack */
  lwipInit(&lwip_opts);

  web_init();

  chThdCreateStatic(contacts_service_wa, sizeof(contacts_service_wa), NORMALPRIO, contacts_service_thread, NULL);
  chThdCreateStatic(environmentals_service_wa, sizeof(environmentals_service_wa), NORMALPRIO, environmentals_service_thread, NULL);

  chThdCreateStatic(usbshell_service_wa, sizeof(usbshell_service_wa), NORMALPRIO, usbshell_service_thread, NULL);

  while(true)
  {
    watchdog_feed(WATCHDOG_DOG_MAIN);
    chThdSleepMilliseconds(100);
  }
}


/* On hard fault, copy HARDFAULT_PSP to the sp reg so gdb can give a trace */
void **HARDFAULT_PSP;
register void *stack_pointer asm("sp");
void HardFault_Handler(void) {
    asm("mrs %0, psp" : "=r"(HARDFAULT_PSP) : :);
    stack_pointer = HARDFAULT_PSP;
    while(1);
}