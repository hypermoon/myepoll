/*************************************************************************
 ************************************************************************/
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "command_define.h"
#include <set>
#include <list>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>

typedef struct{
    char mac[13];
    char filename[128];
}FILENAMEMAC, *PFILENAMEMAC;


typedef struct{
    //char dg_functoken[2];
    int  dg_type;
    long dg_rtuaddr;
    char dg_dgsendtime[13];
    char dg_recordtime[11];
    int dg_rainfalllastfive;
    int dg_thisdayrainfall;               // 0.0
    double dg_waterlevlastfive;            // 00.00
    int dg_waterspeedlastfive;           // 0000
    float dg_watertemp;                   // 00.00
    float dg_volt;                        // 00.00
    char origindg[4096];
}RTUDATAGRAM , *PRTUDATAGRAM;


typedef struct{
   unsigned char qhead[4];
   unsigned char qdatahd[4];
   unsigned char qdataname[7];
   unsigned char qdatanum[1];
   unsigned char qdatasize[2];
   unsigned char qcrc[4];
   unsigned char qdatafin[2];
   unsigned char qend[1];
}QUERYIMGINFO, *PQUERYIMGINFO;

typedef struct{
   unsigned char imghead[4];
   unsigned char imgdatahd[4];
   unsigned char imgdataname[7];
   unsigned char imgdatanum[1];

   unsigned char imgdata[512];

   unsigned char imgcrc[4];
   unsigned char imgdatafin[2];
   unsigned char imgend[1];
}IMGDATA, *PIMGDATA;

enum IMGSTAT{ imgno=0, imgquery,imgtransfer,imgdel} ;

typedef struct{
    int sock;
    int numblocks;
    IMGSTAT  imgstat;
    int sendcounter;
    int cameraorder;
    int reqpair;
    unsigned char imagename[8];
    unsigned char rtuaddress[32];
    char imagedataname[128];
    char onlyimgname[128];
}RTUIMG, *PRTUIMG;

typedef struct{
   unsigned char d_resframebegin[2];
   unsigned char d_resrtuaddr[5];
   unsigned char d_rescenter[1];
   unsigned char d_respsd[2];
   unsigned char d_resfunc[1];
   unsigned char d_restokenlen[2];
   unsigned char d_resbegin[1];
   unsigned char d_resdatagraph[8];
   unsigned char d_resend[1];
   unsigned char d_rescrc[2];
}RESPONEDOWN, *PRESPONEDOWN;

typedef struct{
  int sock;
  long rtunumber;
  long lastmaintaintime;
}RTUNIT, *PRTUNIT;

/*typedef struct{
    int sock;
    FILE *fp;
}DEV;
*/
class TcpServer
{
public:
    TcpServer();
    virtual ~TcpServer();
    bool Initialize(unsigned int nPort, unsigned long recvFunc);
    bool SendData(const unsigned char * szBuf, size_t nLen, int socket);
    bool UnInitialize();

    static int  processimage(char *char_buf, unsigned char * buf , int sock,void * pParam);

    static PRTUDATAGRAM analysiswaterstring(int sock,char* pwaterstr,void * pParam);

     std::list<RTUNIT>m_client_rtusock;

private:
    static void * AcceptThread(void * pParam);
    static void * OperatorThread(void * pParam);
    static void * ManageThread(void * pParam);
    static PFILENAMEMAC CheckMacDirPath(char* pfilename);
private:
    int m_server_socket;
    fd_set m_fdReads;
    pthread_mutex_t m_mutex;
    pCallBack m_operaFunc;

 //  static unsigned int m_hourcount;

    std::list<RTUIMG> m_rtuimg_data;



    //int m_client_socket[MAX_LISTEN];
    std::set<int> m_client_socket;
   // std::set<DEV>m_client_dev;
    std::list<ServerData> m_data;
    pthread_t m_pidAccept;
    pthread_t m_pidManage;
};

#endif
