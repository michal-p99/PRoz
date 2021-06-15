#ifndef KOLEJKAH
#define KOLEJKAH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/* boolean */
#define TRUE 1
#define FALSE 0

typedef struct {
	int process;
	int priority;
}q_element_t;

typedef struct {
	q_element_t* data;
	int size;
	int reserved;
}process_q_t;

void insertElem(process_q_t* process_q, q_element_t elem);
int findPosition(process_q_t* process_q, int process);
void initQueue(process_q_t* process_q, int initReserved);
int biggerPriority(q_element_t element1, q_element_t element2);
void removeProcess(process_q_t* process_q, int process);




#endif