/**
 * @file web.c
 * @brief HTTP server wrapper thread code.
 * @addtogroup WEB_THREAD
 * @{
 */

#include "../main.h"

#include "lwip/api.h"
#include "lwip/netif.h"
#include "web_paths.h"

#include <string.h>

#include "wolfssl_chibios.h"


/* cert.c */
extern unsigned char server_crt[];
extern unsigned int server_crt_len;
extern unsigned char server_key[];
extern unsigned int server_key_len;

static char packet_buffer[WEB_MAX_PACKET_SIZE];
static char url_buffer[WEB_MAX_PATH_SIZE];
/**
 * @brief   Decodes an URL sting.
 * @note    The string is terminated by a zero or a separator.
 *
 * @param[in] url       encoded URL string
 * @param[out] buf      buffer for the processed string
 * @param[in] max       max number of chars to copy into the buffer
 * @return              The conversion status.
 * @retval false        string converted.
 * @retval true         the string was not valid or the buffer overflowed
 *
 * @notapi
 */
#define HEXTOI(x) (isdigit(x) ? (x) - '0' : (x) - 'a' + 10)
static bool decode_url(const char *url, char *buf, size_t max)
{
  while (true) {
    int h, l;
    unsigned char c = *url++;

    switch (c) {
    case 0:
    case '\r':
    case '\n':
    case '\t':
    case ' ':
    case '?':
      *buf = 0;
      return true;
    case '.':
      if (max <= 1)
        return false;

      h = *(url + 1);
      if (h == '.')
        return false;

      break;
    case '%':
      if (max <= 1)
        return false;

      h = tolower((int)*url++);
      if (h == 0)
        return false;
      if (!isxdigit(h))
        return false;

      l = tolower((int)*url++);
      if (l == 0)
        return false;
      if (!isxdigit(l))
        return false;

      c = (char)((HEXTOI(h) << 4) | HEXTOI(l));
      break;
    default:
      if (max <= 1)
        return false;

      if (!isalnum(c) && (c != '_') && (c != '-') && (c != '+') &&
          (c != '/'))
        return false;

      break;
    }

    *buf++ = c;
    max--;
  }
}

static void tls_request_write(void *v_conn, const char *ptr, uint32_t length)
{
  wolfSSL_write(((sslconn *)v_conn)->ssl, ptr, length);
}

static void tls_server_serve(sslconn *sc)
{
  uint32_t packetlen;

  packetlen = wolfSSL_read(sc->ssl, packet_buffer, WEB_MAX_PACKET_SIZE);

  /* Is this an HTTP GET command? (only check the first 5 chars, since
  there are other formats for GET, and we're keeping it very simple )*/
  if(packetlen>=5 && (0 == memcmp("GET /", packet_buffer, 5)))
  {
    if(!decode_url(packet_buffer + (4 * sizeof(char)), url_buffer, WEB_MAX_PATH_SIZE))
    {
      /* URL decode failed.*/
      return;
    }

    web_paths_get(tls_request_write, (void *)sc, url_buffer);
  }
}

/**
 * Stack area for the http thread.
 */
THD_WORKING_AREA(wa_http_server, WEB_THREAD_STACK_SIZE);

/**
 * HTTP server thread.
 */
THD_FUNCTION(http_server, p) {
  //struct netconn *conn, *newconn;
  //err_t err;
  sslconn *sslconn, *newsslconn;

  (void)p;
  chRegSetThreadName("http");

  watchdog_feed(WATCHDOG_DOG_WEB_TLS);

  /* Initialize wolfSSL */
  wolfSSL_Init();

  watchdog_feed(WATCHDOG_DOG_WEB_TLS);

  /* Create a new SSL connection handle */
  sslconn = sslconn_new(NETCONN_TCP, wolfTLSv1_2_server_method());
  if (!sslconn) {
      chThdExit(MSG_RESET);
  }

  /* Load certificate file for the HTTPS server */
  if (wolfSSL_CTX_use_certificate_buffer(sslconn->ctx, server_crt,
              server_crt_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
  {
    chThdExit(MSG_RESET);
  }

  /* Load the private key */
  if (wolfSSL_CTX_use_PrivateKey_buffer(sslconn->ctx, server_key, 
              server_key_len, SSL_FILETYPE_ASN1 ) != SSL_SUCCESS)
  {
    chThdExit(MSG_RESET);
  }

  /* Bind to port 443 (HTTPS) with default IP address */
  netconn_bind(sslconn->conn, NULL, WEB_THREAD_PORT);

  /* Put the connection into LISTEN state */
  netconn_listen(sslconn->conn);

  /* Goes to the final priority after initialization.*/
  chThdSetPriority(WEB_THREAD_PRIORITY);

  /* Listening loop */
  while (true)
  {
    watchdog_feed(WATCHDOG_DOG_WEB_TLS);
    newsslconn = sslconn_accept(sslconn);
    if (!newsslconn)
    {
        chThdSleepMilliseconds(5);
        continue;
    }
    watchdog_feed(WATCHDOG_DOG_WEB_TLS);

    /* New connection: a new SSL connector is spawned */
    tls_server_serve(newsslconn);

    sslconn_close(newsslconn);
  }
}

void web_init(void)
{
  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1, http_server, NULL);
}

/** @} */
