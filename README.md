# minios

Simple timer and mailbox functions to add inter-task communication to a cooperative scheduler. Tasks can delay for a specified time or wait for data/signals via mailboxes. Messages can be sent from tasks or from ISRs.

The cooperative scheduler must define and expose "uint32_t ntasks" (number of active tasks), "uint32_t curtask" (id of current task in range 0..ntasks-1), and "void yield(void)"  (function to yield the CPU).

Application must call mos_init() and pass the address of the scheduler's "yield" function.
  
    void mos_init( void(*yield_func)(void) );   // init OS vars and yield() function ptr

Timers require a periodic interrupt that calls minios_tick() from its ISR.

    void mos_tick( void );      // call on each timer interrupt to provide OS time base
  
Timer and mail provided are as shown below. Timeouts are in units of periodic timer ticks, messages are type pointer to char, and error codes are defined in minios.h

    void  mos_delay  (long timeout);
    void  mos_post   (char **mboxp, char *msg, int *errp);
    char *mos_pend   (char **mboxp, long timeout, int *errp);
    char *mos_accept (char **mboxp, int *errp);
