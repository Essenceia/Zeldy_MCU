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

#define DB_MAX_POST_LENGHT 500
#define DB_MAX_ADDR_LENGHT 100
#define BOOL_TO_STR(b) (b?"true":"false")
/**
 * Field keys are strings. Field values can be floats, integers, strings, or Booleans.
 */
typedef enum FIELD_VALUES_TYPES_E {
    FLOAT = "%f", INTEGER = "%i", BOOLEAN = "%s"
};
/**
 * Union values to represent all mesasurments
 */
typedef union {
    long float f;
    int i;
    bool b;
} influx_db_mesurement_value;

typedef struct {
    string user, pass;
} influx_db_auth_s;

typedef struct {
    string measurement;// < The measurement name. InfluxDB accepts one measurement per point.

    influx_db_mesurement_value data;
    FIELD_VALUES_TYPES_E data_type;
} influx_db_data_s;

typedef struct {
    influx_db_auth_s auth;
    string database;
    influx_db_data_s **data;
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
influx_db_data_s *new_measurement(string name, influx_db_mesurement_value val, FIELD_VALUES_TYPES_E type);

/**
 * Initialises influx DB user auth and creats a reusable object
 * @return
 */
int influx_db_init();

int add_measurement(influx_db_data_s *toadd);

/**
 * Free measurement data used in post request
 */
int free_post_data();

char *build_post_binary(influxdb_post_s *data);

char *build_post_address(influx_post_s *data);

#endif //ZELDY_INFLUXDB_H
