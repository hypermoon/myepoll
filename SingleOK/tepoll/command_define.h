/*************************************************************************
 ************************************************************************/
#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H
#include <stddef.h>

const int MAX_LISTEN = 3000;
const int SERVER_LISTEN_PORT = 9008;
const int MAX_DATA_LEN = 4096;

typedef struct
{
    unsigned char buf[MAX_DATA_LEN];
    size_t nLen;
    int socket;
}ServerData, *pServerData;

typedef void (*pCallBack)(const char * szBuf, size_t nLen, int socket);

#endif
