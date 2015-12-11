#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "adlist.h"
#include "uuid_dvid.h"
#include "debug.h"
#include "protocal.h"


struct map{
        char uuid[LENGTH_UUID];
        int dvid;
};

static int hexstring2char(const char *start)
{
        char tmp = start[0];
        char hi,lo;
        if( (tmp >= '0') && (tmp <= '9') )
                hi = tmp - '0';
        else if( (tmp>= 'a') && (tmp <='f') )
                hi = tmp - 'a' + 10;
        else if( (tmp>='A') && (tmp <='F') )
                hi = tmp - 'A' + 10;
        else
                return -1;

        tmp = start[1];
        if( (tmp >= '0') && (tmp <= '9') )
                lo = tmp - '0';
        else if( (tmp>= 'a') && (tmp <='f') )
                lo = tmp - 'a' + 10;
        else if( (tmp>='A') && (tmp <='F') )
                lo = tmp - 'A' + 10;
        else
                return -1;
        return (hi << 4|lo);
}

int uuid_dvid_string2uuid(const char *string,char *uuid)
{
        int i,r;
        const char *p = string;
        for(i=0;i<LENGTH_UUID;i++){
                r = hexstring2char(p);
                if(r < 0)
                        return -1;
                uuid[i] = r;
                p += 2;
        }
        return LENGTH_UUID;
}

static list *L = NULL;

static struct map *map_create(const char *uuid,int dvid)
{
        struct map *m = malloc(sizeof(struct map));
        int i;
        for(i=0;i<LENGTH_UUID;i++){
                m->uuid[i] = uuid[i];
        }
        m->dvid = dvid;

        return m;
}

static void map_release(struct map *m)
{
        free(m);
}

static void _map_release(void *ptr)
{
        return map_release(ptr);
}

static int map_match_dvid(struct map *m,int dvid)
{
        return m->dvid == dvid;
}

static int map_match_uuid(struct map *m,const char *uuid)
{
        int i;
        for(i=0;i<LENGTH_UUID;i++){
                if(m->uuid[i] != uuid[i])
                        return 0;
        }
        return 1;
}

static int _map_match_dvid(void *ptr,void *key)
{
        int *id = key;
        return map_match_dvid(ptr,*id);
}

static int _map_match_uuid(void *ptr,void *key)
{
        return map_match_uuid(ptr,key);
}

int uuid_dvid_init()
{
        if(L == NULL){
                L  = listCreate();
                L->free = _map_release;
        }
        return 0;
}

static listNode *node_find_by_uuid(list *l,char *uuid)
{
        l->match = _map_match_uuid;
        
        return listSearchKey(l,uuid);
}

static listNode *node_find_by_dvid(list *l,int dvid)
{
        l->match = _map_match_dvid;
        
        return listSearchKey(l,&dvid);
}

int uuid_dvid_add_record(const char *uuid,int dvid)
{

        listNode *n = node_find_by_dvid(L,dvid);
        if(n == NULL){
                struct map *m = map_create(uuid,dvid);
                listAddNodeTail(L,m);
        }else{
                struct map *m = listNodeValue(n);
                m->dvid = dvid;
                memcpy(m->uuid,uuid,LENGTH_UUID);
        }

        return 0;
}

int uuid_dvid_add_record_string(const char *uuidstring,int dvid)
{
        char uuid[LENGTH_UUID];
        uuid_dvid_string2uuid(uuidstring,uuid);

        return uuid_dvid_add_record(uuid,dvid);
}

int uuid_dvid_find_dvid(char *uuid)
{
        list *l = L;
        listNode *n = node_find_by_uuid(l,uuid);
        if( n == NULL)
                return -1;
        struct map *m = n->value;
        return m->dvid;
}

const char *uuid_dvid_find_uuid(int dvid)
{
        list *l = L;
        listNode *n = node_find_by_dvid(l,dvid);
        if(n == NULL)
                return NULL;

        struct map *m = n->value;
        return m->uuid;

}

const char *uuid_dvid_find_heartuuid()
{
        list *l = L;
        listNode *n = l->head;
        if(n == NULL)
                return NULL;
        struct map *m = n->value;
        return m->uuid;
}

void uuid_dvid_del_uuid(char *uuid)
{
        list *l = L;
        listNode *n = node_find_by_uuid(l,uuid);

        return listDelNode(l,n);
}

void uuid_dvid_del_dvid(int dvid)
{
        list *l = L;
        listNode *n = node_find_by_dvid(l,dvid);

        return listDelNode(l,n);
}

void uuid_dvid_debug()
{
        list *l = L;
        listNode *node;
        listIter *iter = listGetIterator(l,AL_START_HEAD);

        while( (node=listNext(iter)) != NULL ){
                struct map *m = listNodeValue(node);
                printf("dvid=%d\t",m->dvid);
                hexprint("uuid:",m->uuid,LENGTH_UUID);
        }
}

struct pkg_cloud{
        char uuid[LENGTH_UUID];
        int paylen;
        char type;
        char payload[];
};

struct pkg_cloud *pkg_cloud_create(const char *buf,int len)
{
        int header_len = LENGTH_UUID + 2 + 1;
        if(len < header_len)
                return NULL;
        int length = buf[16]*256 + buf[17];
        int payload_len = length - header_len;
        struct pkg_cloud *p = malloc(sizeof(*p) + payload_len);

        p->paylen = payload_len;
        p->type = buf[18];
        memcpy(p->uuid,buf,LENGTH_UUID);
        memcpy(p->payload,buf+19,payload_len);
        return p;
}

void pkg_cloud_release(struct pkg_cloud *p)
{
        free(p);
}

void pkg_cloud_debug(struct pkg_cloud *p)
{
        printf("--------pkg_cloud----------\n");
        hexprint("uuid",p->uuid,LENGTH_UUID);
        printf("length=%d,type=%d\n",p->paylen,p->type);
        hexprint("payload",p->payload,p->paylen);
        printf("--------end----------\n");
}

int pkg_cloud_to_pkg(struct pkg_cloud *p,char *buf,int size)
{
        int len = p->paylen + LENGTH_UUID + 2 + 1;
        if(len > size)
                return -1;
        memcpy(buf,p->uuid,LENGTH_UUID);
        buf[16] = (len >> 8) & 0xff;
        buf[17] =  len & 0xff;
        buf[18] = p->type;

        memcpy(buf+19,p->payload,p->paylen);

        return len;
}
