#include "../main.h"

#include "lwip/api.h"
#include "lwip/netif.h"

#include <string.h>
#include "chprintf.h"

#define HTTP_RESPONSE_MAXLENGTH   16384
CC_ALIGN_DATA(32) static char http_response[HTTP_RESPONSE_MAXLENGTH];

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

/* CSS */
//static const char http_css_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: text/css\r\n\r\n";
static const char http_css_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: text/css\r\n\r\n";

#include "htdist/bootstrap_5_1_3_min_css_gz.h"
#include "htdist/dygraph_2_1_0_min_css_gz.h"

/* Javascript */
//static const char http_javascript_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: application/javascript\r\n\r\n";
static const char http_javascript_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: application/javascript\r\n\r\n";

#include "htdist/index_js_gz.h"
#include "htdist/bootstrap_5_1_3_min_js_gz.h"
#include "htdist/mithril_2_0_4_min_js_gz.h"
#include "htdist/dygraph_2_1_0_min_js_gz.h"

/* JSON API */
static const char http_json_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n";

/* PNG Image */
static const char http_png_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: image/png\r\n\r\n";
#include "htdist/favicon_128_png_gz.h"

/* Binary Files */
//static const char http_binary_hdr[] = "HTTP/1.0 200 OK\r\nContent-Type:application/octet-stream\r\n\r\n";
//static const char http_binary_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-Type:application/octet-stream\r\n\r\n";

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

extern contacts_t hub_contacts;
static void web_path_api_contacts(struct netconn *conn)
{
  int32_t str_ptr;
  uint32_t i;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  str_ptr = chsnprintf(http_response, HTTP_RESPONSE_MAXLENGTH,
    "{\"contacts\": ["
  );

  for(i = 0; i < hub_contacts.count; i++)
  {
    str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
      "{\"name\": \"%s\", \"value\": %d}",
      hub_contacts.contact[i].name, hub_contacts.contact[i].value
    );
  }

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    "]}"
  );

  netconn_write(conn, http_response, str_ptr, NETCONN_NOFLAG);
}

extern environmentals_t hub_environmentals;
static void web_path_api_environmentals(struct netconn *conn)
{
  int32_t str_ptr;
  uint32_t i;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  str_ptr = chsnprintf(http_response, HTTP_RESPONSE_MAXLENGTH,
    "{\"environmentals\": ["
  );

  for(i = 0; i < hub_environmentals.count; i++)
  {
    str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
      "{\"name\": \"%s\", \"temperature\": %.2f, \"humidity\": %.2f}",
      hub_environmentals.environmental[i].name, hub_environmentals.environmental[i].temperature, hub_environmentals.environmental[i].humidity
    );
  }

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    "]}"
  );

  netconn_write(conn, http_response, str_ptr, NETCONN_NOFLAG);
}

extern scd30_t hub_scd30;
static void web_path_api_scd30(struct netconn *conn)
{
  int32_t str_ptr;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  str_ptr = chsnprintf(http_response, HTTP_RESPONSE_MAXLENGTH,
    "{\"valid\": "
  );

  if(hub_scd30.valid == true)
  {
    str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
      "true"
    );
  }
  else
  {
    str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
      "false"
    );
  }

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"instant_co2_ppm\": %.1f",
    hub_scd30.instant_co2_ppm
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"instant_temperature_c\": %.1f",
    hub_scd30.instant_temperature_c
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"instant_humidity_rh\": %.1f",
    hub_scd30.instant_humidity_rh
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"avg_co2_ppm\": %.1f",
    hub_scd30.avg_co2_ppm
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"avg_temperature_c\": %.1f",
    hub_scd30.avg_temperature_c
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"avg_humidity_rh\": %.1f",
    hub_scd30.avg_humidity_rh
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"fwver\": "
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    "\"%03d.%03d\"",
    hub_scd30.fwver[0], hub_scd30.fwver[1]
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    "}"
  );

  netconn_write(conn, http_response, str_ptr, NETCONN_NOFLAG);
}

