#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>

#include "devices.h"
#include "debug.h"

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))

static cJSON *temp_v2json(int id,const char *data,int len)
{
        int v = data[1];
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"%d",v);
        return cJSON_CreateString(buf);
}

static int temp_v2chararray(int id,cJSON *value,char *buf,int len)
{
        return -1;
}

static int temp_v2cloud(int id,cJSON *value,char *buf,int len)
{
        int v = atoi(value->valuestring);
        buf[0] = v;
        return 1;
}

static cJSON *light_v2json(int id,const char *data,int len)
{
        int16_t v = *(int16_t*)data;
        v = le16toh(v);
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"%d",v);
        return cJSON_CreateString(buf);
}

static int light_v2chararray(int id,cJSON *value,char *buf,int len)
{
        return -1;
}

static int light_v2cloud(int id,cJSON *value,char *buf,int len)
{
        uint16_t v = atoi(value->valuestring);
        v = htobe16(v);
        memcpy(buf,&v,sizeof(v));

        return sizeof(v);
}

static cJSON *led_v2json(int id,const char *data,int len)
{
        int v = data[0];
        if(v == 1){
                return cJSON_CreateString("true");
        }else{
                return cJSON_CreateString("false");
        }
}

static int led_v2chararray(int id,cJSON *value,char *buf,int len)
{
        switch(value->type){
                case cJSON_False:
                        buf[0] = 3;
                        break;
                case cJSON_True:
                        buf[0] = 1;
                        break;
                case cJSON_String:
                        if(strcasecmp(value->valuestring,"true") == 0)
                                buf[0] = 1;
                        else
                                buf[0] = 3;
                        break;
                default:
                        break;
        }
        return 8;
}

static int led_v2cloud(int id,cJSON *value,char *buf,int len)
{
        switch(value->type){
                case cJSON_False:
                        buf[0] = 3;
                        break;
                case cJSON_True:
                        buf[0] = 1;
                        break;
                case cJSON_String:
                        if(strcasecmp(value->valuestring,"true"))
                                buf[0] = 1;
                        else
                                buf[0] = 3;
                        break;
                default:
                        break;
        }
        return 1;
}

static cJSON *acceleration_v2json(int id,const char *data,int len)
{
         signed short int Ax = data[0]|data[1]<<8;
         signed short int Ay = data[2]|data[3]<<8;
         signed short int Az = data[4]|data[5]<<8;

         signed short int Gx = data[6]|data[7]<<8;
         signed short int Gy = data[8]|data[9]<<8;
         signed short int Gz = data[10]|data[11]<<8;
         /*
         cJSON *value = cJSON_CreateObject();
         cJSON_AddNumberToObject(value,"Ax",Ax);
         cJSON_AddNumberToObject(value,"Ay",Ay);
         cJSON_AddNumberToObject(value,"Az",Az);
         cJSON_AddNumberToObject(value,"Gx",Gx);
         cJSON_AddNumberToObject(value,"Gy",Gy);
         cJSON_AddNumberToObject(value,"Gz",Gz);
         */
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"Ax:%d,Ay:%d,Az:%d,Gx:%d,Gy:%d,Gz:%d",Ax,Ay,Az,Gx,Gy,Gz);

         return cJSON_CreateString(buf);
}

static int acceleration_v2chararray(int id,cJSON *value,char *buf,int len)
{
        return -1;       
}

static int acceleration_v2cloud(int id,cJSON *value,char *buf,int len)
{
        return 0;
}


static cJSON *magnetic_v2json(int id,const char *data,int len)
{
        signed short int X = data[0]|data[1]<<8;
        signed short int Y = data[2]|data[3]<<8;
        signed short int Z = data[4]|data[5]<<8;
        /*
        cJSON *value = cJSON_CreateObject();
        cJSON_AddNumberToObject(value,"X",X);
        cJSON_AddNumberToObject(value,"Y",Y);
        cJSON_AddNumberToObject(value,"Z",Z);
        */
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"X:%d,Y:%d,Z:%d",X,Y,Z);

        return cJSON_CreateString(buf);
}

static int magnetic_v2chararray(int id,cJSON *value,char *buf,int len)
{
        return -1;
}

static int magnetic_v2cloud(int id,cJSON *value,char *buf,int len)
{
        return -1;
}

cJSON *rfid_v2json(int id,const char *data,int len)
{
        int32_t value = *(int32_t*)data; 
        value = le32toh(value); 
        return cJSON_CreateNumber(value);
}

