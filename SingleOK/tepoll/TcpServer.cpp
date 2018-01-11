/*************************************************************************
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
//#include <linux/tcp.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "TcpServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <sys/select.h>
#include<libpq-fe.h>

#define  MAX_DEV 100
#define  IMAGEFOLDPATH     "/var/www/html/WlMonitor/frontend/web/public/rtuimgs"

//bool b_needimg =false;
//int g_sendtime = 0;
//char imgname[7];
 //int m_nblocks = 1;
// int num =0;
 //char g_rtuaddr[16];

unsigned int  m_hourcount = 0;



char Dec2bcd(int dec_data)
{
  unsigned char *s=  (unsigned char*)malloc(10);
  unsigned long bcd_data;
  sprintf((char*)s,"%d",dec_data); //dec_data=12,s="12"
  sscanf((char*)s,"%x",&bcd_data);
  free(s);
  return bcd_data;
}


unsigned char *Dec2hex(int dec_data)
{

    unsigned char ps[10];       //=  (unsigned char*)malloc(10);
    unsigned char trans_data[2];
    unsigned char hex_result[2];
    char *p = NULL;

    memset(ps,0, sizeof(unsigned char) * 10 );

    sprintf((char*)ps,"%x",dec_data);
    sscanf((char*)ps,"%x",&trans_data);

    hex_result[0] = trans_data[1];
    hex_result[1] = trans_data[0];

   // free(ps);

    return hex_result;

}

int refreshonlinestatue(void *pParam)
{

      TcpServer * pThis = (TcpServer*)pParam;

      PGconn *conn;
      PGresult *result;
      char insert_str[1024];

      memset(insert_str,0,1024*sizeof(char));

      char refresh_str[1024];
      memset(refresh_str,0,1024*sizeof(char));

      char onlinertuid[12];
      memset(onlinertuid,0,12*sizeof(char));

      const char *conn_str="host=localhost dbname=WlMonitor user=postgres password=pgsql123 port =5432";

      conn=PQconnectdb(conn_str);

      //sprintf(insert_str,"INSERT INTO %s VALUES (%d,%s,%d,13,29,13,now(),0,'','123456789','12345678','123',23);",name, id, site,stationo );
      //select id, site, current_site, current_level,onlinestat from water_monitor;
      //UPDATE water_monitor SET onlinestat = 1;

      sprintf(insert_str,"UPDATE water_monitor SET onlinestat = 0");

      if(PQstatus(conn) == CONNECTION_BAD)
      {
              fprintf(stderr,"connection to %s failed", conn_str);
              PQerrorMessage(conn);
      }
      else
      {
               printf("\n Db connection database ok\n");
               result = PQexec(conn,insert_str);

                   if(PQresultStatus(result) == PGRES_COMMAND_OK)
                   {
                        printf("reset refresh database ok\n");
                   }
                   else
                   {
                        printf("reset refresh database failded\n");
                   }

               std::list<RTUNIT>::iterator rtusockiters;
               for(rtusockiters = pThis->m_client_rtusock.begin() ; rtusockiters != pThis->m_client_rtusock.end(); ++rtusockiters)
               {
                   memset(refresh_str,0,1024*sizeof(char));
                   RTUNIT rtunode;
                   rtunode = *rtusockiters;
                   if(rtunode.rtunumber >0 && rtunode.rtunumber < 999999)  //valide rtuname
                   {
                       sprintf(onlinertuid,"%ld",rtunode.rtunumber);

                       sprintf(refresh_str,"UPDATE water_monitor SET onlinestat = 1 where current_site = \'%s\'", onlinertuid);
                       printf("^^^^^^ the SQL is :%s\n",refresh_str);
                       result = PQexec(conn,refresh_str);

                           if(PQresultStatus(result) == PGRES_COMMAND_OK)
                           {
                                printf("update refresh database ok\n");
                           }
                           else
                           {
                                printf("update refresh database failded\n");
                           }

                   }

               }



      }

      PQfinish(conn);

      return EXIT_SUCCESS;

}


int saveRTUdatagram(PRTUDATAGRAM prtudata )   //main()
{
    //gcc -I /usr/include/pgsql -L /usr/lib/ -lpq pgsql.c -o pgsql

     PGconn *conn;
     PGresult *result;

     const char *conn_str="host=localhost dbname=WlMonitor user=postgres password=pgsql123 port =5432";

     char insert_str[1024];
     char update_str[1024];
     char warning_insert_str[1024];
     char mobilephonequery[512];
     char strRainfall24[32];
     char strsluice[12];
     char strvolte[12];

     int id = 19;
     char name[16];           //"rtu_123456";

     char *site = "\'重庆\'";
     int stationo =  prtudata->dg_rtuaddr;  //123456;

     char datagramerecordtime[128]; //"to_timestamp('1609052312', 'YYMMDDHH24MI')";  //"now()";   //datagramerecordtime


     memset(datagramerecordtime,0,128);
     sprintf(datagramerecordtime, "to_timestamp(\'%s\', \'YYMMDDHH24MI\')",prtudata->dg_recordtime);

     memset(name, 0, 16);
     sprintf(name,"rtu_%d",stationo);
     // memcpy(g_rtuaddr,name,16);


     memset(update_str,0,1024);

     memset(strsluice,0,12);

     //sprintf(insert_str,"INSERT INTO %s VALUES (%d,%s,%d,13,29,13,now(),0,'','123456789','12345678','123',23);",name, id, site,stationo );
     // sprintf(prtudata->origindg,"\'%s\'",prtudata->origindg);
     memset(insert_str,0,1024*sizeof(char));
     memset(warning_insert_str,0,1024*sizeof(char));
     memset(mobilephonequery,0,512*sizeof(char));

     printf("--------- the  origin str is :%s",prtudata->origindg);

     memset(strvolte,0,12);


     memset(strRainfall24,0,32);
     sprintf(strRainfall24,"\'%d\'",prtudata->dg_thisdayrainfall);
     //prtudata->dg_thisdayrainfall

     //                            name                                                                                                id,  site,stationo,waterlv,   rain, rainall,temp, time    wflow ,  volt,  originstr, dgtype
     sprintf(insert_str,"INSERT INTO %s(state,statno,waterlv,rainfall,rainfallmulti,watertemp,date,waterflow,volte,originstr,dgtype) VALUES (  %s,  %d,       %f,       %d,  %s,   %f,    %s,    %d,    %f   ,  %s      , %d   );",
                                             name,site,stationo,prtudata->dg_waterlevlastfive, prtudata->dg_rainfalllastfive, strRainfall24 ,prtudata->dg_watertemp, datagramerecordtime , prtudata->dg_waterspeedlastfive,
                                             prtudata->dg_volt ,prtudata->origindg, prtudata->dg_type );   //, prtudata->origindg

     printf(insert_str);


     if(prtudata->dg_type ==5) //it is warning datagram, need add into rtuwarning table
     {

     }

     if(prtudata->dg_type ==6) //it is calltest datagram, need add into  table
     {
         //WlMonitor=# UPDATE water_monitor SET current_temp=30,current_level=50  where current_site='332255';


     }

     conn=PQconnectdb(conn_str);

     if(PQstatus(conn) == CONNECTION_BAD)
      {
          fprintf(stderr,"connection to %s failed", conn_str);
          PQerrorMessage(conn);
      }
     else
      {
           printf("\n Db connection database ok\n");

           if(prtudata->dg_type !=6)
           {
               result = PQexec(conn,insert_str);
               if(PQresultStatus(result) == PGRES_COMMAND_OK)
               {
                    printf("insert ok\n");
                }
               else
               {
                   printf("insert failded\n");
                }
           }

            if(prtudata->dg_type ==5)
            {
                char *pmobilenum = NULL;
                sprintf(mobilephonequery,"SELECT fatherpoint from waterstation where sitenumber=%d",stationo);
                result = PQexec(conn,mobilephonequery);
                pmobilenum = PQgetvalue(result,0,0);

                sprintf(warning_insert_str, "INSERT INTO rtuwarning (rtuno,date,waterlv,rainfall,volte,originstr,bakup) VALUES( %d, %s, %f,%d, %f, %s,%s)",
                                 stationo,datagramerecordtime ,prtudata->dg_waterlevlastfive,prtudata->dg_thisdayrainfall,prtudata->dg_volt,prtudata->origindg,pmobilenum);

                printf(warning_insert_str);

                result = PQexec(conn,warning_insert_str);
                    if(PQresultStatus(result) == PGRES_COMMAND_OK)
                    {
                         printf("insert waring table ok\n");
                    }
                    else
                   {
                       printf("insert warning failded\n");
                    }
            }

            if(prtudata->dg_type == 6)
            {
                       sprintf(strsluice,"%d",prtudata->dg_waterspeedlastfive);
                       sprintf(strvolte,"%0.2f", prtudata->dg_volt );

                  sprintf(update_str,"UPDATE water_monitor SET current_level=%f,current_temp=%f,rainfall=%d,datetime=%s,accumulator=%s,sluice=%s where current_site='%d';",
                           prtudata->dg_waterlevlastfive, prtudata->dg_watertemp,prtudata->dg_rainfalllastfive,datagramerecordtime,strvolte, strsluice, stationo
                          );

                           printf("\n Output CallTest:%s\n",update_str);

                           result = PQexec(conn,update_str);

                            if(PQresultStatus(result) == PGRES_COMMAND_OK)
                            {
                              printf("insert calltest table ok\n");
                            }
                            else
                            {
                               printf("insert calltest table failded\n");
                            }

            }

       }
      PQfinish(conn);
      return EXIT_SUCCESS;
}



int saveRTU_IMAGES(PRTUIMG prtuimg )   //main()
{
    //gcc -I /usr/include/pgsql -L /usr/lib/ -lpq pgsql.c -o pgsql

     PGconn *conn;
     PGresult *result;

     const char *conn_str="host=localhost dbname=WlMonitor user=postgres password=pgsql123 port =5432";

     char insert_str[1024];

     char name[16];           //"rtu_123456";


     memset(name, 0, 16);
     //sprintf(name,"rtu_%d",stationo);

     memset(insert_str,0,1024*sizeof(char));

    // printf("--------- the  origin str is :%s",prtudata->origindg);

     //                             name                                                                                   id,  site,stationo,waterlv,   rain,   temp, time    wflow ,  volt,  originstr, dgtype
     //sprintf(insert_str,"INSERT INTO %s(state,statno,waterlv,rainfall,watertemp,date,waterflow,volte,originstr,dgtype) VALUES (  %s,  %d,       %f,       %d,     %f, %s,  %d,    %f   ,  %s      , %d   );",
      //                                       name,site,stationo,prtudata->dg_waterlevlastfive, prtudata->dg_rainfalllastfive ,prtudata->dg_watertemp, datagramerecordtime , prtudata->dg_waterspeedlastfive  ,prtudata->dg_volt ,prtudata->origindg, prtudata->dg_type );   //, prtudata->origindg

      //UPDATE water_monitor SET img1='161025_160050-02.jpg' where current_site='654321'

     //sprintf(insert_str,"INSERT INTO %s VALUES (%d,%s,%d,13,29,13,now(),0,'','123456789','12345678','123',23);",name, id, site,stationo );

      sprintf(insert_str,"UPDATE water_monitor SET img%d = \'%s\' where current_site = \'%s\'", prtuimg->cameraorder,prtuimg->onlyimgname,prtuimg->rtuaddress );


     printf(insert_str);
     conn=PQconnectdb(conn_str);

     if(PQstatus(conn) == CONNECTION_BAD)
      {
          fprintf(stderr,"connection to %s failed", conn_str);
          PQerrorMessage(conn);
      }
     else
      {
         printf("\nconnection ok\n");
          result = PQexec(conn,insert_str);
          if(PQresultStatus(result) == PGRES_COMMAND_OK)
           {

                printf("update img ok\n");
            }
             else
           {
               printf("update img failded\n");
            }

       }
      PQfinish(conn);
      return EXIT_SUCCESS;
}


RESPONEDOWN rtuhourres;
PRESPONEDOWN filldownrtustruct(char* ptime, int type)
{

                    rtuhourres.d_resframebegin = {0x7e,0x7e};
                    rtuhourres.d_resrtuaddr = {0x00,0x00,0x00,0x00,0x00};
                    rtuhourres.d_rescenter = {0x00};
                    rtuhourres.d_respsd = {0x04,0xd2};

                    if(type ==1)
                       rtuhourres.d_resfunc = {0x4A};
                    if(type ==2)
                       rtuhourres.d_resfunc = {0x37};

                    rtuhourres.d_restokenlen={0x80,0x08};
                    rtuhourres.d_resbegin = {0x02};


                     /*  struct tm  *ptm;
                       long ts;
                       int yy,mm,dd,hh,nn,ss;
                       ts = time(NULL);
                       ptm = localtime(&ts);

                       yy=ptm->tm_year+1900 -2000;
                       mm=ptm->tm_mon +1;
                       dd=ptm->tm_mday;
                       hh = ptm->tm_hour;
                       nn = ptm->tm_min;
                       ss = ptm->tm_sec;*/

                       char stryy[3],strmm[3],strdd[3],strhh[3],strnn[3],strss[3];
                       int yy,mm,dd,hh,nn,ss;
                       strncpy(stryy,ptime,2);
                       yy = strtol(stryy,NULL,10);
                       stryy[2] = 0x0;

                       strncpy(strmm,ptime+2,2);
                       mm = strtol(strmm,NULL,10);
                       strmm[2] = 0x0;

                       strncpy(strdd,ptime+4,2);
                       dd = strtol(strdd,NULL,10);
                       strdd[2] = 0x0;

                       strncpy(strhh,ptime+6,2);
                       hh = strtol(strhh,NULL,10);
                       strhh[2] = 0x0;

                       strncpy(strnn,ptime+8,2);
                       nn = strtol(strnn,NULL,10);
                       strnn[2] = 0x0;

                       strncpy(strss,ptime+10,2);
                       ss = strtol(strss,NULL,10);
                       strss[2] = 0x0;

                       m_hourcount++;

                       unsigned char *pcounts = NULL;
                       pcounts = Dec2hex(m_hourcount);

                       rtuhourres.d_resdatagraph[0] = pcounts[0];
                       rtuhourres.d_resdatagraph[1] = pcounts[1];


                       rtuhourres.d_resdatagraph[2] = Dec2bcd(yy);
                       rtuhourres.d_resdatagraph[3] = Dec2bcd(mm);
                       rtuhourres.d_resdatagraph[4] = Dec2bcd(dd);

                       rtuhourres.d_resdatagraph[5] = Dec2bcd(hh);
                       rtuhourres.d_resdatagraph[6] = Dec2bcd(nn);
                       rtuhourres.d_resdatagraph[7] = Dec2bcd(ss);

                         rtuhourres.d_resend = {0x05};
                         rtuhourres.d_rescrc = {0x00,0x48};

                         return &rtuhourres;

                       //  send(sock, &rtuhourres, sizeof(rtuhourres), 0);
}




