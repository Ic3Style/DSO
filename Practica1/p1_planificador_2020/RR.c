#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "my_io.h"

#include "mythread.h"
#include "interrupt.h"

#include "queue.h"

TCB* scheduler();
void activator();
void timer_interrupt(int sig);
void disk_interrupt(int sig);
int global_tick = 0; //TICKS GLOBALES DEL SISTEMA

/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N];

struct queue * ready; // Cola de listos para RR.

/* Current running thread */
static TCB* running;
static TCB* old_running;
static int current = 0;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init=0;

/* Thread control block for the idle thread */
static TCB idle;



static void idle_function()
{
  while(1);
}

void function_thread(int sec)
{
    //time_t end = time(NULL) + sec;
    while(running->remaining_ticks)
    {
    // printf("\nEl running es: ID %d, state: %d", running->tid, running->state);
      //do something
      // printf("\nQUEDAN %d TICKS\n", running->remaining_ticks);
    }

     mythread_exit();
}


/* Initialize the thread library */
void init_mythreadlib()
{
  int i;

  ready = queue_new();

  /* Create context for the idle thread */
  if(getcontext(&idle.run_env) == -1)
  {
    perror("*** ERROR: getcontext in init_thread_lib");
    exit(-1);
  }

  idle.state = IDLE;
  idle.priority = SYSTEM;
  idle.function = idle_function;
  idle.run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
  idle.tid = -1;

  if(idle.run_env.uc_stack.ss_sp == NULL)
  {
    printf("*** ERROR: thread failed to get stack space\n");
    exit(-1);
  }

  idle.run_env.uc_stack.ss_size = STACKSIZE;
  idle.run_env.uc_stack.ss_flags = 0;
  idle.ticks = QUANTUM_TICKS;
  makecontext(&idle.run_env, idle_function, 1);

  t_state[0].state = RUNNING; //MODIFICADO
  t_state[0].priority = LOW_PRIORITY;
  t_state[0].ticks = QUANTUM_TICKS;


  if(getcontext(&t_state[0].run_env) == -1)
  {
    perror("*** ERROR: getcontext in init_thread_lib");
    exit(5);
  }

  for(i=1; i<N; i++)
  {
    t_state[i].state = FREE;
  }

  t_state[0].tid = 0;
  running = &t_state[0];

  /* Initialize disk and clock interrupts */
  // init_disk_interrupt();
  init_interrupt();
}


/* Create and intialize a new thread with body fun_addr and one integer argument */
int mythread_create (void (*fun_addr)(),int priority,int seconds)
{
  int i;

  if (!init) { init_mythreadlib(); init=1;}

  for (i=0; i<N; i++)
    if (t_state[i].state == FREE) break;

  if (i == N) return(-1);

  if(getcontext(&t_state[i].run_env) == -1)
  {
    perror("*** ERROR: getcontext in my_thread_create");
    exit(-1);
  }

  t_state[i].state = INIT;
  t_state[i].priority = priority;
  t_state[i].function = fun_addr;
  t_state[i].execution_total_ticks = seconds_to_ticks(seconds);
  t_state[i].remaining_ticks = t_state[i].execution_total_ticks;
  t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));

  if (QUANTUM_TICKS < t_state[i].remaining_ticks) {
    t_state[i].ticks =  QUANTUM_TICKS;
  }
  else{
    t_state[i].ticks = t_state[i].remaining_ticks;
  } //metemos la rodaja al proceso

  if(t_state[i].run_env.uc_stack.ss_sp == NULL)
  {
    printf("*** ERROR: thread failed to get stack space\n");
    exit(-1);
  }

  t_state[i].tid = i;
  t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
  t_state[i].run_env.uc_stack.ss_flags = 0;
  makecontext(&t_state[i].run_env, fun_addr,2,seconds);

  // struct tcb *thread_copy = &t_state[i]; //CUIDADO

  printf("\nCreado hilo con ID: %d, tiempo: %d segundos, prioridad: %d\n", i, seconds, priority);

  // printf("Check del hilo: ID %d", thread_copy->tid);

  disable_interrupt();
  //disable_disk_interrupt();
  enqueue(ready, &t_state[i]);
  enable_interrupt();
  //enable_disk_interrupt();

  printf("Encolado el hilo con ID: %d \t", i);

  return i;
}
/****** End my_thread_create() ******/


/* Read disk syscall */
int read_disk()
{
   return 1;
}

/* Disk interrupt  */
void disk_interrupt(int sig)
{

}


