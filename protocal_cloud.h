#ifndef PROTOCAL_CLOUD_H__
#define PROTOCAL_CLOUD_H__

#

int sensor_data_to_pkg(struct sensor_data *sd,char *pkg,int size);

int pkg_to_sensor_data(struct sensor_data *sd,const char *pkg,int len);


#endif
