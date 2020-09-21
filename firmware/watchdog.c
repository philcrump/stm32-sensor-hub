#include "main.h"

static const WDGConfig wdg_cfg = {
  // 40 KHz input clock from LSI
  .pr = STM32_IWDG_PR_256, // Prescaler = 256 => 156.25Hz
  .rlr = STM32_IWDG_RL(79), // Reload value = 79 (slightly over 500ms),
  /* STM32F7 WDG is window mode by default */
  .winr = STM32_IWDG_WIN_DISABLED
};

static bool wdg_initialised = false;
static uint32_t mask = WATCHDOG_MASK;
static uint32_t fed = 0;
static bool starve = false;

void watchdog_init(void)
{
  if(wdg_initialised)
  {
    return;
  }

  wdgStart(&WDGD1, &wdg_cfg);
  wdg_initialised = true;
}

THD_FUNCTION(watchdog_service_thread, arg)
{
  (void)arg;

  /* Should have been init-ed by main(), but let's make sure */
  watchdog_init();

  while(1)
  {
    if(!starve && (mask & fed) == mask)
    {
      fed = 0;

      // Feed the hardware dog
      wdgReset(&WDGD1);
    }

    chThdSleepMilliseconds(20);
  }
}

/* To be called by other threads */
void watchdog_feed(uint32_t dog)
{
  fed |= ((1 << dog) & 0xFFFFFFFF);
}

void watchdog_reconfigSlow(void)
{
  wdgReset(&WDGD1);

  WDGD1.wdg->KR   = 0xCCCCU;
  WDGD1.wdg->KR   = 0x5555U;

  /* Write configuration.*/
  WDGD1.wdg->PR   = wdg_cfg.pr;
  WDGD1.wdg->RLR  = STM32_IWDG_RL(1560); // Reload value = 1560 (~10 seconds),

  /* Wait the registers to be updated.*/
  while (WDGD1.wdg->SR != 0) {};

  wdgReset(&WDGD1);
}

void watchdog_reconfigDefault(void)
{
  wdgReset(&WDGD1);

  WDGD1.wdg->KR   = 0xCCCCU;
  WDGD1.wdg->KR   = 0x5555U;

  /* Write configuration.*/
  WDGD1.wdg->PR   = wdg_cfg.pr;
  WDGD1.wdg->RLR  = wdg_cfg.rlr;

  /* Wait the registers to be updated.*/
  while (WDGD1.wdg->SR != 0) {};

  wdgReset(&WDGD1);
}

/* Used for triggering a reboot */
void watchdog_starve(void)
{
  starve = true;
}