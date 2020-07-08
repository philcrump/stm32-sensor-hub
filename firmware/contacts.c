#include "main.h"

#define CONTACTS_COUNT    1

contact_t contacts_array[CONTACTS_COUNT] = {
  /* PC7 */
  {
    .name = "CH1",
    .value = false,
    .line = LINE_ZIO_D21
  }
};
contacts_t hub_contacts = {
  .contact = contacts_array,
  .count = CONTACTS_COUNT
};

THD_FUNCTION(contacts_service_thread, arg)
{
  (void)arg;

  uint32_t i;

  while(1)
  {
    for(i = 0; i < hub_contacts.count; i++)
    {
      hub_contacts.contact[i].value = !palReadLine(hub_contacts.contact[i].line);
    }

    chThdSleepMilliseconds(100);
  }
}