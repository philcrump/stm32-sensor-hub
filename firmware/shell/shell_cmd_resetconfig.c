#include "../main.h"

#include <string.h>

#include "shell.h"
#include "chprintf.h"

extern void watchdog_reconfigSlow(void);
extern void watchdog_reconfigDefault(void);

static char line[SHELL_MAX_LINE_LENGTH];

void shell_cmd_resetconfig(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Are you sure you want to reset all configuration (yes/no)? ");

  if(shellAppGetLine(chp, line, sizeof(line)))
  {
    /* Exit */
    return;
  }
  else if(strcmp("yes", line) != 0)
  {
    chprintf(chp, " - Aborted." SHELL_NEWLINE_STR);
    /* Exit */
    return;
  }

  chprintf(chp, " - Resetting configuration..." SHELL_NEWLINE_STR);
  config_setdefaults();
  chprintf(chp, " - Reloading network interface.." SHELL_NEWLINE_STR);
  config_network_reload();
  chprintf(chp, " - Configuration reset to defaults!" SHELL_NEWLINE_STR);
}