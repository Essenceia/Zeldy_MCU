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
#include <driver/adc.h>
#include <driver/gpio.h>
#include <driver/timer.h>
#include "math.h"

#define TAG "Main"

#define APP_CPU 1
#define ADC_VOLTAGE ADC1_CHANNEL_4 //CH4 = GPIO32 ... CH7 =GPIO35. WARNING DO NOT, repeat NOT use others WARNING
#define ADC_CURRENT ADC1_CHANNEL_5
#define ESP_INTR_FLAG_DEFAULT 0
// TIMER SETUP
#define TIMER_PERIODE_US 100 //TODO
//#define DEBUG

#define PROD

// ADC setup
const int iADCVoltageZeroValue = 1826;      //Adjust value of 0V mesured by ADC
const float fVoltageMultiplier = 0.227014;    //Adjust value
const int iADCCurrentZeroValue = 2048;      //TODO adjust value
const float fCurrentMultiplier = 0.015;      //TODO adjust value



// flag setup
/*volatile int iZCVoltageFlag =0; // flag raised on zero crossing voltage
volatile int iReadADCFlag =0;
volatile int iTimerFlag = 0;
*/
volatile EventGroupHandle_t EventGroup;
#define FLAG_iZCVoltage (1<<0) // flag raised on zero crossing voltage
#define FLAG_iReadADC   (1<<1) // flag raised when reading ADC phase and lowered otherwise when computing data
#define FLAG_iTimer     (1<<2) //flag raised by timer

// mutex for critical flags
portMUX_TYPE muxZCVoltageFlag = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE muxTimerFlag = portMUX_INITIALIZER_UNLOCKED;


// synchro values
int iCountZCVoltage = 0;
int iCurrentADCPos = 0;
#ifndef PROD
int iAction = 2; // 0 read voltage, 1 read current, 2 compute
#else
int iAction = 0; // 0 read voltage, 1 read current, 2 compute
#endif

// Data
float fCosPhi = 1;
int *piADCVoltageRead;
int *piADCCurrentRead;
int iNbMeas = 0; //nb of measures
float fMeanActivePower = 0.0;
float fMeanApparentPower = 0.0;
float fVoltageRMS = 0.0;
float fCurrentRMS = 0.0;

//Timers
static esp_timer_handle_t periodic_timer;


// Convertion and computation rms
float fnADC2Voltage(int ADCVoltageValue) {
    return (fVoltageMultiplier * (ADCVoltageValue - iADCVoltageZeroValue));
}

float fnADC2Current(int ADCCurrentValue) {
    return (fCurrentMultiplier * (ADCCurrentValue - iADCCurrentZeroValue));
}

