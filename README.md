# minios

Simple timer and mailbox functions to add inter-task communication to a cooperative scheduler. Tasks can delay for a specified time or wait for data/signals via mailboxes. Messages can be sent from tasks or from ISRs.

The cooperative scheduler must define and expose "uint32_t ntasks" (number of active tasks), "uint32_t curtask" (id of current task in range 0..ntasks-1), and "void yield(void)"  (function to yield the CPU).

Application must call minios_init() and pass a pointer to the "yield" function.
  
      void minios_init( void(*yield_func)(void) ); // init OS variables and yield_func

Timers require a periodic interrupt that calls minios_tick() from its ISR.

      void minios_tick( void );                 // call on each OS tick
  
Timer and mail provided are as shown below. Timeouts are in units of periodic timer ticks. Messages are type pointer to char. Error codes are defined in minios.h

    void  minios_delay  (long timeout);
    void  minios_post   (char **mboxp, char *msg, int *errp);
    char *minios_pend   (char **mboxp, long timeout, int *errp);
    char *minios_accept (char **mboxp, int *errp);
