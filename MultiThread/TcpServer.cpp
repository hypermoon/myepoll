/*************************************************************************
 ************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "TcpServer.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>




PFILENAMEMAC p_filemacstruct;

TcpServer::TcpServer()
{
    pthread_mutex_init(&m_mutex, NULL);
    FD_ZERO(&m_fdReads);
    m_client_socket.clear();
    m_data.clear();
    m_operaFunc = 0;
    m_pidAccept = 0;
    m_pidManage = 0;
    m_counter =0;
    m_thread_index =0;

    for(int i=0; i<65535; i++)
    {
        m_thread_socket_number[i] =0;
    }


    for(int ct =0; ct < MAX_DEV; ct++)
    {
        strcpy(strmac[ct],"\0");
    }
}

TcpServer::~TcpServer()
{
    FD_ZERO(&m_fdReads);
    m_client_socket.clear();
    m_data.clear();
    m_operaFunc = NULL;
    pthread_mutex_destroy(&m_mutex);
}
PFILENAMEMAC TcpServer::CheckMacDirPath(char* pfilename)
{
    char filename[128];
    char *p_strmac = NULL;
    FILENAMEMAC  fnamemac;

    memset(filename,0,128);

    p_strmac = strchr(pfilename,' ');
    if(p_strmac == NULL)
    {
        printf("Cannot find Mac string,exit");
        return NULL;
    }

    strncpy(filename,pfilename+1,strlen(pfilename) - strlen(p_strmac) -1);
    p_strmac = p_strmac + 1;

    strcpy(fnamemac.mac,p_strmac);
    strcpy(fnamemac.filename,filename);

    return &fnamemac;

}

char * TcpServer::FindIfNewMac(char* mac, void * pParam)
{
       TcpServer * pThis = (TcpServer*)pParam;
       bool bfind =false;
       int i_fd =0;

        for(i_fd=0; i_fd<MAX_DEV; i_fd++)
            {
                if(strcmp(pThis->strmac[i_fd],"\0") != 0)    //not empty string
                {
                    if(strcmp(mac,pThis->strmac[i_fd]) ==0) //same
                    {
                        bfind =true;
                        break;
                    }
                }
                else
                {
                    break;
                }

            }

            if(!bfind)  //not find mac in old string, it is new mac!
            {
                 strcpy(pThis->strmac[pThis->m_counter++],mac);
            }


}

bool TcpServer::Initialize(unsigned int nPort, unsigned long recvFunc)
{
    if(0 != recvFunc)
    {
        //设置回调函数
        m_operaFunc = (pCallBack)recvFunc;
    }
    //先反初始化
    UnInitialize();
    //创建socket
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == m_server_socket)
    {
        printf("socket error:%m\n");
        return false;
    }
    //绑定IP和端口
    sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(nPort);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int res = bind(m_server_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(-1 == res)
    {
        printf("bind error:%m\n");
        return false;
    }
    //监听
    res = listen(m_server_socket, MAX_LISTEN);
    if(-1 == res)
    {
        printf("listen error:%m\n");
        return false;
    }
    //创建线程接收socket连接
    if(0 != pthread_create(&m_pidAccept, NULL, AcceptThread, this))
    {
        printf("create accept thread failed\n");
        return false;
    }
    //创建管理线程
    /*if(0 != pthread_create(&m_pidManage, NULL, ManageThread, this))
    {
        printf("create manage thread failed\n");
        return false;
    }*/
    return true;
}

