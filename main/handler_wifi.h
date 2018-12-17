//
// Created by rick on 12/15/18.
//

#ifndef ZELDY_HANDLER_WIFI_H
#define ZELDY_HANDLER_WIFI_H

#include "freertos/FreeRTOS.h"
#include <freertos/event_groups.h>
#include <esp_event.h>
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int WIFI_CONNECTED_BIT = BIT0;

/**
 * Alloc wifi drivers and open wifi connection with hotspot
 */
void init_wifi();


static esp_err_t _wifi_event_handler(void *ctx, system_event_t *event);

/**
 * Get wait for connection to be established will block task while not true
 */
void wifi_wait_connected();

#endif //ZELDY_HANDLER_WIFI_H
