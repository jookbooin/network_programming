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

// server에 연결된 client socket_fd 번호 저장 
int g_clnt_socks[CLNT_MAX];
int g_clnt_count = 0;

pthread_mutex_t g_mutex;

void send_clnt(thread_data* data, void* arg);
void send_all_clnt(thread_data * data, void* arg);
void * read_clnt(void * arg);
void serv_addr_init(struct sockaddr_in* serv_addr, int port_num);
void serv_bind(int* serv_sock, struct sockaddr_in *serv_addr);
void serv_listen(int* serv_sock, int backlog);

int main(int argc, char ** argv ){

	printf("port : [%s]\n",argv[1]);
	int serv_sock; // 1. listen socket
	int clnt_sock; // 2. communication socket
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;

	pthread_t serv_thread;
	char buff[200];
	int recv_len = 0;
	int * clnt_sock_ptr;
	socklen_t clnt_addr_size;

	// mutex_init
	pthread_mutex_init(&g_mutex,NULL);

	// 1. server_socket create
	serv_sock = socket(PF_INET,SOCK_STREAM,0);

	serv_addr_init(&serv_addr,atoi(argv[1]));

	// 2. server : bind
	serv_bind(&serv_sock, &serv_addr);

	// 3. server : listen
	serv_listen(&serv_sock, 5);

	// main thread
	while(1){
		clnt_addr_size = sizeof(clnt_addr);

		// 4. server accept client
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

		// 5. read clnt_sock 
		clnt_sock_ptr = (int*)malloc(sizeof(int));
		*clnt_sock_ptr = clnt_sock;
		int read_clnt_t = pthread_create(&serv_thread, NULL, read_clnt,(void*) clnt_sock_ptr);

		if(read_clnt_t != 0){
			//free(clnt_sock_ptr);
			error_handling("pthread_create error.\n");
		}
	}

	return 0;
}

void send_clnt(thread_data* data, void* arg){
	int clnt_sock = *(int*)arg; 
	pthread_mutex_lock(&g_mutex);
	write(clnt_sock, data, sizeof(*data)); 
	pthread_mutex_unlock(&g_mutex); 
}

void send_all_clnt(thread_data * data, void* arg){

	int clnt_sock = *(int*)arg;
	pthread_mutex_lock(&g_mutex); 
	printf("send sock id [%d] => ", clnt_sock);

	for(int i = 0; i<g_clnt_count; i++){
		if(g_clnt_socks[i] != clnt_sock){

			// client recv_thread로 
			write(g_clnt_socks[i], data, sizeof(*data));
		}
	}
	pthread_mutex_unlock(&g_mutex); 
}

void * read_clnt(void * arg){

	int clnt_sock = *(int*)arg;
	int str_len = 0;

	thread_data data;
	int i;

	while(1){
		// 1. read
		str_len = read(clnt_sock, &data, sizeof(data));
		if(str_len == -1){
			printf("clnt[%d] read error()\n",clnt_sock);
			break;
		}

		// q 종료 조건 
		if (strcmp(data.msg, END_MSG) == 0) {
			// clnt recv 종료
			send_clnt(&data,arg);
			break;
			
		}else{
			// 2. send
			send_all_clnt(&data, arg);
			printf("[%s] : %s\n",data.id,data.msg);
		}

	}

	// delete clnt_sock 
	pthread_mutex_lock(&g_mutex);
	for(int i = 0; i < g_clnt_count; i++){
		if(clnt_sock == g_clnt_socks[i]){

			//move -1 
			for(int j = i; g_clnt_count - 1; j++){
				g_clnt_socks[j] = g_clnt_socks[j+1];
			}

			//delete 
			g_clnt_count--;
			break;
		}
	}
	pthread_mutex_unlock(&g_mutex); 
	
	printf("클라이언트[%d]가 종료했으므로  소켓이 제거됩니다..\n", clnt_sock);
	close(clnt_sock);
	free(arg);
	pthread_exit(0);

	//return NULL;
}

void serv_addr_init(struct sockaddr_in* serv_addr, int port_num){
	memset(serv_addr, 0, sizeof(*serv_addr)); // port(16bit) + ip(32bit) + sin_zero(8byte)		

	serv_addr->sin_family = AF_INET; // ipv4
	serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr->sin_port = htons(port_num);
}

void serv_bind(int* serv_sock, struct sockaddr_in *serv_addr){

	if(bind(*serv_sock, (struct sockaddr*)serv_addr, sizeof(*serv_addr))==-1){
		error_handling("bind() error\n");		
		return;
	}

	printf("bind success!\n");
}

void serv_listen(int* serv_sock, int backlog){
	if(listen(*serv_sock, backlog) == -1){
		error_handling("listen() error\n");
		return;
	}

	printf("listen success!\n");
}

