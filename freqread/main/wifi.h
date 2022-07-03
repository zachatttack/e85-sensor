#ifndef WIFI_H
#define WIFI_H
#include "lwip/err.h"

/** DEFINES **/
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

void wifi_init(void);
esp_err_t connect_tcp_server(void);
#endif // !WIFI_H
