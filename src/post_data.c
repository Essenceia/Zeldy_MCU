//
// Created by rick on 12/4/18.
//

#include "post_data.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#define TAG "ADC POST"
/*void post_data(int data){
    const char *post_data = "adc=1500";
    esp_err_t err;
    esp_http_client_config_t config = {
            .url = "http://httpbin.org/status/418",
            .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    // esp_http_client_set_url(client, "http://192.168.43.200:8080");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

}*/