#include "kolejka.h"
#include "main.h"
void insertElem(process_q_t* process_q, q_element_t elem)
{
	/*if (process_q->size == process_q->reserved)
		reserveQueue(process_q, process_q->reserved * 2);*/

	int i = 0;
	while (i < process_q->size)
	{
		if (biggerPriority(elem, process_q->data[i]))
			break;
		i++;
	}

	int j = process_q->size;
	while (j > i)
	{
		process_q->data[j] = process_q->data[j - 1];
		j--;
	}
	process_q->data[i] = elem;
	process_q->size++;

}
int biggerPriority(q_element_t element1, q_element_t element2)
{
	if (element1.process == -1)
		return FALSE;
	if (element2.process == -1)
		return TRUE;

	if (element1.priority < element2.priority)
		return TRUE;
	if (element1.priority == element2.priority)
		if (element1.process < element2.process)
			return TRUE;
	debug("elem %d  tab %d", element1.priority, element2.priority);
	return FALSE;
}

void initQueue(process_q_t* process_q, int initReserved)
{
	process_q->size = 0;

	q_element_t* ptr = (q_element_t*)(malloc(sizeof(q_element_t) * initReserved));
	for (int i = 0; i < initReserved; i++) {
		ptr->priority = -1;
		ptr->process = -1;
	}
	process_q->data = ptr;
	process_q->reserved = initReserved;
	process_q->size = 0;

}


int findPosition(process_q_t* process_q, int process)
{
	int pos = -1;
	for (int i = 0; i < process_q->size; i++)
	{
		if (process_q->data[i].process == process)
		{
			pos = i;
			break;
		}
	}
	return pos;
}

void removeProcess(process_q_t* process_q, int process)
{
	int i = 0;
	while (i < process_q->size)
	{
		if (process_q->data[i].process == process)
			break;
		i++;
	}
	while (i < process_q->size - 1)
	{
		process_q->data[i] = process_q->data[i + 1];
		i++;
	}
	process_q->data[process_q->size - 1].process = -1;
	process_q->data[process_q->size - 1].priority = -1;
	process_q->size--;
}