TcpServer::TcpServer()
{
    pthread_mutex_init(&m_mutex, NULL);
    FD_ZERO(&m_fdReads);
    m_client_socket.clear();

    m_client_rtusock.clear();

    m_rtuimg_data.clear();

                     //   m_client_dev.clear();
    m_data.clear();
    m_operaFunc = 0;
    m_pidAccept = 0;
    m_pidManage = 0;



    int n = 0;
    char *names = IMAGEFOLDPATH; // "/var/www/html/WlMonitor/frontend/web/public/rtuimgs";
    umask(0);
    n = mkdir(names,0775);
    //n =error_code();
    printf("errno=%d\n",errno);



    n =1;
    //   saveRTUdatagram();
}

TcpServer::~TcpServer()
{
    FD_ZERO(&m_fdReads);
    m_client_socket.clear();

    m_client_rtusock.clear();
  //  m_client_dev.clear();
    m_data.clear();
    m_rtuimg_data.clear();
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

      //  int reuse = 1;
    //setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));


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


 //char  rtuaddr[10];
 // char functoken[2];

RTUDATAGRAM  rtu_dginfo;

PRTUDATAGRAM TcpServer::analysiswaterstring(int sock,char* pwaterstr, void * pParam)
{
    //weichi bao
    //7e7e16001000000001022f000802000016100916000403b4800a0d

    //xiaoshi bao
    //7e7e1600100000000102340000020001161009160000f1f1001000000052f0f01610091600f460000000000000000000000000f0f016100916002619000100f0f01610091600f5c00004ffffffff0004fffffffffffffffffffffffffffffffff0f016100916003030fffffffffffffffffffffffffffffffffffffffffffffffff0f01610091600381211730319d20a0d

    //ding shi bao
    //7e7e1600100000000102320026020001161011120007f1f1001000000052f0f016101112002619000000392300000000030300003030122738121174030e95

        if(!pParam)
    {
        printf("param is null\n");
        return 0;
    }
    TcpServer * pThis = (TcpServer*)pParam;


   char* pstr = (char*)pwaterstr;


   // printf("the CHAR string is: %s\n", pwaterstr);

   char szout[160];
  // int    v = 0x12345678;
    sprintf(szout,"%02x",0);
       //dgsendtime

    while((pstr = strstr(pstr, "7e7e"))  != NULL)
    {
            char functoken[3];
            char dglength[3];

            char rtuaddr[11];
            char dgsendtime[13];
            char recordtime[11];
            char rainfallperfive[24];
            char thisdayrainfall[6];
            char waterlevperfive[48];
            char waterspeedperfive[48];
            char watertemptory[4];
            char volt[4];

            char opstr[1024];

            strncpy(dglength,pstr+ 12*2 ,2);
            long dglen = strtol(dglength ,NULL,16);
            printf ("rainlast is %d\n",dglen);

            memset(opstr,0,1024);
            strncpy(opstr,pstr,(dglen+17) *2 );

            memset(rtuaddr,0,11);

            memset(dgsendtime,0, 13);

            memset(recordtime,0, 11);

            memset(&rtu_dginfo,0,sizeof(RTUDATAGRAM));

            //功能码
            strncpy(functoken,opstr+20,2);


                if(functoken[0] == '2' && functoken[1] == 'f')           //维持报
                {
                        //std::set<RTUNIT>::iterator rtuiter = pThis->m_client_rtusock.begin();  //rtu地址

                        rtu_dginfo.dg_type = 0;
                        printf("this is maintain dg \n");

                        strncpy(rtuaddr, pstr+6 ,10);
                        long rtunumber = strtol(rtuaddr,NULL,10);
                        rtu_dginfo.dg_rtuaddr = rtunumber;
                        printf("this is maintain dg number is %d\n",rtunumber);

                        std::list<RTUNIT>::iterator rtuiter = pThis->m_client_rtusock.begin();

                        for(; rtuiter != pThis->m_client_rtusock.end(); ++rtuiter)
                        {
                            RTUNIT tbdrtu;
                            tbdrtu = *rtuiter;

                            if(tbdrtu.sock == sock)
                            {
                                struct timeval nowtime;
                                gettimeofday(&nowtime,NULL);
                                tbdrtu.lastmaintaintime = nowtime.tv_sec; //refresh the maintain time
                                tbdrtu.rtunumber = rtunumber;
                                *rtuiter = tbdrtu;
                                break;
                            }

                        }

                         for(rtuiter = pThis->m_client_rtusock.begin(); rtuiter != pThis->m_client_rtusock.end(); ++rtuiter)
                         {
                              RTUNIT tbdrtu;
                              tbdrtu = *rtuiter;
                              printf("\ndddddddd the rtu sock is %d and number is %ld\n",tbdrtu.sock,tbdrtu.rtunumber);

                         }

                         std::set<int>::iterator tpiter = pThis->m_client_socket.begin();
                         for(tpiter = pThis->m_client_socket.begin(); tpiter != pThis->m_client_socket.end(); ++tpiter)
                         {
                             printf("\nyyyyyyy the rtu sock number is %d\n",*tpiter);
                         }

                         refreshonlinestatue(pParam);


                }

                if(functoken[0] == '3' && functoken[1] == '4')              //小时报
                {
                        // long temp = strtol("32",NULL,10);
                       // printf ("is %d\n",temp);

                        printf("this is hour dg \n");
                        rtu_dginfo.dg_type = 1;


                        //rtu地址
                        strncpy(rtuaddr, pstr+6 ,10);
                        long rtunumber = strtol(rtuaddr,NULL,10);
                        rtu_dginfo.dg_rtuaddr = rtunumber;


                      //   b_needimg = true;
                      std::list<RTUIMG>::iterator iter = pThis->m_rtuimg_data.begin();  //here we scan the sock list to find the matched sock, and save rtu address!
                      for(; iter != pThis->m_rtuimg_data.end(); ++iter)
                      {
                          RTUIMG rtuimg;
                          rtuimg = *iter;
                          if(rtuimg.sock == sock)   //same socke
                          {
                              rtuimg.imgstat = imgquery;
                              rtuimg.reqpair = 2;
                              //memcpy(rtuimg.rtuaddress,rtuaddr,11);
                              sprintf((char*)rtuimg.rtuaddress,"%ld",rtunumber);
                              *iter = rtuimg;
                          }
                      }

                        //发报时间
                        strncpy(dgsendtime,pstr+ 16 * 2, 12);
                        strcpy(rtu_dginfo.dg_dgsendtime,dgsendtime);

                        //记录时间
                        strncpy(recordtime,pstr+ 32 *2,10);
                        strcpy(rtu_dginfo.dg_recordtime,recordtime);

                        //每5分降雨量
                        strncpy(rainfallperfive,pstr + 39 * 2,24);
                        char rf[3] = {rainfallperfive[22],rainfallperfive[23]};
                        long rainlast = strtol(rf ,NULL,10);
                        printf ("rainlast is %d\n",rainlast);
                        rtu_dginfo.dg_rainfalllastfive = rainlast;

                        //当天降雨量
                        strncpy(thisdayrainfall, pstr + 60 * 2,6);
                        char rfn[7] = {thisdayrainfall[0],thisdayrainfall[1],thisdayrainfall[2],thisdayrainfall[3],thisdayrainfall[4],thisdayrainfall[5]};
                        int rainfallnow = strtol(rfn,NULL,10);
                        rtu_dginfo.dg_thisdayrainfall = rainfallnow;
                        printf ("rainnow is %d\n",rainfallnow);

                        //每5分钟水位
                        strncpy(waterlevperfive, pstr + 72 *2,48);
                        char wl[5] = {waterlevperfive[44],waterlevperfive[45],waterlevperfive[46],waterlevperfive[47]};
                        int wlast = strtol(wl,NULL,10);
                        rtu_dginfo.dg_waterlevlastfive = (float)wlast * 0.01;
                        printf("waterlast is %f\n", rtu_dginfo.dg_waterlevlastfive);

                        //每5分钟流量
                        strncpy(waterspeedperfive, pstr + 105 *2 ,48);
                        char ws[5] = {waterspeedperfive[44],waterspeedperfive[45],waterspeedperfive[46],waterspeedperfive[47]};
                        int wslast = strtol(ws,NULL,10);
                        rtu_dginfo.dg_waterspeedlastfive = wslast;
                        printf(" waterspeed is %d\n",wslast);

                        //电压
                        strncpy(volt, pstr + 138 * 2 ,4);
                        char vt[5] = {volt[0], volt[1],volt[2],volt[3]};
                        int vtvalue =  strtol(vt,NULL,10);
                        rtu_dginfo.dg_volt = (float)vtvalue * 0.01;
                        printf("rtu_dginfovolt is %f\n", rtu_dginfo.dg_volt);


                        strcpy(rtu_dginfo.origindg,"\'");
                        strcat(rtu_dginfo.origindg, pstr); //pwaterstr);
                        strcat(rtu_dginfo.origindg,"\'");

                        saveRTUdatagram(&rtu_dginfo);

                        RESPONEDOWN hourres;
                        hourres.d_resframebegin = {0x7e,0x7e};
                        hourres.d_resrtuaddr = {0x00,0x00,0x00,0x00,0x00};
                        hourres.d_rescenter = {0x00};
                        hourres.d_respsd = {0x04,0xd2};
                        hourres.d_resfunc = {0x34};
                        hourres.d_restokenlen={0x80,0x08};
                        hourres.d_resbegin = {0x02};

                        m_hourcount++;

                       struct tm  *ptm;
                       long ts;
                       int yy,mm,dd,hh,nn,ss;
                       ts = time(NULL);
                       ptm = localtime(&ts);

                       yy=ptm->tm_year+1900 -2000;
                       mm=ptm->tm_mon +1;
                       dd=ptm->tm_mday;
                       hh = ptm->tm_hour;
                       nn = ptm->tm_min;
                       ss = ptm->tm_sec;

                      // char flownum[4];
                       //sprintf(flownum,"%x",m_hourcount);

                                       // sprintf(hourres.d_resdatagraph[])
                      // hourres.d_resdatagraph[0] = flownum[0];
                      // hourres.d_resdatagraph[1] = flownum[1];
                      // hourres.d_resdatagraph[2] = flownum[2];
                       //hourres.d_resdatagraph[3] = flownum[3];

                       unsigned char *pcounts = NULL;
                       pcounts = Dec2hex(m_hourcount);

                       hourres.d_resdatagraph[0] = pcounts[0];
                       hourres.d_resdatagraph[1] = pcounts[1];


                       //sprintf(&hourres.d_resdatagraph[4],"%x",yy );
                       //sprintf(&hourres.d_resdatagraph[6],"%x",mm );
                       //sprintf(&hourres.d_resdatagraph[8],"%x",dd );

                       //sprintf(&hourres.d_resdatagraph[10],"%x",hh );
                       //sprintf(&hourres.d_resdatagraph[12],"%x",nn );
                       //sprintf(&hourres.d_resdatagraph[14],"%x",ss );


                       hourres.d_resdatagraph[2] = Dec2bcd(yy);
                       hourres.d_resdatagraph[3] = Dec2bcd(mm);
                       hourres.d_resdatagraph[4] = Dec2bcd(dd);

                       hourres.d_resdatagraph[5] = Dec2bcd(hh);
                       hourres.d_resdatagraph[6] = Dec2bcd(nn);
                       hourres.d_resdatagraph[7] = Dec2bcd(ss);



                       hourres.d_resend = {0x1b};
                       hourres.d_rescrc = {0x00,0x48};

                       send(sock, &hourres, sizeof(hourres), 0);

                }

                if(functoken[0] == '3' && functoken[1] == '2')             //定时报
                {
                        printf("this is certain time dg \n");

                       // long temp = strtol("32",NULL,10);
                       // printf ("is %d\n",temp);

                         rtu_dginfo.dg_type = 2;

                        strcpy(rtu_dginfo.origindg, pstr);     //pwaterstr);

                        //rtu地址
                        strncpy(rtuaddr, pstr+6 ,10);
                        long rtunumber = strtol(rtuaddr,NULL,10);
                        rtu_dginfo.dg_rtuaddr = rtunumber;

                        //发报时间
                        strncpy(dgsendtime,pstr+ 16 * 2, 12);
                        strcpy(rtu_dginfo.dg_dgsendtime,dgsendtime);

                        //观测时间
                        strncpy(recordtime,pstr+ 32 *2,10);
                        strcpy(rtu_dginfo.dg_recordtime,recordtime);

                       // 累计降雨量
                       // strncpy(rainfallperfive,pstr + 39 * 2,24);
                       // char rf[3] = {rainfallperfive[22],rainfallperfive[23]};
                       //  long rainlast = strtol(rf ,NULL,10);
                       //  printf ("rainlast is %d\n",rainlast);
                       //  rtu_dginfo.dg_rainfallperfive = rainlast;

                        ////当天降雨量
                        strncpy(thisdayrainfall, pstr + 39 * 2,6);
                        char rfn[7] = {thisdayrainfall[0],thisdayrainfall[1],thisdayrainfall[2],thisdayrainfall[3],thisdayrainfall[4],thisdayrainfall[5]};
                        int rainfallnow = strtol(rfn,NULL,10);
                        rtu_dginfo.dg_thisdayrainfall = rainfallnow;
                        printf ("rainnow is %d\n",rainfallnow);

                        //水位
                        strncpy(waterlevperfive, pstr + 44 *2,8);
                        char wl[9] = {waterlevperfive[0],waterlevperfive[1],waterlevperfive[2],waterlevperfive[3],waterlevperfive[4],waterlevperfive[5],waterlevperfive[6],waterlevperfive[7]};
                        int wlast = strtol(wl,NULL,10);
                        rtu_dginfo.dg_waterlevlastfive = (float)wlast * 0.001;
                        printf("waterlast is %f\n", rtu_dginfo.dg_waterlevlastfive);

                        //水温
                        strncpy(watertemptory, pstr + 50 *2 ,4);
                        char wt[5] = {watertemptory[0],watertemptory[1],watertemptory[2],watertemptory[3]};
                        int wtlast = strtol(wt,NULL,10);
                        rtu_dginfo.dg_watertemp = (float)wtlast * 0.01;
                        printf(" watertemp is %d\n",rtu_dginfo.dg_watertemp);

                        //流量
                        strncpy(waterspeedperfive, pstr + 54 *2 ,4);
                        char ws[5] = {waterspeedperfive[0],waterspeedperfive[1],waterspeedperfive[2],waterspeedperfive[3]};
                        int wslast = strtol(ws,NULL,10);
                        rtu_dginfo.dg_waterspeedlastfive = wslast ;
                        printf(" waterspeed is %d\n",rtu_dginfo.dg_waterspeedlastfive);

                        //电压
                        strncpy(volt, pstr + 58 * 2 ,4);
                        char vt[5] = {volt[0], volt[1],volt[2],volt[3]};
                        int vtvalue =  strtol(vt,NULL,10);
                        rtu_dginfo.dg_volt = (float)vtvalue * 0.01;
                        printf("rtu_dginfovolt is %f\n", rtu_dginfo.dg_volt);

                        strcpy(rtu_dginfo.origindg,"\'");
                        strcat(rtu_dginfo.origindg, pstr); //pwaterstr);
                        strcat(rtu_dginfo.origindg,"\'");

                        saveRTUdatagram(&rtu_dginfo);

                }

                if(functoken[0] == '3' && functoken[1] == '3')           //加报报
                {
                        printf("this is Extra-add  datagram \n");

                        rtu_dginfo.dg_type = 5;

                        strcpy(rtu_dginfo.origindg, pstr);     //pwaterstr);

                        //rtu地址
                        strncpy(rtuaddr, pstr+6 ,10);
                        long rtunumber = strtol(rtuaddr,NULL,10);
                        rtu_dginfo.dg_rtuaddr = rtunumber;

                        //发报时间
                        strncpy(dgsendtime,pstr+ 16 * 2, 12);
                        strcpy(rtu_dginfo.dg_dgsendtime,dgsendtime);

                        //观测时间
                        strncpy(recordtime,pstr+ 32 *2,10);
                        strcpy(rtu_dginfo.dg_recordtime,recordtime);

                       // 累计降雨量
                       // strncpy(rainfallperfive,pstr + 39 * 2,24);
                       // char rf[3] = {rainfallperfive[22],rainfallperfive[23]};
                       //  long rainlast = strtol(rf ,NULL,10);
                       //  printf ("rainlast is %d\n",rainlast);
                       //  rtu_dginfo.dg_rainfallperfive = rainlast;

                        ////当天降雨量
                        strncpy(thisdayrainfall, pstr + 39 * 2,6);
                        char rfn[7] = {thisdayrainfall[0],thisdayrainfall[1],thisdayrainfall[2],thisdayrainfall[3],thisdayrainfall[4],thisdayrainfall[5]};
                        int rainfallnow = strtol(rfn,NULL,10);
                        rtu_dginfo.dg_thisdayrainfall = rainfallnow;
                        printf ("rainnow is %d\n",rainfallnow);


                        //水位
                        strncpy(waterlevperfive, pstr + 44 *2,8);
                        char wl[9] = {waterlevperfive[0],waterlevperfive[1],waterlevperfive[2],waterlevperfive[3],waterlevperfive[4],waterlevperfive[5],waterlevperfive[6],waterlevperfive[7]};
                        int wlast = strtol(wl,NULL,10);
                        rtu_dginfo.dg_waterlevlastfive = (double)wlast * 0.01;
                        printf("waterlast is %0.2f\n", rtu_dginfo.dg_waterlevlastfive);

                        /*
                        //水温
                        strncpy(watertemptory, pstr + 50 *2 ,4);
                        char wt[5] = {watertemptory[0],watertemptory[1],watertemptory[2],watertemptory[3]};
                        int wtlast = strtol(wt,NULL,10);
                        rtu_dginfo.dg_watertemp = (float)wtlast * 0.01;
                        printf(" watertemp is %d\n",rtu_dginfo.dg_watertemp);

                        //流量
                        strncpy(waterspeedperfive, pstr + 54 *2 ,4);
                        char ws[5] = {waterspeedperfive[0],waterspeedperfive[1],waterspeedperfive[2],waterspeedperfive[3]};
                        int wslast = strtol(ws,NULL,10);
                        rtu_dginfo.dg_waterspeedlastfive = wslast ;
                        printf(" waterspeed is %d\n",rtu_dginfo.dg_waterspeedlastfive);

                        */

                        //电压
                        strncpy(volt, pstr + 50 * 2 ,4);
                        char vt[5] = {volt[0], volt[1],volt[2],volt[3]};
                        int vtvalue =  strtol(vt,NULL,10);
                        rtu_dginfo.dg_volt = (float)vtvalue * 0.01;
                        printf("rtu_dginfovolt is %f\n", rtu_dginfo.dg_volt);

                        strcpy(rtu_dginfo.origindg,"\'");
                        strcat(rtu_dginfo.origindg, pstr); //pwaterstr);
                        strcat(rtu_dginfo.origindg,"\'");

                        saveRTUdatagram(&rtu_dginfo);
                }

                     if(functoken[0] == '3' && functoken[1] == '7')             //召测报
                {
                        printf("this is callTest dg \n");

                        // long temp = strtol("32",NULL,10);
                        // printf ("is %d\n",temp);

                        rtu_dginfo.dg_type = 6;

                        strcpy(rtu_dginfo.origindg, pstr);     //pwaterstr);

                        //rtu地址
                        strncpy(rtuaddr, pstr+6 ,10);
                        long rtunumber = strtol(rtuaddr,NULL,10);
                        rtu_dginfo.dg_rtuaddr = rtunumber;

                        //发报时间
                        strncpy(dgsendtime,pstr+ 16 * 2, 12);
                        strcpy(rtu_dginfo.dg_dgsendtime,dgsendtime);

                        //观测时间
                        strncpy(recordtime,pstr+ 32 *2,10);
                        strcpy(rtu_dginfo.dg_recordtime,recordtime);

                       // 累计降雨量
                       // strncpy(rainfallperfive,pstr + 39 * 2,24);
                       // char rf[3] = {rainfallperfive[22],rainfallperfive[23]};
                       //  long rainlast = strtol(rf ,NULL,10);
                       //  printf ("rainlast is %d\n",rainlast);
                       //  rtu_dginfo.dg_rainfallperfive = rainlast;

                        ////当天降雨量
                        strncpy(thisdayrainfall, pstr + 39 * 2,6);
                        char rfn[7] = {thisdayrainfall[0],thisdayrainfall[1],thisdayrainfall[2],thisdayrainfall[3],thisdayrainfall[4],thisdayrainfall[5]};
                        int rainfallnow = strtol(rfn,NULL,10);
                        rtu_dginfo.dg_thisdayrainfall = rainfallnow;
                        printf ("zhaoce rainnow is %d\n",rainfallnow);

                        //水位
                        strncpy(waterlevperfive, pstr + 44 *2,8);
                        char wl[9] = {waterlevperfive[0],waterlevperfive[1],waterlevperfive[2],waterlevperfive[3],waterlevperfive[4],waterlevperfive[5],waterlevperfive[6],waterlevperfive[7]};
                        int wlast = strtol(wl,NULL,10);
                        rtu_dginfo.dg_waterlevlastfive = (float)wlast * 0.001;
                        printf("zhaoce waterlast is %f\n", rtu_dginfo.dg_waterlevlastfive);

                        //水温
                        strncpy(watertemptory, pstr + 50 *2 ,4);
                        char wt[5] = {watertemptory[0],watertemptory[1],watertemptory[2],watertemptory[3]};
                        int wtlast = strtol(wt,NULL,10);
                        rtu_dginfo.dg_watertemp = (float)wtlast * 0.01;
                        printf("zhaoce watertemp is %d\n",rtu_dginfo.dg_watertemp);

                        //流量
                        strncpy(waterspeedperfive, pstr + 54 *2 ,4);
                        char ws[5] = {waterspeedperfive[0],waterspeedperfive[1],waterspeedperfive[2],waterspeedperfive[3]};
                        int wslast = strtol(ws,NULL,10);
                        rtu_dginfo.dg_waterspeedlastfive = wslast ;
                        printf("zhaoce waterspeed is %d\n",rtu_dginfo.dg_waterspeedlastfive);

                        //电压
                        strncpy(volt, pstr + 58 * 2 ,4);
                        char vt[5] = {volt[0], volt[1],volt[2],volt[3]};
                        int vtvalue =  strtol(vt,NULL,10);
                        rtu_dginfo.dg_volt = (float)vtvalue * 0.01;
                        printf("zhaoce rtu_dginfovolt is %f\n", rtu_dginfo.dg_volt);

                        strcpy(rtu_dginfo.origindg,"\'");
                        strcat(rtu_dginfo.origindg, pstr); //pwaterstr);
                        strcat(rtu_dginfo.origindg,"\'");

                        saveRTUdatagram(&rtu_dginfo);

                }


                //17 is the fixed length of datagram header.
                pstr  +=  (dglen + 17) *2 ;
    }


    return &rtu_dginfo;
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
    int nMax_fd = 0;
    char *pfilename = NULL;
    FILENAMEMAC l_filemac;
    PFILENAMEMAC p_filemacstruct;
   // int i = 0;
    FILE* fp[MAX_DEV];
    int nsocketvalue[MAX_DEV];

    char strmac[MAX_DEV][13];

    int ndevicenum = 0;
    int counter =0;

    char str_wholefilepath[128];

    memset(str_wholefilepath,0,128);

    for(int ct =0; ct < MAX_DEV; ct++)
    {
        strcpy(strmac[ct],"\0");
    }
    while(1)
    {
        FD_ZERO(&pThis->m_fdReads);
        //把服务器监听的socket添加到监听的文件描述符集合
        FD_SET(pThis->m_server_socket, &pThis->m_fdReads);
        //设置监听的最大文件描述符
        nMax_fd = nMax_fd > pThis->m_server_socket ? nMax_fd : pThis->m_server_socket;
        std::set<int>::iterator iter = pThis->m_client_socket.begin();

        //std::list<RTUNIT>::iterator rtuiter = pThis->m_client_rtusock.begin();

        //把客户端对应的socket添加到监听的文件描述符集合
        for(; iter != pThis->m_client_socket.end(); ++iter)
        {
           // int keepAlive = 1;
           // int keepIdle = 60;
           // int keepInterval = 5;
          //  int keepCount =3;

           // setsockopt(*iter,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive));
           // setsockopt(*iter,SOL_SOCKET, 4,(void*)&keepIdle,sizeof(keepIdle));
          //  setsockopt(*iter,SOL_SOCKET,5,(void*)&keepInterval,sizeof(keepInterval));
           // setsockopt(*iter,SOL_SOCKET,6,(void*)&keepCount,sizeof(keepCount));

            FD_SET(*iter, &pThis->m_fdReads);

        }
        //判断最大的文件描述符
        if(!pThis->m_client_socket.empty())
        {
            --iter;
            nMax_fd = nMax_fd > (*iter) ? nMax_fd : (*iter);
        }

        //调用select监听所有文件描述符
        int selres = select(nMax_fd + 1, &pThis->m_fdReads, 0, 0, NULL);
        if(-1 == selres)
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

            //刷新在线链表，踢掉僵尸进程
            std::list<RTUNIT>::iterator rtusockiter = pThis->m_client_rtusock.begin();
            struct timeval newtimes;
            gettimeofday(&newtimes,NULL);

            for(rtusockiter = pThis->m_client_rtusock.begin() ; rtusockiter != pThis->m_client_rtusock.end(); ++rtusockiter)
            {
                   long offlinetime =0;
                   RTUNIT zombiertu;
                   zombiertu = *rtusockiter;

                   offlinetime = newtimes.tv_sec - zombiertu.lastmaintaintime;
                   if(offlinetime > 360)   // disconect more than 6 mins
                   {
                       for(iter = pThis->m_client_socket.begin(); iter != pThis->m_client_socket.end(); ++iter)
                       {
                              if(*iter == zombiertu.sock)   //same zombie sock
                              {
                                  close(*iter);
                                  pThis->m_client_socket.erase(iter);
                                  printf("########### Erase sock:%d\n",*iter);
                                  break;
                              }

                       }


                        printf("~~~~~~~~~~ Erase sock rtu name:%d\n",zombiertu.rtunumber);
                        rtusockiter = pThis->m_client_rtusock.erase(rtusockiter);

                   }


            }

            //添加新连接的客户
            pThis->m_client_socket.insert(fd);

            struct timeval newunitime;
            gettimeofday(&newunitime,NULL);

            RTUNIT rtunit;
            rtunit.sock = fd;
            rtunit.rtunumber = 0;
            rtunit.lastmaintaintime = newunitime.tv_sec;

            pThis->m_client_rtusock.push_back(rtunit);

            int inumber = pThis->m_client_socket.size();  //the number of device connected!

            if(inumber > ndevicenum) // new dev connect
            {
                ndevicenum = inumber;
            }

            printf("connected ok\n");
            printf("the connected device number is %d\n",pThis->m_client_rtusock.size());

            RTUIMG rtuimg;
            rtuimg.imgstat = imgno;
            rtuimg.numblocks = 0;
            rtuimg.sock = fd;
            memset(rtuimg.rtuaddress,0,32);

            pThis->m_rtuimg_data.push_back(rtuimg); //insert(rtuimg);

        //    refreshonlinestatue();

        }

        int res = 0;

        for(iter = pThis->m_client_socket.begin(); iter != pThis->m_client_socket.end(); ++iter)
        {
            //判断客户是否可读
            if(-1 != *iter && FD_ISSET(*iter, &pThis->m_fdReads))
            {                   // use unsigned is mandotory.

                  unsigned char buf[MAX_DATA_LEN] = {0};
                  char char_buf[MAX_DATA_LEN] = {0};

                     //   int buf[MAX_DATA_LEN] = {0};

                res = recv(*iter, buf, sizeof(buf), 0);
                if(res > 0)       // valid data
                {
                      printf("Get RTUData, data lenth is %d, the unsigned string is %s\n",res,buf);
                      //printf("Get RTUData is: %s\n",buf);
                      //printf("Get RTUData hex type is: %x\n",buf);

                      if((strstr((char*)buf,"PageStart")!=NULL) && (strstr((char*)buf,"PageEnd")!=NULL))
                      {
                          //printf("lalallala\n");
                          char timebuff[13];
                          char tg_rtu[7];
                          char type[2];
                          char* ptm = NULL;
                          char* ptype = NULL;
                          char* prtuname = NULL;
                          long rtunumbers =0;


                          ptm = strstr((char*)buf, "Time:");
                          strncpy(timebuff,ptm+5,12);

                          ptype =strstr((char*)buf,"Type:");
                          strncpy(type,ptype+5,1);
                          type[1] = 0x0;

                          prtuname=strstr((char*)buf,"Rtu:");
                          strncpy(tg_rtu,prtuname+4,6);
                          tg_rtu[6]=0x0;
                          rtunumbers = strtol(tg_rtu,NULL,10);
                         // printf("yyyyyyyyyy this rtunumber is :%ld\n",rtunumbers);

                          timebuff[12] = 0x0;
                          //printf("lallalala  %s\n", timebuff);



                          PRESPONEDOWN presdown = NULL;

                          int msgtype =0;
                          msgtype = strtol(type,NULL,10);

                          presdown = filldownrtustruct(timebuff,msgtype);

                          std::set<int>::iterator timeiter = pThis->m_client_socket.begin();

                          std::list<RTUNIT>::iterator tg_rtuiter = pThis->m_client_rtusock.begin();


                          bool b_targetrtu = false;
                         // for(timeiter = pThis->m_client_socket.begin(); timeiter != pThis->m_client_socket.end(); ++timeiter)
                         // {
                              //   b_targetrtu = false;

                                 for(tg_rtuiter = pThis->m_client_rtusock.begin(); tg_rtuiter != pThis->m_client_rtusock.end(); ++tg_rtuiter)
                                 {

                                      RTUNIT tg_tbdrtu;
                                      tg_tbdrtu = *tg_rtuiter;
                                      b_targetrtu = false;

                                     // printf("\ndddddddd the rtu sock is %d and number is %ld\n",tg_tbdrtu.sock,tg_tbdrtu.rtunumber);
                                      if(rtunumbers == tg_tbdrtu.rtunumber)
                                      {

                                         b_targetrtu = true;
                                         send(tg_tbdrtu.sock, presdown, sizeof(RESPONEDOWN), 0);
                                         printf("find target rtu and downsend and  leave,online-sock is %d\n",tg_tbdrtu.sock);
                                         break;
                                      }

                                 }

                                // refreshonlinestatue(pParam);

                                /* if(b_targetrtu )
                                 {
                                     send(*timeiter, presdown, sizeof(RESPONEDOWN), 0);
                                     printf("find target rtu and leave,sock is %d\n",*timeiter);
                                     break;
                                 }*/

                                  // send(*timeiter, presdown, sizeof(RESPONEDOWN), 0);
                       //   }

                          printf("time down command send %s\n", timebuff);

                          continue;
                      }



                      printf("the val is:\n");

                      for(int i=0;i<res;i++)
                      {
                             printf("%02x" ,buf[i]);
                             sprintf(&char_buf[i*2],"%02x",buf[i]);
                             //printf(":%02x~",char_buf[i]);

                      }
                      printf("\n the end \n");


                      PRTUDATAGRAM p_rtuDg = NULL;


                        printf("********** test **********\n");
                        p_rtuDg = analysiswaterstring(*iter,char_buf,pParam);
                      //  printf("string is :%s\n", p_rtuDg->origindg);
                        printf("volt is %f\n",p_rtuDg->dg_volt);
                        printf("********** end test **********\n");

                        //m_rtuimg_data

                        processimage(char_buf, buf,*iter,pParam);


                    // ************************** query and search image data  *******************




                }
                else if(0 == res)  //even ctrl+c send from client to interrept, it will send close gracely by python!
                {
                    int leaveid = *iter;

                     pThis->m_client_socket.erase(iter);


                     std::list<RTUNIT>::iterator rtuiter = pThis->m_client_rtusock.begin();

                    for(; rtuiter != pThis->m_client_rtusock.end(); ++rtuiter)
                    {
                        RTUNIT tbdrtu;
                        tbdrtu = *rtuiter;

                        if(tbdrtu.sock == *iter)
                        {
                            rtuiter = pThis->m_client_rtusock.erase(rtuiter);
                            break;
                        }

                    }

                    close(leaveid);

                    //refreshonlinestatue(pParam); //in this private func, cannot invoke this function
                    printf("one client leave,socket id: %d, the size of client list is %d\n",leaveid,pThis->m_client_rtusock.size());
                }
                else
                {
                    printf("recv error\n");
                    close(*iter);
                }
            }
        }
    }
}

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

