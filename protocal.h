#ifndef PROTOCAL_H__
#define PROTOCAL_H__
#include <cJSON.h>
struct sensor_data{
        int id;
        int type;
        cJSON *value;
        char data[8];   //just for ms208
        char *transfer_type;
        char asctime[20];
        time_t timestamp;
};

struct sensor_data *sensor_data_create(int id,int type,cJSON *value,const char *transfer_type);
int sensor_data_init(struct sensor_data *sd,int id,cJSON *value,const char *transfer_type);
void sensor_data_release(struct sensor_data *sd);
int sensor_data_match_id(struct sensor_data *sd1,int id);
struct sensor_data *sensor_data_dup(struct sensor_data *sd);
void sensor_data_debug(struct sensor_data *sd);
int sensor_data_to_slip(struct sensor_data *sd,char *slip,int size);
struct sensor_data *slip_to_sensor_data(const char *slip,int len);

int sensor_data_to_cloud(struct sensor_data *sd,char *cloud,int size);
#endif