/* Free terminated thread and exits */
void mythread_exit() {
  int tid = mythread_gettid();

  printf("\n*** THREAD %d FINISHED\n", tid);
  t_state[tid].state = FREE;
  free(t_state[tid].run_env.uc_stack.ss_sp);

  int tiempo = t_state[tid].execution_total_ticks;
  printf("tiempo ejecución %d Hilo %d\n", tiempo, tid);

  old_running = running;

  running = scheduler(); //MODIFICADO
  running->state = RUNNING;
  activator(running);
}


void mythread_timeout(int tid) {

    printf("*** THREAD %d EJECTED\n", tid);
    t_state[tid].state = FREE;
    free(t_state[tid].run_env.uc_stack.ss_sp);

    // old_running = running;

    running = scheduler(); //MODIFICADO
    running->state = RUNNING;
    activator(running);
}


/* Sets the priority of the calling thread */
void mythread_setpriority(int priority)
{
  int tid = mythread_gettid();
  t_state[tid].priority = priority;
  if(priority ==  HIGH_PRIORITY){
    t_state[tid].remaining_ticks = 195;
  }
}

/* Returns the priority of the calling thread */
int mythread_getpriority(int priority)
{
  int tid = mythread_gettid();
  return t_state[tid].priority;
}


/* Get the current thread id.  */
int mythread_gettid(){
  if (!init) { init_mythreadlib(); init=1;}
  return current;
}


/* SJF para alta prioridad, RR para baja*/

TCB* scheduler()
{
  int i;
  for(i=0; i<N; i++)
  {
    if (t_state[i].state == INIT)
    {
      current = i;
	    // return &t_state[i];
    }
  }
  if(!queue_empty(ready)){


    disable_interrupt();
    //disable_disk_interrupt();
    TCB *thread_copy = dequeue(ready);
    enable_interrupt();
    //enable_disk_interrupt();

    printf("Desencola elemento de Ready, ID: %d", thread_copy->tid);

    return thread_copy;

  } //Else: la cola está vacía.

  printf("mythread_free: No thread in the system\nExiting...\n");
  exit(1);
}


/* Timer interrupt */
void timer_interrupt(int sig)
{
  if(global_tick%10==0){
    // printf("\nTICK: %d\t", global_tick);
    // printf("\nLe quedan %d TICKS\t", running->remaining_ticks);
  }

  global_tick++;

  // if(running->remaining_ticks <= 0){
  //   // printf("\n\nENTRA CONIO\n");
  //   // mythread_exit();
  //   // old_running = running;
  // }
  // else{
    if(running->state == RUNNING){

      running->ticks--; //se reduce su rodaja
      running->remaining_ticks--; //se reduce su ejecucion global
// printf("\nLe quedan %d TICKS\t", running->remaining_ticks);
    if(running->remaining_ticks == 0){
      // printf("\n\nENTRA CONIO\n");
      // mythread_exit();
      // old_running = running;
    }
    else{
      if(running->ticks <= 0){
          // printf("\n Proceso ID: %d acaba su rodaja\n", running->tid);

          running->state = INIT;
          if (QUANTUM_TICKS < running->remaining_ticks) {
            running->ticks =  QUANTUM_TICKS;
          }
          else{
            running->ticks = running->remaining_ticks;
          }

          if(running->remaining_ticks > 0){
            disable_interrupt();
            enqueue(ready, running);
            enable_interrupt();
          }

          old_running = running;
          // printf("Old_running es ID: %d\n", old_running->tid);

          if(running->remaining_ticks < 0){
            printf("Hilo ID: %d, procede a exit\n", mythread_gettid());
            // running->state = FREE;
            // if(running->tid == mythread_gettid())
            mythread_timeout(running->tid);
          }

          running = scheduler();
          running->state = RUNNING;
          activator(running);

          // printf("\n Se sustituye por ID: %d\n", running->tid);

        }
      }
    }
  // }
// printf("\nEjecutando interrupt hw");
}

/* Activator */
void activator(TCB* next)
{
  // printf("\nOld_running remaining_ticks: %d\n", old_running->remaining_ticks);
  if(old_running->tid == next->tid){
    return;
  }
  if (old_running->remaining_ticks==0){
      current = next->tid;
      printf("\n*** THREAD %d TERMINATED : SETCONTEXT OF %d\n", old_running->tid, next->tid);
      setcontext (&(next->run_env));
      printf("mythread_free: After setcontext, should never get here!!...\n");
    }
  else{
   //   if (old_running->state == FREE){
   //   setcontext (&(next->run_env));
   // }
   // else{
    current = next->tid;
    printf("\n*** SWAPCONTEXT FROM %d TO %d\n", old_running->tid, next->tid);
     swapcontext(&(old_running->run_env), &(next->run_env));
   // }
  }
  //
//
  // printf("mythread_free: After setcontext, should never get here!!...\n");
}
