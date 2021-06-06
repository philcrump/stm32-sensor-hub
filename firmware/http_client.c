#include "main.h"

#include "lwip/api.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/dhcp.h"

#include <string.h>
#include "chprintf.h"

/*
  eg.
    netif_ptr = netif_find("ms0");
    request_string = "GET /status.xml HTTP/1.0\r\n\r\n";
    request_string_length = sizeof(request_string)
*/

bool http_client_request(struct netif *netif_ptr, char *request_string, uint32_t request_string_length, char *response_buffer, uint32_t response_buffer_size)
{

  if(netif_ptr == NULL
    || !netif_is_link_up(netif_ptr)
    || (
      netif_dhcp_data(netif_ptr) != NULL
      && netif_dhcp_data(netif_ptr)->state != 10
      )
    )
  {
    /* Interface not connected or up */
    return false;
  }
  
  ip_addr_t *local_ip_ptr;
  err_t err;
  struct netconn *conn;
  ip_addr_t remote_ip;

  struct netbuf *inbuf;
  uint32_t packetlen;

  local_ip_ptr = (ip_addr_t *)netif_ip4_addr(netif_ptr);

  /* Create connection */
  conn = netconn_new(NETCONN_TCP);
  if(conn == NULL)
  {
    debugPrintf("[HTTP Client] netconn_new error\r\n");
    return false;
  }

  err = netconn_bind(conn, local_ip_ptr, 0);
  if(err != ERR_OK)
  {
    netconn_delete(conn);
    debugPrintf("[HTTP Client] bind error: %+d\r\n", err);
    return false;
  }

  remote_ip.addr = IP4_ADDR_VALUE(10,6,1,82);
  err = netconn_connect(conn, &remote_ip, 80);

  if(err != ERR_OK)
  {
    netconn_close(conn);
    netconn_delete(conn);
    debugPrintf("[HTTP Client] connect error: %+d\r\n", err);
    return false;
  }

  /* Send Request */
  err = netconn_write(conn, request_string, request_string_length, NETCONN_COPY);

  if(err != ERR_OK)
  {
    netconn_close(conn);
    netconn_delete(conn);
    debugPrintf("[HTTP Client] send error: %+d\r\n", err);
    return false;
  }

  err = netconn_recv(conn, &inbuf);

  netconn_close(conn);
  netconn_delete(conn);

  if(err != ERR_OK)
  {
    if(inbuf != NULL) netbuf_delete(inbuf);
    debugPrintf("[HTTP Client] receive error: %+d\r\n", err);
    return false;
  }

  packetlen = netbuf_len(inbuf);

  /* Check we've got room for the packet */
  if(packetlen >= response_buffer_size)
  {
    /* We haven't, fail */
    netbuf_delete(inbuf);
    return false;
  }

  netbuf_copy(inbuf, response_buffer, packetlen);
  response_buffer[packetlen] = '\0';

  netbuf_delete(inbuf);

  return true;
}