static int rfid_v2chararray(int id,cJSON *value,char *buf,int len)
{
        return -1;
}
static int rfid_v2cloud(int id,cJSON *value,char *buf,int len)
{
        int32_t v = atoi(value->valuestring);
        v = htole32(v);
        memcpy(buf,&v,sizeof(v));
        return sizeof(v);
}
static cJSON *temp_and_humi_v2json(int id,const char *data,int len)
{
        int humi = (unsigned char)data[0];
        int temp = (unsigned char)data[1];
        cJSON *value = cJSON_CreateObject();
        cJSON_AddNumberToObject(value,"humidity",humi);
        cJSON_AddNumberToObject(value,"temperature",temp);


        return value;
}

static int temp_and_humi_v2chararray(int id,cJSON *value,char *buf,int len)
{
        return -1;
}

static int temp_and_humi_v2cloud(int id,cJSON *value,char *buf,int len)
{
        return -1;
}

static cJSON *closet_v2json(int id,const char *data,int len)
{
        int v = data[0];
        switch(v){
                case 1:
                        return cJSON_CreateString("left");
                case 2:
                        return cJSON_CreateString("right");
                case 3:
                        return cJSON_CreateString("stop");
                default:
                        return cJSON_CreateString("closet unknown");
        }
}
static int closet_v2chararray(int id,cJSON *value,char *buf,int len)
{
        if(strcasecmp(value->valuestring,"left") == 0){
                buf[0] = 1;
        }else if(strcasecmp(value->valuestring,"right") == 0){
                buf[0] = 2;
        }else{
                buf[0] = 3;
        }
        return 8;
}
static int closet_v2cloud(int id,cJSON *value,char *buf,int len)
{
        if(strcasecmp(value->valuestring,"left") == 0){
                buf[0] = 1;
        }else if(strcasecmp(value->valuestring,"right") == 0){
                buf[0] = 2;
        }else{
                buf[0] = 3;
        }
        return 1;
}

#if 0
static const char *xueya_v2string(const char *data,int len,char *strbuf,int size)
{
        if(data[0] == 0x5B){
                strcpy(strbuf,"sleep");
        }else if(data[0] == 0x5A){
                strcpy(strbuf,"wakeup");
        }else if(data[0] == 0x54){
                snprintf(strbuf,size,"QYH:%02x,QYL:%02x",data[1],data[2]);
        }else if(data[0] == 0x55){
                snprintf(strbuf,size,"SSYH:%02x,SSYL:%02x,SZYH:%02x,SZYL:%02x,XL:%02x",data[1],data[2],data[3],data[4],data[5]);
        }else if(data[0] == 0x56){
                snprintf(strbuf,size,"X:%02x",data[1]);
        }else{
                strcpy(strbuf,"error");
        }
        return strbuf;
}

static int xueya_v2charray(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"sleep") == 0){
                buf[0] = 0xAB;
                return 1;
        }else if(strcasecmp(str,"wakeup") == 0){
                buf[0] = 0xAA;
                return 1;
        }else if(strcasecmp(str,"start") == 0){
                buf[0] = 0xA0;
                return 1;
        }else if(strcasecmp(str,"stop") == 0){
                buf[0] = 0xA3;
                return 1;
        }else{
                buf[0] = 0xAA;
                return 1;
        }
}

static int xueya_v2cloud(const char *str,char *buf,int len)
{
        buf[0] = 0;
        return 1;
}

static const char *maibo_v2string(const char *data,int len,char *strbuf,int size)
{
        if(data[0] == 0xA0){
                snprintf(strbuf,size,"MBH:%02x,MBL:%02x",data[1],data[2]);
        }else if(data[0] == 0xA1){
                snprintf(strbuf,size,"stop");
        }else if(data[0] == 0xA4){
                snprintf(strbuf,size,"adjust success");
        }else if(data[0] == 0xA2){
                snprintf(strbuf,size,"SN0:%02x,SN1:%02x,SN2:%02x,SN3:%02x",data[1],data[2],data[3],data[4]);
        }else if(data[0] == 0xA3){
                snprintf(strbuf,size,"T1:%02x,T2:%02x,T3:%02x,T4:%02x",data[1],data[2],data[3],data[4]);
        }else{
                snprintf(strbuf,size,"error");
        }
        return strbuf;
}

static int maibo_v2charray(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"start") == 0){
                buf[0] = 0xA0;
                return 1;
        }else if(strcasecmp(str,"stop") == 0){
                buf[0] = 0xA1;
                return 1;
        }else if(strcasecmp(str,"read_id") == 0){
                buf[0] = 0xA2;
                return 1;
        }else if(strcasecmp(str,"read_date") == 0){
                buf[0] = 0xA3;
                return 1;
        }else if(strstr(str,"adjust_mb") != NULL){
                char *start = strchr(str,':') + 1;
                int x = atoi(start);
                if(x > 0 && x < 16){
                        buf[0] = 0xA4;
                        buf[1] = x;
                        return 2;
                }
                return 0;
        }else{
                buf[0] = 0xAA;
                return 1;
        }
        
}

