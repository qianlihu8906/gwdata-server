CROSS=arm-linux-gnueabihf-

CC=$(CROSS)gcc
CPP=$(CROSS)g++
LD=$(CROSS)ld
AS=$(CROSS)as
AR=$(CROSS)ar
OBJCOPY=$(CROSS)objcopy
OBJDUMP=$(CROSS)objdump

CFLAGS =  -Wall -I.  -D_GNU_SOURCE 
LDFLAGS = -lm 

COPY        := cp
MKDIR       := mkdir -p
MV          := mv
RM          := rm -f
DIRNAME     := dirname

EXEC = gwdata-server


SRCS := adlist.c ae.c anet.c buffer.c cJSON.c devices.c gw.c json_server.c protocal.c sdlist.c seriport.c p208_server.c debug.c protocal_208.c uuid_dvid.c cloud_client.c

OBJS := $(SRCS:%.c=%.o)

$(EXEC):$(OBJS)
	$(CC)  $^ -o $@ $(LDFLAGS)

$(OBJS):$(SRCS)
	$(CC)  $(CFLAGS) -c $^
clean:
	$(RM) *.o $(EXEC)
