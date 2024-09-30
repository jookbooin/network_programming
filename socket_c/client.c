#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "utils.h"

#define BUFFSIZE 100
#define NAMESIZE 20

//int stoHEX(char fi, char sc);
//void error_handling(char * msg);
//void * send_message(void * arg);
//void * recv_message(void * arg);

char message[BUFFSIZE];

void * recv_msg(void * arg){

	printf("recv_thread created!\n");
	int recv_sock = *(int*)arg;
	char buff[500];
	int len;
	while(1){

		len = read(recv_sock,buff, sizeof(buff));
		if(len == -1){
			printf("sock close\n");
			break;
		}
		printf("%s",buff);
	}

	printf("recv_thread end!\n");
	free(arg);
	pthread_exit(0);
	return 0;
}

int main(int argc, char ** argv){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t send_thread, recv_thread;
	void* thread_result;

	if(argc != 3)	{
		printf("you have to enter port, ID\n");
		return 0;
	}

	char id[100];
	strcpy(id, argv[2]);
	printf("id : %s\n",id);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		error_handling("socket() error\n");	
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(atoi(argv[1]));	

	if(connect(sock,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		error_handling("clnt_connect() error");\
	}else{
		printf("connection success\n");
	}

	// 1. recv_thread
	int* recv_sock_ptr;
	recv_sock_ptr = (int*)malloc(sizeof(int));
	*recv_sock_ptr = sock;
	int t_create = pthread_create(&recv_thread, NULL, recv_msg, (void*)recv_sock_ptr);
	if(t_create != 0){
		free(recv_sock_ptr);
		error_handling("pthread_create() error!\n");
	}	

	// 2. send_thread
	char input[300];
	char msg[1000];
	while(1){
		printf("채팅입력 : ");
		fgets(input, 250, stdin);
		sprintf(msg,"[%s] : %s\n",id, input);
		printf("전송 : %s\n",msg);
		write(sock,msg,strlen(msg)+1);	
		sleep(1);
	}

	close(sock);
	return 0;
}
