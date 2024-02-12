#ifndef __MAIN_ROUTES_HEADER
#define __MAIN_ROUTES_HEADER

#include <esp_http_server.h>

esp_err_t handle_ping(httpd_req_t *r);
esp_err_t handle_webpage(httpd_req_t *r);

#endif
