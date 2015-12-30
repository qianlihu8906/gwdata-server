LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
        adlist.c ae.c anet.c buffer.c cJSON.c devices.c gw.c json_server.c \
        protocal.c sdlist.c seriport.c p208_server.c debug.c protocal_208.c \
        uuid_dvid.c cloud_client.c

LOCAL_CFLAGS:=-O2 -g
#LOCAL_CFLAGS+=-DLINUX

LOCAL_MODULE_TAGS := optional


LOCAL_MODULE:=gwdata-server

# gold in binutils 2.22 will warn about the usage of mktemp
LOCAL_LDFLAGS += -Wl,--no-fatal-warnings

include $(BUILD_EXECUTABLE)

