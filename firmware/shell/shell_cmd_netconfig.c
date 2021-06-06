#include "../main.h"

#include <string.h>

#include "shell.h"
#include "chprintf.h"

static bool config_param_ready;
static app_config_network_t config_network_new;
static char line[SHELL_MAX_LINE_LENGTH];

void shell_cmd_netconfig(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Press enter to submit. Submit 'q' or press Ctrl+D to abort." SHELL_NEWLINE_STR);

  config_param_ready = false;
  while(!config_param_ready)
  {
    chprintf(chp, " - Network Configuration (dhcp/static)? ");
    if(shellAppGetLine(chp, line, sizeof(line)))
    {
      /* Exit */
      return;
    }
    else if(line[1] == '\0' && strcmp("q", line) == 0)
    {
      /* Exit */
      return;
    }
    else if(strcmp("dhcp", line) == 0)
    {
      config_network_new.address_mode = NET_ADDRESS_DHCP;
      config_param_ready = true;
    }
    else if(strcmp("static", line) == 0)
    {
      config_network_new.address_mode = NET_ADDRESS_STATIC;
      config_param_ready = true;
    }
  }
  

  if(config_network_new.address_mode == NET_ADDRESS_DHCP)
  {
    config_param_ready = false;
    while(!config_param_ready)
    {
      chprintf(chp, " - DHCP Hostname? ");
      if(shellAppGetLine(chp, line, sizeof(line)))
      {
        /* Exit */
        return;
      }
      else if(line[1] == '\0' && strcmp("q", line) == 0)
      {
        /* Exit */
        return;
      }
      int32_t line_strlen = strlen(line);
      if(line_strlen < 1)
      {
        continue;
      }
      else if(line_strlen > 63)
      {
        chprintf(chp, "Error: Cannot be more than 63 characters in length!" SHELL_NEWLINE_STR);
        continue;
      }
      else if(line[0] == '-')
      {
        chprintf(chp, "Error: Cannot start with a hyphen!" SHELL_NEWLINE_STR);
        continue;
      }
      else if(line[line_strlen-1] == '-')
      {
        chprintf(chp, "Error: Cannot end with a hyphen!" SHELL_NEWLINE_STR);
        continue;
      }
      for(int32_t i = 0; i < line_strlen; i++)
      {
        if(!(
             line[i] == '-'
          || (line[i] >= 48 && line[i] <= 57) /* 0-9 */
          || (line[i] >= 65 && line[i] <= 90) /* A-Z */
          || (line[i] >= 97 && line[i] <= 122) /* a-z */
        ))
        {
          chprintf(chp, "Error: Invalid character at index [%d]!" SHELL_NEWLINE_STR, i);
          chprintf(chp, "       Valid characters are: -,[0-9],[a-Z]" SHELL_NEWLINE_STR);
          continue;
        }
      }

      strncpy(config_network_new.hostname, line, 63);
      config_param_ready = true;
    }
  }
  else if(app_config.network.address_mode == NET_ADDRESS_STATIC)
  {
    /* IP address */
    config_param_ready = false;
    while(!config_param_ready)
    {
      chprintf(chp, " - Device IP Address (v4)? ");
      if(shellAppGetLine(chp, line, sizeof(line)))
      {
        /* Exit */
        return;
      }
      else if(line[1] == '\0' && strcmp("q", line) == 0)
      {
        /* Exit */
        return;
      }
      int32_t line_strlen = strlen(line);
      if(line_strlen < 4)
      {
        continue;
      }
      else if(line_strlen > 15)
      {
        continue;
      }

      config_network_new.address = ipaddr_addr(line);
      if(config_network_new.address == IPADDR_NONE)
      {
        chprintf(chp, "Error: Invalid Address!" SHELL_NEWLINE_STR);
        continue;
      }

      config_param_ready = true;
    }

    /* Netmask */
    config_param_ready = false;
    while(!config_param_ready)
    {
      chprintf(chp, " - Netmask? ");
      if(shellAppGetLine(chp, line, sizeof(line)))
      {
        /* Exit */
        return;
      }
      else if(line[1] == '\0' && strcmp("q", line) == 0)
      {
        /* Exit */
        return;
      }
      int32_t line_strlen = strlen(line);
      if(line_strlen < 4)
      {
        continue;
      }
      else if(line_strlen > 15)
      {
        continue;
      }

      config_network_new.netmask = ipaddr_addr(line);
      if(config_network_new.netmask == IPADDR_NONE
        || ip4_addr_netmask_valid(config_network_new.netmask) == 0)
      {
        chprintf(chp, "Error: Invalid Netmask!" SHELL_NEWLINE_STR);
        continue;
      }

      config_param_ready = true;
    }

    /* Gateway */
    config_param_ready = false;
    while(!config_param_ready)
    {
      chprintf(chp, " - Gateway? ");
      if(shellAppGetLine(chp, line, sizeof(line)))
      {
        /* Exit */
        return;
      }
      else if(line[1] == '\0' && strcmp("q", line) == 0)
      {
        /* Exit */
        return;
      }
      int32_t line_strlen = strlen(line);
      if(line_strlen < 4)
      {
        continue;
      }
      else if(line_strlen > 15)
      {
        continue;
      }

      config_network_new.gateway = ipaddr_addr(line);
      if(config_network_new.gateway == IPADDR_NONE)
      {
        chprintf(chp, "Error: Invalid Address!" SHELL_NEWLINE_STR);
        continue;
      }
      else if(!((config_network_new.address & config_network_new.netmask)
                == (config_network_new.gateway & config_network_new.netmask)))
      {
        chprintf(chp, "Error: Gateway is not in network subnet!" SHELL_NEWLINE_STR);
        continue;
      }

      config_param_ready = true;
    }
  }

  chprintf(chp, "Saving new network configuration..." SHELL_NEWLINE_STR);

  config_setnetwork(&config_network_new);

  chprintf(chp, "Reloading network interface.." SHELL_NEWLINE_STR);

  config_network_reload();

  chprintf(chp, "New network configuration saved successfully." SHELL_NEWLINE_STR);
}