#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "devices.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static cJSON *car_v2json(int id,const unsigned char *data,int len)
{
	int function = (unsigned char)data[0];
	char buf[128] = {0};
	printf("Enter %s\n",__func__);
	switch (data[0]){
		case 0x01:{	//小车状态
			int value = (unsigned char)data[1];
			snprintf(buf,sizeof(buf),"%d,%d",function,value);}
			break;
		case 0x02:{	//小车速度
			int value = (unsigned char)data[1];
			snprintf(buf,sizeof(buf),"%d,%d",function,value);}
			break;
		case 0x03:{	//小车GPS定位信息
			int coordinate_x = (unsigned char)data[1];
			int coordinate_y = (unsigned char)data[2];
			snprintf(buf,sizeof(buf),"%d,%d,%d",function,coordinate_x,coordinate_y);}
			break;
		case 0x04:{	//RFID卡号
			int32_t card_id;
			card_id = data[1]<<24|data[2]<<16|data[3]<<8|data[4];
			snprintf(buf,sizeof(buf),"%d,%u",function,card_id);}
			break;
		default:
        		break;
	}
	printf("exit %s\n",__func__);
        return cJSON_CreateString(buf);
}

static int car_v2chararray(int id,cJSON *value,unsigned char *buf,int len)
{
        printf("Enter %s\n",__func__);
	memset(buf,0,2);
	char bufer[10] = {0};
	strcpy(bufer,value->valuestring);
	char *car_value;
	car_value = strtok(bufer,",");
	buf[0]=atoi(car_value);
	car_value = strtok(NULL,",");
	buf[1]=atoi(car_value);
	printf("exit %s\n",__func__);
	return 2;
}

static int car_v2cloud(int id,cJSON *value,unsigned char *buf,int len)
{
        int v = atoi(value->valuestring);
        buf[0] = v;
        return 1;
}

static cJSON *temp_v2json(int id, const unsigned char *data, int len) {
        int v = data[1];
        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "%d", v);
        return cJSON_CreateString(buf);
}

static int temp_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static int temp_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        int v = atoi(value->valuestring);
        buf[0] = v;
        return 1;
}

static cJSON *light_v2json(int id, const unsigned char *data, int len) {
        int16_t v = *(int16_t *)data;
        v = le16toh(v);
        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "%d", v);
        return cJSON_CreateString(buf);
}

static int light_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) // NOTICE  p208 need it
{
        return -1;
}

static int light_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        uint16_t v = atoi(value->valuestring);
        v = htobe16(v);
        memcpy(buf, &v, sizeof(v));

        return sizeof(v);
}

static cJSON *motor_v2json(int id,const unsigned char *data,int len)
{
        int v_1 = data[0];
	int v_2 = data[1];
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"%d,%d",v_1,v_2);
        return cJSON_CreateString(buf);
}

static int motor_v2chararray(int id,cJSON *value,unsigned char *buf,int len)
{
        memset(buf,0,2);
	char bufer[10] = {0};
	strncpy(bufer,value->valuestring,sizeof(bufer)-1);
	char *tmp;
	tmp = strtok(bufer,",");
	buf[0]=atoi(tmp);
	tmp = strtok(NULL,",");
	if(tmp == NULL)
		return 1;
	buf[1]=atoi(tmp);
	return 2;
}

static int motor_v2cloud(int id,cJSON *value,unsigned char *buf,int len)
{
	
        return -1;
}

static cJSON *display_v2json(int id,const unsigned char *data,int len)
{
	signed short int v = data[0]|data[1]<<8;
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"%d",v);
        return cJSON_CreateString(buf);
}

static int display_v2chararray(int id,cJSON *value,unsigned char *buf,int len)
{
	int v = atoi(value->valuestring);
        buf[0] = v & 0xFF;
	buf[1] = v >> 8;
        return 2;
}

static int display_v2cloud(int id,cJSON *value,unsigned char *buf,int len)
{
        return -1;
}

static cJSON *led_v2json(int id, const unsigned char *data, int len) {
        int v = data[0];
        if (v == 1) {
                return cJSON_CreateString("true");
        } else {
                return cJSON_CreateString("false");
        }
}

