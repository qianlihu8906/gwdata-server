#ifndef JSON_SERVER_H__
#define JSON_SERVER_H__

#include "ae.h"
#include <stdint.h>

void json_server_acceptHandler(aeEventLoop *el,int fd,void *privdata,int mask);
int json_server_broadcast_str(const char *buf);

#endif
