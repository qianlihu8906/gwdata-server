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
#include "uuid_dvid.h"
#include "debug.h"

struct gw_json_client{
        uint64_t id;
        int fd;
        int report;
        struct buffer *recvbuf;
};

static struct gw_json_client *gw_json_client_create(uint64_t id,int fd);
static void gw_json_client_release(struct gw_json_client *c);
static void readQueryFromClient(aeEventLoop *el,int fd,void *client_data,int mask);
static void json_server_process_line(struct gw_json_client *c,const char *str);


int json_server_broadcast_str(const char *buf)
{
        struct gw_json_client *c;
        
        listNode *node;
        listIter *iter = listGetIterator(server.json_clients,AL_START_HEAD);

        int len = strlen(buf);
        while( (node=listNext(iter)) != NULL ){
                c = node->value;
                if(c->report){
                        write(c->fd,buf,len);
                        write(c->fd,"\n",1);
                }
        }
        listReleaseIterator(iter);
        
        return 0;
}

static struct gw_json_client *gw_json_client_create(uint64_t id,int fd)
{
        struct gw_json_client *c = malloc(sizeof(*c));
        if(c == NULL)
                return NULL;
        anetNonBlock(NULL,fd);
        c->recvbuf = buffer_create(1024);
        if(server.tcpkeepalive)
                anetKeepAlive(NULL,fd,server.tcpkeepalive);
        if(aeCreateFileEvent(server.el,fd,AE_READABLE,readQueryFromClient,c) == AE_ERR){
                close(fd);
                buffer_release(c->recvbuf);
                free(c);
                return NULL;
        }

        c->id = server.json_next_client_id++;
        c->fd = fd;
        c->report = 0;

        listAddNodeTail(server.json_clients,c);

        return c;

}

static void gw_json_client_release(struct gw_json_client *c)
{
        listNode *ln = listSearchKey(server.json_clients,c);
        assert(ln != NULL);

        listDelNode(server.json_clients,ln);

        aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
        aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
        close(c->fd);
        buffer_release(c->recvbuf);
        free(c);
}

void json_server_acceptHandler(aeEventLoop *el,int fd,void *privdata,int mask)
{
        int max = 1000;
        int cfd;
        char buf[100];

        int cport;
        while(max--){
                cfd = anetTcpAccept(server.json_neterr,fd,buf,sizeof(buf),&cport);
                if(cfd == ANET_ERR){
                        if(errno != EWOULDBLOCK)
                                printf("Json Server error:%s\n",server.json_neterr);
                        return;
                }
                printf("JSON Server Accepted %s:%d\n",buf,cport);
                gw_json_client_create(server.json_next_client_id++,cfd);
                
        }
}
static void readQueryFromClient(aeEventLoop *el,int fd,void *client_data,int mask)
{
        struct gw_json_client *c = client_data;
 
        int nread = buffer_read_append(c->recvbuf,fd);
        if(nread == -1){
                if(errno == EAGAIN){
                        return;
                }else{
                        fprintf(stdout,"Read from json client: %s\n",strerror(errno));
                        gw_json_client_release(c);
                        return;
                }
        }else if(nread == 0){
                fprintf(stdout,"json client closed connection\n");
                gw_json_client_release(c);
                return;
        }


        char buf[512] = {0};
        while(buffer_read_line(c->recvbuf,buf,sizeof(buf)) >= 0){
                json_server_process_line(c,buf);
        }
        
        
}


static int cmd_request_push(struct gw_json_client *c,const char* cmd,cJSON *args,cJSON *reply)
{
        c->report = 1;
        cJSON_AddStringToObject(reply,"err_msg","success");

        return 0;
}

static int cmd_cancel_push(struct gw_json_client *c,const char *cmd,cJSON *args,cJSON *reply)
{
        c->report = 0;
        cJSON_AddStringToObject(reply,"err_msg","success");

        return 0;
}

static int cmd_query(struct gw_json_client *c,const char *cmd,cJSON *args,cJSON *reply)
{
        if(args == NULL){
                cJSON_AddStringToObject(reply,"err_msg","query args error");
                return -1;
        }

        cJSON *dvid = cJSON_GetObjectItem(args,"device_id");
        if(dvid == NULL){
                cJSON_AddStringToObject(reply,"err_msg","query need device id");
                return -1;
        }

	

        int device_id = dvid->valueint;

        struct sensor_data *sd = sdlist_find_by_id(server.global_sensor_data,device_id);
        if(sd == NULL){
                char error[100] = {0};
                snprintf(error,sizeof(error),"device_id error(%d)",device_id);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }
        
        cJSON* root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root,"device_id",sd->id);
        cJSON_AddStringToObject(root,"transfer_type",sd->transfer_type);
        cJSON_AddStringToObject(root,"device_value",sd->value);
        cJSON_AddStringToObject(root,"timestamp",sd->asctime);
        cJSON_AddItemToObject(reply,"ret_data",root);
        cJSON_AddStringToObject(reply,"err_msg","success");

        return 0;
}

