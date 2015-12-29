#ifndef JSON_SERVER_H__
#define JSON_SERVER_H__

#include <stdint.h>
#include "ae.h"
#include "protocal.h"

void json_server_acceptHandler(aeEventLoop *el,int fd,void *privdata,int mask);
int json_server_broadcast(struct sensor_data *sd);

#endif
