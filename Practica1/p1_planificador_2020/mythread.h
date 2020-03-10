#ifndef _MYTHREAD_H_
#define _MYTHREAD_H_


#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "interrupt.h"

#define N 1000
#define FREE 0
#define INIT 1
#define WAITING 2
#define IDLE 3

#define STACKSIZE 10000
#define QUANTUM_TICKS 40 //Quantum /TICKS
#define QUANTUM_TIME 0.2 // QUANTUM /SEC

#define LOW_PRIORITY 0
#define HIGH_PRIORITY 1
#define SYSTEM 2
/* Structure containing thread state  */
typedef struct tcb{
  int state; /* the state of the current block: FREE or INIT */
  int tid; /* thread id*/
  int priority; /* thread priority*/
  int ticks;
  int execution_total_ticks; /*Thread time to complete execution*/
  int remaining_ticks; /*Remaining ticks to complete the process execution*/
  void (*function)(int);  /* the code of the thread */
  ucontext_t run_env; /* Context of the running environment*/
}TCB;

int mythread_create (void (*fun_addr)(), int priority,int seconds); /* Creates a new thread with one argument */
void mythread_setpriority(int priority); /* Sets the thread priority */
int mythread_getpriority(); /* Returns the priority of calling thread*/
void mythread_exit(); /* Frees the thread structure and exits the thread */
int mythread_gettid(); /* Returns the thread id */
int read_disk(); /* */
int seconds_to_ticks(int seconds);

static inline int data_in_page_cache() { return rand() & 0x01; }

#endif

