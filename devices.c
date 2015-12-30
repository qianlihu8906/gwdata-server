#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>

#include "devices.h"
#include "debug.h"

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))


static const char *temp_v2string(const char *data,int len,char *strbuf,int size)
{
        int value = data[1];
        snprintf(strbuf,size,"%d",value);
        return strbuf;
}

static int temp_v2chararray(const char *str,char *buf,int len)
{
        int value = atoi(str);
        memset(buf,0,8);
        buf[1] = value;
        return 8;
}

static int temp_v2cloud(const char *str,char *buf,int len)
{
        int value = atoi(str);
        buf[0] = value;
        return 1;
}

static const char *light_v2string(const char *data,int len,char *strbuf,int size) //本质上是将数组转换为字符串
{
        int16_t value = *(int16_t*)data; //转换成16位数据，并将第一个数据赋值给value
        le16toh(value); //将16位小端字节序转换为主机字节序
        snprintf(strbuf,size,"%d",value);
        return strbuf;
}

static int light_v2chararray(const char *str,char *buf,int len) //本质上是将字符串转为数组
{
        int16_t value = atoi(str);
        memset(buf,0,8);
        htole16(value);
        memcpy(buf,&value,sizeof(value));
        return 8;
}

static int light_v2cloud(const char *str,char *buf,int len)
{
        uint16_t value = atoi(str);
        uint16_t v = htobe16(value);
        memcpy(buf,&v,sizeof(v));

        return sizeof(v);
}

static const char* led_v2string(const char *data,int len,char *strbuf,int size)
{
        int v = data[0];
        if(v == 1){
                strcpy(strbuf,"true");
        }else{
                strcpy(strbuf,"false");
        }
        return strbuf;
}

static int led_v2chararray(const char* str,char *buf,int len)
{
        if(strcasecmp(str,"true") == 0){
                buf[0] = 1;
        }else{
                buf[0] = 3;
        }
        return 8;
}

static int led_v2cloud(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"true") == 0){
                buf[0] = 1;
        }else{
                buf[0] = 3;
        }
        return 1;
}
static const char *acceleration_v2string(const char *data,int len,char *strbuf,int size)
{
         signed short int Ax = data[0]|data[1]<<8;
         signed short int Ay = data[2]|data[3]<<8;
         signed short int Az = data[4]|data[5]<<8;

         signed short int Gx = data[6]|data[7]<<8;
         signed short int Gy = data[8]|data[9]<<8;
         signed short int Gz = data[10]|data[11]<<8;
         snprintf(strbuf,size,"Ax:%hd,Ay:%d,Az:%hd,Gx:%hd,Gy:%hd,Gz:%hd",Ax,Ay,Az,Gx,Gy,Gz);
         
         return strbuf;
}

static int acceleration_v2chararray(const char* str,char *buf,int len)
{
        return 0;       
}

static int acceleration_v2cloud(const char *str,char *buf,int len)
{
        return 0;
}

static const char * magnetic_v2string(const char *data,int len,char *strbuf,int size)
{
        signed short int X = data[0]|data[1]<<8;
        signed short int Y = data[2]|data[3]<<8;
        signed short int Z = data[4]|data[5]<<8;
        snprintf(strbuf,size,"X:%hd,Y:%hd,Z:%hd",X,Y,Z);
        return strbuf;
}

static int magnetic_v2chararray(const char* str,char *buf,int len)
{
        const char *start;
        char *end;
        char **pend = &end;
        if(str){
                start = strchr(str,':') + 1;
                int16_t x = strtol(start,pend,10);
                start = strchr(end,':') + 1;
                int16_t y = strtol(start,pend,10);
                start = strchr(end,':') + 1;
                int16_t z = strtol(start,pend,10);
                
                htole16(x);htole16(y);htole16(z);

                memcpy(buf,&x,2);
                memcpy(buf+2,&y,2);
                memcpy(buf+4,&z,2);
        }
        return 8;
}

static int magnetic_v2cloud(const char *str,char *buf,int len)
{
        return 0;
}

static const char *rfid_v2string(const char *data,int len,char *strbuf,int size)
{
        int32_t value = *(int32_t*)data; //转换成32位数据，并将第一个数据赋值给value
        le32toh(value); //将32位小端字节序转换为主机字节序
        snprintf(strbuf,size,"%u",value);
        return strbuf;
}