static int cmd_set_sensor(struct gw_json_client *c,const char *cmd,cJSON *args,cJSON *reply)
{
        char error[100] = {0};

        if(args == NULL){
                snprintf(error,sizeof(error),"%s args error",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }

        cJSON *dvid = cJSON_GetObjectItem(args,"device_id");
        if(dvid == NULL){
                snprintf(error,sizeof(error),"%s need device_id",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }
        cJSON *dv_value = cJSON_GetObjectItem(args,"device_value");
        if(dv_value == NULL){
                snprintf(error,sizeof(error),"%s need device_value",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }

	int device_type = -1;
	cJSON *dvtype = cJSON_GetObjectItem(args,"device_type");

	if(dvtype){
                device_type  = dvtype->valueint;
           
        }

        int device_id = dvid->valueint;
        struct sensor_data *sd = sdlist_find_by_id(server.global_sensor_data,device_id);
        if(sd == NULL && device_type == -1){
                snprintf(error,sizeof(error),"device_id error(%d)",device_id);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }

	const char *transfer_type = NULL;
        cJSON *tftype = cJSON_GetObjectItem(args,"transfer_type");
        if(tftype){
                transfer_type = tftype->valuestring;       
        }
	
	if(sd){
                if(device_type == -1)
        		device_type = sd->type;
		transfer_type = sd->transfer_type;
	}
        if(transfer_type == NULL)
                transfer_type = "zigbee";
	
        struct gwseriport *s = find_transfer_media(transfer_type);
       	if(s == NULL){
                cJSON_AddStringToObject(reply,"err_msg","error ttypath");
                return -1;
        }
        char slip[256] = {0};  
        struct sensor_data *sd1 = sensor_data_create(device_id,device_type,dv_value->valuestring,transfer_type);


        int r = sensor_data_to_slip(sd1,slip,sizeof(slip));

        if(r < 0){
                cJSON_AddStringToObject(reply,"err_msg","unknown error");
                return -1;
        }
        cJSON_AddStringToObject(reply,"err_msg","success");
        sensor_data_release(sd1);

        write_seriport(s,slip,r);

        return 0;
}

static int cmd_add_uuid(struct gw_json_client *c,const char *cmd,cJSON *args,cJSON *reply)
{
        char error[100] = {0};

        if(args == NULL){
                snprintf(error,sizeof(error),"%s args error",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }

        cJSON *dvid = cJSON_GetObjectItem(args,"device_id");
        if(dvid == NULL){
                snprintf(error,sizeof(error),"%s need device_id",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }
        cJSON *uuid = cJSON_GetObjectItem(args,"uuid");
        if(uuid == NULL){
                snprintf(error,sizeof(error),"%s need uuid",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }
        uuid_dvid_init();
        int device_id = dvid->valueint;
        const char *uuid_str = uuid->valuestring;
        char uuid_char[100] = {0};
        int r = uuid_dvid_string2uuid(uuid_str,uuid_char);
        if( r < 0){
                snprintf(error,sizeof(error),"%s uuid error:%s",cmd,uuid_str);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }

        uuid_dvid_add_record(uuid_char,device_id);
        cJSON_AddStringToObject(reply,"err_msg","success");
        uuid_dvid_debug();

        return 0;

}

struct gw_cloud_client{
        uint64_t id;
        int fd;
        struct buffer *recvbuf;
};

static void read_from_platform(aeEventLoop *el,int fd,void *client_data,int mask);

int gw_cloud_broadcast(const char *buf,int len)
{
        struct gw_cloud_client *c;
        
        listNode *node;
        listIter *iter = listGetIterator(server.cloud_clients,AL_START_HEAD);

        while( (node=listNext(iter)) != NULL ){
                c = node->value;
                write(c->fd,buf,len);
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
#if 1
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
        int r = buffer_read_cloud(c->recvbuf,buf,sizeof(buf));
        if(r > 0){
                hexprint("recv cloud",buf,r);
                if( (buf[16]&0x01) == 0){
                        buf[16] = buf[16] + 1;
                        buf[17] = 0;
                        buf[18] = LENGTH_UUID + 2 + 1;
                        write(fd,buf,buf[18]);
                }
                int dvid = uuid_dvid_find_dvid(buf);
                if(dvid < 0)
                        return;
                struct sensor_data *sd = sdlist_find_by_id(server.global_sensor_data,dvid);
                const char *transfer_type = "zigbee";
                if(sd)
                        transfer_type = sd->transfer_type;
                struct gwseriport *s = find_transfer_media(transfer_type);
                struct sensor_data *sd1;
                int r;
                char slip[256] = {0};

                switch(buf[16]){    //type
                        case REQ_SWITCH_ON:{
                                sd1 = sensor_data_create(sd->id,sd->type,"true",transfer_type);
                                r = sensor_data_to_slip(sd1,slip,sizeof(slip));
                                if(r > 0){
                                        write_seriport(s,slip,r);
                                }
                                sensor_data_release(sd1);
                                break;
                        }
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
#else
        char buf[512] = {0};
        int n = read(fd,buf,sizeof(buf));
        hexprint("recv cloud",buf,n);
#endif

}

static int gw_cloud_heartbeat(struct aeEventLoop *el,long long id,void *client_data)
{
        struct gw_cloud_client *c = client_data;
        char buf[100] = {0};
        int r = heart_to_cloud(buf,sizeof(buf));
        write(c->fd,buf,r);
        return 2000;
}

static void newCloudClient(aeEventLoop *el,int fd,void *client_data,int mask)
{
        struct gw_cloud_client *c = gw_cloud_client_create(server.cloud_next_client_id++,fd);
        aeCreateTimeEvent(server.el,2000,gw_cloud_heartbeat,c,NULL);
        aeDeleteFileEvent(el,fd,AE_WRITABLE);
}

static int cmd_connect_to_platfrom(struct gw_json_client *c,const char *cmd,cJSON *args,cJSON *reply)
{
        char error[100] = {0};

        if(args == NULL){
                snprintf(error,sizeof(error),"%s args error",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }

        cJSON *ip = cJSON_GetObjectItem(args,"ip");
        if(ip == NULL){
                snprintf(error,sizeof(error),"%s need ip",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }
        cJSON *port = cJSON_GetObjectItem(args,"port");
        if(port == NULL){
                snprintf(error,sizeof(error),"%s need port",cmd);
                cJSON_AddStringToObject(reply,"err_msg",error);
                return -1;
        }
        char *ipaddr = ip->valuestring;
        int port_num = port->valueint;

        int fd = anetTcpNonBlockConnect(error,ipaddr,port_num);
        
        aeCreateFileEvent(server.el,fd,AE_WRITABLE,newCloudClient,c);

        return 0;
}

struct cmd {
        const char *cmd;
        int (*func)(struct gw_json_client *c,const char *cmd,cJSON *args,cJSON *reply);
};

static struct cmd cmds[]= {
        {"request_push",cmd_request_push},
        {"cancel_push",cmd_cancel_push},
        {"query",cmd_query},
        {"set_switch",cmd_set_sensor},
        {"set_lcd",cmd_set_sensor},
        {"set_sensor",cmd_set_sensor},
        {"add_uuid",cmd_add_uuid},
        {"connect_to_platform",cmd_connect_to_platfrom},
        {NULL,NULL}
};

static struct cmd *find_cmd(const char *str)
{
        if(str == NULL)
                return NULL;
        int i;
        for(i=0;cmds[i].cmd != NULL;i++){
                if(strcasecmp(cmds[i].cmd,str) == 0){
                        return &cmds[i];
                }
        }
        return NULL;
}

static void json_server_process_line(struct gw_json_client *c,const char *str)
{
        cJSON *root = cJSON_Parse(str);
        if(root){
                cJSON *cmd = cJSON_GetObjectItem(root,"cmd");
                cJSON *args = cJSON_GetObjectItem(root,"args");                
                cJSON *reply = cJSON_CreateObject();
                if(cmd){
                        struct cmd *cmd_t = find_cmd(cmd->valuestring);

                        if(cmd_t)
                                cmd_t->func(c,cmd->valuestring,args,reply);
                        else{
                                char buf[100];
                                snprintf(buf,sizeof(buf),"%s non-existent",cmd->valuestring);
                                cJSON_AddStringToObject(reply,"err_msg",buf);
                        }
                }else{
                        cJSON_AddStringToObject(reply,"err_msg","cmd key non-existent");
                }

                cJSON *seq_no = cJSON_GetObjectItem(root,"seq_no");
                if(seq_no){
                        if(seq_no->valuestring)
                                cJSON_AddStringToObject(reply,"seq_no",seq_no->valuestring);
                }
                char *json = cJSON_PrintUnformatted(reply);
        

                write(c->fd,json,strlen(json));
                write(c->fd,"\n",1);
                free(json);
                cJSON_Delete(reply);
                cJSON_Delete(root);
        }else{
                char* r = "{\"err_msg\":\"json fromat error\"}\n";

                write(c->fd,r,strlen(r));
        }
}
