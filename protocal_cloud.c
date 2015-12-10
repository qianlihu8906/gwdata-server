#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protocal.h"

static char *uuid_string_to_raw(const char *uuid,char *raw)
{
}

static char *uuid_raw_to_string():q


int sensor_data_to_pkg(struct sensor_data *sd,char *pkg,int size)
{
        int pkg_len = LENGTH_UUID + 2 + 1+ sd->len;       //2 byte for length of pkg; 1 byte for type

        if(pkg_len > size)
                return -1;

        memcpy(pkg,sd->uuid,LENGTH_UUID);

        pkg[16] = (pkg_len&0xffff) >> 8;
        pkg[17] = pkg_len & 0xff;
        pkg[18] = sd->type;

        memcpy(pkg+19,sd->data,sd->len);

        return pkg_len;
}
