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

/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N];

struct queue * lp_queue; // Cola de listos para RR.
struct queue * hp_queue; // Cola de listos de SJF
struct queue * bl_queue; // Cola de procesos bloqueados

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
      //el hilo no hace nada mientras se ejecuta
    }

     mythread_exit(); //al terminar su tiempo total de ejecucion hace exit
}

void function_thread_aux(int sec) //funcion auxiliar creada para RRSD
{
  while(running->remaining_ticks > (running->execution_total_ticks/2))
  {
    //el hilo no hace nada mientras se ejecuta
  }
  read_disk(); //llama a disco en la mitad de su ejecucion
  while(running->remaining_ticks)
  {
    //el hilo no hace nada mientras se ejecuta
  }

  mythread_exit(); //al terminar su tiempo total de ejecucion hace exit
}

/* Initialize the thread library */
void init_mythreadlib()
{
  int i;

  lp_queue = queue_new();  //creamos la cola de listos para low priority RR
  hp_queue = queue_new(); //creamos la cola de listos para high priority
  bl_queue = queue_new(); //creamos la cola de hilos bloqueados por peticion de disco

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

  t_state[0].state = RUNNING;
  t_state[0].priority = LOW_PRIORITY; //se le asigna originalmente low al main
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
  t_state[i].function = fun_addr;
  t_state[i].execution_total_ticks = seconds_to_ticks(seconds);
  t_state[i].remaining_ticks = t_state[i].execution_total_ticks;
  t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));

  if (QUANTUM_TICKS < t_state[i].remaining_ticks) { //si la rodaja es menor que el tiempo que le queda de ejecucion se asigna la rodaja entera
    t_state[i].ticks =  QUANTUM_TICKS;
  }
  else{
    t_state[i].ticks = t_state[i].remaining_ticks; //sino se le asigna el tiempo que le quede
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

  //disable_disk_interrupt();
  //enable_disk_interrupt();

  if (running->priority == LOW_PRIORITY && t_state[i].priority == HIGH_PRIORITY){

    running->state = INIT;
    if (QUANTUM_TICKS < running->remaining_ticks) { //si la rodaja es menor que el tiempo que le queda de ejecucion se asigna la rodaja entera
      running->ticks =  QUANTUM_TICKS;
    }
    else{
      running->ticks = running->remaining_ticks;  //sino se le asigna lo que le quede de ejecucion
    }

    disable_interrupt();
    enqueue(lp_queue, running);  //se encola en la lista de lp_queue
    enable_interrupt();


    old_running = running;  //se actualiza el proceso anterior
    running = &t_state[i];  //se actualiza el proceso en marcha
    current = running->tid; //se actualiza el id del current

    running->state = RUNNING;
    activator (running); //cambio de contexto
  }

  else if (running->priority == HIGH_PRIORITY && t_state[i].priority == HIGH_PRIORITY && t_state[i].remaining_ticks < running->remaining_ticks){

    // si el nuevo que entra tiene menor tiempo de ejecucion habra que ejecutarlo antes q el que ay esta ejecutando

    running->state = INIT;
    old_running = running; //se actualiza el proceso anterior

    disable_interrupt();
    sorted_enqueue(hp_queue, running, running->execution_total_ticks);  //se encola en la lista de hp
    enable_interrupt();


    running = &t_state[i]; //se actualiza el proceso en marcha
    running->state = RUNNING;
    current = running->tid; //se actualiza el id del current
    activator(running);

  }

  else{

    if(t_state->priority == HIGH_PRIORITY){ // es de alta prioridad pero el tiempo de ejecucion es mayor que el que esta ejecutando
      disable_interrupt();
      sorted_enqueue(hp_queue, &t_state[i], t_state[i].execution_total_ticks);  //se encola en la lista de hp
      enable_interrupt();

    }
    else{ // Si es de baja prioridad se encola
      disable_interrupt();
      enqueue(lp_queue, &t_state[i]);  //se encola en la lista de lp
      enable_interrupt();

    }
  }

  return i;
}

/****** End my_thread_create() ******/


