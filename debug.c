#include <stdio.h>


void hexprint(const char *prefix,char *data,char len)
{
        printf("%s ",prefix);
        int i;
        for(i = 0;i < len;i++){
                printf("%02x ",(unsigned char)data[i]);
        }
        printf("\n");
}

