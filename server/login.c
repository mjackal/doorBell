#include "login.h"

user * head = NULL;//the head of user list
user * last = NULL;//the end of user list
user * u_sock = NULL;//keeping login user
user * act_sock = NULL;//activate socket

//creating user list
int create_list(){

	head = (user *)malloc(sizeof(user));
	if(head == NULL){
		perror("create failed");
		return -1;
	}
	head->next = NULL;
	last = head;

	return 0;
}

//adding into user list
int add_user(int dev_id,int sock_id){

	user * ptu = (user*)malloc(sizeof(user));
	if(!ptu){
		perror("add failed");
		return -1;
	}
	ptu->dev_id = dev_id;
	ptu->sock_id = sock_id;
	ptu->next = NULL;
	last->next = ptu;
	last = ptu;

	return 0;
}

// deleting from user list
int del_user(int sock_id){
	user * p = head;
	user * s = NULL;

	while(p){
		if(p->next->sock_id == sock_id){
			if(p->next == last){
				s = last;
				last = p;
				last->next = NULL;
			}else{
				s = p->next;
				p->next = p->next->next;
				s->next = NULL;
			}
			free(s);
			s = NULL;
			return 0;
		}
		p = p->next;
	}
	printf("user is not in the user list\n");
	return -1;
}

//adding login user
int login_user(int dev_id,int sock_id){

	user * p = head->next;
	user * ptu = NULL;
	while(p){
		if(p->dev_id == dev_id)
			break;
		else if(p->sock_id == sock_id)
			ptu = p;
		p = p->next;
	}

	if(p == NULL && ptu){
		ptu->dev_id = dev_id;
		printf("user login success\n");
		return 0;
	}else if(p){
		printf("user logged in\n");
	}

	return -1;
}
//search device
int check_user(int dev_id){

	user * p = head->next;
	while(p){
		if(p->dev_id == dev_id)
			return p->sock_id;
		p = p->next;
	}
	/*if(p){
	  return p->sock_id;
	  }*/

	return 0;
}

//free all memory
void free_all(){
	//user * s = head;
	user * p = head;
	while(p){
		head->next = NULL;
		p = p->next;
		free(head);
		head = NULL;
		head = p;
	}

	last = head->next;

}

//displaying user list
void show_list(){
	if(!head)
		return;
	user * p = head->next;
	while(p){
		printf("dev_id is %d,sock_id is %d\n",p->dev_id,p->sock_id);
		p = p->next;
	}

}


