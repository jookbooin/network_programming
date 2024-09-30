#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <pthread.h>
#include <netinet/in.h>
#include "utils.h"

#define CLNT_MAX 10
#define BUFFSIZE 200

int g_clnt_socks[CLNT_MAX];
int g_clnt_count = 0;

pthread_mutex_t g_mutex;

void send_all_clnt(char * msg, void* arg){

	int my_sock = *(int*)arg;
	pthread_mutex_lock(&g_mutex); 
	printf("send sock id [%d] => ", my_sock);

	for(int i = 0; i<g_clnt_count; i++){
		if(g_clnt_socks[i] != my_sock){
			write(g_clnt_socks[i], msg, strlen(msg)+1);
		}
	}
	pthread_mutex_unlock(&g_mutex); 
}

void * read_connected_clnt(void * arg){

	int clnt_sock = *(int*)arg;
	int str_len = 0;

	char msg_buff[BUFFSIZE];
	int i;

	while(1){
		// 1. read
		str_len = read(clnt_sock, msg_buff, sizeof(msg_buff));
		if(str_len == -1){
			printf("clnt[%d] read error()\n",clnt_sock);
			break;
		}

		// 2. send
		send_all_clnt(msg_buff, arg);
		printf("%s\n",msg_buff);

	}

	// delete socket 
	pthread_mutex_lock(&g_mutex);
	for(int i = 0; i < g_clnt_count; i++){
		if(clnt_sock == g_clnt_socks[i]){
			
			// move -1 
			for(int j = i; g_clnt_count - 1; j++){
				g_clnt_socks[j] = g_clnt_socks[j+1];
			}
		
			// delete 
			g_clnt_count--;
			break;
		}
	}
	pthread_mutex_unlock(&g_mutex); 

	close(clnt_sock);
	free(arg);
	pthread_exit(0);

	//return NULL;
}

int main(int argc, char ** argv ){

	printf("port : [%s]\n",argv[1]);
	int serv_sock; // 1. listen socket
	int clnt_sock; // 2. communication socket

	pthread_t serv_thread;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	pthread_mutex_init(&g_mutex,NULL);

	serv_sock = socket(PF_INET,SOCK_STREAM,0);

	// port(16bit) + ip(32bit) + sin_zero(8byte)		
	memset(&serv_addr, 0, sizeof(serv_addr));	

	serv_addr.sin_family = AF_INET; // ipv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	// 1. server : bind
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
		error_handling("bind() error\n");		
	}else{
		printf("bind success!\n");
	}

	// 2. server : listen
	if(listen(serv_sock, 5) == -1){
		error_handling("listen() error\n");
	}else{
		printf("listen success!\n");
	}

	char buff[200];
	int recv_len = 0;

	int * clnt_sock_ptr;

	// main
	while(1){
		clnt_addr_size = sizeof(clnt_addr);

		// server accept client
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

		if(clnt_sock == -1){
			error_handling("accept() error.\n");
		}else{
			printf("client[%d] enter [%s]port \n",clnt_sock,argv[1]);
		}

		//critical section
		pthread_mutex_lock(&g_mutex); 
		g_clnt_socks[g_clnt_count++] = clnt_sock;
		pthread_mutex_unlock(&g_mutex);

		// 
		clnt_sock_ptr = (int*)malloc(sizeof(int));
		*clnt_sock_ptr = clnt_sock;
		int create = pthread_create(&serv_thread, NULL, read_connected_clnt,(void*) clnt_sock_ptr);

		if(create != 0){
			free(clnt_sock_ptr);
			error_handling("pthread_create error.\n");
		}
	}

	return 0;
}
