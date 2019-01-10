/**
* Created by rick on 12/11/18.
*
* Small library to interact with influx db
*
 * Line protocol syntax
 * <measurement>[,<tag_key>=<tag_value>[,<tag_key>=<tag_value>]] <field_key>=<field_value>[,<field_key>=<field_value>] [<timestamp>]
*/
#ifndef ZELDY_INFLUXDB_H
#define ZELDY_INFLUXDB_H

#include <stddef.h>
#include "newlib.h"
#define DB_MAX_POST_LENGHT 800
#define DB_MAX_ADDR_LENGHT 100
#define DB_MAX_STD_STR_LENGTH 20
#define DB_MAX_MEASUREMENT_DATA 20

#define BOOL_TO_STR(b) (b?"true":"false")
/**
 * Field keys are strings. Field values can be floats, integers, strings, or Booleans.
 */
typedef enum  {
    FLOAT , INTEGER , BOOLEAN
}FIELD_VALUES_TYPES_E;
/**
 * Union values to represent all mesasurments
 */
typedef union {
    float f;
    int i;
    unsigned char b;
} influx_db_mesurement_value_u;
/**
 * User identification for database
 */
typedef struct {
    char user[DB_MAX_STD_STR_LENGTH];
    char pass[DB_MAX_STD_STR_LENGTH];
} influx_db_auth_s;
/**
 * Strucutured data to be sent
 */
typedef struct {
    char measurement[DB_MAX_STD_STR_LENGTH] ;// < The measurement name. InfluxDB accepts one measurement per point.

    influx_db_mesurement_value_u data;
    FIELD_VALUES_TYPES_E data_type;
} influx_db_data_s;
/**
 * All data includung measurements and authentification to be sent to the database
 */
typedef struct {
    influx_db_auth_s auth;
    char database[DB_MAX_STD_STR_LENGTH];
    influx_db_data_s data[DB_MAX_MEASUREMENT_DATA];
    size_t data_length;

} influxdb_post_s;

/**
 * Create new data point for request
 * Any errors will need to be identified on caller ( is null )
 * @param name
 * @param val
 * @param type
 * @return
 */
influx_db_data_s new_measurement(char name[], influx_db_mesurement_value_u val, FIELD_VALUES_TYPES_E type);
/**
 * Create new data point pointer for request
 * Any errors will need to be identified on caller ( is null )
 * @param name
 * @param val
 * @param type
 * @return
 */
influx_db_data_s* new_measurement_ptr(char name[], influx_db_mesurement_value_u val, FIELD_VALUES_TYPES_E type);

/**
 * Initialises influx DB user auth and creats a reusable object
 * @return
 */
int influx_db_init();
/**
 * Add a measurement to the next post request to the database
 * @param toadd
 * @return
 */
int add_measurement(influx_db_data_s toadd);

/**
 * Free measurement data used in post request
 */
void free_post_data();
/**
 * Build post request body
 * @param data
 * @return
 */
char *build_post_binary();
/**
 * Build post request address
 * @param data
 * @return
 */
char *build_post_address();
/**
 * Get server configurations : DB name
 */
char * get_header_db();
/**
 * Get server configurations : DB username
 */
char * get_header_user();
/**
 * Get server configurations : DB password
 */
char * get_header_pass();

//int get_data_length();
#endif //ZELDY_INFLUXDB_H
