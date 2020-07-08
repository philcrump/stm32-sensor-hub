#include "../main.h"

#include "lwip/api.h"
#include "lwip/netif.h"

#include <string.h>
#include "chprintf.h"

static char http_response[4096];

static const char http_robots_txt_hdr[] = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n";
static const char http_robots_txt_body[] = "User-agent: *\r\nDisallow: /";
static void web_path_robots_txt(struct netconn *conn);

/* 403 - Forbidden */
//static const char http_403_json_hdr[] = "HTTP/1.0 HTTP/1.0 403 Forbidden\r\nContent-type: application/javascript\r\n\r\n";
//static const char http_403_hdr[] = "HTTP/1.0 HTTP/1.0 403 Forbidden\r\nContent-type: text/html\r\n\r\n";
//static const char http_403_body[] = "<html><body><h4>Forbidden</h4></body></html>";

/* 404 - File Not Found */
static const char http_404_hdr[] = "HTTP/1.0 404 Not Found\r\nContent-type: text/html\r\n\r\n";
static const char http_404_body[] = "<html><body><h4>Path not found</h4></body></html>";
static void web_path_404(struct netconn *conn);

/* HTML */
//static const char http_html_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n";
static const char http_html_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: text/html\r\n\r\n";
#include "htdist/index_html_gz.h"
static void web_path_index_html(struct netconn *conn);

/* CSS */
//static const char http_css_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: text/css\r\n\r\n";
//static const char http_css_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: text/css\r\n\r\n";

/* Javascript */
//static const char http_javascript_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: application/javascript\r\n\r\n";
static const char http_javascript_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: application/javascript\r\n\r\n";
#include "htdist/index_js_gz.h"
static void web_path_index_js(struct netconn *conn);
#include "htdist/mithril_min_js_gz.h"
static void web_path_mithril_min_js(struct netconn *conn);

/* JSON API */
static const char http_json_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n";
static void web_path_api_status(struct netconn *conn);

/* PNG Image */
//static const char http_png_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: image/png\r\n\r\n";
//#include "dist/favicon_png_gz.h"
//static void web_path_favicon_png(struct netconn *conn);

/* Binary Files */
//static const char http_binary_hdr[] = "HTTP/1.0 200 OK\r\nContent-Type:application/octet-stream\r\n\r\n";
//static const char http_binary_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-Type:application/octet-stream\r\n\r\n";

void web_paths_get(struct netconn *conn, char *url_buffer)
{
  if(strcmp("/", url_buffer) == 0 || strcmp("/index.html", url_buffer) == 0)
  {
    web_path_index_html(conn);
  }
  else if(strcmp("/index.js", url_buffer) == 0)
  {
    web_path_index_js(conn);
  }
  else if(strcmp("/mithril.min.js", url_buffer) == 0)
  {
    web_path_mithril_min_js(conn);
  }
  else if(strcmp("/api/status", url_buffer) == 0)
  {
    web_path_api_status(conn);
  }
  else if(strcmp("/robots.txt", url_buffer) == 0)
  {
    web_path_robots_txt(conn);
  }
  else
  {
    web_path_404(conn);
  }
}

static void web_path_404(struct netconn *conn)
{
  netconn_write(conn, http_404_hdr, sizeof(http_404_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, http_404_body, sizeof(http_404_body), NETCONN_NOCOPY);
}

static void web_path_index_html(struct netconn *conn)
{
  netconn_write(conn, http_html_gz_hdr, sizeof(http_html_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, index_html_gz, index_html_gz_len, NETCONN_NOCOPY);
}

static void web_path_robots_txt(struct netconn *conn)
{
  netconn_write(conn, http_robots_txt_hdr, sizeof(http_robots_txt_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, http_robots_txt_body, sizeof(http_robots_txt_body)-1, NETCONN_NOCOPY);
}

static void web_path_index_js(struct netconn *conn)
{
  netconn_write(conn, http_javascript_gz_hdr, sizeof(http_javascript_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, index_js_gz, index_js_gz_len, NETCONN_NOCOPY);
}

static void web_path_mithril_min_js(struct netconn *conn)
{
  netconn_write(conn, http_javascript_gz_hdr, sizeof(http_javascript_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, mithril_min_js_gz, mithril_min_js_gz_len, NETCONN_NOCOPY);
}

extern contacts_t hub_contacts;
extern environmentals_t hub_environmentals;
static void web_path_api_status(struct netconn *conn)
{
  int str_ptr;
  uint32_t i;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  str_ptr = chsnprintf(http_response, 4096,
    "{"
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (4096 - str_ptr),
    "\"contacts\": ["
  );

  for(i = 0; i < hub_contacts.count; i++)
  {
    str_ptr += chsnprintf(&http_response[str_ptr], (4096 - str_ptr),
      "{\"name\": \"%s\", \"value\": %d}",
      hub_contacts.contact[i].name, hub_contacts.contact[i].value
    );
  }
  str_ptr += chsnprintf(&http_response[str_ptr], (4096 - str_ptr),
    "], \"environmentals\": ["
  );

  for(i = 0; i < hub_environmentals.count; i++)
  {
    str_ptr += chsnprintf(&http_response[str_ptr], (4096 - str_ptr),
      "{\"name\": \"%s\", \"temperature\": %.2f, \"humidity\": %.2f}",
      hub_environmentals.environmental[i].name, hub_environmentals.environmental[i].temperature, hub_environmentals.environmental[i].humidity
    );
  }

  str_ptr += chsnprintf(&http_response[str_ptr], (4096 - str_ptr),
    "]}"
  );

  netconn_write(conn, http_response, str_ptr, NETCONN_NOFLAG);
}
