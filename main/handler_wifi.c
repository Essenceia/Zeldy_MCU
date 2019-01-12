//
// Created by rick on 12/15/18.
//

#include "handler_wifi.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#define USING_SSL

static const char *TAG = "WIFI";

void init_wifi(){
        wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(_wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = CONFIG_WIFI_SSID,
                    .password = CONFIG_WIFI_PASSWORD,
            },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static esp_err_t _wifi_event_handler(void *ctx, system_event_t *event)
{
    ESP_LOGI(TAG,"Event[%d] recieved", event->event_id);
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG,"START");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG,"GOT IP");
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG,"DISCONNECTED");

            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            ESP_LOGI(TAG,"UNHANDELED");
            break;
    }
    return ESP_OK;
}

void wifi_wait_connected()
{
    EventBits_t current;
    ESP_LOGI(TAG,"Wait for wifi to connect");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG,"Finished waiting for wifi to connect");
   /* current = xEventGroupGetBits(wifi_event_group);
    if ( current && WIFI_SMARTCONFIG_BIT){
        //doing smart config wait until smart config is finished
        xEventGroupWaitBits(wifi_event_group, WIFI_SMARTCONFIG_FINISHED_BIT, true , true, portMAX_DELAY);
        //re-check we are connected to wifi just in case
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    }*/

}

void close_wifi(){

}

void smart_config_wifi(){
    //wait for the smart config to be called
    xEventGroupWaitBits(wifi_event_group, WIFI_SMARTCONFIG_BIT, false , true, portMAX_DELAY);
    //additional wait just in case we are in the middle of a send
    vTaskDelay(1000);
    //un-init current wifi connection

    //start new wifi as a hotspot

    //start smart connect app

    //save password and ssid in non volatile flash

    //close wifi hotspot

    //restart wifi connection to router with new password and ssid

    //clear|set synchronisation flags
    xEventGroupClearBits(wifi_event_group, WIFI_SMARTCONFIG_BIT);
    xEventGroupSetBits(wifi_event_group, WIFI_SMARTCONFIG_FINISHED_BIT);
}