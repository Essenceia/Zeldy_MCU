/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include <freertos/queue.h>
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "influxdb.h"
#include "handler_https.h"

#define TAG "Main"

void app_main() {
    //Definitions
    QueueHandle_t *xQueue;
    float sensor_data = 0.5;
    influx_db_mesurement_value_u mval;
    influx_db_data_s nvdata;
    //Initialisation of flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //Allocation
    //Create queue to serve as a mail box between our task and the collection of sensor raw data
    //also adds the possibility of buffering data and sending it out in bursts.
    xQueue = (QueueHandle_t *) malloc(sizeof(QueueHandle_t));
    if (xQueue != NULL)*xQueue = xQueueCreate(10, sizeof(influx_db_data_s));
    else ESP_LOGE(TAG, "xQueue not created");
    //Init
    init_wifi();

    ESP_LOGI(TAG, "DB init %d", influx_db_init());
    ESP_LOGI(TAG, "Starting task to handle https requests to server");
    xTaskCreate(&https_post, "https_post", 4096, (void *) xQueue, 5, NULL);
    while (1) {
        //read some new data
        sensor_data++;

        mval.f = sensor_data;
        ESP_LOGI(TAG, "Adding measurement");
        nvdata = new_measurement("DUMDATA\0", mval, FLOAT);
            if (xQueueSendToBack(*xQueue, &nvdata, (TickType_t) 10) != pdPASS) {
                ESP_LOGI(TAG, "Failed to post to queue");
            }

        vTaskDelay(1000);//1000ms
    }
    /*
        add_measurement(new_measurement("DUMDATA",mval,FLOAT));
        esp_sleep_enable_timer_wakeup(1000000);
        ESP_LOGI(TAG,"Sleep start");
        esp_light_sleep_start();
    */
}
