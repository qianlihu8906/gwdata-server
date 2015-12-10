#ifndef P208_SERVER_H__
#define P208_SERVER_H__

#include "ae.h"
#include <stdint.h>

void p208_server_acceptHandler(aeEventLoop *el,int fd,void *privdata,int mask);

#endif
