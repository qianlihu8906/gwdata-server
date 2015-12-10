#ifndef SERIPORT_H__
#define SERIPORT_H__

struct gwseriport;
struct gwseriport *gwseriport_create(const char *ttypath,int uart_speed);
void gwseriport_release(struct gwseriport *s);
struct gwseriport *find_transfer_media(const char *transfer_type);
int open_seriport(const char *ttypath,int uart_speed);
int write_seriport(struct gwseriport *s,char *buf,int len);


#endif
