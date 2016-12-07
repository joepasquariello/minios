/******************************************************************************
* MINIOS.C	Timers and mailboxes for cooperative multi-tasking.
******************************************************************************/
#include "minios.h"                 // constants, array sizes, etc.

char* mos_mbox[MOS_MAXTASKS];       // one mailbox per task (convenience only)
static long timer[MOS_MAXTASKS];    // one timer per task (required)
static void (*wait)( void );        // pointer to wait (yield) function

/******************************************************************************
* mos_init()   Init FSOS wait() function pointer, empty mailboxes, zero timers
******************************************************************************/
void mos_init( void(*wait_func)(void) )
{
  int i;
  wait = wait_func;                     // init wait() function pointer
  for (i=0; i<MOS_MAXTASKS; i++) {      // for each possible task
    mos_mbox[i] = 0;                    //   init mailbox = EMPTY
    timer[i] = 0;                       //   init timer = DONE
  }
}

/******************************************************************************
* mos_tick()   Decrement non-zero task timers (called by system tick ISR)
******************************************************************************/
void mos_tick( void )
{
  int i;						        // local index
  for (i=0; i<(int)ntasks; i++) {       // for each task
    if (timer[i])                       //   if timer not expired
      timer[i]--;                       //     decrement timer
  }
}

/******************************************************************************
* mos_delay()	Suspend task with timeout (if timeout==0, do a task switch)
******************************************************************************/
void mos_delay( long timeout )
{
  if (timeout) {                        // if non-zero timeout...
    timer[curtask] = timeout;           //   init timer = timeout
    while (timer[curtask]) wait();      //   while (!done) wait()
  }
  else wait();                          // else yield CPU
}

/******************************************************************************
* mos_post()	Post to mailbox (error if not empty or message==0)
******************************************************************************/
void mos_post( char **mboxp, char *msg, int *errp )
{
  if (*mboxp != 0) {                    // if mailbox is not empty
    *errp = MOS_MIU;                    //   return error "mailbox in use"
  }
  else if (msg == 0) {                  // else if message == NULL
    *errp = MOS_ZMW;                    //   return error "zero message"
  }
  else {                                // else
    *errp = MOS_OK;                     //   return OK
    *mboxp = msg;                       //   write message to mailbox
  }
}

/******************************************************************************
* mos_pend()	Pend on mailbox with timeout (error only if timeout)
*
* if mailbox is not empty
*   return msg
* else if timeout != 0
*   wait until mailbox has data or timeout expires (MOS_TMO)
* else
*   wait until mailbox has data
******************************************************************************/
char *mos_pend( register char **mboxp, long timeout, int *errp )
{
  char *msg = 0;                        // init msg = 0 (empty)
  if (*mboxp) {                         // if mailbox is not empty
    *errp = MOS_OK;                     //   error value = MOS_OK
  }
  else if (timeout) {                   // else if timeout != 0
    timer[curtask] = timeout;           //   init timer = timeout
    while (*mboxp==0 && timer[curtask]) //   while no mail and !timeout
      wait();                           //     wait()
    *errp = *mboxp ? MOS_OK : MOS_TMO;  //   set error value
  }
  else {                                // else timeout==0 (forever)
    while (*mboxp==0)                   //   while mailbox is empty
      wait();                           //     wait
    *errp = MOS_OK;                     //   error value = MOS_OK
  }
  
  if (*errp==MOS_OK) {                  // if mail was received
    msg = *mboxp;                       //   get message
    *mboxp = 0;	                        //   empty mailbox
  }
  return msg;                           // return message
}

/******************************************************************************
* mos_accept()	Get message from mailbox without waiting
******************************************************************************/
char *mos_accept( char **mboxp, int *errp )
{
  char *msg = *mboxp;                   // get the message (may be NULL)
  if (msg == 0) {                       // if mailbox was empty
    *errp = MOS_NMP;                    //   error value = MOS_NMP
  }
  else {                                // else if mailbox not empty
    *errp = MOS_OK;                     //   error value = MOS_OK
    *mboxp = 0;                         //   empty the mailbox
  }
  return msg;                           // return the message value
}

