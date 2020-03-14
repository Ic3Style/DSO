#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include "queue.h"

struct queue* enqueue(struct queue* s, void * i)
{
  struct my_struct* p = malloc(sizeof(struct my_struct) );

  if( NULL == p )
    {
      fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "list_add");
      return s;
    }
  p->data = i;
  p->next = NULL;
  if( NULL == s )
    {
      printf("Queue not initialized\n");
      free(p);
      return s;
    }
  else if( NULL == s->head && NULL == s->tail )
    {
      /* printf("Empty list, adding p->data: %d\n\n", p->data);  */
      s->head = s->tail = p;
      return s;
    }
  else if( NULL == s->head || NULL == s->tail )
    {
      fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
      free(p);
      return NULL;
    }
  else
    {
      /* printf("List not empty, adding element to tail\n"); */
      s->tail->next = p;
      s->tail = p;
    }
  return s;
}

struct queue* sort_queue_by_execution_time(struct queue* s){
    struct queue  * aux_queue = queue_new();
    struct my_struct * p = s->head;
    if(s == NULL){
        printf("Fallo en la ejecucion\n");
    }else{
        for(;p;p = p->next){
            enqueue(aux_queue,p);
        }
    }


    free(aux_queue);
    return s;
}


struct queue* sorted_enqueue(struct queue* s, void * tcb, int sort)
{

	struct my_struct* p = malloc(sizeof(struct my_struct) );
	if( NULL == p )
    {
      fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "list_add");
      return s;
    }

	p->data = (TCB*) tcb;
	p->next = NULL;
	p->sort = sort;
  if( NULL == s )
    {
      //printf("Queue not initialized\n");
      free(p);
      return s;
    }
  else if( NULL == s->head && NULL == s->tail )
    {
      //printf("Empty list, adding p->data: %d\n\n", p->data);
      s->head = s->tail = p;
      return s;
    }
  else if( NULL == s->head || NULL == s->tail )
    {
      fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
      free(p);
      return NULL;
    }
  else
    {
      struct my_struct * aux = s->head;
      struct my_struct * aux1 = NULL;

      int inserted = 0;
      while(aux && !inserted){
          if(p->sort <= aux->sort || aux == s->tail) {
              inserted = 1;
              if (aux == s->head && p->sort <= aux->sort) {
                  p->next = aux;
                  s->head = p;
              } else if (aux == s->tail && p->sort > aux->sort) {
              //}else if (aux == s->tail) {
                  aux->next = p;
                  p->next = NULL;
                  s->tail = p;
              } else {
                  p->next = aux;
                  aux1->next = p;
              }
          }
          aux1 = aux;
          aux = aux->next;
      }
    }

		/*DEBUG INFO*/
		/*struct my_struct * st_1 = s->head;
		int i = 0;
		while(st_1 != NULL){
			TCB * tcb = (TCB*) st_1->data;
			printf("ORDERED :: %d ---> data :: %d\n",i,tcb->remaining_ticks);
			i++;
			st_1 = st_1->next;
		}*/


  return s;
}

/* Remove the first element */
void* dequeue( struct queue* s )
{
  struct my_struct* h = NULL;
  struct my_struct* p = NULL;
  void * ret;

  if( NULL == s )
    {
      //printf("List is empty\n");
      return NULL;
    }
  else if( NULL == s->head && NULL == s->tail )
    {
      //printf("Well, List is empty\n");
      return NULL;
    }
  else if( NULL == s->head || NULL == s->tail )
    {
      printf("There is something seriously wrong with your list\n");
      printf("One of the head/tail is empty while other is not \n");
      return NULL;
    }
  h = s->head;
  p = h->next;
  ret = h->data;
  free(h);
  s->head = p;
  if( NULL == s->head )  s->tail = s->head;   /* The element tail was pointing to is free(), so we need an update */
  return ret;
}

/* Search an element and remove it from queue if found
   This function can be specialized for comparing the contents of structures s->data and data instead of pointers to structures
*/
void* queue_find_remove(struct queue* s, void * data )
{
  void * ret;

 if( NULL == s )
   {
     //printf("List is empty\n");
     return NULL;
   }
 else
   if( NULL == s->head && NULL == s->tail )
     {
       //printf("Well, List is empty\n");
       return NULL;
     }
   else if( NULL == s->head || NULL == s->tail )
     {
       printf("There is something seriously wrong with your list\n");
       printf("One of the head/tail is empty while other is not \n");
       return NULL;
     }

 if ( s->head->data == data) {
   ret = data;
   if (s->head == s->tail){
     free(s->head);
     s->head = s->tail = NULL;
   }
   else {
     struct my_struct* aux = s->head;
     s->head = s->head->next;
     free(aux);
   }
   return ret;
 }
 else {
   struct my_struct* aux;

   for ( aux = s->head; aux->next && (aux->next->data != data); aux = aux->next);
   if (aux->next == NULL)
     return NULL;
   else {
     struct my_struct* aux2 =  aux->next;
     ret = aux->next->data;
     if (aux->next->next == NULL )  // last element contains the searched data
       s->tail = aux;
     aux->next = aux->next->next;
     free(aux2);
     return ret;
   }
 }
}

int queue_empty ( struct queue* s ) { return (s->head == NULL); }

struct queue* queue_new(void)
{
  struct queue* p = malloc(sizeof(struct queue));
  if( NULL == p )
      fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
  p->head = p->tail = NULL;
  return p;
}


void queue_print(struct queue* ps )
{
  struct my_struct* p = NULL;
  printf("Queue contents:\n");
  if( ps )
    {
      if (queue_empty(ps))
	printf("\t\tEmpty QUEUE\n");
      else
	for( p = ps->head; p; p = p->next )
	  queue_print_element(p);
    }
}


/* This function needs to be specialized depending on the content of p->data */
void queue_print_element(struct my_struct* p )
{
  if( p )
    printf("\t\tp->data pointer=%ld \n", (long) (p->data));
  else
      printf("Can not print NULL struct \n");
}
