#include "main.h"

static bool app_random_initialised = false;

static void app_random_init(void)
{
  if(!app_random_initialised)
  {
    app_random_initialised = true;

    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    RNG->CR |= RNG_CR_RNGEN;
  }
}

uint32_t app_random(uint32_t max)
{
  uint32_t out;

  app_random_init();

  while (!(RNG->SR & (RNG_SR_DRDY)));
  out = RNG->DR;
  
#ifdef RANDOM_FIPS
  uint32_t prev;
  prev = out;
  while(prev == out)
  {
    prev = out;
    while (!(RNG->SR & (RNG_SR_DRDY)));
    out = RNG->DR;
  }
#endif
  
  return out % max;
}
