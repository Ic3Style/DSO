#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "my_io.h"

//#include "mythread.h"
#include "interrupt.h"

#include "queue.h"

TCB* scheduler();
void activator();
void timer_interrupt(int sig);
void disk_interrupt(int sig);


/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N];

/* Current running thread */
static TCB * running;
static int current = 0;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init=0;

/* Thread control block for the idle thread */
static TCB idle;

/*Global Variables*/
static struct queue* ready_low;
static struct queue* ready_high;
static TCB* lastRunning;



static void idle_function()
{
  while(1);
}

void function_thread(int sec)
{
    //time_t end = time(NULL) + sec;
    while(running->remaining_ticks)
    {
      //do something
    }
    mythread_exit();
}


/* Initialize the thread library */
void init_mythreadlib()
{
  int i;
  /*Initialize queue*/
  ready_low = queue_new();
  ready_high = queue_new();

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

  t_state[0].state = INIT;
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
  init_disk_interrupt();
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
  t_state[i].ticks = QUANTUM_TICKS;
  t_state[i].function = fun_addr;
  t_state[i].execution_total_ticks = seconds_to_ticks(seconds);
  t_state[i].remaining_ticks = t_state[i].execution_total_ticks;
  t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));

  if(t_state[i].run_env.uc_stack.ss_sp == NULL)
  {
    printf("*** ERROR: thread failed to get stack space\n");
    exit(-1);
  }

  t_state[i].tid = i;
  t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
  t_state[i].run_env.uc_stack.ss_flags = 0;
  makecontext(&t_state[i].run_env, fun_addr,2,seconds);
  TCB* t = &t_state[i];

  if (running->priority == LOW_PRIORITY && t->priority == HIGH_PRIORITY){
    running->state = INIT;
    running->ticks = QUANTUM_TICKS;
    disable_interrupt();
    enqueue(ready_low, running);
    enable_interrupt();
    //CCI
    printf("\nCreado HP");
    lastRunning = running;
    running = t;
    current = running->tid;
    activator(t);
  }else if (running->priority == HIGH_PRIORITY && t->priority == HIGH_PRIORITY && t->remaining_ticks<running->remaining_ticks){
    running->state = INIT;
    disable_interrupt();
    sorted_enqueue(ready_high, t, t->execution_total_ticks);
    enable_interrupt();
    //CCI
        printf("\nCreado HP");
    lastRunning = running;
    running = t;
    current = running -> tid;
    activator(t);
  }else{
    if(t->priority == HIGH_PRIORITY){
      disable_interrupt();
      sorted_enqueue(ready_high, t, t->execution_total_ticks);
          printf("\nCreado HP");
      enable_interrupt();
    }else {
      disable_interrupt();
      enqueue(ready_low, t);
          printf("\nCreado LP");
      enable_interrupt();
    }
  }
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

  printf("*** THREAD %d FINISHED\n", tid);
  t_state[tid].state = FREE;
  free(t_state[tid].run_env.uc_stack.ss_sp);

  TCB* next = scheduler();
  activator(next);
}


void mythread_timeout(int tid) {

    printf("*** THREAD %d EJECTED\n", tid);
    t_state[tid].state = FREE;
    free(t_state[tid].run_env.uc_stack.ss_sp);

    TCB* next = scheduler();
    activator(next);
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


/* SJF for high priority, RR for low*/

TCB* scheduler()
{
  if(!queue_empty(ready_high)){
    disable_interrupt();
    TCB *s = dequeue(ready_high);
    enable_interrupt();
    lastRunning = running;
    running = s;
    current = running -> tid;
    return s;
  }else if(!queue_empty(ready_low)){
    disable_interrupt();
    TCB *s = dequeue(ready_low);
    enable_interrupt();
    lastRunning = running;
    running = s;
    current = running -> tid;
    return s;
  }
  printf("*** FINISH\n");
  exit(1);
}


/* Timer interrupt */
void timer_interrupt(int sig)
{
  if(running->state == INIT && running->priority == HIGH_PRIORITY){
    running ->remaining_ticks = running ->remaining_ticks - 1;
    if(running -> remaining_ticks < 0){
      mythread_timeout(current);
    }
  }else{
    running->ticks = running->ticks - 1;
    running ->remaining_ticks = running ->remaining_ticks - 1;
    if(running -> remaining_ticks < 0){
      mythread_timeout(current);
    }
    if(running->ticks <= 0){
      running->state = INIT;
      running->ticks = QUANTUM_TICKS;
      disable_interrupt();
      enqueue(ready_low, running);
      enable_interrupt();
      running = scheduler();
      activator(running);
    }
  }
}

/* Activator */
void activator(TCB* next)
{
  if (lastRunning->state == FREE){
   printf("*** THREAD %d TERMINATED: SET CONTEXT OF %d\n", lastRunning -> tid, next -> tid);
   setcontext (&(next->run_env));
   printf("mythread_free: After setcontext, should never get here!!...\n");
 } else if (lastRunning -> priority == LOW_PRIORITY && next->priority == HIGH_PRIORITY){
    printf("*** THREAD %d PREEMTED: SET CONTEXT OF %d\n", lastRunning -> tid, next -> tid );
    if(swapcontext(&(lastRunning->run_env),&(next->run_env)) == -1){
      printf("ERROR\n");
    }
 }else if (lastRunning -> tid != next -> tid){
    printf("*** SWAPCONTEXT FROM %i TO %i\n", lastRunning -> tid, next -> tid );
    if(swapcontext(&(lastRunning->run_env),&(next->run_env)) == -1){
      printf("ERROR\n");
    }

  }
}
