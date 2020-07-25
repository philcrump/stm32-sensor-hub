#ifndef WEB_PATHS_H
#define WEB_PATHS_H

void web_paths_get(void (*request_write)(void *, const char *, uint32_t), void *v_conn, char *url_buffer);

#endif /* WEB_PATHS_H */