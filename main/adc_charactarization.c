//
// Created by rick on 12/18/18.
//

#include "adc_charactarization.h"
#include "adc.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "driver/timer.h"
#include "driver/periph_ctrl.h"


#define TIMER_DIVIDER         80  //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds

#define TAG "ADC characterize"
void adc_charactarize(void * params){
    ESP_LOGI(TAG,"Init");
    uint32_t values[10];
    double time;
     timer_config_t tconf;
    tconf.divider = TIMER_DIVIDER;
    tconf.counter_dir = TIMER_COUNT_UP;
    timer_init(TIMER_GROUP_0, TIMER_0 , &tconf);
    setup_adc();
    while(1) {
        ESP_LOGI(TAG,"start");
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
        timer_start(TIMER_GROUP_0, TIMER_0);
        //adc
        for(unsigned int i = 0 ; i < 10 ; i ++){
            values[i]=get_ADC();
        }
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0 , &time);
        ESP_LOGI(TAG,"read %lf", time);
        //create a new timer
        vTaskDelay(3000);//1000ms
    }
    close_adc();
}
