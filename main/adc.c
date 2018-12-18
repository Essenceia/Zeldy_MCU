//
// Created by rick on 12/4/18.
//
#include <driver/adc.h>
#include "esp_log.h"
#include "esp_adc_cal.h"
#define TAG "ADC"
#define DEFAULT_VREF 3300
//Reading voltage on ADC1 channel 0 (GPIO 36)
void setup_adc(){
    esp_err_t err;
    adc_power_on();
    err= adc1_config_width(ADC_WIDTH_BIT_12);
    if (err != ESP_OK){
                ESP_LOGE(TAG, "parameter error %s", esp_err_to_name(err));
        }
    /*
     * The default ADC full-scale voltage is 1.1V. To read higher voltages (up to the pin maximum voltage, usually 3.3V)
     * requires setting >0dB signal attenuation for that ADC channel.
     */
    err = adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "parameter error %s", esp_err_to_name(err));
    }
}

void close_adc(){
    adc_power_off();
}
//read raw adc value
uint32_t get_ADC(){
    return adc1_get_raw(ADC1_CHANNEL_0);
}

//read voltage
uint32_t read_voltage(){
    uint32_t reading, voltage;
    esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    //Check type of calibration value used to characterize ADC
    /*
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(TAG, "eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(TAG,"Two Point");
    } else {
        ESP_LOGI(TAG,"Default");
    }*/

    reading = get_ADC();
   voltage =  esp_adc_cal_raw_to_voltage(reading,adc_chars );
    free(adc_chars);
    return voltage;
}