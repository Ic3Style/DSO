#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <interrupt.h>
#include <time.h>

static sigset_t maskval_interrupt,oldmask_interrupt;


void reset_timer(long usec) {
  struct itimerval quantum;
  /* Intialize an interval corresponding to round-robin quantum*/
  quantum.it_interval.tv_sec = usec / 1000000;
  quantum.it_interval.tv_usec = usec % 1000000;
  /* Initialize the remaining time from current quantum */
  quantum.it_value.tv_sec = usec / 1000000;
  quantum.it_value.tv_usec = usec % 1000000;
  /* Activate the virtual timer to generate a SIGVTALRM signal when the quantum is over */
  if(setitimer(ITIMER_VIRTUAL, &quantum, (struct itimerval *)0) == -1){
    perror("setitimer");
    exit(3);
  }
}

void enable_interrupt(){
  sigprocmask(SIG_SETMASK, &oldmask_interrupt, NULL);
}

void disable_interrupt(){
  sigaddset(&maskval_interrupt, SIGVTALRM);
  sigprocmask(SIG_BLOCK, &maskval_interrupt, &oldmask_interrupt);
}

void my_handler ()
{
   reset_timer(TICK_TIME) ;
   timer_interrupt() ;
}


void init_interrupt()
{
  void timer_interrupt(int sig);
  struct sigaction sigdat;
  /* Initializes the signal mask to empty */
  sigemptyset(&maskval_interrupt); 
  /* Prepare a virtual time alarm */
  sigdat.sa_handler = my_handler;
  sigemptyset(&sigdat.sa_mask);
  sigdat.sa_flags = SA_RESTART;
  if(sigaction(SIGVTALRM, &sigdat, (struct sigaction *)0) == -1){
    perror("signal set error");
    exit(2);
  }
   reset_timer(TICK_TIME) ;
}

static sigset_t maskval_net_interrupt,oldmask_net_interrupt;

void reset_disk_timer(long usec) {
  struct itimerval quantum;

  /* Intialize an interval corresponding to round-robin quantum*/
  quantum.it_interval.tv_sec = usec / 1000000;
  quantum.it_interval.tv_usec = usec % 1000000;
  /* Initialize the remaining time from current quantum */
  quantum.it_value.tv_sec = usec / 1000000;
  quantum.it_value.tv_usec = usec % 1000000;
  /* Activate the virtual timer to generate a SIGALRM signal when the quantum is over */
  if(setitimer(ITIMER_PROF, &quantum, (struct itimerval *)0) == -1){
    perror("setitimer");
    exit(3);
  }
}

void enable_disk_interrupt(){
  sigprocmask(SIG_SETMASK, &oldmask_net_interrupt, NULL);
}

void disable_disk_interrupt(){
  sigaddset(&maskval_net_interrupt, SIGPROF);
  sigprocmask(SIG_BLOCK, &maskval_net_interrupt, &oldmask_net_interrupt);
}

void my_disk_handler ()
{
  // reset_disk_timer(PACK_TIME) ;
   disk_interrupt() ;
}


void init_disk_interrupt()
{
  void disk_interrupt(int sig);
  struct sigevent event;
  timer_t timer_id;
  struct timespec periodTime;
  struct itimerspec timerdata; 
  struct sigaction sigdat;
 /* Create timer */
 event.sigev_notify = SIGEV_SIGNAL;
 event.sigev_signo = SIGPROF;
 timer_create (CLOCK_REALTIME, &event, &timer_id);
  
 // set periodTime time
 periodTime.tv_sec=1;
 periodTime.tv_nsec=0;

 /* Initializes the signal mask to empty */
 sigemptyset(&maskval_net_interrupt); 
 /* Prepare a virtual time alarm */

 sigdat.sa_handler = my_disk_handler;
 sigemptyset(&sigdat.sa_mask);
 sigdat.sa_flags = SA_RESTART;

 /* Arm periodic timer */
 timerdata.it_interval = periodTime;
 timerdata.it_value = periodTime;
 timer_settime (timer_id, 0, &timerdata, NULL);

 if(sigaction(SIGPROF, &sigdat, (struct sigaction *)0) == -1){
    perror("signal set error");
    exit(2);
 }
 //reset_disk_timer(PACK_TIME) ;

#ifdef DISK_INTERRUPT_SEED
 srand(DISK_INTERRUPT_SEED);
#else
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 srand(ts.tv_nsec);
#endif
}
