#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>

#include "gw.h"
#include "buffer.h"
#include "seriport.h"
#include "uuid_dvid.h"
#include "protocal.h"
#include "debug.h"

struct gw_cloud_client{
        uint64_t id;
        int fd;
        struct buffer *recvbuf;
};

static int heart_to_cloud(char *cloud,int size)
{
        const char *uuid = uuid_dvid_find_heartuuid();
        int len = LENGTH_UUID + 2 + 1;
        if(len > size)
                return -1;
        memcpy(cloud,uuid,LENGTH_UUID);
        cloud[LENHI_INDEX] = (len&0xffff) >> 8;
        cloud[LENLO_INDEX] = len & 0xff;
        cloud[TYPE_INDEX] = REQ_HEARTBEAT;
        return len;
}
static void read_from_platform(aeEventLoop *el,int fd,void *client_data,int mask);

int gw_cloud_broadcast(struct sensor_data *sd)
{
        char buf[512] = {0};
        struct gw_cloud_client *c;
        listNode *node;

        int r = sensor_data_to_cloud(sd,buf,sizeof(buf));
        if(r < 0)
                return -1;

        listIter *iter = listGetIterator(server.cloud_clients,AL_START_HEAD);

        while( (node=listNext(iter)) != NULL ){
                c = node->value;
                write(c->fd,buf,r);
        }
        listReleaseIterator(iter);

        return 0;
        
}

static struct gw_cloud_client *gw_cloud_client_create(uint64_t id,int fd)
{
        struct gw_cloud_client *c = malloc(sizeof(*c));
        if(c == NULL)
                return NULL;
        anetNonBlock(NULL,fd);
        c->recvbuf = buffer_create(1024);
        if(server.tcpkeepalive)
                anetKeepAlive(NULL,fd,server.tcpkeepalive);
        if(aeCreateFileEvent(server.el,fd,AE_READABLE,read_from_platform,c) == AE_ERR){
                close(fd);
                buffer_release(c->recvbuf);
                free(c);
                return NULL;
        }

        c->id = server.cloud_next_client_id++;
        c->fd = fd;
        listAddNodeTail(server.cloud_clients,c);

        return c;
}

static void gw_cloud_client_release(struct gw_cloud_client *c)
{
        listNode *ln = listSearchKey(server.cloud_clients,c);
        assert(ln != NULL);

        listDelNode(server.cloud_clients,ln);

        aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
        aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
        close(c->fd);
        buffer_release(c->recvbuf);
        free(c);
}

static void read_from_platform(aeEventLoop *el,int fd,void *client_data,int mask)
{
        struct gw_cloud_client *c = client_data;
        int nread = buffer_read_append(c->recvbuf,fd);
        if(nread == -1){
                if(errno == EAGAIN){
                        return;
                }else{
                        fprintf(stdout,"Read from cloud: %s\n",strerror(errno));
                        gw_cloud_client_release(c);
                        return;
                }
        }else if(nread == 0){
                fprintf(stdout,"cloud  closed connection\n");
                gw_cloud_client_release(c);
                return;
        }

        char buf[512] = {0};
        char type;
        int r = buffer_read_cloud(c->recvbuf,buf,sizeof(buf));
        if(r > 0){
                type = buf[TYPE_INDEX];
                if( (type&0x01) == 0){
                        buf[TYPE_INDEX] = type + 1;
                        buf[LENHI_INDEX] = 0;
                        buf[LENLO_INDEX] = LENGTH_UUID + 2 + 1;
                        write(fd,buf,buf[LENLO_INDEX]);
                }
                int dvid = uuid_dvid_find_dvid(buf);
                if(dvid < 0){
                        return;
                }
                struct sensor_data *sd = sdlist_find_by_id(server.global_sensor_data,dvid);
                const char *transfer_type = "zigbee";
                if(sd)
                        transfer_type = sd->transfer_type;
                struct gwseriport *s = find_transfer_media(transfer_type);
                struct sensor_data *sd1;
                int r;
                char slip[256] = {0};

                switch(type){    //type
                        case REQ_SWITCH_ON:
                                sd1 = sensor_data_create(sd->id,sd->type,"true",transfer_type);
                                r = sensor_data_to_slip(sd1,slip,sizeof(slip));
                                if(r > 0){
                                        write_seriport(s,slip,r);
                                }
                                sensor_data_release(sd1);
                                break;
                        case REQ_SWITCH_OFF:
                                sd1 = sensor_data_create(sd->id,sd->type,"false",transfer_type);
                                r = sensor_data_to_slip(sd1,slip,sizeof(slip));
                                if(r > 0){
                                        write_seriport(s,slip,r);
                                }
                                sensor_data_release(sd1);
                                break;
                        default:
                                break;
                }
        }
}

static int gw_cloud_heartbeat(struct aeEventLoop *el,long long id,void *client_data)
{
        struct gw_cloud_client *c = client_data;
        char buf[100] = {0};
        int r = heart_to_cloud(buf,sizeof(buf));
        if(r > 0){
                write(c->fd,buf,r);
        }
        return HEART_TIME;
}

void gw_cloud_new_client(aeEventLoop *el,int fd,void *client_data,int mask)
{
        struct gw_cloud_client *c = gw_cloud_client_create(server.cloud_next_client_id++,fd);
        aeCreateTimeEvent(server.el,HEART_TIME,gw_cloud_heartbeat,c,NULL);
        aeDeleteFileEvent(el,fd,AE_WRITABLE);
}

