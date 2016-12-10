/***********************************************************************************
* MINIOS timers and inter-task communication functions for cooperative scheduler.
* Copyright (c) 2016, Joe Pasquariello https://github.com/joepasquariello
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice, development funding notice, and this permission
* notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
***********************************************************************************/
#if (!defined(__MINIOS_H_))
#define __MINIOS_H_

#define MOS_MAXTASKS (32)           // should match num of tasks def'd in OS

// variables that must be defined in cooperative scheduler
extern unsigned long ntasks;        // number of tasks
extern unsigned long curtask;       // current task id (0..ntasks-1)

// variables defined in MINIOS
extern char *mos_mbox[MOS_MAXTASKS];// one mbox per task (for convenience)
extern unsigned long mos_ticks;     // count of calls to mos_tick()

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
