#ifndef BUFFER_H__
#define BUFFER_H__

struct buffer;

struct buffer *buffer_create(int size);
void buffer_release(struct buffer *buf);

int buffer_buf_append(struct buffer *buf,const char *buffer,int len);
int buffer_read_append(struct buffer *buf,int fd);

int buffer_read_slip(struct buffer *buf,char *slip,int size);
int buffer_read_line(struct buffer *buf,char *line,int size);
#endif