static int led_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        switch (value->type) {
                case cJSON_False:
                        buf[0] = 3;
                        break;
                case cJSON_True:
                        buf[0] = 1;
                        break;
                case cJSON_String:
                        if (strcasecmp(value->valuestring, "true") == 0)
                                buf[0] = 1;
                        else
                                buf[0] = 3;
                        break;
                default:
                        break;
        }
        return 8;
}

static int led_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        switch (value->type) {
                case cJSON_False:
                        buf[0] = 3;
                        break;
                case cJSON_True:
                        buf[0] = 1;
                        break;
                case cJSON_String:
                        if (strcasecmp(value->valuestring, "true"))
                                buf[0] = 1;
                        else
                                buf[0] = 3;
                        break;
                default:
                        break;
        }
        return 1;
}

static cJSON *acceleration_v2json(int id, const unsigned char *data, int len) {
        signed short int Ax = data[0] | data[1] << 8;
        signed short int Ay = data[2] | data[3] << 8;
        signed short int Az = data[4] | data[5] << 8;

        signed short int Gx = data[6] | data[7] << 8;
        signed short int Gy = data[8] | data[9] << 8;
        signed short int Gz = data[10] | data[11] << 8;

        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "Ax:%d,Ay:%d,Az:%d,Gx:%d,Gy:%d,Gz:%d", Ax, Ay, Az,
                        Gx, Gy, Gz);

        return cJSON_CreateString(buf);
}

static int acceleration_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        return -1;
}

static int acceleration_v2cloud(int id, cJSON *value, unsigned char *buf,
                int len) {
        return 0;
}

static cJSON *magnetic_v2json(int id, const unsigned char *data, int len) {
        signed short int X = data[0] | data[1] << 8;
        signed short int Y = data[2] | data[3] << 8;
        signed short int Z = data[4] | data[5] << 8;

        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "X:%d,Y:%d,Z:%d", X, Y, Z);

        return cJSON_CreateString(buf);
}

static int magnetic_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        return -1;
}

static int magnetic_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

cJSON *rfid_v2json(int id, const unsigned char *data, int len) {
        int32_t value = *(int32_t *)data;
        value = le32toh(value);
        char buf[128] = {0};
        snprintf(buf,sizeof(buf),"%u",value);
        return cJSON_CreateString(buf);
}

static int rfid_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}
static int rfid_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        int32_t v = atoi(value->valuestring);
        v = htole32(v);
        memcpy(buf, &v, sizeof(v));
        return sizeof(v);
}
static cJSON *temp_and_humi_v2json(int id, const unsigned char *data, int len) {
        int humi = (unsigned char)data[0];
        int temp = (unsigned char)data[1];
        cJSON *value = cJSON_CreateObject();
        cJSON_AddNumberToObject(value, "humidity", humi);
        cJSON_AddNumberToObject(value, "temperature", temp);

        return value;
}

static int temp_and_humi_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        return -1;
}

static int temp_and_humi_v2cloud(int id, cJSON *value, unsigned char *buf,
                int len) {
        return -1;
}

static cJSON *closet_v2json(int id, const unsigned char *data, int len) {
        int v = data[0];
        switch (v) {
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
static int closet_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        if (strcasecmp(value->valuestring, "left") == 0) {
                buf[0] = 1;
        } else if (strcasecmp(value->valuestring, "right") == 0) {
                buf[0] = 2;
        } else {
                buf[0] = 3;
        }
        return 8;
}
static int closet_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        if (strcasecmp(value->valuestring, "left") == 0) {
                buf[0] = 1;
        } else if (strcasecmp(value->valuestring, "right") == 0) {
                buf[0] = 2;
        } else {
                buf[0] = 3;
        }
        return 1;
}

static cJSON *heart_v2json(int id, const unsigned char *data, int len) {
        int status = data[0];
        int freq = data[1];

        if (status == 2)
                return cJSON_CreateString("testing");
        char buf[100] = {0};
        snprintf(buf, sizeof(buf), "%d", freq);
        return cJSON_CreateString(buf);
}

