#include "srv.h"
#include "login.h"

int sfd;//create socket
int bfd;
fd_set fdsr;//create select
int * amount = NULL;//current user number
int main(){

	initSrv();
	free_all();
	return 0;
}

void initSrv(){

	sfd = socket(AF_INET,SOCK_STREAM,0);//create socket
	if(sfd == -1)
	{
		printf("start socket error\n");
		return;
	}
	printf("socket ok \n");

	//IPv4 parameters
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(7777);

	if((bfd = bind(sfd,(struct sockaddr *)(&addr) ,sizeof(addr)) )== -1)//bind ip and port
	{
		printf("bind error \n");
		return;
	}
	printf("bind ok ,ip is:%s,port is %d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));

	if(listen(sfd,3) == -1)//set listen
	{
		printf("listen error\n");
		return;
	}
	printf("listen ok\n");

	//client parameters
	socklen_t cli_len;
	struct sockaddr_in cli_addr;
	memset(&cli_addr,0,sizeof(cli_addr));
	cli_len =  sizeof(cli_addr);

	ssize_t num;
	char buf[BUFSIZE];

	struct timeval tv;//select struct time
	int maxsock = sfd;//max of select
	
	int ready,ret;//return value
	int curr_count = 0;//the number of online
	amount = &curr_count;
	int curr_user = 0;

	//create user list
	if(create_list()){
		return;
	}

	while(1)
	{

		//select setting
		FD_ZERO(&fdsr);
		FD_SET(0,&fdsr);
		FD_SET(sfd,&fdsr);
		tv.tv_sec = 30;
		tv.tv_usec = 0;

		//adding all user into select
		u_sock = head->next;
		while(u_sock){

			FD_SET(u_sock->sock_id,&fdsr);
			u_sock = u_sock->next;
		}

		ready = select(maxsock + 1,&fdsr,NULL,NULL,&tv);//init select

		if(ready == -1){
			printf("select error\n");
			break;
		}
		else if(ready == 0){
			//printf("select timeout\n");
			continue;
		}

		//listen event
		if(FD_ISSET(0,&fdsr)){
			int opt ;
			scanf("%d",&opt);
			if(opt == 1)
				show_list();	
		}
		if(FD_ISSET(sfd,&fdsr))
		{


			int cfd = accept(sfd,(struct sockaddr *)&cli_addr,&cli_len);
			if(cfd == -1)
			{
				printf("accept error\n");
				continue;
			}

			//adding socket into select
			if(curr_count < MAXSOCK )
			{

				//fd_sock[curr_count++] = cfd;
				//list_ret = add_user(-1,cfd);
				if(add_user(-1,cfd)){
					printf("lack of memory\n");
				}else{
					curr_count++;
				}
				//printf("fd is %d\tip is: %s,port is %d\n",cfd,inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
				printf("the number of online: %d\n",curr_count);
				if(cfd > maxsock)
					maxsock = cfd;
			}
			else {

				printf("over the max value of the number of user online\n");
				close(cfd);

			}

		}

		//client event
		act_sock = head->next;
		while(act_sock){

			read_cln(ret,act_sock->sock_id,buf);
			act_sock = act_sock->next;
		}


	}


}

void read_cln(int ret,int sid,char * buf){

	//which client events
	if(FD_ISSET(sid,&fdsr)){

		//read from client
		ret = recv(sid,buf,BUFSIZE,0);

		if(ret <= 0){
			printf("client %d is closed\n",sid);
			close(sid);
			FD_CLR(sid,&fdsr);
			if(del_user(sid)){
				printf("user does not exist\n");
			}
			*amount -= 1;
			printf("the number of online: %d\n",*amount);

		}
		else{

			dealMsg(sid,ret,buf);

		}

	}



}

//server forwarding
void dealMsg(int act,int ret,char * buf){

	Msg * msg = (Msg *)malloc(sizeof(Msg));
	if(msg == NULL){
		perror("msg malloc");
		exit(1);
	}

	memset(msg,0,sizeof(Msg));

	//printf("recv num is:%d\n",ret);
	memcpy(msg,buf,ret);
	printf("cmd is %d,dst is %d,dev_id is %d,type is %d\n",ntohl(msg->cmd),ntohl(msg->dst),ntohl(msg->dev_id),ntohl(msg->type));
	
	//login operation
	if(ntohl(msg->type) == 1){
		msg->type = htonl(110);
		//login sucess or not
		if(login_user(ntohl(msg->dev_id),act)){
			msg->confirm = htonl(404);	
		}else{
			msg->confirm = htonl(400);	
		}
		memcpy(buf,msg,sizeof(Msg));
		//reply client
		if((ret = send(act,buf,sizeof(Msg),0)) < 0){
			perror("login response fail");
		}
	}
	//control operation
	else if(ntohl(msg->type) == 2){	
		int sid = check_user(ntohl(msg->dst));
		if(!sid){
			printf("device is not online \n");
		}
		else {
			if((ret = send(sid,buf,ret,0)) > 0){
				printf("send success\n");
			}else if(ret < 0){
				perror("client expcetion");
			}
		}
	}


}



