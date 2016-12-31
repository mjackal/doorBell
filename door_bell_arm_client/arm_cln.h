#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/time.h>
#include<sys/select.h>
#include<unistd.h>
#include<pthread.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<fcntl.h>


#define BUFLEN 20

typedef struct msg_tag{

	int type;//1 is login,2 is control,110 is login response
	int confirm;//400 is success,404 is fail
	int dev_id;//the device id
	int dst;//target device
	int cmd;//control command

}Msg;


void sendMsg(int ,int ,int ,char * ,Msg * );//send into server
void initConn(char *);//init connection
void recvMsg(int,int *,char * ,Msg*);//read from server
int login(int,int *,char *,Msg *);//device login
void read_fd();//thread read from fd
