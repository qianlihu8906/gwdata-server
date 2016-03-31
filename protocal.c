#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include "debug.h"
#include "devices.h"
#include "protocal.h"
#include "uuid_dvid.h"
#include "cJSON.h"


struct sensor_data *sensor_data_create(int id,int type,cJSON *value,const char *transfer_type)
{
        struct sensor_data *sd = malloc(sizeof(*sd));
        assert(sd != NULL);

        sd->id = id;
        sd->type = type;
        sd->value = cJSON_Duplicate(value,1);
        assert(sd->value != NULL);
        
        sd->transfer_type = strdup(transfer_type);
        assert(sd->transfer_type != NULL);
        
        time(&sd->timestamp);
        struct tm *tmp = localtime(&sd->timestamp);
        sprintf(sd->asctime,"%d-%02d-%02d %02d:%02d:%02d",tmp->tm_year+1900,tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
        return sd;
}

static struct sensor_data *sensor_data_create_with_208(int id,int type,cJSON *value,const char *transfer_type,const char *data,int len)
{
        struct sensor_data *sd = sensor_data_create(id,type,value,transfer_type);
        if(len > 8)                   //208 need it,we will forget it
                len = 8;
        memset(sd->data,0,8);
        memcpy(sd->data,data,len);
        return sd;
}

void sensor_data_release(struct sensor_data *sd)
{
        cJSON_Delete(sd->value);
        free(sd->transfer_type);
        free(sd);
}

int sensor_data_match_id(struct sensor_data *sd1,int id)
{
        return (sd1->id == id);
}

struct sensor_data *sensor_data_dup(struct sensor_data *sd)
{
        return sensor_data_create_with_208(sd->id,sd->type,sd->value,sd->transfer_type,sd->data,8);
}

void sensor_data_debug(struct sensor_data *sd)
{
        printf("device_id=%d\t,device_type=%d\n",sd->id,sd->type);
        char *json = cJSON_PrintUnformatted(sd->value);
        printf("value(p)=%s(%p)\t",json,sd->value);
        free(json);
        printf("transfer_type=%s\t",sd->transfer_type);
        printf("time:%s\n",sd->asctime);
}

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

static int slip_decode(const char *src,int len_src,char *dest) //返回值不包含校验和
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

static const char *transfer_types[] = {"zigbee","wifi","ipv6","bluetooth"};

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))

static int find_transfertype(const char *str)
{
        int i;
        for(i=0;i<ARRAY_SIZE(transfer_types);i++){
                if(strcasecmp(transfer_types[i],str) == 0){ 
                        return i;
                }
        }

        return 0;
}

int sensor_data_to_slip(struct sensor_data *sd,char *slip,int size)
{
        char buf[100] ={0};
        int r = device_v2chararray(sd->id,sd->type,sd->value,buf,sizeof(buf));
        if(r < 0){
                printf("device_v2chararray error\n");
                sensor_data_debug(sd);
                return -1;
        }
        int len = 3+r; 			// dvid  dvtype cmd_type

        if(2*len > size)                
                return -1;

        char data[len];
        data[0] = sd->id;
        data[1] = sd->type;
        data[2] = ( (find_transfertype(sd->transfer_type) << 4)&0xf0 ) | 0x03;
        memcpy(data+3,buf,r);

        return slip_encode(data,len,slip);
  
}

int sensor_data_to_cloud(struct sensor_data *sd,char *cloud,int size)
{
        const char *uuid = uuid_dvid_find_uuid(sd->id);
        if(uuid == NULL)
                return -1;
        char buf[100] = {0};
        int r = device_v2cloud(sd->id,sd->type,sd->value,buf,sizeof(buf));
        if(r < 0)
                return -1;
        
        int len = LENGTH_UUID + 2 + 1 + r;
        if(len > size)
                return -1;
        memcpy(cloud,uuid,LENGTH_UUID);
        cloud[16] = (len&0xffff) >> 8;
        cloud[17] = len&0xff;
        cloud[18] = REQ_DATA;

        memcpy(cloud+19,buf,r);

        return len;
}


struct sensor_data *slip_to_sensor_data(const char *slip,int len)
{
        char data[len];
        int r = slip_decode(slip,len,data);
        printf("decode r=%d\n",r);
        if(r < 3){                            // dvid,type,transfer_type
                return NULL;
        }
        int dvid = (unsigned char)data[0];
        int dvtype =(unsigned char)data[1];
        int data_len = r - 3;
        int transfer_type = (data[2]&0xf0) >> 4;
        if(transfer_type < 0||transfer_type >= (ARRAY_SIZE(transfer_types))){
                return NULL;
        }
        const char *transfer_type_str = transfer_types[transfer_type];

        cJSON *value = device_v2json(dvid,dvtype,data+3,data_len);
        if(value == NULL){
                return NULL;
        }

        struct sensor_data* sd = sensor_data_create_with_208(dvid,dvtype,value,transfer_type_str,data+3,data_len);
        cJSON_Delete(value);

        return sd;
}
