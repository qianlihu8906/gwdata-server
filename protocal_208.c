#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include "devices.h"
#include "protocal.h"
#include "protocal_208.h"
#include "sdlist.h"
#include "debug.h"
#include "cJSON.h"
#include "gw.h"

static char checksum(const char *data,int len)
{
        char sum = 0;
        int i;
        for(i = 0; i < len;i++){
                sum += data[i];
        }
        sum = ~sum + 1;
        return sum;
}

static int slip_encode_208(const char *src,int len_src,char *dest)
{
        int i,j;
        dest[0] = 0x7e;
        for(i=0,j=1;i<len_src;i++,j++){
                if(src[i]==0x7e || src[i]== 0x7d || src[i] < 0x20){
                        dest[j] = 0x7d;
                        j++;
                        dest[j] = src[i]^0x20;
                }else{
                        dest[j] = src[i];
                }
        }
        char sum = checksum(src,len_src);
        if(sum == 0x7d || sum == 0x7e){
                dest[j] = 0x7d;
                j++;
                dest[j] = sum^0x20;
        }else{
                dest[j] = sum;
        }
        j++;
        dest[j] = 0x7e;
        j++;


        return j;
}

static int slip_encode(const char *src,int len_src,char *dest)
{
        int i,j;
        dest[0] = 0x7e;
        for(i=0,j=1;i<len_src;i++,j++){
                if(src[i]==0x7e || src[i]== 0x7d){
                        dest[j] = 0x7d;
                        j++;
                        dest[j] = src[i]^0x20;
                }else{
                        dest[j] = src[i];
                }
        }
        char sum = checksum(src,len_src);
        if(sum == 0x7d || sum == 0x7e){
                dest[j] = 0x7d;
                j++;
                dest[j] = sum^0x20;
        }else{
                dest[j] = sum;
        }
        j++;
        dest[j] = 0x7e;
        j++;


        return j;
}
static int slip_decode(const char *src,int len_src,char *dest)
{
 
        int i,j;
        for(i=0,j=0;i<len_src;i++,j++){
                if(src[i] == 0x7d){
                        i++;
                        dest[j] = src[i]^0x20;
                }else{
                        dest[j] = src[i];
                }
        }
        char sum = 0;
        for(i = 0;i < j;i++){
                sum += dest[i];
        }
        if(sum != 0){
                return -1;
        }

        return i-1;
}


int sensor_data_to_slip_208(struct sensor_data *sd,char *slip,int size)
{
        char buf[8] ={0};

        sensor_data_debug(sd);
        int r = protocal208_sd2data(sd,buf,sizeof(buf));
        if(r < 0){
                printf("protocal208_sd2data\n");
                return -1;
        }
        int len = 24; 			// magic number 
        if(2*len > size){
                printf("error..........\n");
                return -1;
        }

        char data[len];
        memset(data,0,len);
        data[0] = 1;data[1] = 2;data[2] = 3;data[3] = 4;data[4] = 5;data[5] = 6;
        data[6] =sd->id;
        data[12] = sd->type;
        data[13] = 3;
        data[14] = 8;
        memcpy(data+16,buf,8);
        hexprint("snesor data 208 before encode",data,24);
        return slip_encode_208(data,len,slip);
  
}

int eof_to_slip_208(char *slip,int size)
{
        char data[100] = {0};
        int len = 24;
        memset(data,0,sizeof(data));
        data[0] = 1;data[1] = 2;data[2] = 3;data[3] = 4;data[4] = 5;data[5] = 6;
        data[6] = 1;
        data[12] = 1;
        data[13] = 5;
        data[14] = 8;

        return slip_encode_208(data,len,slip);
}

int protocal208_cmd_to_slip(struct protocal208_cmd *cmd,char *slip,int size)
{
        int len = 3+ 8;            //dvid dvtype cmd_type;
        if(2*len > size)
                return -1;
        char data[len];
        memset(data,0,sizeof(data));
        data[0] = cmd->device_id;
        data[1] = cmd->device_type;
        data[2] = 3;
        memcpy(data+3,cmd->data,8);
        return slip_encode(data,len,slip);
}

int slip_to_protocal208_cmd(struct protocal208_cmd *cmd,const char *slip,int len)
{
        char data[len];
        memset(data,0,len);

        int r = slip_decode(slip,len,data);
        if(r < 4)
                return -1;
        cmd->device_id = data[6];
        cmd->device_type = data[12];
        cmd->cmd = data[13];
        memcpy(cmd->data,data+16,8);
        return 0;
}



void protocal208_cmd_debug(struct protocal208_cmd *cmd)
{
        printf("cmd:%d\tdevice_id:%d\n",cmd->cmd,cmd->device_id);
}

int protocal208_sd2data(struct sensor_data *sd,char *buf,int len)
{
        if(len >8)
                len = 8;
        memcpy(buf,sd->data,len);
        return len;
}
