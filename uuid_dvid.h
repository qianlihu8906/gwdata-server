#ifndef UUID_DVID_H__
#define UUID_DVID_H__


#define LENGTH_UUID     16

#define REQ_DATA        0x00
#define RSP_DATA        0x01
#define REQ_HEARTBEAT   0x02
#define RSP_HEARTBEAT   0x03
#define REQ_RESET       0x04
#define RSP_RESET       0x05
#define REQ_SWITCH_ON   0x06
#define RSP_SWITCH_ON   0x07
#define REQ_SWITCH_OFF  0x08
#define RSP_SWITCH_OFF  0x09
#define REQ_CUSTOM      0x0e
#define RSP_CUSTOM      0x0f

int uuid_dvid_init();

int uuid_dvid_string2uuid(const char *string,char *uuid);

const char *uuid_dvid_find_uuid(int dvid);

int uuid_dvid_find_dvid(char *uuid);

const char *uuid_dvid_find_heartuuid();

int uuid_dvid_add_record(const char *uuid,int dvid);

void uuid_dvid_del_uuid(char *uuid);

void uuid_dvid_del_dvid(int dvid);

void uuid_dvid_debug();

#endif