void fnComputeRMS(QueueHandle_t *queueHandle) {
    influx_db_mesurement_value_u mval;
    influx_db_data_s nvdata;
#ifdef DEBUG
    ESP_LOGI("ComputeRMS::", "Computing for iNbMeas=%d", iNbMeas);
#endif
    for (int i = 0; i < iNbMeas; i++) {
        float tempV = fnADC2Voltage(piADCVoltageRead[i]);
        float tempI = fnADC2Current(piADCCurrentRead[i]);
        fMeanActivePower += tempV * tempI;
        fVoltageRMS += tempV * tempV;
        fCurrentRMS += tempI * tempI;
    }
    fMeanActivePower = fMeanActivePower / iNbMeas;
    fVoltageRMS = sqrt(fVoltageRMS) / iNbMeas;
    fCurrentRMS = sqrt(fCurrentRMS) / iCurrentADCPos;
    fMeanApparentPower = fVoltageRMS * fCurrentRMS;
    fCosPhi = fMeanActivePower / fMeanApparentPower;
#ifndef PROD
    if(!isnormal(fMeanActivePower))fMeanActivePower = 2.546;
    if(!isnormal(fVoltageRMS))fVoltageRMS= 15.0;
    if(!isnormal(fCurrentRMS))fCurrentRMS = 20.0;
    if(!isnormal(fMeanApparentPower))fMeanApparentPower = 2.008;
    if(!isnormal(fCosPhi))fCosPhi = 10.0;
#endif

#ifdef DEBUG
    ESP_LOGI("ComputeRMS::", "Results CosPhi=%f fCurrentRMS=%f fVoltageRMS=%f data to queue", fCosPhi, fCurrentRMS, fVoltageRMS);
    ESP_LOGI("ComputeRMS::", "Checking space in queue");
    ESP_LOGI("ComputeRMS::", "Gotten %d space in queue",uxQueueSpacesAvailable(*queueHandle));
#endif
    if((uxQueueSpacesAvailable(*queueHandle)>0)&& isnormal(fMeanActivePower)) {
        //adding information to be sent to database
        mval.f = fMeanActivePower;
        nvdata = new_measurement("mean_active_power\0", mval, FLOAT);
#ifdef DEBUG
        ESP_LOGI("ComputeRMS::", "Sending data 0.1");
#endif
        if (xQueueSendToBack(*queueHandle, (void * ) &nvdata, (TickType_t) 5) != pdPASS) {
#ifdef DEBUG
            ESP_LOGI(TAG, "Failed to post to queue");
#endif
        }

    }
    if((uxQueueSpacesAvailable(*queueHandle)>0)&& isnormal(fMeanApparentPower)) {
        //adding information to be sent to database
        mval.f = fMeanApparentPower;
        nvdata = new_measurement("mean_appearent_power\0", mval, FLOAT);
#ifdef DEBUG
        ESP_LOGI("ComputeRMS::", "Sending data 0.0");
#endif
        if (xQueueSendToBack(*queueHandle, (void * ) &nvdata, (TickType_t) 5) != pdPASS) {
#ifdef DEBUG
            ESP_LOGI(TAG, "Failed to post to queue");
#endif
        }

    }
    if((uxQueueSpacesAvailable(*queueHandle)>0)&& isnormal(fVoltageRMS)) {
        //adding information to be sent to database
        mval.f = fVoltageRMS;
        nvdata = new_measurement("voltage_rms\0", mval, FLOAT);
#ifdef DEBUG
        ESP_LOGI("ComputeRMS::", "Sending data 1.1");
#endif
        if (xQueueSendToBack(*queueHandle,(void * ) & nvdata, (TickType_t) 5) != pdPASS) {
#ifdef DEBUG
            ESP_LOGI(TAG, "Failed to post to queue");
#endif
        }
    }

    if((uxQueueSpacesAvailable(*queueHandle)>0)&& isnormal(fCurrentRMS)) {
        //adding information to be sent to database
        mval.f = fCurrentRMS;
        nvdata = new_measurement("current_rms\0", mval, FLOAT);
#ifdef DEBUG
        ESP_LOGI("ComputeRMS::", "Sending data 1.2");
#endif
        if (xQueueSendToBack(*queueHandle,(void * ) & nvdata, (TickType_t) 5) != pdPASS) {
#ifdef DEBUG
            ESP_LOGI(TAG, "Failed to post to queue");
#endif
        }
    }

    if((uxQueueSpacesAvailable(*queueHandle)>0)&& isnormal(fCosPhi)) {
        //adding information to be sent to database
        mval.f = fCosPhi;
        nvdata = new_measurement("cos_phi\0", mval, FLOAT);
#ifdef DEBUG
        ESP_LOGI("ComputeRMS::", "Sending data 2");
#endif
        if (xQueueSendToBack(*queueHandle,(void * ) & nvdata, (TickType_t) 5) != pdPASS) {
#ifdef DEBUG
            ESP_LOGI(TAG, "Failed to post to queue");
#endif
        }
    }

}


// interupt handlers
static void IRAM_ATTR ISRZCVoltage(void *args) {
    BaseType_t xHigherPriorityTaskWoken;

    // Rst timer
    ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, TIMER_PERIODE_US));

    /*
     *  used as a light weight and faster binary or counting semaphore alternative.
     */
    xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(
            EventGroup,
            FLAG_iZCVoltage,
            &xHigherPriorityTaskWoken);
}

static void ISRTimer(void *stuff) {
    BaseType_t xHigherPriorityTaskWoken;
#ifdef DEBUG
    ESP_LOGI("ISR Timer::", "called");
#endif
    xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(
            EventGroup,   /* The event group being updated. */
            FLAG_iTimer, /* The bits being set. */
            &xHigherPriorityTaskWoken);

#ifdef DEBUG
    ESP_LOGI("ISR Timer::", "exit");
#endif
}


// Process flags
void fnProcessZCVoltageFlag(QueueHandle_t *queueHandle) {
    iCountZCVoltage++;

    if (iCountZCVoltage % 2 == 0) {
        iAction = (iAction + 1) % 3;

    }

#ifdef DEBUG
    ESP_LOGI("Process ZC Voltage::", "iCountZCVoltage %d, iAction %d", iCountZCVoltage, iAction);
#endif
    if (iAction == 2) {

        xEventGroupClearBits(
                EventGroup,  /* The event group being updated. */
                FLAG_iReadADC);/* The bits being cleared. */
        fnComputeRMS(queueHandle);
        iNbMeas = 0;
        iCurrentADCPos = 0;

        xEventGroupSetBits(
                EventGroup,  /* The event group being updated. */
                FLAG_iReadADC);/* The bits being cleared. */
    } else {
    }
    xEventGroupClearBits(
            EventGroup,  /* The event group being updated. */
            FLAG_iZCVoltage);/* The bits being cleared. */

}