static int heart_v2chararay(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static int heart_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static cJSON *tiwen_v2json(int id, const unsigned char *data, int len) {
        if (data[0] == 0xA0) {
                int v = data[1] * 256 + data[2];
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "TW", v);
                return value;
        } else if (data[0] == 0xA1) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "stop");
                return value;
        } else if (data[0] == 0xA2) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "SN0", data[1]);
                cJSON_AddNumberToObject(value, "SN1", data[2]);
                cJSON_AddNumberToObject(value, "SN2", data[3]);
                cJSON_AddNumberToObject(value, "SN3", data[4]);
                return value;
        } else if (data[0] == 0xA3) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "T1", data[1]);
                cJSON_AddNumberToObject(value, "T2", data[2]);
                cJSON_AddNumberToObject(value, "T3", data[3]);
                cJSON_AddNumberToObject(value, "T4", data[4]);
        } else if (data[0] == 0xCA) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "adjust up success");
                return value;
        } else if (data[0] == 0xCD) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "adjust down success");
                return value;
        } else if (data[0] == 0x5A) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "online");
                return value;
        }
        cJSON *value = cJSON_CreateObject();
        cJSON_AddStringToObject(value, "status", "error");
        return value;
}

static int tiwen_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        const char *str = value->valuestring;
        if (strcasecmp(str, "start") == 0) {
                buf[0] = 0xA0;
                return 1;
        } else if (strcasecmp(str, "stop") == 0) {
                buf[0] = 0xA1;
                return 1;
        } else if (strcasecmp(str, "read_id") == 0) {
                buf[0] = 0xA2;
                return 1;
        } else if (strcasecmp(str, "read_date") == 0) {
                buf[0] = 0xA3;
                return 1;
        } else if (strstr(str, "adjust_up") != NULL) {
                char *start = strchr(str, ':') + 1;
                int x = atoi(start);
                buf[0] = 0xA4;
                buf[1] = x;
                return 2;
        } else if (strstr(str, "adjust_down") != NULL) {
                char *start = strchr(str, ':') + 1;
                int x = atoi(start);
                buf[0] = 0xA4;
                buf[1] = x;
                return 2;
        } else {
                buf[0] = 0xAA;
                return 1;
        }
}

static int tiwen_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        buf[0] = 0;
        return 1;
}

static cJSON *xueyang_v2json(int id, const unsigned char *data, int size) {
        if (data[0] == 0xA0) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "MB", data[1]);
                cJSON_AddNumberToObject(value, "XY", data[2]);
                cJSON_AddNumberToObject(value, "XL", data[3]);
                return value;
        } else if (data[0] == 0xA1) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "stop");
                return value;
        } else if (data[0] == 0xA2) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "SN0", data[1]);
                cJSON_AddNumberToObject(value, "SN1", data[2]);
                cJSON_AddNumberToObject(value, "SN2", data[3]);
                cJSON_AddNumberToObject(value, "SN3", data[4]);
                return value;
        } else if (data[0] == 0xA3) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "T1", data[1]);
                cJSON_AddNumberToObject(value, "T2", data[2]);
                cJSON_AddNumberToObject(value, "T3", data[3]);
                cJSON_AddNumberToObject(value, "T4", data[4]);
                return value;
        } else if (data[0] == 0x5A) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "online");
                return value;
        } else {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "error");
                return value;
        }
}

static int xueyang_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        const char *str = value->valuestring;
        if (strcasecmp(str, "start") == 0) {
                buf[0] = 0xA0;
                return 1;
        } else if (strcasecmp(str, "stop") == 0) {
                buf[0] = 0xA1;
                return 1;
        } else if (strcasecmp(str, "read_id") == 0) {
                buf[0] = 0xA2;
                return 1;
        } else if (strcasecmp(str, "read_date") == 0) {
                buf[0] = 0xA3;
                return 1;
        } else {
                buf[0] = 0xAA;
                return 1;
        }
}

static int xueyang_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        buf[0] = 0;
        return 1;
}

