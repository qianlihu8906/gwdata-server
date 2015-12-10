#ifndef SDLIST_H__
#define SDLIST_H__

#include "adlist.h"
#include "protocal.h"

list *sdlist_create();
int sdlist_push(list *sdl,struct sensor_data *sd);
struct sensor_data *sdlist_pop(list *sdl);
int sdlist_check_push(list *sdl,struct sensor_data *sd);
void sdlist_check_over_time(list *sdl,int sec);
struct sensor_data *sdlist_find_by_id(list *sdl,int id);
void sdlist_debug(list *sdl);

#endif
