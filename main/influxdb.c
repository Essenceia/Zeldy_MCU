//
// Created by rick on 12/11/18.
//

#include "influxdb.h"
#include <stdio.h>

/**
 * Persistent hidden object for post requests
 */
influxdb_post_s *db_post;

influx_db_data_s *new_measurement(string name, influx_db_mesurement_value val, FIELD_VALUES_TYPES_E type) {
    influx_db_data_s *nv = (influx_db_data_s *) malloc(sizeof(influx_db_data_s));
    if (nv != null) {
        nv->measurement = name;
        nv->data = val;
        nv->data_type = type;
    }
    return nv;
}

int influx_db_init() {
    db_post = (influxdb_post_s *) malloc(sizeof(influxdb_post_s));
    if (db_post != null) {
        db_post->auth.pass = DB_PASS;
        db_post->auth.user = DB_USER;
        db_post->data_length = 0;
        return 0;
    }
    return -1;

}

char *build_post_binary(influxdb_post_s *data) {
    char str[DB_MAX_POST_LENGHT];
    char *retdata;
    if ((data != null) && (db_post != null)) {
        for (int i = 0; i < db_post->data_length; i++) {
            sprintf(str, "%s,value=", db_post->data[i]->measurement);
            switch(db_post->data[i]->data_type){
                case FLOAT:
                    sprintf(str, "%s%f\n", db_post->data[i]->data.f);
                    break;
                case INTEGER:
                    sprintf(str, "%s%d\n", db_post->data[i]->data.i);
                    break;
                case BOOLEAN:
                    sprintf(str, "%s%s\n", BOOL_TO_STR(db_post->data[i]->data.b));
                    break;
            }
            sprintf(str, str, db_post->data[i]->data);
        }
        retdata = (char *) malloc(sizeof(char) * strlen(str));
        if (retdata != null) {
            memcpy(retdata, str, strlen(str));
        }
        return retdata;
    }
    return null;
}

char *build_post_address(influx_post_s *data) {
    char str[DB_MAX_ADDR_LENGHT];
    char *retdata;
    if ((data != null) && (db_post != null)) {
        sprintf(str, "%s/write?db=%s&u=%s&p=%s", DB_ADDRESS, data->database, data->auth.user, data->auth.pass);
        retdata = (char *) malloc(sizeof(char) * strlen(str));
        strcpy(retdata, str, strlent(str));
        return retdata;
    }
    return null;
}

int add_measurement(influx_db_data_s *toadd) {
    if ((toadd != null) && (db_post != null)) {
        influx_db_data_s[db_post->data_length] = toadd;
        db_post->data_length++;
        return 0;
    }
    return -1;
}

int free_post_data() {
    int reterr = 0;
    if (db_post != null) {
        for (int i = 0; i < db_post->data; i++) {
            if (db_post->data[i] != null) {
                reterr -= free(db_post->data[i]);
            } else {
                reterr -= 1;
            }
        }
        db_post->data_length = 0;
        return reterr;
    }
    return -1;
}