static int maibo_v2cloud(const char *str,char *buf,int len)
{
        buf[0] = 0;
        return 1;
}

static const char *xindian_v2string(const char *data,int len,char *strbuf,int size)
{
        if(data[0] == 0xA0){
                snprintf(strbuf,size,"MBH:%02x,MBL:%02x",data[1],data[2]);
        }else if(data[0] == 0xA1){
                snprintf(strbuf,size,"stop");
        }else if(data[0] == 0xA2){
                snprintf(strbuf,size,"SN0:%02x,SN1:%02x,SN2:%02x,SN3:%02x",data[1],data[2],data[3],data[4]);
        }else if(data[0] == 0xA3){
                snprintf(strbuf,size,"T1:%02x,T2:%02x,T3:%02x,T4:%02x",data[1],data[2],data[3],data[4]);
        }else{
                snprintf(strbuf,size,"error");
        }
        return strbuf;

}

static int xindian_v2charray(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"start") == 0){
                buf[0] = 0xA0;
                return 1;
        }else if(strcasecmp(str,"stop") == 0){
                buf[0] = 0xA1;
                return 1;
        }else if(strcasecmp(str,"read_id") == 0){
                buf[0] = 0xA2;
                return 1;
        }else if(strcasecmp(str,"read_date") == 0){
                buf[0] = 0xA3;
                return 1;
        }else{
                buf[0] = 0xAA;
                return 1;
        }
        
}

static int xindian_v2cloud(const char *str,char *buf,int len)
{
        buf[0] = 0;
        return 1;
}

static const char *tiwen_v2string(const char *data,int len,char *strbuf,int size)
{
        if(data[0] == 0xA0){
                snprintf(strbuf,size,"TWH:%02x,TWL:%02x",data[1],data[2]);
        }else if(data[0] == 0xA1){
                snprintf(strbuf,size,"stop");
        }else if(data[0] == 0xA2){
                snprintf(strbuf,size,"SN0:%02x,SN1:%02x,SN2:%02x,SN3:%02x",data[1],data[2],data[3],data[4]);
        }else if(data[0] == 0xA3){
                snprintf(strbuf,size,"T1:%02x,T2:%02x,T3:%02x,T4:%02x",data[1],data[2],data[3],data[4]);
        }else if(data[0] == 0xCA){
                snprintf(strbuf,size,"adjust_up success");
        }else if(data[0] == 0xCD){
                snprintf(strbuf,size,"adjust_down success");
        }else{
                snprintf(strbuf,size,"error");
        }
        return strbuf;

}

static int tiwen_v2charray(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"start") == 0){
                buf[0] = 0xA0;
                return 1;
        }else if(strcasecmp(str,"stop") == 0){
                buf[0] = 0xA1;
                return 1;
        }else if(strcasecmp(str,"read_id") == 0){
                buf[0] = 0xA2;
                return 1;
        }else if(strcasecmp(str,"read_date") == 0){
                buf[0] = 0xA3;
                return 1;
        }else if(strstr(str,"adjust_up") != NULL){
                char *start = strchr(str,':') + 1;
                int x = atoi(start);
                buf[0] = 0xA4;
                buf[1] = x;
                return 2;
        }else if(strstr(str,"adjust_down") != NULL){
                char *start = strchr(str,':') + 1;
                int x = atoi(start);
                buf[0] = 0xA4;
                buf[1] = x;
                return 2;
        }else{
                buf[0] = 0xAA;
                return 1;
        }
        

}

static int tiwen_v2cloud(const char *str,char *buf,int len)
{
        buf[0] = 0;
        return 1;
}

static const char *xueyang_v2string(const char *data,int len,char *strbuf,int size)
{
        if(data[0] == 0xA0){
                snprintf(strbuf,size,"MB:%02x,XY:%02x,XL:%02x",data[1],data[2],data[3]);
        }else if(data[0] == 0xA1){
                snprintf(strbuf,size,"stop");
        }else if(data[0] == 0xA2){
                snprintf(strbuf,size,"SN0:%02x,SN1:%02x,SN2:%02x,SN3:%02x",data[1],data[2],data[3],data[4]);
        }else if(data[0] == 0xA3){
                snprintf(strbuf,size,"T1:%02x,T2:%02x,T3:%02x,T4:%02x",data[1],data[2],data[3],data[4]);
        }else{
                snprintf(strbuf,size,"error");
        }
        return strbuf;
}

static int xueyang_v2chararray(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"start") == 0){
                buf[0] = 0xA0;
                return 1;
        }else if(strcasecmp(str,"stop") == 0){
                buf[0] = 0xA1;
                return 1;
        }else if(strcasecmp(str,"read_id") == 0){
                buf[0] = 0xA2;
                return 1;
        }else if(strcasecmp(str,"read_date") == 0){
                buf[0] = 0xA3;
                return 1;
        }else{
                buf[0] = 0xAA;
                return 1;
        }
        
}

