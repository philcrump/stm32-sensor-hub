#include "main.h"

#define ENVIRONMENTALS_COUNT    1

environmental_t environmentals_array[ENVIRONMENTALS_COUNT] = {
  {
    .name = "CH2-FAKE",
    .temperature = 0.0,
    .humidity = 0.0,
    /* TODO: Environmental Sensor Drivers Config */
  }
};
environmentals_t hub_environmentals = {
  .environmental = environmentals_array,
  .count = ENVIRONMENTALS_COUNT
};

THD_FUNCTION(environmentals_service_thread, arg)
{
  (void)arg;

  uint32_t i;

  while(1)
  {
    for(i = 0; i < hub_environmentals.count; i++)
    {
      /* TODO: Environmental Sensor Drivers */
      hub_environmentals.environmental[i].temperature = 10.0 + (random(300)/10.0);
      hub_environmentals.environmental[i].humidity = 60.0 + (random(200)/10.0);
    }

    chThdSleepMilliseconds(100);
  }
}