extern scd30_history_t hub_scd30_history;
static void web_path_api_scd30_history_co2(struct netconn *conn)
{
  int32_t str_ptr;
  int32_t j;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  str_ptr = chsnprintf(http_response, HTTP_RESPONSE_MAXLENGTH,
    "{\"interval_seconds\": 30, \"data\": ["
  );

  j = 0;
  if(hub_scd30_history.started == true)
  {
    hub_scd30_history.copy_lock = true;

    if(hub_scd30_history.rolled == true)
    {
      /* From tail to end of array */
      for(int32_t i = (hub_scd30_history.head+1); i < SCD30_HISTORY_ROW_COUNT; i++)
      {
        str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
          "%.1f,",
          hub_scd30_history.rows[i].co2_ppm
        );
        j++;
      }
    }
    /* From start to head */
    for(int32_t i = 0; i < hub_scd30_history.head; i++)
    {
      str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
        "%.1f,",
        hub_scd30_history.rows[i].co2_ppm
      );
      j++;
    }

    hub_scd30_history.copy_lock = false;

    if(http_response[str_ptr-1] == ',')
    {
      str_ptr--;
    }
  }

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    "]"
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    ", \"length\": %d",
    j
  );

  str_ptr += chsnprintf(&http_response[str_ptr], (HTTP_RESPONSE_MAXLENGTH - str_ptr),
    "}"
  );

  netconn_write(conn, http_response, str_ptr, NETCONN_NOFLAG);
}
typedef struct {
    float vbat_volts;
    float temperature_degrees;
    char *firmware_name;
    char *firmware_version;
} mcu_t;

extern mcu_info_t mcu_info;
static void web_path_api_mcu_info(struct netconn *conn)
{
  int32_t str_ptr;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  str_ptr = chsnprintf(http_response, HTTP_RESPONSE_MAXLENGTH,
    "{\"firmware_name\": \"%s\",\"firmware_version\": \"%s\",\"vbat_volts\": %.2f,\"temperature_degrees\": %.1f}",
    mcu_info.firmware_name,
    mcu_info.firmware_version,
    mcu_info.vbat_volts,
    mcu_info.temperature_degrees
  );

  netconn_write(conn, http_response, str_ptr, NETCONN_NOFLAG);
}

void web_paths_get(struct netconn *conn, char *url_buffer)
{
  if(strcmp("/", url_buffer) == 0 || strcmp("/index.html", url_buffer) == 0)
  {
    web_path_index_html(conn);
  }
  else if(strcmp("/api/contacts", url_buffer) == 0)
  {
    web_path_api_contacts(conn);
  }
  else if(strcmp("/api/environmentals", url_buffer) == 0)
  {
    web_path_api_environmentals(conn);
  }
  else if(strcmp("/api/scd30", url_buffer) == 0)
  {
    web_path_api_scd30(conn);
  }
  else if(strcmp("/api/scd30_history_co2", url_buffer) == 0)
  {
    web_path_api_scd30_history_co2(conn);
  }
  else if(strcmp("/api/mcu_info", url_buffer) == 0)
  {
    web_path_api_mcu_info(conn);
  }
  else if(strcmp("/index.js", url_buffer) == 0)
  {
    web_path_index_js(conn);
  }
  else if(strcmp("/mithril_2.0.4.min.js", url_buffer) == 0)
  {
    netconn_write(conn, http_javascript_gz_hdr, sizeof(http_javascript_gz_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, mithril_2_0_4_min_js_gz, mithril_2_0_4_min_js_gz_len, NETCONN_NOCOPY);
  }
  else if(strcmp("/dygraph_2.1.0.min.js", url_buffer) == 0)
  {
    netconn_write(conn, http_javascript_gz_hdr, sizeof(http_javascript_gz_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, dygraph_2_1_0_min_js_gz, dygraph_2_1_0_min_js_gz_len, NETCONN_NOCOPY);
  }
  else if(strcmp("/dygraph_2.1.0.min.css", url_buffer) == 0)
  {
    netconn_write(conn, http_css_gz_hdr, sizeof(http_css_gz_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, dygraph_2_1_0_min_css_gz, dygraph_2_1_0_min_css_gz_len, NETCONN_NOCOPY);
  }
  else if(strcmp("/bootstrap_5.1.3.min.js", url_buffer) == 0)
  {
    netconn_write(conn, http_javascript_gz_hdr, sizeof(http_javascript_gz_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, bootstrap_5_1_3_min_js_gz, bootstrap_5_1_3_min_js_gz_len, NETCONN_NOCOPY);
  }
  else if(strcmp("/bootstrap_5.1.3.min.css", url_buffer) == 0)
  {
    netconn_write(conn, http_css_gz_hdr, sizeof(http_css_gz_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, bootstrap_5_1_3_min_css_gz, bootstrap_5_1_3_min_css_gz_len, NETCONN_NOCOPY);
  }
  else if(strcmp("/favicon_128.png", url_buffer) == 0)
  {
    netconn_write(conn, http_png_gz_hdr, sizeof(http_png_gz_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, favicon_128_png_gz, favicon_128_png_gz_len, NETCONN_NOCOPY);
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