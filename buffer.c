#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "uuid_dvid.h"
#include "debug.h"

struct buffer{
        char *buf;
        int size;
        int len;
};

struct buffer *buffer_create(int size)
{
        struct buffer *buf = malloc(sizeof(*buf) + size);
        if(buf == NULL)
                return NULL;
        memset(buf,0,sizeof(*buf)+size);

        buf->buf = (char *)(buf+1);
        buf->size = size;
        buf->len = 0;

        return buf;
}

void buffer_release(struct buffer *buf)
{
        free(buf);
}


static int get_index(char ch,const char *buf,int len)
{
        int i;

        for(i=0;i<len;i++){
                if(buf[i] == ch)
                        return i;
        }

        return -1;
}

int buffer_buf_append(struct buffer *buf,const char *buffer,int len)
{
        int unused = buf->size - buf->len;

        if(unused < len)
                return -1;

        memcpy(buf->buf+buf->len,buffer,len);
        buf->len += len;

        return len;
}

int buffer_read_append(struct buffer *buf,int fd)
{
        int unused = buf->size - buf->len;

        int r = read(fd,buf->buf+buf->len,unused);
        if( r <= 0)
                return r;

        buf->len += r;
        return r;
}

static int buffer_read_sep(struct buffer *buf,char sep,char *buffer,int size)
{
        int index = get_index(sep,buf->buf,buf->len);
        if(index < 0)
                return -1;
        
        int len = index + 1;
        if(len > size)
                return -1;

        memcpy(buffer,buf->buf,len);
        buffer[index] = 0;

        buf->len = buf->len - len;
        memmove(buf->buf,buf->buf+len,buf->len);

        return len-1;
}

int buffer_read_slip(struct buffer *buf,char *slip,int size)
{
        return buffer_read_sep(buf,0x7e,slip,size);
}

int buffer_read_line(struct buffer *buf,char *line,int size)
{
        return buffer_read_sep(buf,'\n',line,size);
}

int buffer_read_cloud(struct buffer *buf,char *cloud,int size)
{
        int head_len = LENGTH_UUID + 2 + 1;
        if(buf->len < head_len){
                return -1;
        }
        int data_len = buf->buf[17]*256 + buf->buf[18];
        if(buf->len < data_len){
                return -1;
        }
        if(data_len > size){
                return -1;
        }
        memcpy(cloud,buf->buf,data_len);
        buf->len = 0;
        return data_len;
}
