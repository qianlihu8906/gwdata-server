#ifndef CLOUD_CLIENT_H__
#define CLOUD_CLINET_H__
int gw_cloud_broadcast(struct sensor_data *sd);
void gw_cloud_new_client(aeEventLoop *el,int fd,void *client_data,int mask);
#endif
