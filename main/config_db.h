//
// Created by rick on 12/14/18.
//

#ifndef ZELDY_CONFIG_DB_H
#define ZELDY_CONFIG_DB_H
/*
#define DB_NAME "zeldy"
#define DB_USER "sensor"
#define DB_PASS "eiy7aepheeXaokaeseiw"
#define _DB_ADDRESS_BASE "https://zeldy-influxdb.brelivet.fr"
#define _DB_ADDRESS_PORT "443"
*/

#define DB_NAME "zeldy"
#define DB_USER "sensor"
#define DB_PASS "sensor_password"

#define USING_SSL
#define USING_SIMON

#ifdef USING_SSL
#define _DB_ADDRESS_HEADER "https"
#else
#define _DB_ADDRESS_HEADER "http"
#endif

#ifdef USING_SIMON
#define _DB_ADDRESS_BASE _DB_ADDRESS_HEADER "://zeldy-influxdb.brelivet.fr"
#define _DB_ADDRESS_PORT "443"

#else
#define _DB_ADDRESS_BASE _DB_ADDRESS_HEADER "://192.168.43.200"
#define _DB_ADDRESS_PORT "8086"

#endif

#define DB_ADDRESS _DB_ADDRESS_BASE ":" _DB_ADDRESS_PORT
#define DB_WRITE_ADDRESS DB_ADDRESS "/write?db=" DB_NAME "&u=" DB_USER "&p=" DB_PASS
#endif //ZELDY_CONFIG_DB_H