//接收socket连接线程
void * TcpServer::AcceptThread(void * pParam)
{
    if(!pParam)
    {
        printf("param is null\n");
        return 0;
    }


    TcpServer * pThis = (TcpServer*)pParam;

    int localcount =0;
    FILE *fp = NULL;
    int nMax_fd = 0;
    int i = 0;
    const char *pneedle = "put ";
    char *pfilename = NULL;
      FILENAMEMAC l_filemac;
        pthread_t pid;


    while(1)
    {
        FD_ZERO(&pThis->m_fdReads);
        //把服务器监听的socket添加到监听的文件描述符集合
        FD_SET(pThis->m_server_socket, &pThis->m_fdReads);
        //设置监听的最大文件描述符
        nMax_fd = nMax_fd > pThis->m_server_socket ? nMax_fd : pThis->m_server_socket;
        std::set<int>::iterator iter = pThis->m_client_socket.begin();
        //把客户端对应的socket添加到监听的文件描述符集合
        for(; iter != pThis->m_client_socket.end(); ++iter)
        {
            FD_SET(*iter, &pThis->m_fdReads);
        }
        //判断最大的文件描述符
        if(!pThis->m_client_socket.empty())
        {
            --iter;
            nMax_fd = nMax_fd > (*iter) ? nMax_fd : (*iter);
        }
        //调用select监听所有文件描述符
        int res = select(nMax_fd + 1, &pThis->m_fdReads, 0, 0, NULL);
        if(-1 == res)
        {
            printf("select error:%m\n");
            continue;
        }
       // printf("select success\n");
        //判断服务器socket是否可读
        if(FD_ISSET(pThis->m_server_socket, &pThis->m_fdReads))
        {
            //接收新的连接
            int fd = accept(pThis->m_server_socket, 0,0);
            if(-1 == fd)
            {
                printf("accept error:%m\n");
                continue;
            }
            //添加新连接的客户
            pThis->m_client_socket.insert(fd);
            printf("connected ok\n");

                pThis->m_thread_index = fd; //localcount;
               // pThis->m_thread_socket_number[localcount++] = fd;
                if(0 != pthread_create(&pid, NULL, OperatorThread, pParam))
                {
                    printf("create manage thread failed\n");
                    return false;
                }
                else
                {
                    printf("create one new thread\n");
                }

        }

        for(iter = pThis->m_client_socket.begin(); iter != pThis->m_client_socket.end(); ++iter)
        {
            ;

        }
    }
}



//数据处理线程
void * TcpServer::OperatorThread(void * pParam)
{

     TcpServer * pThis = (TcpServer *)pParam;

    // int idx = pThis->m_thread_index ;
    // int lsocket = pThis->m_thread_socket_number[idx];

     int lsocket =pThis->m_thread_index;

     ServerData  sdata;

      int nMax_fd = 0;
      int res =0;
      FILE *fp = NULL;
      char *pfilename = NULL;
      FILENAMEMAC l_filemac;
      const char *pneedle = "put ";

      int iaccompsock = 0;

    // fp = pThis->m_fp[idx];
     //fd_set l_trdfdReads;


      // send(lsocket,"ready",5,0);


     while(1)   //FD_ISSET(lsocket, &l_trdfdReads))
     {

         if(-1 != lsocket && FD_ISSET(lsocket, &pThis->m_fdReads))
            {
                unsigned char buf[MAX_DATA_LEN] = {0};
                res = recv(lsocket, buf, sizeof(buf), 0);
               // printf("buffer is %s\n",buf);

                if(res > 0)
                {
                                        if(strstr((char*)buf,pneedle) != NULL)   //it is put command
                                        {
                                                     char str_wholefilepath[128];
                                                          memset(str_wholefilepath,0,128);


                                                        pfilename = strrchr((char*)buf,'\/');
                                                        if(pfilename == NULL)
                                                        {
                                                            printf("format string include invalid path!\n");
                                                            continue;
                                                        }
                                                        //here we split the buf into 'mac' and 'filename'
                                                        p_filemacstruct = CheckMacDirPath(pfilename);

                                                        strcpy(l_filemac.filename,p_filemacstruct->filename);
                                                        strcpy(l_filemac.mac,p_filemacstruct->mac);

                                                       // FD_CLR(*iter, &pThis->m_fdReads);


                                                        //create fp
                                                        //1. create mac folder
                                                        if(access(l_filemac.mac,0) == -1)  //this mac folder don't exist
                                                        {
                                                            if(mkdir(l_filemac.mac,0777))
                                                            {
                                                                printf("create file folder failed!\n");
                                                            }

                                                        }

                                                       //2. allocate fp pointer
                                                        strcpy(str_wholefilepath,l_filemac.mac);
                                                        strcat(str_wholefilepath,"\/");
                                                        strcat(str_wholefilepath,l_filemac.filename);

                                                        fp = fopen(str_wholefilepath,"wb");
                                                        if(fp == NULL)
                                                        {
                                                            printf("Create file failed, please check !!!\n");
                                                            //continue;
                                                            return NULL ;
                                                        }
                                                        //sdata.fp = fp;
                                                        //create new thread!

                                                       // pThis->m_thread_index = localcount;
                                                       // pThis->m_fp[localcount] = fp;
                                                       // pThis->m_thread_socket_number[localcount++] = *iter;


                                                     //  FD_CLR(*iter, &pThis->m_fdReads);
                                                       send(lsocket,"ready",5,0);
                                                       continue;
                                            } //put ended
                                            else
                                            {
                                                if(strstr((char*)buf,"Accomplished") != NULL)
                                                {
                                                    iaccompsock = lsocket;
                                                    printf("********** end of file transfer *****\n");
                                                    //bsend = true;
                                                    continue;
                                                }
                                            }

                                             //ServerData  sdata;
                                             memset(sdata.buf,0,sizeof(unsigned char) * MAX_DATA_LEN);
                                             memcpy(sdata.buf,buf,res);
                                             sdata.nLen = res;
                                             sdata.socket = lsocket;
                                             sdata.fp = fp;


                                             pThis->m_operaFunc((char *)&sdata, sizeof(sdata), sdata.socket);

                }
                else if(0 == res)
                {

                    pThis->m_client_socket.erase(lsocket);

                   // if(lsocket != iaccompsock)
                   // {

                    //printf("client thread ended!\n");

                    break;
                   // }

                }
                else
                {
                    printf("recv error\n");
                    break;
                }
            }

         //防止抢占CPU资源
         usleep(100);
     }
     //close(lsocket);
     printf("the thread:%d exit!\n",lsocket);
}

