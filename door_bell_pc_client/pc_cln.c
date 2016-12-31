#include "pc_cln.h"

int sfd,cfd;//socket id
static int dev = -1;//device id

int main(){

	initConn();
	return 0;
}

//init connection
void initConn(){

	char * ip = "127.0.0.1";
	ssize_t len;
	char buf[BUFLEN];

	Msg * msg = (Msg *)malloc(sizeof(Msg));

	//select parameters
	fd_set rfds;
	struct timeval tv;
	int retval ,maxfd;

	//exit or not	
	int flag = 0;

	sfd = socket(AF_INET,SOCK_STREAM,0);//create socket
	if(sfd == -1)
	{
		printf("socket error\n");
		return;
	}

	printf("socket ok\n");

	//ipv4 parameters
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	inet_pton(AF_INET,ip,&addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7777);

	//connect into server
	if((cfd = connect(sfd,(struct sockaddr *)&addr,sizeof(addr))) == -1)
	{
		printf("connect error\n");
		return;
	}

	printf("connect ok\n");
	//device login
	while(login(sfd,&flag,buf,msg));

	//select events
	while(1)
	{
		//select setting
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);//0 is stdin,1 is stdout,2 is stderr
		maxfd = 0;
		FD_SET(sfd,&rfds);
		if(maxfd < sfd)
			maxfd = sfd;

		//select time-out
		tv.tv_sec = 30;
		tv.tv_usec = 0;

		//init select
		retval = select(maxfd + 1,&rfds,NULL,NULL,&tv);

		switch(retval)
		{
			case 0:
				break;
			case -1:
				printf("select error\n");
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
					if(dev == -1){
						break;
					}
					bzero(buf,BUFLEN);
					bzero(msg,sizeof(Msg));
					scanf("%d%d",&msg->dst,&msg->cmd);
					msg->type = htonl(2);
					msg->cmd = htonl(msg->cmd);
					msg->dst = htonl(msg->dst);
					msg->dev_id = htonl(dev);
					memcpy(buf,msg,sizeof(Msg));
					if((len = send(sfd,buf,sizeof(buf),0)) > 0){
						printf("send success\n");
					}else{
						printf("send fail\n");
					}
				}
				break;

		}
		if(flag)
			break;

	}
	close(sfd);
	free(msg);

}


void  recvMsg(int len,int * flag,char * buf,Msg * msg){

	if(len > 0){

		//printf("recv len is:%d\n",len);
		memcpy(msg,buf,len);
		printf("recv cmd is %d,dev_id is %d,type is %d,confirm is %d\n",ntohl(msg->cmd),ntohl(msg->dev_id),ntohl(msg->type),ntohl(msg->confirm));
		if(ntohl(msg->type) == 110){
			if(ntohl(msg->confirm) == 400){	
				printf("login success\n");
				dev = ntohl(msg->dev_id);
				printf("input dst and cmd\n");
			}
			else if(ntohl(msg->confirm) == 404){
				printf("login fial\n");	
				while(login(sfd,flag,buf,msg));

			}
		}
		else if(ntohl(msg->type) == 2 && ntohl(msg->dev_id) == 1001){
			printf("有人来啦……有人来啦……\n");
		}
	}else
	{

		printf("server exit\n");
		*flag = 1;
	}


}

//adding device into server
int login(int sid,int * flag,char * buf,Msg *msg){
	int len;
	bzero(msg,sizeof(Msg));
	bzero(buf,BUFLEN);
	printf("please set device id\n");
	scanf("%d",&msg->dev_id);
	msg->type = htonl(1);
	msg->dev_id = htonl(msg->dev_id);
	memcpy(buf,msg,sizeof(Msg));
	if( send(sid,buf,sizeof(Msg),0) > 0){
		len = recv(sfd,buf,BUFLEN,0);
		recvMsg(len,flag,buf,msg);
		return 0;
	}else{
		printf("login fail\n");
		return -1;
	}
}
