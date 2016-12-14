#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/time.h>
#include<sys/select.h>
#include<unistd.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<fcntl.h>

#define BUFLEN 12

typedef struct msg_tag{

	int cmd;
	int dev_id;
	int dst;

}Msg;

void sendMsg(int sfd,int fd,int len,char * buf,Msg * msg);
void initConn(char *);


int main(int arg,char *agv[]){

	initConn(agv[1]);
	return 0;
}

void initConn(char * ip){

	int sfd,cfd;
	char * file="/dev/bell";
	int fd;
	ssize_t len;
	char buf[BUFLEN];

	Msg * msg = (Msg *)malloc(sizeof(Msg));
	fd_set rfds;
	struct timeval tv;
	int retval ,maxfd;	
	int flag = 0;

	sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd == -1)
	{
		printf("socket error\n");
		return;
	}

	printf("socket ok\n");


	if((fd = open(file,O_RDWR)) < 0){
		perror("open file fail\n");
		exit(1);
	}

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	inet_pton(AF_INET,ip,&addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7777);

	if((cfd = connect(sfd,(struct sockaddr *)&addr,sizeof(addr))) == -1)
	{
		printf("connect error\n");
		exit(1);
	}

	printf("connect ok\n");

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(fd,&rfds);
		maxfd = fd;
		FD_SET(sfd,&rfds);
		if(maxfd < sfd)
			maxfd = sfd;
		tv.tv_sec = 30;
		tv.tv_usec = 0;

		retval = select(maxfd + 1,&rfds,NULL,NULL,&tv);

		switch(retval)
		{
			case 0:
				//printf("没有消息可读写\n");
				break;
			case -1:
				printf("select出错\n");
				flag = 1;
				break;
			default:
				if(FD_ISSET(sfd,&rfds))
				{
					bzero(buf,BUFLEN);
					bzero(msg,sizeof(msg));
					len = recv(sfd,buf,BUFLEN,0);
					if(len < 0)
						printf("msg recv fail\n");
					else
					{ 
						printf("srv exit\n");
						flag = 1;
					}
					break;

				}

				if(FD_ISSET(fd,&rfds))
				{
					bzero(buf,BUFLEN);
					bzero(msg,sizeof(Msg));
					sendMsg(sfd,fd,sizeof(Msg),buf,msg);
				}

		}
		if(flag)
			break;

	}
	close(fd);
	close(sfd);

}


void sendMsg(int sfd,int fd,int rlen,char * buf,Msg * msg){

	int len;
	if(read(fd,&msg->dev_id,4) > 0){
		msg->dev_id = htonl(msg->dev_id);
		msg->dst = htonl(4);
		memcpy(buf,msg,rlen);
		if((len = send(sfd,buf,rlen,0)) > 0){
			printf(" msg send success and len is %d\n",len);
		}else if(len < 0){
			printf("msg send fail\n");
		}

		int i,ret;
		int cmd = 1;
		for(i = 0;i < 2;i++){
			ret = ioctl(fd,cmd,0);
			if(ret < 0){
				perror("ioctl");
			}
			cmd = 0;
			sleep(1);
			ret = ioctl(fd,cmd,0);
			if(ret < 0){
				perror("ioctl");
			}
			cmd = 1;
			sleep(1);

		}
	}



}
