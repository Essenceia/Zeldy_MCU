//
// Created by rick on 12/11/18.
//

#include "influxdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_db.h"

#include "esp_log.h"

#define TAG "InfluxDB"
/**
 * Persistent hidden object for post requests
 */
static influxdb_post_s *db_post;
//static int length;

influx_db_data_s new_measurement(char name[],int len ,  influx_db_mesurement_value_u val, FIELD_VALUES_TYPES_E type) {
    influx_db_data_s nv;
    strncpy(nv.measurement, name, len);
    nv.data = val;
    nv.data_type = type;
    return nv;
}

influx_db_data_s *new_measurement_ptr(char name[], influx_db_mesurement_value_u val, FIELD_VALUES_TYPES_E type) {
    influx_db_data_s *nv;
    nv = (influx_db_data_s *) malloc(sizeof(influx_db_data_s));
    if (nv != NULL) {
        strncpy(nv->measurement, name, strlen(name));
        memset(name, 0, strlen(name));
        memcpy(&(nv->data), &val, sizeof(influx_db_mesurement_value_u));
        nv->data_type = type;
    }
    return nv;
}

int influx_db_init() {
    size_t str_l;
    db_post = (influxdb_post_s *) malloc(sizeof(influxdb_post_s));
    if (db_post != NULL) {

        str_l = strlen(DB_PASS);
        str_l = (DB_MAX_STD_STR_LENGTH > str_l) ? str_l : DB_MAX_STD_STR_LENGTH;
        strncpy(db_post->auth.pass, DB_PASS, str_l);
        db_post->auth.pass[strlen(DB_PASS)] = '\0';


        str_l = strlen(DB_USER);
        str_l = (DB_MAX_STD_STR_LENGTH > str_l) ? str_l : DB_MAX_STD_STR_LENGTH;
        strncpy(db_post->auth.user, DB_USER, str_l);
        db_post->auth.user[str_l] = '\0';

        str_l = strlen(DB_NAME);
        str_l = (DB_MAX_STD_STR_LENGTH > str_l) ? str_l : DB_MAX_STD_STR_LENGTH;
        strncpy(db_post->database, DB_NAME, str_l);
        db_post->database[str_l] = '\0';

        db_post->data_length = 0;
        return 0;
    }
    return -1;

}

char *build_post_binary() {
    char str[DB_MAX_POST_LENGHT] = {0};
    char *retdata;
    int len;
    if (db_post != NULL) {
        for (int i = 0; i < db_post->data_length; i++) {

            sprintf(str, "%s%s,host=server01,region=us-west value=",str,db_post->data[i].measurement);
            switch (db_post->data[i].data_type) {
                case FLOAT:
                    sprintf(str, "%s%f\n", str, db_post->data[i].data.f);
                    break;
                case INTEGER:
                    sprintf(str, "%s%d\n", str, db_post->data[i].data.i);
                    break;
                case BOOLEAN:
                    sprintf(str, "%s%s\n", str, BOOL_TO_STR(db_post->data[i].data.b));
                    break;
            }
        }
        len = strlen(str);
        retdata = (char *) malloc(sizeof(char) * len +1);
        if (retdata != NULL) {
            memcpy(retdata, str, len);
            //retdata[len-1] = '';
             retdata[len] = '\0';
        }
        ESP_LOGI(TAG, "Length of data %d", len);
        //length =len;
        return retdata;
    }
    return NULL;
}
/*
int get_data_length(){
    return length;
}
char *build_post_address() {
    char str[DB_MAX_ADDR_LENGHT];
    char *retdata;
    if (db_post != NULL) {
        sprintf(str, "%s/write?db=%s&u=%s&p=%s", DB_ADDRESS, db_post->database, db_post->auth.user, db_post->auth.pass);
        retdata = (char *) malloc(sizeof(char) * strlen(str));
        strncpy(retdata, str, strlen(retdata));
        return retdata;
    }
    return NULL;
}*/
char *build_post_address() {
    return DB_WRITE_ADDRESS;
}

int add_measurement(influx_db_data_s toadd) {
    ESP_LOGI(TAG, "Adding measurement data <%s>(%d):<%f>", toadd.measurement, toadd.data_type, toadd.data.f);
    db_post->data[db_post->data_length] = toadd;
    db_post->data_length++;
    return 0;
}

void free_post_data() {
    int reterr = 0;
    ESP_LOGI(TAG, "Freeing data, lenght %d", db_post->data_length);
    if (db_post != NULL) {
        db_post->data_length = 0;
    } else {
        ESP_LOGE(TAG, "Error freeing data");
    }
}

char *get_header_db() {
    return db_post->database;
}

char *get_header_user() {
    return db_post->auth.user;
}

char *get_header_pass() {
    return db_post->auth.pass;
}