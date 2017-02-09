#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/time.h>
#include<sys/select.h>
#include<unistd.h>

#define MAXSOCK 1024
#define BUFSIZE 20

typedef struct msg_tag{

	int type;//1 is login,2 is control,110 is login response
	int confirm;//400 is success,404 is fail
	int dev_id;//the device id
	int dst;//target device
	int cmd;//control command

}Msg;

void initSrv();//init connection
void read_cln(int,int,char *);//client event
void dealMsg(int,int,char *);//server forwarding
