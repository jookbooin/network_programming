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


char message[BUFFSIZE];

void * recv_msg(void * arg);
void * send_msg(void * arg);

typedef struct send_data{
	int clnt_sock;
	thread_data* data;
} send_data;

int main(int argc, char ** argv){
	int clnt_sock;
	struct sockaddr_in serv_addr;
	void *thread_result;	
	//int *recv_sock_ptr,*send_sock_ptr;
	pthread_t send_thread, recv_thread;

	thread_data tdata;
	send_data sdata;

	if(argc != 3)	{
		printf("you have to enter port, ID\n");
		return 0;
	}
		
	// 1. client socket 생성
	clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(clnt_sock == -1){
		error_handling("socket() error\n");	
	}
	
	// 2. 초기화
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(atoi(argv[1]));	

	// 3. clnt -> server connect
	if(connect(clnt_sock,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		error_handling("clnt_connect() error");\
	}else{
		printf("connection success\n");
	}
	
	// thread_data, send_data 초기화
	strcpy(tdata.id, argv[2]);
	printf("[%d] :  %s 입장!!!!!!!!!!!!\n",clnt_sock,tdata.id);
	sdata.clnt_sock = clnt_sock;
	sdata.data = &tdata;

	// 4. recv thread

	int recv_t = pthread_create(&recv_thread, NULL, recv_msg, (void*)&clnt_sock);

	if(recv_t != 0){
		error_handling("pthread_create() error!\n");
	}
	
	// 4. send thread
	int send_t = pthread_create(&send_thread, NULL, send_msg,(void*)&sdata);

	if(send_t != 0){
		error_handling("pthread_create() error!\n");
	}


	// 자원 회수 
	pthread_detach(recv_thread);
	pthread_join(send_thread,NULL);

	//close(clnt_sock);
	return 0;
}

void * recv_msg(void * arg){

	printf("client recv_thread created!\n");
	int clnt_sock = *(int*)arg;

	//char buff[500];
	thread_data temp;
	int len;
	while(1){

		//len = read(recv_sock,buff, sizeof(buff));
		len = read(clnt_sock,&temp, sizeof(temp));
		if(len == -1){
			printf("sock close\n");
			break;
		}

		if(strcmp(temp.msg,END_MSG) == 0){
			break;
		}

		printf("[%s] : %s",temp.id, temp.msg);
	}

	printf("recv_thread end!\n");
	//free(arg);

	// thread 종료
	//pthread_exit(0);
	return 0;
}

void * send_msg(void * arg){
	char input[300];
	
	printf("client send_thread created!\n");
	//int send_sock = *(int*)arg;
	send_data temp = *(send_data*)arg;

	while(1){
		printf("채팅입력 : ");
		fgets(input, 250, stdin);
		if(!strcmp(input,"q\n") || !strcmp(input,"Q\n")){
			printf("%s가 종료합니다.\n",temp.data->id);
			strcpy(temp.data->msg,END_MSG);
			write(temp.clnt_sock, temp.data, sizeof(*temp.data));

			// recv_t 종료		
			break;
		}

		//sprintf(msg,"[%s] : %s\n",id, input);
		strcpy(temp.data->msg,input);
		write(temp.clnt_sock,temp.data,sizeof(*temp.data));	
		sleep(1);
	}

	printf("send_thread end!\n");
	//free(arg);
	pthread_exit(0);
	return 0;

}

