//
// Created by rick on 12/15/18.
//
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "handler_https.h"
#include "influxdb.h"
#include "config_ssl.h"
#define TAG "HTTP_HANDLER"

#define USING_SSL

esp_http_client_handle_t *peristant_client;

void https_post(void *ptr_post_data) {
    ESP_LOGI(TAG, "Launched");
    esp_err_t err;
    char *raw_post_data;
    //waist of memory in order to compile with the -Werror=maybe-uninitialized error
    influx_db_data_s recv_data;
    QueueHandle_t *xQueue = (QueueHandle_t *) ptr_post_data;
    //wait for wifi to have been connected
    while (1) {
        vTaskDelay(1000);// ms
        ESP_LOGI(TAG, "Waiting wifi connected");
        wifi_wait_connected();
        if (peristant_client == NULL) {
            esp_http_client_config_t config = {
                    .url = build_post_address(),
                    .event_handler = _http_event_handler,
#ifdef USING_SSL
                    .cert_pem = ssl_com_root_cert_pem_start,
#endif
                    .max_redirection_count=20,
            };
            peristant_client = (esp_http_client_handle_t *) malloc(sizeof(esp_http_client_handle_t));
            *peristant_client = esp_http_client_init(&config);

        }

        //check if we have data to send
        if ((peristant_client != NULL) && (uxQueueSpacesAvailable(*xQueue) != 10)) {
            ESP_LOGI(TAG, "Data to be posted available");
            //queue is not empty- get data from queue
            while (uxQueueSpacesAvailable(*xQueue) != 10) {
                if (xQueueReceive(*xQueue, (void *)&recv_data, 100) == pdTRUE) {

                    ESP_LOGI(TAG, "Adding data from queue");
                    add_measurement(recv_data);
                    //free(recv_data);

                }

            }
            //add headers
            esp_http_client_set_header(*peristant_client, "db", get_header_db());
            esp_http_client_set_header(*peristant_client, "u", get_header_user());
            esp_http_client_set_header(*peristant_client, "p", get_header_pass());

            esp_http_client_set_method(*peristant_client, HTTP_METHOD_POST);
            raw_post_data = build_post_binary();
            ESP_LOGI(TAG,"Data body \n%s",raw_post_data);
            ESP_LOGI(TAG,"End of data body");
            ESP_LOGI(TAG, "length %d", strlen(raw_post_data));
            ESP_LOGI(TAG,"Posting");
            err = esp_http_client_set_post_field(*peristant_client, raw_post_data, strlen(raw_post_data));
            if (err != ESP_OK) {
                ESP_LOGI(TAG, "Post body set failed ");
            }
            ESP_LOGI(TAG, "Performing request");
            err = esp_http_client_perform(*peristant_client);

            if (err == ESP_OK) {
                ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                         esp_http_client_get_status_code(*peristant_client),
                         esp_http_client_get_content_length(*peristant_client));
            } else {
                ESP_LOGI(TAG, "Error perform http request %s", esp_err_to_name(err));
            }
            if(raw_post_data!= NULL )free(raw_post_data);

        }

        if (peristant_client != NULL) {
            esp_http_client_cleanup(*peristant_client);
            free(peristant_client);
            peristant_client = NULL;
        }
        free_post_data();
    }

}

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

