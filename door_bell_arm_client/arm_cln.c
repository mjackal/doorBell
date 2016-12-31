#include "arm_cln.h"

int sfd,cfd;//socket id
int fd;//file of device
static int dev = 1001;//device id
int main(int arg,char *agv[]){

	initConn(agv[1]);
	return 0;
}


//init connection
void initConn(char * ip){

	char * file="/dev/bell";
	ssize_t len;
	char buf[BUFLEN];

	Msg * msg = (Msg *)malloc(sizeof(Msg));

	//select parameters
	fd_set rfds;
	struct timeval tv;
	int retval ,maxfd = 0;

	//exit or not	
	int flag = 0;


	sfd = socket(AF_INET,SOCK_STREAM,0);//create socket
	if(sfd == -1)
	{
		printf("socket error\n");
		return;
	}

	printf("socket ok\n");


	//open device node
	if((fd = open(file,O_RDWR|O_NDELAY)) < 0){
		perror("open file fail\n");
		exit(1);
	}

	//create pthread to deal with reading from fd
	pthread_t pid;
	pthread_create(&pid,NULL,(void*)read_fd,NULL);


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
		exit(1);
	}

	printf("connect ok\n");

	//device login
	while(login(sfd,&flag,buf,msg));

	//select events
	while(1)
	{

		//select setting
		FD_ZERO(&rfds);
		//FD_SET(fd,&rfds);
		//maxfd = fd;
		FD_SET(sfd,&rfds);
		if(maxfd < sfd)
			maxfd = sfd;

		//select time-out
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		//init select
		retval = select(maxfd + 1,&rfds,NULL,NULL,&tv);
		//printf("select return is %d\n",retval);
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
					len = recv(sfd,buf,BUFLEN,0);//read from server
					if(len <= 0){
						printf("msg recv fail\n");
						printf("srv exit\n");
						flag = 1;
					}else{
						//deal msg
						printf("read form server\n");
						recvMsg(len,&flag,buf,msg);
					}
					break;

				}

				/*if(FD_ISSET(fd,&rfds))
				  {
				  printf("fd===b===\n");
				  bzero(buf,BUFLEN);
				  bzero(msg,sizeof(Msg));
				  sendMsg(sfd,fd,sizeof(Msg),buf,msg);
				  printf("fd===e===\n");
				  }*/

		}
		if(flag)
			break;

	}
	close(fd);
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
				printf("input dst and cmd\n");
			}
			else if(ntohl(msg->confirm) == 404){
				printf("login fial\n");	
				while(login(sfd,flag,buf,msg));
			}
		}else{
			//read from server,client cmd
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
	msg->type = htonl(1);
	msg->dev_id = htonl(dev);
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

//send into server
void sendMsg(int sfd,int fd,int len,char * buf,Msg * msg){

	int i,ret;
	int cmd = 1;
	int dev_wq;
	if(read(fd,&dev_wq,4) > 0){
		if(dev_wq == 1000){
			msg->type = htonl(2);
			msg->dev_id = htonl(dev);
			msg->dst = htonl(1002);
			memcpy(buf,msg,len);
			if((ret = send(sfd,buf,len,0)) > 0){
				printf(" msg send success and len is %d\n",ret);
			}else if(ret < 0){
				printf("msg send fail\n");
			}
		}

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

void read_fd(){

	char buf[BUFLEN];
	Msg * msg = (Msg *)malloc(sizeof(Msg));
	while(1){
		bzero(buf,BUFLEN);
		bzero(msg,sizeof(Msg));
		sendMsg(sfd,fd,sizeof(Msg),buf,msg);
	}
	free(msg);
}