//管理线程，用于创建处理线程
void * TcpServer::ManageThread(void * pParam)
{
    if(!pParam)
    {
        return 0;
    }
    pthread_t pid;

    int lcount =0;

    //char strmac[MAX_DEV][13];
    TcpServer * pThis = (TcpServer *)pParam;

   // char str_wholefilepath[128];

    FILE *fp[MAX_DEV];

    //memset(str_wholefilepath,0,128);

    ServerData data ;

   // while(1)
    //{
        //使用互斥量
        pthread_mutex_lock(&pThis->m_mutex);
        int nthreadnumber =pThis->m_client_socket.size();
        int nCount = pThis->m_data.size();
        pthread_mutex_unlock(&pThis->m_mutex);

                        //for(int i=0; i<nthreadnumber;i++)
                        //{
                             //when get a connect from client, we open a thread to process
                    if(nCount > 0)
                    {

                           for(std::set<int>::iterator iter = pThis->m_client_socket.begin(); iter != pThis->m_client_socket.end(); ++iter)
                            {
                                 pThis->m_thread_index = lcount;
                                 pThis->m_thread_socket_number[lcount++] = *iter;

                                 pid = 0;
                                 //创建处理线程
                                 if( 0 != pthread_create(&pid, NULL, OperatorThread, pParam))
                                  {
                                         printf("creae new operator thread failed\n");
                                  }
                                  else
                                  {
                                         printf("A New thread had created!!! \n");
                                  }
                                  //counter++;
                            }

                    }


        //防止抢占CPU资源
   //     usleep(100);
  //  }
}