static int xueyang_v2cloud(const char *str,char *buf,int len)
{
        buf[0] = 0;
        return 1;
}
#endif

#if 0  //{食品溯源
static const char *lcd_v2string(const char *data,int len,char *strbuf,int size)
{
	strcpy(strbuf,"error");
        return strbuf;
}
static int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
        
        iconv_t cd;
        char **pin = &inbuf;
        char **pout = &outbuf;
        cd = iconv_open(to_charset,from_charset);
        if(cd == 0)
                return -1;
        memset(outbuf,0,outlen);
        if(iconv(cd,pin,&inlen,pout,&outlen) == -1)
                return -1;
        iconv_close(cd);

        return 0;

}

static int u2g(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
        return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}


static int lcd_v2chararray(const char *str,char *buf,int len)
{
        char *instr = strdup(str);
        if(instr == NULL)
                return 0;
        int inlen = strlen(instr);
        int rc = u2g(instr,inlen,buf,len);
        free(instr);
        if(rc == -1){
                return 0;
        }
        rc = strlen(buf);

        return rc;
}
#endif // 食品溯源}


#if 0   //交通沙盘{
static const char *traffic_light_v2string(const char *data,int len,char *strbuf,int size)
{
        int  sn = data[0];
        int  ew = data[1];
        snprintf(strbuf,size,"%d,%d",sn,ew);
        return strbuf;
}



#endif //交通沙盘}

struct devices
{
        int type; 
        cJSON* (*v2json)(int id,const char *data,int len);
        int (*v2chararray)(int id,cJSON *value,char *buf,int size);
        int (*v2cloud)(int id,cJSON *value,char *buf,int len);
};


static struct  devices devices[] = {
        {0x10,temp_v2json,temp_v2chararray,temp_v2cloud}, //温度
        {0x11,temp_v2json,temp_v2chararray,temp_v2cloud}, //湿度
        {0x12,light_v2json,light_v2chararray,light_v2cloud}, //光照
        {0x16,light_v2json,light_v2chararray,light_v2cloud}, //气压
        {0x13,light_v2json,light_v2chararray,light_v2cloud}, //可燃气体
        {0x1d,light_v2json,light_v2chararray,light_v2cloud}, //烟雾
        {0x1a,light_v2json,light_v2chararray,light_v2cloud}, //二氧化碳
        {0x18,led_v2json,led_v2chararray,led_v2cloud},     //继电器
        {0x14,led_v2json,led_v2chararray,led_v2cloud},     //人体红外
        {0x22,led_v2json,led_v2chararray,led_v2cloud},     //红外反射
        {0x23,led_v2json,led_v2chararray,led_v2cloud},     //触摸按键
        {0x24,led_v2json,led_v2chararray,led_v2cloud},     //声音
        {0x25,led_v2json,led_v2chararray,led_v2cloud},     //雨滴
        {0x26,led_v2json,led_v2chararray,led_v2cloud},     //火焰
        {0x27,led_v2json,led_v2chararray,led_v2cloud},     //震动
        {0x29,rfid_v2json,rfid_v2chararray,rfid_v2cloud},     //震动
        {0x15,acceleration_v2json,acceleration_v2chararray,acceleration_v2cloud}, //加速度
        {0x20,magnetic_v2json,magnetic_v2chararray,magnetic_v2cloud},  //磁场
        {0x29,rfid_v2json,rfid_v2chararray,rfid_v2cloud},
        {0x41,temp_and_humi_v2json,temp_and_humi_v2chararray,temp_and_humi_v2cloud},
	{0x43,light_v2json,light_v2chararray,light_v2cloud}, //ph
        {0x2A,closet_v2json,closet_v2chararray,closet_v2cloud},
        
};

static struct devices *find_device(int device_type)
{
        int i;
        for(i=0;i<ARRAY_SIZE(devices);i++){
                if(device_type == devices[i].type)
                        return &devices[i];
        }
        return NULL;
}

cJSON* device_v2json(int id,int type,const char *data,int len)
{
        struct devices *d = find_device(type);
        if(d == NULL){
                return NULL;
        }

        return d->v2json(id,data,len);
}

int device_v2chararray(int id,int type,cJSON *value,char *buf,int size)
{
        struct devices *d = find_device(type);

        if(d == NULL)
                return -1;

        return d->v2chararray(id,value,buf,size);
}
int device_v2cloud(int id,int type,cJSON *value,char *buf,int size)
{
        struct devices *d = find_device(type);
        if(d == NULL)
                return -1;
        return d->v2cloud(id,value,buf,size);
}
