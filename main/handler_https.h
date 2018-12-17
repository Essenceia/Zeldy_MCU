//
// Created by rick on 12/15/18.
//


#ifndef ZELDY_HANDLER_HTTP_H
#define ZELDY_HANDLER_HTTP_H

#include "esp_http_client.h"
#include "handler_wifi.h"


esp_err_t _http_event_handler(esp_http_client_event_t *evt);

void https_post(void *ptr_post_data);


#endif //ZELDY_HANDLER_HTTP_H