/* Read disk syscall */
int read_disk()
{
  if (!init) { init_mythreadlib(); init=1;}
  // printf("\nENTRA A read_disk EL HILO ID: %d", mythread_gettid());
  if (data_in_page_cache() != 0){ //ve si el bloque esta en cache o no

  //Pasar proceso a lista de bloqueados y llamada a scheduler y activator.

      old_running = running; //se actualiza el proceso anterior
      old_running -> state = WAITING;
      // old_running->state = INIT;

      disable_interrupt();
      disable_disk_interrupt();
      printf("\n*** THREAD %d READ FROM DISK\n", running->tid);
      enqueue(bl_queue, running); //Se mete el hilo en la cola de bloqueados
      enable_disk_interrupt();
      enable_interrupt();

      running->state = RUNNING; //se cambia el estado del running
      running = scheduler(); //se asigna el nuevo proceso a ejecutar
      activator(running); //se cambia el contexto

    }

   return 1;
}

/* Disk interrupt  */
void disk_interrupt(int sig)
{
  //Procesos bloqueados a listos y los mete en su cola correspondiente
  if(!queue_empty(bl_queue)){ //si la cola de bloqueados no esta vacia

    disable_interrupt();
    disable_disk_interrupt();
    TCB * aux = dequeue(bl_queue); //se desencola el primero de la lista
    enable_disk_interrupt();
    enable_interrupt();


    aux->state= INIT;
    if(aux->priority == HIGH_PRIORITY){ //si el proceso que vuelve es hp
      if(running->priority == LOW_PRIORITY){//si el actual es lp se sustituye
        running->state = INIT;
        if (QUANTUM_TICKS < running->remaining_ticks) { //si la rodaja es menor que el tiempo que le queda de ejecucion se asigna la rodaja entera
          running->ticks =  QUANTUM_TICKS;
        }
        else{
          running->ticks = running->remaining_ticks;  //sino se le asigna lo que le quede de ejecucion
        }

        disable_interrupt();
        enqueue(lp_queue, running);  //se encola en la lista de lp_queue
        printf("\n*** THREAD %d READY", aux->tid);
        enable_interrupt();

        old_running = running;  //se actualiza el proceso anterior
        running = aux;  //se actualiza el proceso en marcha
        current = running->tid; //se actualiza el id del current

        running->state = RUNNING;
        activator (running);
      }
      else if(running->priority == HIGH_PRIORITY && aux->remaining_ticks < running->remaining_ticks){//si el proceso que vuelve es mas corto que el actual
        running->state = INIT;
        old_running = running; //se actualiza el proceso anterior

        disable_interrupt();
        sorted_enqueue(hp_queue, running, running->remaining_ticks);  //se encola en la lista de hp
        printf("\n*** THREAD %d READY", aux->tid);
        enable_interrupt();

        running = aux; //se actualiza el proceso en marcha
        running->state = RUNNING;
        current = running->tid; //se actualiza el id del current
        activator(running);
      }
      else{//si el proceso que vuelve es mas largo que el actual
        disable_interrupt();
        disable_disk_interrupt();
        sorted_enqueue(hp_queue, aux, aux->remaining_ticks);
        printf("\n*** THREAD %d READY", aux->tid);
        // queue_print(hp_queue);
        enable_disk_interrupt();
        enable_interrupt();
      }
    }else{ //Si el que viene es de baja prioridad se encola directamente

      disable_interrupt();
      disable_disk_interrupt();
      enqueue(lp_queue, aux);
      printf("\n*** THREAD %d READY\n", aux->tid);
      // queue_print(lp_queue);
      enable_disk_interrupt();
      enable_interrupt();

    }
  }
}


/* Free terminated thread and exits */
void mythread_exit() {
  int tid = mythread_gettid();

  printf("\n*** THREAD %d FINISHED\n", tid);
  t_state[tid].state = FREE;
  free(t_state[tid].run_env.uc_stack.ss_sp);

  old_running = running;  //se actualiza el proceso anterior
  running = scheduler(); //se asigna el nuevo proceso a ejecutar
  running->state = RUNNING; //se cambia el estado del running
  activator(running); //se cambia el contexto
}


void mythread_timeout(int tid) {

    printf("\n*** THREAD %d EJECTED\n", tid);
    t_state[tid].state = FREE;
    free(t_state[tid].run_env.uc_stack.ss_sp);

    running = scheduler(); //se asigna el nuevo proceso a ejecutar
    running->state = RUNNING; //se cambia el estado del running
    activator(running); //se cambia el contexto
}


