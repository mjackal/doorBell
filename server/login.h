#include<stdio.h>
#include<stdlib.h>

typedef struct user_tag{

	int dev_id;
	int sock_id;
	struct user_tag * next;
}user;

extern user * head;//the head of user list
extern user * last;//the end of user list
extern user * u_sock;//keeping login user
extern user * act_sock;//activate socket
int create_list();//creating user ist
int add_user(int dev_id,int sock_id);//adding into user list
int login_user(int dev_id,int sock_id);//adding login user
int del_user(int sock_id);// deleting from user list
int check_user(int dev_id);//search device
void free_all();//free all memory
void show_list();//displaying user list