//管理线程，用于创建处理线程
void * TcpServer::ManageThread(void * pParam)
{
    if(!pParam)
    {
        return 0;
    }
    pthread_t pid;
    TcpServer * pThis = (TcpServer *)pParam;
    while(1)
    {
        //使用互斥量
        pthread_mutex_lock(&pThis->m_mutex);
        int nCount = pThis->m_data.size();
        pthread_mutex_unlock(&pThis->m_mutex);
        if(nCount > 0)
        {
            pid = 0;
            //创建处理线程
            if( 0 != pthread_create(&pid, NULL, OperatorThread, pParam))
            {
                printf("creae operator thread failed\n");
            }
        }
        //防止抢占CPU资源
        usleep(100);
    }
}

//数据处理线程
void * TcpServer::OperatorThread(void * pParam)
{
    if(!pParam)
    {
        return 0;
    }

    TcpServer * pThis = (TcpServer*)pParam;

    pthread_mutex_lock(&pThis->m_mutex);
    if(!pThis->m_data.empty())
    {
        ServerData data = pThis->m_data.front();
        pThis->m_data.pop_front();
        if(pThis->m_operaFunc)
        {
            //把数据交给回调函数处理
            pThis->m_operaFunc((char *)&data, sizeof(data), data.socket);
        }
    }
    pthread_mutex_unlock(&pThis->m_mutex);
    return NULL;
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
    return true;
}


