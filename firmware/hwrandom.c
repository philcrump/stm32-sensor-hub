#include "main.h"

static bool hwrandom_initialised = false;

static inline void hwrandom_init(void)
{
  if(!hwrandom_initialised)
  {
    hwrandom_initialised = true;

    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    RNG->CR |= RNG_CR_RNGEN;
  }
}

uint32_t hwrandom(uint32_t max)
{
  uint32_t out;

  hwrandom_init();

  while (!(RNG->SR & (RNG_SR_DRDY)));
  out = RNG->DR;
  
#ifdef HWRANDOM_FIPS
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
