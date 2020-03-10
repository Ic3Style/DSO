#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>


#include <time.h>



#define TICK_TIME 5000
#define PACK_TIME 1
#define STARVATION 200

// Define this macro for predictable and consistent behavior between executions
//#define DISK_INTERRUPT_SEED 0xff00ff00

void timer_interrupt ();
void init_interrupt();
void disable_interrupt();
void enable_interrupt();

void disk_interrupt ();
void init_disk_interrupt();
void disable_disk_interrupt();
void enable_disk_interrupt();
