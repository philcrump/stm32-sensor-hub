#include "main.h"

#include "lwip/dhcp.h"

static struct netif* _ip_netif_ptr = NULL;

uint32_t app_ip_link_status(void)
{
  if(_ip_netif_ptr == NULL)
  {
    _ip_netif_ptr = netif_find("ms0");
  }
  if(_ip_netif_ptr != NULL && netif_is_link_up(_ip_netif_ptr))
  {
    if(netif_dhcp_data(_ip_netif_ptr) == NULL)
    {
      return APP_IP_LINK_STATUS_BOUND;
    }
    else
    {
      if(netif_dhcp_data(_ip_netif_ptr)->state == 10) // DHCP_STATE_BOUND = 10
      {
        return APP_IP_LINK_STATUS_BOUND;
      }
      else
      {
        return APP_IP_LINK_STATUS_UPBUTNOIP;
      }
    }
  }
  else
  {
    return APP_IP_LINK_STATUS_DOWN;
  }
}