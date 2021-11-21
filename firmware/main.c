#include "main.h"

#include <math.h>

#include <string.h>
#include "chprintf.h"

static THD_WORKING_AREA(watchdog_service_wa, 128);
static THD_WORKING_AREA(contacts_service_wa, 128);
static THD_WORKING_AREA(environmentals_service_wa, 128);
static THD_WORKING_AREA(scd30_service_wa, 128);
static THD_WORKING_AREA(usbshell_service_wa, 128);
static THD_WORKING_AREA(mcu_service_wa, 128);

static THD_WORKING_AREA(blinker_wa, 128);
static THD_FUNCTION(blinker_thread, arg)
{
  (void)arg;
  chRegSetThreadName("led blinker");
  bool heartbeat_flip = false;
  while (true)
  {
    /* Green LED is heartbeat */
    if((heartbeat_flip = !heartbeat_flip))
    {
      palToggleLine(LINE_LED_GREEN);
    }

    /* Yellow LED shows IP Link Status */
    app_ip_link_status_update();
    if(app_ip_link_status == APP_IP_LINK_STATUS_DOWN)
    {
      palClearLine(LINE_LED_YELLOW);
    }
    else if(app_ip_link_status == APP_IP_LINK_STATUS_UPBUTNOIP)
    {
      palToggleLine(LINE_LED_YELLOW);
    }
    else if(app_ip_link_status == APP_IP_LINK_STATUS_BOUND)
    {
      palSetLine(LINE_LED_YELLOW);
    }

    chThdSleepMilliseconds(100);
  }
}

static int32_t reset_hold_count = 100; // *50ms
int main(void)
{
  halInit();
  chSysInit();

    /* Set up watchdog */
  watchdog_init();
  chThdCreateStatic(watchdog_service_wa, sizeof(watchdog_service_wa), HIGHPRIO, watchdog_service_thread, NULL);

  /* Enable CRC clock */
  rccEnableCRC(0);

  chThdCreateStatic(blinker_wa, sizeof(blinker_wa), NORMALPRIO + 1, blinker_thread, NULL);

  /* Set up debug serial (115200 on ttyACM0 [ST-Link]) */
  sdStart(&SD3, NULL);

  debugWriteStr("==================================\r\n");
  debugWriteStr("Sensor Hub booting..\r\n");

  /* Hold USER down for 5 seconds on boot to spin this loop and reset all config */
  if(palReadLine(LINE_BUTTON))
  {
    debugWriteStr("Button is pressed, hold for 5 seconds to reset config:\r\n");
    while(palReadLine(LINE_BUTTON))
    {
      palToggleLine(LINE_LED3);
      chThdSleepMilliseconds(50);
      if(--reset_hold_count <= 0)
      {
        debugWriteStr("\r\nButton was held! Will reset config!\r\n");
        config_setdefaults();
        break;
      }
      debugWriteChar('.');
    }
  }
  
  debugWriteStr("Loading config..\r\n");
  config_load();

  /* Set up IP stack */
  debugWriteStr("Initialising IP stack..\r\n");
  config_network_load_and_start();

  debugWriteStr("Initialising HTTP server..\r\n");
  web_init();

  debugWriteStr("Initialising device interface threads..\r\n");
  chThdCreateStatic(contacts_service_wa, sizeof(contacts_service_wa), NORMALPRIO, contacts_service_thread, NULL);
  chThdCreateStatic(environmentals_service_wa, sizeof(environmentals_service_wa), NORMALPRIO, environmentals_service_thread, NULL);
  chThdCreateStatic(scd30_service_wa, sizeof(scd30_service_wa), NORMALPRIO, scd30_service_thread, NULL);
  chThdCreateStatic(mcu_service_wa, sizeof(mcu_service_wa), NORMALPRIO, mcu_service_thread, NULL);

  debugWriteStr("Initialising USB configuration shell..\r\n");
  chThdCreateStatic(usbshell_service_wa, sizeof(usbshell_service_wa), NORMALPRIO, usbshell_service_thread, NULL);

  debugWriteStr("Initialisation complete.\r\n");

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