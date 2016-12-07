/******************************************************************************
* MINIOS.H	Timers and mailboxes for cooperative multi-tasking.
******************************************************************************/
#if (!defined(__MINIOS_H_))
#define __MINIOS_H_

#define MOS_MAXTASKS (32)           // should match number of tasks defined in OS

// variables that must be defined in cooperative scheduler
extern unsigned long ntasks;        // number of tasks
extern unsigned long curtask;       // current task id (0..ntasks-1)

// one mailbox per task (for convenience)
extern char *mos_mbox[MOS_MAXTASKS];

// initialization and timer tick functions
void mos_init( void(*wait_func)(void) ); // init MOS variables
void mos_tick( void );                   // call on each MOS tick

// error/return values
#define	MOS_OK   (0)  // okay
#define	MOS_TID  (1)  // task ID error
#define	MOS_MIU  (2)  // mailbox in use
#define	MOS_ZMW  (3)  // zero message
#define	MOS_TMO  (4)  // timeout
#define	MOS_NMP  (5)  // no message present

// useful message definitions
#define MOS_EMPTY  ((char*)0)  // empty (NULL) message
#define MOS_SIGNAL ((char*)1)  // arbitrary non-empty message

// timer and mailbox functions
void  mos_delay  (long timeout);
void  mos_post   (char **mboxp, char *msg, int *errp);
char *mos_pend   (char **mboxp, long timeout, int *errp);
char *mos_accept (char **mboxp, int *errp);

#endif
