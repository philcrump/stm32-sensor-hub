#include "../main.h"

#include <string.h>

#include "shell.h"
#include "chprintf.h"

extern void watchdog_starve(void);

static char line[SHELL_MAX_LINE_LENGTH];

void shell_cmd_reboot(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Are you sure you want to reboot (yes/no)? ");

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

  chprintf(chp, " - Rebooting!" SHELL_NEWLINE_STR);

  watchdog_starve();
}