/*************************************************************************

 ************************************************************************/
#include <iostream>
using namespace std;
#include "TcpServer.h"
#include <stdio.h>
#include <unistd.h>

TcpServer tcpServer;
void printMessage(const char * buf, size_t nlen, int sock)
{
    if(NULL == buf)
    {
        return;
    }
    ServerData * serverData = (ServerData*)buf;
    //printf("Message:%s\n", serverData->buf);
    //tcpServer.SendData(serverData->buf, sizeof(serverData->buf), serverData->socket);
    // tcpServer.WriteData(serverData->buf, serverData->nLen, serverData->socket,serverData->fp);

    tcpServer.WriteData((char*)serverData->buf, serverData->nLen, serverData->socket,serverData->fp);
}



int main()
{
    if(false == tcpServer.Initialize(8768, (unsigned long)printMessage))
    {
        printf("Initialize failed\n");
        return -1;
    }
    printf("tcpserver:%d\n", sizeof(tcpServer));
    while(1)
    {
        sleep(2);
    }
    return 0;
}
