#ifndef PROTOCAL_208_H__
#define PROTOCAL_208_H__

#define CMD_QUERY               1
#define CMD_QUERY_HISTORY       4
#define CMD_SET                 3

#include "protocal.h"

struct protocal208_cmd{
        int cmd;
        int device_id;
        int device_type;
        char data[8];
};
int sensor_data_to_slip_208(struct sensor_data *sd,char *slip,int size);

int slip_to_protocal208_cmd(struct protocal208_cmd *cmd,const char *slip,int len);

void protocal208_cmd_debug(struct protocal208_cmd *cmd);

int eof_to_slip_208(char *slip,int size);
int protocal208_cmd_to_slip(struct protocal208_cmd *cmd,char *slip,int size);
int protocal208_sd2data(struct sensor_data *sd,char *buf,int len);
#endif
