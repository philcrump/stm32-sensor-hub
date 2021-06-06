#include "main.h"

#include "lwip/dhcp.h"

uint32_t app_ip_link_status;

static struct netif* _ip_netif_ptr = NULL;

uint32_t app_ip_link_status_update(void)
{
  if(_ip_netif_ptr == NULL)
  {
    _ip_netif_ptr = netif_find("ms0");
  }
  if(_ip_netif_ptr != NULL && netif_is_link_up(_ip_netif_ptr))
  {
    if(netif_dhcp_data(_ip_netif_ptr) == NULL)
    {
      app_ip_link_status = APP_IP_LINK_STATUS_BOUND;
    }
    else
    {
      if(netif_dhcp_data(_ip_netif_ptr)->state == 10) // DHCP_STATE_BOUND = 10
      {
        app_ip_link_status = APP_IP_LINK_STATUS_BOUND;
      }
      else
      {
        app_ip_link_status = APP_IP_LINK_STATUS_UPBUTNOIP;
      }
    }
  }
  else
  {
    app_ip_link_status = APP_IP_LINK_STATUS_DOWN;
  }
  return app_ip_link_status;
}

void app_ip_link_netif_get(struct netif** netif_ptr_ptr)
{
  if(_ip_netif_ptr == NULL)
  {
    _ip_netif_ptr = netif_find("ms0");
  }
  *netif_ptr_ptr = _ip_netif_ptr;
}