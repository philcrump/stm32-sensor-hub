#include <string.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "../config.h"

void shell_cmd_getconfig(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    shellUsage(chp, "getconfig");
    return;
  }

  if(app_config.params.addrMode == NET_ADDRESS_DHCP)
  {
  	chprintf(chp, "Network:   DHCP");
  }
  else if(app_config.params.addrMode == NET_ADDRESS_STATIC)
  {
  	chprintf(chp, "Network:   Static");
  	chprintf(chp, "<address details...>");
  }
  else
  {
  	chprintf(chp, "Network:   [Unknown]");
  }
}