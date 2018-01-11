
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<libpq-fe.h>
int savertudg()   //main()
{
 PGconn *conn;
 PGresult *result;

 const char *conn_str="host=localhost dbname=WlMonitor user=postgres password=pgsql123 port =5432";
// const char *insert_str="INSERT INTO rtu_12345678 VALUES (4,'wanzou3',12345678,43,35,23,now(),0,'','123456789','12345678','123',23);";

  char insert_str[256];
  char *name="rtu_12345678";
  int id = 9;
  char *site = "\'巫山1\'";
  int stationo = 12345678;


  memset(insert_str,0,256*sizeof(char));
  sprintf(insert_str,"INSERT INTO %s VALUES (%d,%s,%d,23,19,13,now(),0,'','123456789','12345678','123',23);",name, id, site,stationo );
//  sprintf(insert_str,"INSERT INTO %s VALUES (%d,%s,%d,43,35,23,now(),0);",name, id, site,stationo );

  printf(insert_str);

 conn=PQconnectdb(conn_str);

 if(PQstatus(conn) == CONNECTION_BAD)
  {
      fprintf(stderr,"connection to %s failed", conn_str);
      PQerrorMessage(conn);
  }
 else
  {
     printf("connection ok\n");
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
  PQfinish(conn);
  return EXIT_SUCCESS;
}
