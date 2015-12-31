#ifndef DEVICES_H__
#define DEVICES_H__
#include <cJSON.h>

int device_v2chararray(int id,int type,cJSON *value,char *buf,int size);

int device_v2cloud(int id,int type,cJSON *value,char *buf,int size);

cJSON* device_v2json(int id,int type,const char *data,int len);
#endif
