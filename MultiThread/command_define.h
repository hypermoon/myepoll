/*************************************************************************
 ************************************************************************/
#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H
#include <stddef.h>
#include <stdio.h>

const int MAX_LISTEN = 3000;
const int SERVER_LISTEN_PORT = 8768;
const int MAX_DATA_LEN = 4096;

typedef struct
{
    unsigned char buf[MAX_DATA_LEN];
    char mac[13];
    char filename[128];
    size_t nLen;
    int socket;
    FILE *fp;
}ServerData, *pServerData;

typedef void (*pCallBack)(const char * szBuf, size_t nLen, int socket);

#endif
