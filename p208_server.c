#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>


#include <unistd.h>
#include <time.h>

#include "gw.h"
#include "json_server.h"
#include "buffer.h"
#include "seriport.h"
#include "cJSON.h"
#include "debug.h"
#include "protocal_208.h"

struct gw_p208_client{
        uint64_t id;
        int fd;
        struct buffer *recvbuf;
};

static struct gw_p208_client* gw_p208_client_create(uint64_t id,int fd);
static void gw_p208_client_release(struct gw_p208_client *c);
static void p208_read_handler(aeEventLoop *el,int fd,void *client_data,int mask);

static void p208_read_handler(aeEventLoop *el,int fd,void *privdata,int mask)
{
        struct gw_p208_client *c = privdata;

        int nread = buffer_read_append(c->recvbuf,fd);
        if(nread == -1){
                if(errno == EAGAIN){
                        return;
                }else{
                        fprintf(stdout,"Read from 208 client:%s\n",strerror(errno));
                        gw_p208_client_release(c);
                        return;
                }
        }else if(nread == 0){
                fprintf(stdout,"208 client closed connection\n");
                gw_p208_client_release(c);
                return;
        }
        char buf[512] = {0};
        int r;
        struct sensor_data *sd;
        struct protocal208_cmd cmd;
        const char *transfer_type = NULL;
        struct gwseriport *seriport;
        while((r=buffer_read_slip(c->recvbuf,buf,sizeof(buf))) >= 0){
                if(r > 10){
                        hexprint("recv from 208 client",buf,r);
                        int res = slip_to_protocal208_cmd(&cmd,buf,r);
                        if(res < 0)
                                continue;
                        printf("cmd:%d\tdevice_id:%d\t\n",cmd.cmd,cmd.device_id);
                        if(cmd.cmd == CMD_QUERY||cmd.cmd == CMD_QUERY_HISTORY){
                                
                                sd = sdlist_find_by_id(server.global_sensor_data,cmd.device_id);
                                if(sd == NULL){
                                        printf("无此传感器数据 device_id:%d\n",cmd.device_id);
                                        if(cmd.cmd == CMD_QUERY_HISTORY){
                                                int l = eof_to_slip_208(buf,sizeof(buf));
                                                write(c->fd,buf,l);
                                                continue;
                                        }
                                }else{
                                        printf("有此传感器数据 device_id:%d\n",cmd.device_id);
                                        int l = sensor_data_to_slip_208(sd,buf,sizeof(buf));
                                        printf("l=%d\n",l);
                                        hexprint("sensor data encode",buf,l);
                                        if(l > 0){
                                                printf("返回给208 client device_id:%d\n",cmd.device_id);
                                                write(c->fd,buf,l);
                                                continue;
                                        }
                                }
                        }
                        if(cmd.cmd == CMD_SET){
                                printf("cmd:%d\tdevice_id:%d\t-------------------------\n",cmd.cmd,cmd.device_id);
                                sd = sdlist_find_by_id(server.global_sensor_data,cmd.device_id);
                                transfer_type = "zigbee";
                                if(sd)
                                        transfer_type = sd->transfer_type;
                                
                                seriport = find_transfer_media(transfer_type);
                                int l = protocal208_cmd_to_slip(&cmd,buf,sizeof(buf));
                                if(l > 0)
                                        write_seriport(seriport,buf,l);
                        }
                }
        }

}


static struct gw_p208_client* gw_p208_client_create(uint64_t id,int fd)
{
        struct gw_p208_client *c = malloc(sizeof(*c));
        if(c == NULL)
                return NULL;
        anetNonBlock(NULL,fd);
        c->recvbuf = buffer_create(1024);
        if(server.tcpkeepalive)
                anetKeepAlive(NULL,fd,server.tcpkeepalive);
        if(aeCreateFileEvent(server.el,fd,AE_READABLE,p208_read_handler,c) == AE_ERR){
                close(fd);
                buffer_release(c->recvbuf);
                free(c);
                return NULL;
        }
        c->id = server.p208_next_client_id++;
        c->fd = fd;
        
        listAddNodeTail(server.p208_clients,c);
        return c;
}
static void gw_p208_client_release(struct gw_p208_client *c)
{
        listNode *ln = listSearchKey(server.p208_clients,c);
        assert(ln != NULL);
        listDelNode(server.p208_clients,ln);

        aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
        aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
        close(c->fd);
        buffer_release(c->recvbuf);
        free(c);
}
void p208_server_acceptHandler(aeEventLoop *el,int fd,void *privdata,int mask)
{
        int max = 1000;
        int cfd;
        char buf[100];

        int cport;
        while(max--){
                cfd = anetTcpAccept(server.p208_neterr,fd,buf,sizeof(buf),&cport);
                if(cfd == ANET_ERR){
                        if(errno != EWOULDBLOCK)
                                printf("208 Server error:%s\n",server.p208_neterr);
                        return;
                }
                printf("P208 Server Accepted %s:%d\n",buf,cport);
                gw_p208_client_create(server.p208_next_client_id++,cfd);
                
        }
}
