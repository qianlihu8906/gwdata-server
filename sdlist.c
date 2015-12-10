#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include "adlist.h"
#include "protocal.h"


static void _sensor_data_release(void *ptr)
{
        sensor_data_release(ptr);
}

static int _sensor_data_match_id(void *ptr,void *key)
{
        int *id = key;
        return sensor_data_match_id(ptr,*id);
}

list *sdlist_create()
{
        list *l = listCreate();
        l->match = _sensor_data_match_id;
        l->free = _sensor_data_release;
        return l;
}

int sdlist_push(list *sdl,struct sensor_data *sd)
{
        struct sensor_data *sdata = sensor_data_dup(sd);
        if(sdata == NULL)
                return -1;

        sdl = listAddNodeTail(sdl,sdata);

        return sdl==NULL? -1:1;
}

struct sensor_data *sdlist_pop(list *sdl)
{
        listNode *node = sdl->head;
        struct sensor_data *sd = listNodeValue(node);
        if(sd == NULL)
                return NULL;
        struct sensor_data *ret = sensor_data_dup(sd);
        listDelNode(sdl,node);

        return ret;
}

int sdlist_check_push(list *sdl,struct sensor_data *sd)
{

        struct sensor_data *sdata = sensor_data_dup(sd);
        if(sdata == NULL)
                return -1;

        listNode *sdn = listSearchKey(sdl,&sd->id);
        if(sdn == NULL){
                sdl = listAddNodeTail(sdl,sdata);
                return sdl==NULL? -1:1;
        }
        listNodeValue(sdn) = sdata;
        return 0;
}

void sdlist_check_over_time(list *sdl,int sec)
{

        listNode *node;
        listIter *iter;
        
        time_t now = time(NULL);
        iter = listGetIterator(sdl,AL_START_HEAD);
        while( (node=listNext(iter)) != NULL ){
                struct sensor_data *sd = listNodeValue(node);
                if( (now-sd->timestamp) > sec )
                        listDelNode(sdl,node);
        }
        listReleaseIterator(iter);
}

struct sensor_data *sdlist_find_by_id(list *sdl,int id)
{
        
        listNode *node = listSearchKey(sdl,&id);
        if(node == NULL)
                return NULL;

        return listNodeValue(node);
}


void sdlist_debug(list *sdl)
{

        printf("----length=%lu----\n",listLength(sdl));

        listNode *node;
        listIter *iter;

        iter = listGetIterator(sdl,AL_START_HEAD);
        while( (node=listNext(iter)) != NULL ){
                struct sensor_data *sd = listNodeValue(node);
                sensor_data_debug(sd);
        }
        listReleaseIterator(iter);
}
