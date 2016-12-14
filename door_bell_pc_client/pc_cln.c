#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/time.h>
#include<sys/select.h>
#include<unistd.h>
#define BUFLEN 12

typedef struct msg_tag{

	int cmd;
	int dev_id;
	int dst;

}Msg;

void initConn();
void recvMsg(int,int *,char * ,Msg*);

int main(){

	initConn();
	return 0;
}

void initConn(){

	int sfd,cfd;
	char * ip = "127.0.0.1";
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

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	inet_pton(AF_INET,ip,&addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7777);

	if((cfd = connect(sfd,(struct sockaddr *)&addr,sizeof(addr))) == -1)
	{
		printf("connect error\n");
		return;
	}

	printf("connect ok\n");

	while(1)
	{
		FD_ZERO(&rfds);
		//0标准输入，1标准输出、2标准错误输出
		FD_SET(0,&rfds);
		maxfd = 0;
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
					bzero(msg,sizeof(Msg));
					len = recv(sfd,buf,BUFLEN,0);
					recvMsg(len,&flag,buf,msg);

				}else if(FD_ISSET(0,&rfds)){
					bzero(buf,BUFLEN);
					bzero(msg,sizeof(Msg));
					scanf("%d%d%d",&msg->dst,&msg->cmd,&msg->dev_id);
					msg->cmd = htonl(msg->cmd);
					msg->dst = htonl(msg->dst);
					msg->dev_id = htonl(msg->dev_id);
					memcpy(buf,msg,sizeof(Msg));
					if((len = send(sfd,buf,sizeof(buf),0)) > 0){
						printf("发送信息成功\n");
					}else{
						printf("消息发送失败\n");
					}
				}
				break;

		}
		if(flag)
			break;

	}
	close(sfd);

}


void  recvMsg(int len,int * flag,char * buf,Msg * msg){

	if(len > 0){

		printf("recv len is:%d\n",len);
		memcpy(msg,buf,len);
		printf("recv cmd is %d,dev_id is %d,dst is %d\n",ntohl(msg->cmd),ntohl(msg->dev_id),ntohl(msg->dst));
		if(ntohl(msg->dev_id) == 1001){
			printf("有人来啦……有人来啦……\n");
		}
	}else
	{
		if(len < 0)
			printf("接受消息失败\n");
		else
		{ 
			printf("服务器退出\n");
			*flag = 1;
		}
	}


}