//数据处理线程
/*
void * TcpServer::OperatorThread(void * pParam)
{

    if(!pParam)
    {
        return 0;
    }
    int order = 0;
    int res =0;

    TcpServer * pThis = (TcpServer *)pParam;

  //  order = pThis->m_threadorder;

    ServerData  sdata;

    sdata.fp = pThis->m_devicedata[order].fp;
    sdata.socket = pThis->m_devicedata[order].socket;
    sdata.nLen   = pThis->m_devicedata[order].nLen;
    strcpy(sdata.mac,pThis->m_devicedata[order].mac);
    strcpy(sdata.filename,pThis->m_devicedata[order].filename);
    memcpy(sdata.buf,pThis->m_devicedata[order].buf,sdata.nLen);



                        if(nCount > 0)
                        {
                              // we need update the mac into maclist!!! and allocate corresponding thread!
                               pthread_mutex_lock(&pThis->m_mutex);
                                if(!pThis->m_data.empty())
                                {
                                    //ServerData
                                    data = pThis->m_data.front();
                                    pThis->m_data.pop_front();

                                }

                                pthread_mutex_unlock(&pThis->m_mutex);

                         }

    while(1)
    {
              //1. create mac folder
            if(access(data.mac,0) == -1)  //this mac folder don't exist
            {
                if(mkdir(data.mac,0777))
                {
                    printf("create file folder failed!\n");
                }

            }
             //2. allocate fp pointer
                strcpy(str_wholefilepath,data.mac);
                strcat(str_wholefilepath,"\/");
                strcat(str_wholefilepath,data.filename);

                fp[counter] = fopen(str_wholefilepath,"wb");
                if(fp[counter] == NULL)
                {
                    printf("Create file failed, please check !!!\n");
                    continue;
                }
                data.fp = fp[counter];


                 pThis->m_devicedata[counter].socket = data.socket;
                 pThis->m_devicedata[counter].nLen  = data.nLen;
                 pThis->m_devicedata[counter].fp = data.fp;
                 strcpy(pThis->m_devicedata[counter].mac,data.mac);
                 //strcpy((char*)pThis->m_devicedata[counter].buf,data.buf);
                 memcpy(pThis->m_devicedata[counter].buf,data.buf,data.nLen);

                 strcpy(pThis->m_devicedata[counter].filename,data.filename);



          //  pthread_mutex_lock(&pThis->m_mutex);
            if(strcmp(sdata.mac,"\0") !=0 && sdata.fp != NULL)  //valid mac
            {

                FD_SET(sdata.socket, &pThis->m_fdReads);

                //send "ready" command
                send(sdata.socket,"ready",5,0);

                //receive data

                     FD_SET(sdata.socket, &pThis->m_fdReads);


                        if(-1 != sdata.socket && FD_ISSET(sdata.socket, &pThis->m_fdReads))
                        {
                            unsigned char buf[MAX_DATA_LEN] = {0};
                            res = recv(sdata.socket, buf, sizeof(buf), 0);

                            if(pThis->m_operaFunc)
                            {
                                //strcpy(sdata.buf,buf);
                                memcpy(sdata.buf,buf,res);
                                sdata.nLen = res;
                                //把数据交给回调函数处理
                            //   pThis->m_operaFunc((char *)&sdata, sizeof(sdata), sdata.socket);
                            }
                        }
                   //  usleep(100);


            }
          //  pthread_mutex_unlock(&pThis->m_mutex);


    }
}
*/


//发送数据到指定的socket
bool TcpServer::SendData(const unsigned char * buf, size_t len, int sock)
{
    if(NULL == buf)
    {
        return false;
    }
    int res = send(sock, buf, len, 0);
    if(-1 == res)
    {
        printf("send error:%m\n");
        return false;
    }
    return true;
}

bool TcpServer::WriteData(const  char * szBuf, size_t nLen, int socket,FILE *fp)
{
    if(NULL == szBuf)
    {
        return false;
    }

    int res =0;    //= send(sock, buf, len, 0);
   // szBuf[nLen] = '\0';

    if(strcmp((const char*)szBuf,"EOF") == 0) //equal ,same string EOF
    {
        printf("write file finished\n");
        if(fp!= NULL)
        {
        fclose(fp);
        fp = NULL;
        }
    }
    else
    {
        fwrite(szBuf,sizeof(unsigned char),nLen,fp);
    }

    if(-1 == res)
    {
        printf("send error:%m\n");
        return false;
    }
    return true;

}


bool TcpServer::UnInitialize()
{
    close(m_server_socket);
    for(std::set<int>::iterator iter = m_client_socket.begin(); iter != m_client_socket.end(); ++iter)
    {
        close(*iter);
    }
    m_client_socket.clear();
    if(0 != m_pidAccept)
    {
        pthread_cancel(m_pidAccept);
    }
    if(0 != m_pidManage)
    {
        pthread_cancel(m_pidManage);
    }
}
