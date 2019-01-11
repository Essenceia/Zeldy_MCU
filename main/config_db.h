//
// Created by rick on 12/14/18.
//

#ifndef ZELDY_CONFIG_DB_H
#define ZELDY_CONFIG_DB_H


#define DB_NAME CONFIG_DB_NAME
#define DB_USER CONFIG_DB_USER
#define DB_PASS CONFIG_DB_PASS


#define DB_ADDRESS CONFIG_DB_ADDRESS
#define DB_WRITE_ADDRESS DB_ADDRESS "/write?db=" DB_NAME "&u=" DB_USER "&p=" DB_PASS
#endif //ZELDY_CONFIG_DB_H
