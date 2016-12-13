# minios1

Simple timing and inter-task communication functions to extend a cooperative scheduler. Tasks can delay for a specified time or wait for data/signals. Messages can be sent to tasks from other tasks or from ISRs. ISRs cannot delay or receive messages.

The cooperative scheduler must define and expose "uint32_t ntasks" (number of active tasks), "uint32_t curtask" (id of current task in range 0..ntasks-1), and maintain those values as tasks are created and swapped.

Application must call mos_init( yield ), passing in a yield() function pointer.
  
    void mos_init( void(*yield_func)(void) );   // init OS vars and yield() function ptr

Timers require a periodic interrupt that calls mos_tick() from its ISR.

    void mos_tick( void );      // call on each timer interrupt to provide OS time base
  
Timer and mail functions are shown below. Timeouts are in units of periodic timer ticks, messages are type pointer to char, and error codes are defined in minios.h

    void  mos_delay  (long timeout);
    void  mos_post   (char **mboxp, char *msg, int *errp);
    char *mos_pend   (char **mboxp, long timeout, int *errp);
    char *mos_accept (char **mboxp, int *errp);
    