static int rfid_v2chararray(const char *str,char *buf,int len)
{
        return 0;
}
static int rfid_v2cloud(const char *str,char *buf,int len)
{
        int32_t value = atoi(str);
        uint32_t v = htole32(value);
        memcpy(buf,&v,sizeof(v));
        return sizeof(v);
}
static const char *temp_and_humi_v2string(const char *data,int len,char *strbuf,int size)
{
        int humi = (unsigned char)data[0];
        int temp = (unsigned char)data[1];

        snprintf(strbuf,size,"%d,%d",humi,temp);

        return strbuf;
}

static int temp_and_humi_v2chararray(const char *str,char *buf,int len)
{
        return 0;
}

static int temp_and_humi_v2cloud(const char *str,char *buf,int len)
{
        return 0;
}

static const char *closet_v2string(const char *data,int len,char *strbuf,int size)
{
        int v = data[0];
        switch(v){
                case 1:
                        snprintf(strbuf,size,"left");
                        break;
                case 2:
                        snprintf(strbuf,size,"right");
                        break;
                case 3:
                        snprintf(strbuf,size,"stop");
                        break;
                default:
                        snprintf(strbuf,size,"closet unknown");
                        break;
        }
        return strbuf;
}
static int closet_v2charray(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"left") == 0){
                buf[0] = 1;
        }else if(strcasecmp(str,"right") == 0){
                buf[0] = 2;
        }else{
                buf[0] = 3;
        }
        return 8;
}
static int closet_v2cloud(const char *str,char *buf,int len)
{
        if(strcasecmp(str,"left") == 0){
                buf[0] = 1;
        }else if(strcasecmp(str,"right") == 0){
                buf[0] = 2;
        }else{
                buf[0] = 3;
        }
        return 1;
}

#if 1
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

struct devices
{
        int type; 
        const char *(*v2string)(const char *data,int len,char *strbuf,int size);
        int (*v2chararray)(const char *str,char *buf,int len);
        int (*v2cloud)(const char *str,char *buf,int len);
};


static struct  devices devices[] = {
        {0x10,temp_v2string,temp_v2chararray,temp_v2cloud}, //温度
        {0x11,temp_v2string,temp_v2chararray,temp_v2cloud}, //湿度
        {0x12,light_v2string,light_v2chararray,light_v2cloud}, //光照
        {0x16,light_v2string,light_v2chararray,light_v2cloud}, //气压
        {0x13,light_v2string,light_v2chararray,light_v2cloud}, //可燃气体
        {0x1d,light_v2string,light_v2chararray,light_v2cloud}, //烟雾
        {0x1a,light_v2string,light_v2chararray,light_v2cloud}, //二氧化碳
        {0x18,led_v2string,led_v2chararray,led_v2cloud},     //继电器
        {0x14,led_v2string,led_v2chararray,led_v2cloud},     //人体红外
        {0x22,led_v2string,led_v2chararray,led_v2cloud},     //红外反射
        {0x23,led_v2string,led_v2chararray,led_v2cloud},     //触摸按键
        {0x24,led_v2string,led_v2chararray,led_v2cloud},     //声音
        {0x25,led_v2string,led_v2chararray,led_v2cloud},     //雨滴
        {0x26,led_v2string,led_v2chararray,led_v2cloud},     //火焰
        {0x27,led_v2string,led_v2chararray,led_v2cloud},     //震动
        {0x29,rfid_v2string,rfid_v2chararray,rfid_v2cloud},     //震动
        {0x15,acceleration_v2string,acceleration_v2chararray,acceleration_v2cloud}, //加速度
        {0x20,magnetic_v2string,magnetic_v2chararray,magnetic_v2cloud},  //磁场
        {0x29,rfid_v2string,rfid_v2chararray,rfid_v2cloud},
        {0x41,temp_and_humi_v2string,temp_and_humi_v2chararray,temp_and_humi_v2cloud},
//        {0x42,lcd_v2string,lcd_v2chararray},
	{0x43,light_v2string,light_v2chararray,light_v2cloud}, //ph
        {0x2A,closet_v2string,closet_v2charray,closet_v2cloud},
        {0x44,tiwen_v2string,tiwen_v2charray,tiwen_v2cloud},
        
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

const char *device_v2string(int id,int type,const char *data,int len,char *strbuf,int size)
{
        struct devices *d = find_device(type);
        if(d == NULL)
                return "unknown";
        return d->v2string(data,len,strbuf,size);
}

int device_v2chararray(int id,int type,const char *str,char *buf,int size)
{
        struct devices *d = find_device(type);

        if(d == NULL)
                return -1;

        return d->v2chararray(str,buf,size);
}
int device_v2cloud(int id,int type,const char *str,char *buf,int size)
{
        struct devices *d = find_device(type);
        if(d == NULL)
                return -1;
        return d->v2cloud(str,buf,size);
}
