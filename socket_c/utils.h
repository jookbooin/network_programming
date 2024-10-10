#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define END_MSG "end"
void error_handling(char * message);
typedef struct thread_data{
	char id[100];
	char msg[1000];
} thread_data; 
#endif
