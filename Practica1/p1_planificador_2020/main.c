#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "mythread.h"


//Each thread executes this function
extern void function_thread(int sec);
extern void function_thread_aux(int sec); //funcion para pruebas de RRSD



int main(int argc, char *argv[])
{
  int j,k,l,m,a,b,f;

  read_disk();

  mythread_setpriority(LOW_PRIORITY);
  if((f = mythread_create(function_thread,HIGH_PRIORITY,5)) == -1){
      printf("thread failed to initialize\n");
      exit(-1);
  }

  read_disk();
  // read_disk();

  if((j = mythread_create(function_thread_aux,LOW_PRIORITY, 2)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }

  read_disk();

  if((k = mythread_create(function_thread_aux,HIGH_PRIORITY, 4)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
  if((l = mythread_create(function_thread,LOW_PRIORITY, 3)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }

  read_disk();

  if((m = mythread_create(function_thread_aux,HIGH_PRIORITY, 1)) == -1){
    printf("thread failed to initialize\n");
    exit(-1);
  }
  // read_disk();


  for (a=0; a<10000000; ++a) {
    for (b=0; b<30000000; ++b);
  }

  mythread_exit();

  printf("This program should never come here\n");

  return 0;
} /****** End main() ******/