int TcpServer::processimage(char *char_buf, unsigned char * buf , int sock, void * pParam)
{

        if(!pParam)
        {
            printf("param is null\n");
            return 0;
        }

        char *pstr = char_buf;
        char *pcomboimg = NULL;

        unsigned char *pimgbuf = buf;

        //not image package
        /*if(((pcomboimg = strstr(pstr,"7ba2")) == NULL)  && ((pcomboimg = strstr(pstr,"7ba1")) == NULL) )
        {
            return 0;
        }*/

        while((pstr = strstr(pstr, "7e7e"))  != NULL)
        {
            char functoken[3];
            char dglength[3];

                    strncpy(dglength,pstr+ 12*2 ,2);
                    strncpy(functoken,char_buf+20,2);

            long dglen = strtol(dglength ,NULL,16);

                     if(((pcomboimg = strstr(pstr,"7ba2")) != NULL)  || ((pcomboimg = strstr(pstr,"7ba1")) != NULL) )      //7e7ewith 7ba2,7ba1,need seperate
                     {
                                //   pimgbuf = buf + dglen + 17;
                                int noffset = (pcomboimg - pstr);
                                pimgbuf + (noffset / 2);
                               // printf("************ the offset origin is %d   @@@@@@@@@@@@@\n",dglen +17);
                                printf("$$$$$$$$$$$ the offset modify is %d   @@@@@@@@@@@@@\n",noffset );
                     }
                     else                                           //single datagram
                    {
                        if(functoken[0] == '2' && functoken[1] == 'f')  //weichi
                        {
                                    return 0;
                        }
                        if(functoken[0] == '3' && functoken[1] == '2')   //ding shi
                        {
                                  return 0;
                        }


                    }

                     //17 is the fixed length of datagram header.
                pstr  +=  (dglen + 17) *2 ;

      }


   /* if(buf[0] == 0x7e && buf[1] == 0x7e )
    {
        return 0;
    }*/


      // char  onlyimgname[128];
     //  memset(onlyimgname, 0, 128);



    TcpServer * pThis = (TcpServer*)pParam;

                     //response for img query
                     QUERYIMGINFO quinfo , imgreq;

                     quinfo.qhead = {0x7B,0xA1,0x00,0x0A};
                     imgreq.qhead = {0x7B,0xA2,0x00,0x0A};

                     quinfo.qdatahd = {'I','M','A','G'};
                     imgreq.qdatahd = {'I','M','A','G'};

                     quinfo.qdataname = {0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

                     quinfo.qdatanum[0] = 0x00;

                     imgreq.qdatasize[0] = 0x00;
                     imgreq.qdatasize[1] = 0x00;

                     quinfo.qdatasize[0] = 0x00;
                     quinfo.qdatasize[1] = 0x00;

                     quinfo.qcrc = {'X','X','X','X'};
                     quinfo.qdatafin = {0x0D,0x0A};
                     quinfo.qend[0] = 0x7B;

                     imgreq.qcrc = {'X','X','X','X'};
                     imgreq.qdatafin = {0x0D,0x0A};
                     imgreq.qend[0] = 0x7B;

                     FILE *imgfp;

                     IMGSTAT  t_imgstat;   //imgno=0, imgquery,imgtransfer,imgdel

                      //here we scan the sock list to find the matched sock, and save rtu address!
                      std::list<RTUIMG>::iterator iter = pThis->m_rtuimg_data.begin();
                      for(; iter != pThis->m_rtuimg_data.end(); ++iter)
                      {
                          RTUIMG currRtuimg;
                          currRtuimg = *iter;

                          if(currRtuimg.sock == sock)   //same socke
                          {

                                   char imagedataname[128];
                                        memset(imagedataname,0,128);
                                   char names[128];
                                        memset(names,0,128);

                                if(pimgbuf[0] == 0x7B && pimgbuf[1] == 0xA1)          //qreceiveinfo.qhead[0] == 0x7B && qreceiveinfo.qhead[1] == 0xA1  )
                                {
                                   // memset(imgname,0,7);
                                   PQUERYIMGINFO  pgetinfo = NULL;                //IMGDATA, *PIMGDATA;

                                   char *pimgfoldpath = IMAGEFOLDPATH;

                                   pgetinfo = (PQUERYIMGINFO)pimgbuf;

                                   memcpy(currRtuimg.imagename,pgetinfo->qdataname,7);

                                   currRtuimg.numblocks = pgetinfo->qdatanum[0];
                                   currRtuimg.imgstat =  imgtransfer;

                                   if(currRtuimg.numblocks ==0)        //empty image
                                   {
                                       printf("No images found!\n");
                                       return -1;

                                   }

                                     memcpy(imgreq.qdataname,currRtuimg.imagename,7);
                                     // sprintf(names,"rtuimg\/%s",currRtuimg.rtuaddress);
                                     sprintf(names,"%s\/%s",pimgfoldpath,currRtuimg.rtuaddress);

                                     umask(0);
                                     mkdir(names,0775);    // here need to determine wether the path is valid


                                    sprintf(currRtuimg.imagedataname,"%d",imgreq.qdataname[6]);

                                    //cameraorder
                                    currRtuimg.cameraorder = strtol(currRtuimg.imagedataname,NULL,10);

                                    sprintf(currRtuimg.imagedataname,"%s\/%02d%02d%02d_%02d%02d%02d-%02d.jpg",names, imgreq.qdataname[0],imgreq.qdataname[1],imgreq.qdataname[2],
                                             imgreq.qdataname[3],imgreq.qdataname[4],imgreq.qdataname[5],imgreq.qdataname[6]);


                                    sprintf(currRtuimg.onlyimgname,"%02d%02d%02d_%02d%02d%02d-%02d.jpg", imgreq.qdataname[0],imgreq.qdataname[1],imgreq.qdataname[2],
                                             imgreq.qdataname[3],imgreq.qdataname[4],imgreq.qdataname[5],imgreq.qdataname[6]);

                                      *iter = currRtuimg;

                                    printf("******* the pic path is %s\n", currRtuimg.imagedataname);


                                   //return 0;

                                   //pThis->m_rtuimg_data
                                   //g_sendtime =2;
                                   //return  g_sendtime ;                           //set status 'transfer'
                                }
                               // if(buf[0] == 0x7B && buf[1] == 0xA2)
                               // {
                               //     currRtuimg.imgstat =  imgtransfer;
                               // }

                                   t_imgstat = currRtuimg.imgstat ;
                                   //memcpy(*iter.rtuaddress,rtuaddr,11);

                                    struct timeval tv;
                                    switch(t_imgstat)                               //if is not 0x7ba1 or 0x7ba2
                                     {
                                         case imgno:
                                              break;
                                         case imgquery:     //query imginfo

                                                       tv.tv_sec = 18;
                                                       tv.tv_usec = 0;
                                                       select(0,NULL,NULL,NULL, &tv);
                                                       send(sock, &quinfo, sizeof(quinfo), 0);
                                                       currRtuimg.imgstat  = imgtransfer;
                                                       currRtuimg.sendcounter = 1;
                                              break;
                                         case imgtransfer:

                                                 if(currRtuimg.numblocks != 0)             //get image data , status is transfer
                                                 {

                                                    imgfp = fopen(currRtuimg.imagedataname,"a+");  //"image.jpg","a+");
                                                    unsigned char databuffs[512] = {0};
                                                    PIMGDATA  pimgdata = NULL;


                                                          if( currRtuimg.sendcounter  <= currRtuimg.numblocks +1)
                                                          {
                                                                   printf("the block is %d\n", currRtuimg.sendcounter);
                                                                   printf("the num is %d\n", currRtuimg.numblocks);

                                                                   imgreq.qdatanum[0] = currRtuimg.sendcounter++;

                                                                   memcpy(imgreq.qdataname,currRtuimg.imagename,7);

                                                                    if(pimgbuf[0] == 0x7B && pimgbuf[1] == 0xA2)          //qreceiveinfo.qhead[0] == 0x7B && qreceiveinfo.qhead[1] == 0xA1  )
                                                                    {
                                                                       //  printf("the block is %d\n", currRtuimg.sendcounter);
                                                                        // printf("the num is %d\n", currRtuimg.numblocks);

                                                                        size_t lenwrite =0;
                                                                        pimgdata = (PIMGDATA)pimgbuf;
                                                                        memcpy(databuffs,pimgdata->imgdata,512);
                                                                        lenwrite =fwrite(databuffs, sizeof(unsigned char),512,imgfp);
                                                                        //m_nblocks++;
                                                                        printf("send pacage num is %d\n", currRtuimg.sendcounter);
                                                                        // send(*iter, &imgreq, sizeof(imgreq), 0);

                                                                        send(sock, &imgreq, sizeof(imgreq), 0);
                                                                    }
                                                                   if(imgreq.qdatanum[0] == 1)
                                                                   {
                                                                        send(sock,&imgreq, sizeof(imgreq), 0);
                                                                           //send(*iter, &imgreq, sizeof(imgreq), 0);
                                                                   }
                                                                   if(currRtuimg.sendcounter == currRtuimg.numblocks +2)
                                                                   {
                                                                        currRtuimg.numblocks = 0;           //reset
                                                                        //b_needimg = false;

                                                                        currRtuimg.imgstat = imgdel;

                                                                        saveRTU_IMAGES(&currRtuimg);

                                                                        printf("------------- IMAGE recieved ! -----\n");

                                                                        quinfo.qhead = {0x7B,0xA3,0x00,0x0A};
                                                                        memcpy(quinfo.qdataname,currRtuimg.imagename,7);
                                                                        quinfo.qdatanum = {0x00};
                                                                        quinfo.qdatasize = {0x00,0x00};

                                                                        send(sock, &quinfo, sizeof(quinfo), 0);
                                                                        currRtuimg.reqpair--;

                                                                        //ready = 0;         //reset trsfer status.

                                                                   }

                                                        }
                                                        else
                                                        {
                                                                         currRtuimg.numblocks = 0;           //reset
                                                        }

                                                                   fclose(imgfp);

                                                  }


                                              break;
                                         case imgdel:

                                                                        //send(*iter, &quinfo, sizeof(quinfo), 0);
                                                                        printf("------------- IMAGE del command send! -----\n");
                                                                        if(currRtuimg.reqpair == 1)
                                                                        {
                                                                           tv.tv_sec = 30;
                                                                           tv.tv_usec = 0;
                                                                           select(0,NULL,NULL,NULL, &tv);
                                                                           send(sock, &quinfo, sizeof(quinfo), 0);
                                                                           currRtuimg.sendcounter = 1;
                                                                           currRtuimg.imgstat  = imgtransfer;


                                                                        }
                                                                        if(currRtuimg.reqpair == 0)
                                                                        {
                                                                           currRtuimg.imgstat = imgno;
                                                                        }

                                              break;
                                         default:
                                              *iter = currRtuimg;
                                              break;
                                     }


                               *iter = currRtuimg;

                          }
                      }


}