static cJSON *xueya_v2json(int id, const unsigned char *data, int len) {
        if (data[0] == 0x5B) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "stop");
                return value;
        } else if (data[0] == 0x5A) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddStringToObject(value, "status", "online");
                return value;
        } else if (data[0] == 0x54) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "QYH", data[1]);
                cJSON_AddNumberToObject(value, "QYL", data[2]);
                return value;
        } else if (data[0] == 0x55) {
                cJSON *value = cJSON_CreateObject();
                int shuzhang = data[3] * 256 + data[4];
                int shousuo = (data[1] & 0x7f) * 256 + data[2];
                cJSON_AddNumberToObject(value, "shuzhang", shuzhang);
                cJSON_AddNumberToObject(value, "shousuo", shousuo);
                cJSON_AddNumberToObject(value, "XL", data[5]);
                return value;
        } else if (data[0] == 0x56) {
                cJSON *value = cJSON_CreateObject();
                cJSON_AddNumberToObject(value, "X", data[1]);
                return value;
        }
        cJSON *value = cJSON_CreateObject();
        cJSON_AddStringToObject(value, "status", "error");
        return value;
}

static int xueya_v2chararray(int id, cJSON *value, unsigned char *buf,
                int len) {
        const char *str = value->valuestring;
        if (strcasecmp(str, "sleep") == 0) {
                buf[0] = 0xAB;
                return 1;
        } else if (strcasecmp(str, "wakeup") == 0) {
                buf[0] = 0xAA;
                return 1;
        } else if (strcasecmp(str, "start") == 0) {
                buf[0] = 0xA0;
                return 1;
        } else if (strcasecmp(str, "stop") == 0) {
                buf[0] = 0xA3;
                return 1;
        } else {
                buf[0] = 0xAA;
                return 1;
        }
}

static int xueya_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        buf[0] = 0;
        return 1;
}
/*环境监测*/
static cJSON *environment_v2json(int id, const unsigned char *data, int len) {
	int16_t v = *(int16_t *)data;
        v = be16toh(v);
        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "%.1f", v/10.0);
        return cJSON_CreateString(buf);
}

static int environment_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static int environment_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        uint16_t v = atoi(value->valuestring);
        v = htobe16(v);
        memcpy(buf, &v, sizeof(v));

        return sizeof(v);
}
/*环境监测--bool量*/
static cJSON *environment_b_v2json(int id, const unsigned char *data, int len) {
	int16_t v = *(int16_t *)data;
	if(v == 0x0000)
        	return cJSON_CreateString("false");
	else
		return cJSON_CreateString("true");
}

static int environment_b_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static int environment_b_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        uint16_t v = atoi(value->valuestring);
        v = htobe16(v);
        memcpy(buf, &v, sizeof(v));

        return sizeof(v);
}

/*环境监测--大气压*/
static cJSON *environment_p_v2json(int id, const unsigned char *data, int len) {
	int16_t v = *(int16_t *)data;
        v = be16toh(v);
        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "%d", v);
        return cJSON_CreateString(buf);
}

static int environment_p_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static int environment_p_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        uint16_t v = atoi(value->valuestring);
        v = htobe16(v);
        memcpy(buf, &v, sizeof(v));

        return sizeof(v);
}

/*浇花*/
static cJSON *flowers_v2json(int id, const unsigned char *data, int len) {
	int v = (int)data[1];
        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "%d", v);
        return cJSON_CreateString(buf);
}

static int flowers_v2chararray(int id, cJSON *value, unsigned char *buf, int len) {
        return -1;
}

static int flowers_v2cloud(int id, cJSON *value, unsigned char *buf, int len) {
        uint16_t v = atoi(value->valuestring);
        v = htobe16(v);
        memcpy(buf, &v, sizeof(v));

        return sizeof(v);
}

struct devices {
        int type;
        cJSON *(*v2json)(int id, const unsigned char *data, int len);
        int (*v2chararray)(int id, cJSON *value, unsigned char *buf, int size);
        int (*v2cloud)(int id, cJSON *value, unsigned char *buf, int len);
};

