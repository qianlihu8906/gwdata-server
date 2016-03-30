#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "gw.h"
#include "cJSON.h"
#include "debug.h"
#include "buffer.h"
#include "seriport.h"
#include "json_server.h"
#include "cloud_client.h"

struct gwseriport{
        int fd;

        char *ttypath;
        char transfer_media[20];
        
        struct buffer *recvbuf;
};

struct gwseriport *find_transfer_media(const char *transfer_type)
{
        struct gwseriport *s;
        listNode *node;
        listIter *iter;
        
        iter = listGetIterator(server.seriports,AL_START_HEAD);

        while( (node=listNext(iter)) != NULL){
                s = node->value;
                if(strcasecmp(transfer_type,s->transfer_media) == 0){
                        listReleaseIterator(iter);
                        return s;
                }
        }

        listReleaseIterator(iter);
        return NULL;
}

struct gwseriport *gwseriport_create(const char *ttypath,int uart_speed);
void gwseriport_release(struct gwseriport *s);
int open_seriport(const char *ttypath,int uart_speed);

static void seriportHandler(aeEventLoop *el,int fd,void *privdata,int mask)
{
        struct gwseriport *s = privdata;

        int nread = buffer_read_append(s->recvbuf,fd);
        if(nread == -1){
                if(errno == EAGAIN){
                        return;
                }else{
                        fprintf(stdout,"Read from seriport: %s\n",strerror(errno));
                        gwseriport_release(s);
                        return;
                }
        }else if(nread == 0){
                fprintf(stdout,"Seriport closed\n");
                gwseriport_release(s);
                return;
        }

        char buf[512] = {0};
        int r;
        struct sensor_data *sd;
        while((r=buffer_read_slip(s->recvbuf,buf,sizeof(buf))) >= 0){
                hexprint("read seriport",buf,r);
                sd = slip_to_sensor_data(buf,r);
                if(sd != NULL){
                        snprintf(s->transfer_media,sizeof(s->transfer_media),"%s",sd->transfer_type);
                        sdlist_check_push(server.global_sensor_data,sd);

                        json_server_broadcast(sd);
                        gw_cloud_broadcast(sd);
                        sensor_data_release(sd);

                }
        }
}

struct gwseriport *gwseriport_create(const char *ttypath,int uart_speed)
{
        struct gwseriport *s = malloc(sizeof(*s));
        if(s == NULL)
                return NULL;
        memset(s,0,sizeof(*s));

        int fd = open_seriport(ttypath,uart_speed);
        if(fd < 0){
                free(s);
                return NULL;
        }

        s->recvbuf = buffer_create(1024);
        s->fd = fd;
        s->ttypath = strdup(ttypath);

        if(aeCreateFileEvent(server.el,fd,AE_READABLE,seriportHandler,s) == AE_ERR){
                close(fd);
                buffer_release(s->recvbuf);
                free(s);
                return NULL;
        }

        listAddNodeTail(server.seriports,s);
        
        return s;
}

void gwseriport_release(struct gwseriport *s)
{
        aeDeleteFileEvent(server.el,s->fd,AE_READABLE);
        aeDeleteFileEvent(server.el,s->fd,AE_WRITABLE);
        close(s->fd);
        buffer_release(s->recvbuf);

        listNode *ln = listSearchKey(server.seriports,s);
        assert(ln != NULL);
        listDelNode(server.seriports,ln);

        free(s);
}

static int look_up_uart_speed(long int uart_speed)
{
	switch(uart_speed){
	case 9600:		return B9600;
	case 19200: 	        return B19200;
	case 38400:		return B38400;
	case 57600:		return B57600;
	case 115200:	        return B115200;
	default:		return -1;
	}
}

int open_seriport(const char *ttypath,int uart_speed)
{
        int fd = open(ttypath,O_RDWR|O_NOCTTY|O_NONBLOCK);
	struct termios tios;
	memset(&tios,0,sizeof(struct termios));
	/* 将串口设置为原始模式 */
	cfmakeraw(&tios);
	/* baud rate */
	cfsetospeed(&tios,look_up_uart_speed(uart_speed));
	cfsetispeed(&tios,look_up_uart_speed(uart_speed));
	/* 流控制选项 */
	tios.c_iflag &= ~IXOFF;
	tios.c_cflag &= ~CRTSCTS;

        tios.c_cflag |= CS8;

        //控制模式 parity check
        tios.c_cflag &= ~PARENB;     //no parity check

        tios.c_cflag &= ~CSTOPB; //1 stop bits
	/* 设置串口缓冲队列 */
	tios.c_cc[VTIME] = 0;
	tios.c_cc[VMIN]  = 0;
	/* 使串口设置生效 */
	if(tcsetattr(fd,TCSADRAIN,&tios) < 0){
                return fd;
	}

        return fd;
}

int write_seriport(struct gwseriport *s,char *buf,int len)
{
        if(s){
                hexprint("seriport send",buf,len);
                return write(s->fd,buf,len);
        }
        return -1;
}