void fnProcessTimerFlag() {
#ifdef DEBUG
    ESP_LOGI("Process timer::", "Process timer called action %d", iAction);
#endif
    int tmp;
    if (iAction == 0) {
        tmp = adc1_get_raw(ADC_VOLTAGE);
#ifdef DEBUG
        ESP_LOGI("Process timer::", "Read ADC voltage returns %d", tmp);
#endif
        if (tmp >= 0) {
            piADCVoltageRead[iNbMeas] = tmp;
            iNbMeas++;
#ifdef DEBUG
            ESP_LOGI("Process timer::", "Increment iNbMeas new val %d", iNbMeas);
#endif
        }
    }
    if (iAction == 1) {
        tmp = adc1_get_raw(ADC_CURRENT);
#ifdef DEBUG
        ESP_LOGI("Process timer::", "Read ADC current returns %d", tmp);
#endif
        if (tmp >= 0) {
            piADCCurrentRead[iCurrentADCPos] = tmp;
#ifdef DEBUG
            ESP_LOGI("Process timer::", "Increment iCurrentADCPos new val %d", iCurrentADCPos);
#endif

            iCurrentADCPos++;
        }
    }
    xEventGroupClearBits(
            EventGroup,  /* The event group being updated. */
            FLAG_iTimer);/* The bits being cleared. */

}

void app_main() {

    printf("Hello world!\n");

    /********* Hardware init *************/
    BaseType_t uxBits;
    esp_err_t err;

    //Initialisation of flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");


    //Event signaling
    EventGroup = xEventGroupCreate();
    if (EventGroup == NULL) {
        ESP_LOGE("App main", "Event group creation failed");
    }




// ISR settings
    gpio_config_t io_conf;
//interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO 5 here
    io_conf.pin_bit_mask = GPIO_NUM_5;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_pad_select_gpio(GPIO_NUM_5);
    // ESP_ERROR_CHECK(gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT));
    //ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_NUM_5, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_NUM_5, GPIO_INTR_NEGEDGE));
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // ESP_ERROR_CHECK(gpio_intr_enable(GPIO_NUM_5));
    // ESP_ERROR_CHECK(gpio_isr_register(ISRZCVoltage, NULL, 0, 0));
    //hook isr handler for specific gpio pin
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_5, ISRZCVoltage, (void *) GPIO_NUM_5));


    printf("Start config\n");

// timer setting
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &ISRTimer,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, TIMER_PERIODE_US));


    piADCVoltageRead = malloc(1000 * sizeof(int));
    piADCCurrentRead = malloc(1000 * sizeof(int));


    //setup ADC
    adc_power_on();
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    //ESP_ERROR_CHECK(adc1_vref_to_gpio(GPIO_NUM_25));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_VOLTAGE, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_CURRENT, ADC_ATTEN_DB_11));



    /********* Original main *************/
    //Definitions
    QueueHandle_t *xQueue;
    float sensor_data = 0.5;
    influx_db_mesurement_value_u mval;
    influx_db_data_s nvdata;

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
   // xTaskCreate(&https_post, "https_post", 8192, (void *) xQueue, 5,NULL);
   xTaskCreatePinnedToCore(&https_post, "https_post", 8192, (void *) xQueue, 5, NULL, 0);
    while (1) {
        vTaskDelay(10);
        uxBits = xEventGroupWaitBits(
                EventGroup,   /* The event group being tested. */
                FLAG_iZCVoltage | FLAG_iTimer, /* The bits within the event group to wait for. */
                pdFALSE,        /* Flag bits should not be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                portMAX_DELAY);/* Wait a maximum */
#ifndef PROD
        if ((uxBits & FLAG_iZCVoltage) == 0) {
#else

        if ((uxBits & FLAG_iZCVoltage) != 0) {
#endif
            fnProcessZCVoltageFlag(xQueue);
        } else if ((uxBits & FLAG_iTimer) != 0) {
            fnProcessTimerFlag();
        }
#ifdef DEBUG
        ESP_LOGI("Main Loop::", "Event has been triggered");
#endif
        //read some new data
        /*sensor_data++;

        mval.f = sensor_data;
        ESP_LOGI(TAG, "Adding measurement");
        nvdata = new_measurement("cpu_load_short", mval, FLOAT);
        if (xQueueSendToBack(*xQueue, &nvdata, (TickType_t) 10) != pdPASS) {
            ESP_LOGI(TAG, "Failed to post to queue");
        }*/

    }

}
