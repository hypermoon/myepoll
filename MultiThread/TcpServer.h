/*************************************************************************
 ************************************************************************/
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "command_define.h"
#include <set>
#include <list>
#include <sys/select.h>
#include <pthread.h>

#define MAX_DEV 250
typedef struct{
    char mac[13];
    char filename[128];
}FILENAMEMAC, *PFILENAMEMAC;

class TcpServer
{
public:
    TcpServer();
    virtual ~TcpServer();
    bool Initialize(unsigned int nPort, unsigned long recvFunc);
    bool SendData(const unsigned char * szBuf, size_t nLen, int socket);
    bool WriteData(const char * szBuf, size_t nLen, int socket, FILE *fp);
    bool UnInitialize();

private:
    static void * AcceptThread(void * pParam);
    static void * OperatorThread(void * pParam);
    static void * ManageThread(void * pParam);
    static PFILENAMEMAC CheckMacDirPath(char* pfilename);
    static char * FindIfNewMac(char* mac,void * pParam);
private:
    int m_server_socket;
    int m_thread_socket_number[65535];

    FILE *m_fp[65535];
    int m_thread_index;
    fd_set m_fdReads;
    pthread_mutex_t m_mutex;
    pCallBack m_operaFunc;

    //int m_client_socket[MAX_LISTEN];

    std::set<int> m_client_socket;
    std::list<ServerData> m_data;

    ServerData m_devicedata[MAX_DEV];
    fd_set m_fdreadarr[MAX_DEV];

    char strmac[MAX_DEV][13];
    int m_counter ;
    pthread_t m_pidAccept;
    pthread_t m_pidManage;
};

#endif
