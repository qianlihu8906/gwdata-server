#ifndef DEVICES_H__
#define DEVICES_H__


const char *device_v2string(int id,int type,const char *data,int len,char *strbuf,int size);
int device_v2chararray(int id,int type,const char *str,char *buf,int size);

int device_v2cloud(int id,int type,const char *str,char *buf,int size);
#endif