/* Sets the priority of the calling thread */
void mythread_setpriority(int priority)
{
  int tid = mythread_gettid();
  t_state[tid].priority = priority;
  if(priority ==  HIGH_PRIORITY || priority == LOW_PRIORITY){
    t_state[tid].remaining_ticks = 195; //se asigna unos ticks por defecto al thread 0
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


TCB* scheduler(){

  if(!queue_empty(hp_queue)){ //Cola de alta prioridad no vacia

    //disable_disk_interrupt();
    disable_interrupt();
    TCB *thread_copy = dequeue(hp_queue);  //se desencola el primer proceso de alta prioridad
    enable_interrupt();
    //enable_disk_interrupt();

    current = thread_copy->tid; //se actualia el id del currrent

    return thread_copy;

  } //Else: la cola de baja prioridad esta vacia. Se desencola el primer proceso de baja prioridad
  if(!queue_empty(lp_queue)){
  disable_interrupt();
  TCB *thread_copy = dequeue(lp_queue);  //se desencola el primer proceso de baja prioridad
  enable_interrupt();

  current = thread_copy->tid;  //se actualia el id del currrent

  return thread_copy; //Devuelve el proceso
  }

  else if(!queue_empty(bl_queue)){
    running->state = IDLE;
    return &idle;
  }

  printf("*** FINISH\n"); ///no quedan hilos por ejecutar
  exit(1);
}


/* Timer interrupt */
void timer_interrupt(int sig)
{
  // printf("%d", running->state);
  if(running->state != IDLE){
    // printf("\nRemaining ticks: %d", running->remaining_ticks);
    if(running->state == RUNNING && running->priority == HIGH_PRIORITY){// si es de alta
      running->remaining_ticks--; //se reduce su ejecucion global

      if(running->remaining_ticks < 0){ //si se pasa de 0 se eyecta el thread
        mythread_timeout(running->tid);
      }

    }// de  baja prioridad

    else{

      running->ticks--; //se reduce su rodaja si es de baja prioridad
      running->remaining_ticks--; //se reduce su ejecucion global

      if(running->remaining_ticks == 0){
        //aqui se terminaria el while del function_thread y llama a mythread_exit
      }
      else{
        if(running->ticks <= 0){

            running->state = INIT;
            if (QUANTUM_TICKS < running->remaining_ticks) { //si la rodaja es menor que el tiempo que le queda de ejecucion se asigna la rodaja entera
              running->ticks =  QUANTUM_TICKS;
            }
            else{
              running->ticks = running->remaining_ticks;  //sino se le asigna lo que le quede de ejecucion
            }

            old_running = running;  //se actualiza el proceso anterior
            old_running->state = INIT;

            if(running->remaining_ticks < 0){
              mythread_timeout(running->tid);
            }

            disable_interrupt();
            enqueue(lp_queue, running);  //se encola el proceso anterior
            enable_interrupt();


            running = scheduler();  //se asigna el nuevo proceso a ejecutar
            running->state = RUNNING; //se cambia el estado de runnin
            activator(running); //se cambia el contexto

          }
        }

    }
  }
  else{
    if(!queue_empty(hp_queue) || !queue_empty(lp_queue)){
      old_running = running;  //se actualiza el proceso anterior
      running = scheduler();  //se asigna el nuevo proceso a ejecutar
      running->state = RUNNING; //se cambia el estado de runnin
      activator(running); //se cambia el contexto
    }
  }
  // printf("\nIDLE");
}

/* Activator */
void activator(TCB* next)
{
  if(old_running->tid == next->tid){  //si solo queda un hilo, no hay cambio de contexto
    return;
  }
  if(old_running->tid == -1){
    current = next->tid;  //se actualiza current thread
    printf("*** THREAD READY: SETCONTEXT TO %d\n\n", next->tid);
    setcontext (&(next->run_env));
    printf("mythread_free: After setcontext, should never get here!!...\n");
  }
  else if (old_running->remaining_ticks<=0){ //si se ha acabado el hilo, se procede a un setcontext
      current = next->tid;  //se actualiza current thread
      printf("*** THREAD %d TERMINATED : SETCONTEXT OF %d\n\n", old_running->tid, next->tid);
      setcontext (&(next->run_env));
      printf("mythread_free: After setcontext, should never get here!!...\n");
  }
  else if(old_running->priority == LOW_PRIORITY && next->priority == HIGH_PRIORITY){ //si se expulsa un hilo de baja prioridad con uno de alta
        printf("*** THREAD %d PREEMTED: SET CONTEXT OF %d\n", old_running->tid, next->tid );
        swapcontext(&(old_running->run_env), &(next->run_env));
  }
  else{ //se cambia el contexto
    current = next->tid; //se actualiza current thread
    printf("*** SWAPCONTEXT FROM %d TO %d\n", old_running->tid, next->tid);
    swapcontext(&(old_running->run_env), &(next->run_env));
  }

}
