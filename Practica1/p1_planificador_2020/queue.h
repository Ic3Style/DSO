#ifndef _QUEUE_H_
#define _QUEUE_H_

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include "mythread.h"
struct my_struct
{
  void *data;
  struct my_struct* next;
  int sort;
};


struct queue
{
  struct my_struct* head;
  struct my_struct* tail;
};

/* Enqueue an element */
struct queue* enqueue( struct queue*, void * data);
/* Enqueue an element */
struct queue* sorted_enqueue(struct queue*, void * tcb, int sort);
/* Dequeue an element */
void* dequeue( struct queue*);
/* Return 1 if the queue is empty and 0 otherwise*/
int queue_empty ( struct queue* s );
/* If it finds the data in the queue it removes it and returns it. Otherwise it returns NULL */
void* queue_find_remove(struct queue* s, void * data);
/* Create an empty queue */
struct queue* queue_new(void);

void queue_print(struct queue* );
void queue_print_element(struct my_struct* );

#endif