static struct devices devices[] = {
	{0x09, flowers_v2json, flowers_v2chararray, flowers_v2cloud},       //浇花土壤温度
        {0x10, temp_v2json, temp_v2chararray, temp_v2cloud},    //温度
        {0x11, temp_v2json, temp_v2chararray, temp_v2cloud},    //湿度
        {0x12, light_v2json, light_v2chararray, light_v2cloud}, //光照
	{0x13, light_v2json, light_v2chararray, light_v2cloud}, //可燃气体
	{0x14, led_v2json, led_v2chararray, led_v2cloud},       //人体红外
	{0x15, acceleration_v2json, acceleration_v2chararray,acceleration_v2cloud},  //六轴
        {0x16, light_v2json, light_v2chararray, light_v2cloud}, //气压
	{0x18, led_v2json, led_v2chararray, led_v2cloud},       //继电器
	{0x19,motor_v2json,motor_v2chararray,motor_v2cloud},	//直流电机
	{0x1a, light_v2json, light_v2chararray, light_v2cloud}, //二氧化碳
        {0x1d, light_v2json, light_v2chararray, light_v2cloud}, //烟雾
	{0x1f,display_v2json,display_v2chararray,display_v2cloud}, //数码管
	{0x20, magnetic_v2json, magnetic_v2chararray, magnetic_v2cloud}, //磁场
	{0x21,motor_v2json,motor_v2chararray,motor_v2cloud},	//步进电机
        {0x22, led_v2json, led_v2chararray, led_v2cloud},       //红外反射
        {0x23, led_v2json, led_v2chararray, led_v2cloud},       //触摸按键
        {0x24, led_v2json, led_v2chararray, led_v2cloud},       //声音
        {0x25, led_v2json, led_v2chararray, led_v2cloud},       //雨滴
        {0x26, led_v2json, led_v2chararray, led_v2cloud},       //火焰
        {0x27, led_v2json, led_v2chararray, led_v2cloud},       //震动
	{0x28, rfid_v2json, rfid_v2chararray, rfid_v2cloud},    // 125读卡器
        {0x29, rfid_v2json, rfid_v2chararray, rfid_v2cloud},    // 13.56读卡器
	{0x2A, closet_v2json, closet_v2chararray, closet_v2cloud},    //衣柜
	{0x2C, flowers_v2json, flowers_v2chararray, flowers_v2cloud}, //土壤湿度
	{0x30, environment_v2json, environment_v2chararray, environment_v2cloud},       //空气温度
	{0x31, environment_v2json, environment_v2chararray, environment_v2cloud},       //空气湿度
	{0x34, environment_p_v2json, environment_p_v2chararray, environment_p_v2cloud}, //大气压力
	{0x35, environment_v2json, environment_v2chararray, environment_v2cloud},       //风速
	{0x36, environment_v2json, environment_v2chararray, environment_v2cloud},       //风向
	{0x37, environment_b_v2json, environment_b_v2chararray, environment_b_v2cloud}, //雨雪
	{0x38, environment_v2json, environment_v2chararray, environment_v2cloud},       //PM2.5
	{0x39, motor_v2json,motor_v2chararray,motor_v2cloud},	      //浇花水泵
        {0x3A, heart_v2json, heart_v2chararay, heart_v2cloud},        // 心率
        {0x41, temp_and_humi_v2json, temp_and_humi_v2chararray,temp_and_humi_v2cloud},//温湿度
	{0x42, xueyang_v2json, xueyang_v2chararray, xueyang_v2cloud}, //血氧
        {0x43, light_v2json, light_v2chararray, light_v2cloud},       // ph
        {0x44, tiwen_v2json, tiwen_v2chararray, tiwen_v2cloud},       //体温
        {0x45, xueya_v2json, xueya_v2chararray, xueya_v2cloud},       //血压
	{0xA1, car_v2json,car_v2chararray,car_v2cloud},//小车
};

static struct devices *find_device(int device_type) {
        int i;
        for (i = 0; i < ARRAY_SIZE(devices); i++) {
                if (device_type == devices[i].type)
                        return &devices[i];
        }
        return NULL;
}

cJSON *device_v2json(int id, int type, const char *data, int len) {
        struct devices *d = find_device(type);
        if (d == NULL) {
                return NULL;
        }

        return d->v2json(id, (const unsigned char *)data, len);
}

int device_v2chararray(int id, int type, cJSON *value, char *buf, int size) {
        struct devices *d = find_device(type);

        if (d == NULL)
                return -1;

        return d->v2chararray(id, value, (unsigned char *)buf, size);
}
int device_v2cloud(int id, int type, cJSON *value, char *buf, int size) {
        struct devices *d = find_device(type);
        if (d == NULL)
                return -1;
        return d->v2cloud(id, value, (unsigned char *)buf, size);
}
