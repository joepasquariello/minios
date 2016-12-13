/***********************************************************************************
* Minimal cooperative scheduler for Teensy LC/3.x, derived from full-feature Zilch.
* minicoop Copyright (c) 2016, Joe Pasquariello https://github.com/joepasquariello
* Zilch Copyright (c) 2016, Colin Duffy https://github.com/duff2013
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
#include "minicoop.h"

// task context to save on task's stack
typedef struct {
    uint32_t  sp;                             // stack pointer
    uint32_t  r4,r5,r6,r7,r8,r9,r10,r11,r12;  // registers
    uint32_t  lr;                             // return address (pc)
} task_context_t;

// task structure contains context (and possibly more someday)
typedef struct {
  task_context_t context;
} task_t;

static task_t tasks[MAX_TASKS];
uint32_t ntasks=1, curtask=0;

/*******************************************************************************
* task_start() launches a task. Each task context is initialized in task_create
* to start execution here (context->lr = task_start). The task function call is
* made using context->r11 (function address) and context->r12 (argument).
******************************************************************************/
static void task_start( void ) {
  // call function whose address is in r11 w/ argument whose value is in r12
  asm volatile( "mov r0,r12 \n" );  // load r0 with argument in r12
  asm volatile( "blx r11    \n" );  // branch to function address in r11
  // if task returns, loop here forever
  for (;;) yield( );
}

/*******************************************************************************
* task_create() inits context for new task so it executes when first swapped in
*******************************************************************************/
uint16_t task_create( task_func_t func, uint32_t *stack, size_t stack_size, void *arg ) {
  uint16_t i;
  // return error if MAX_TASKS already exist
  if (ntasks >= MAX_TASKS)
    return( -1 );
  // get pointer to new task_t (and post-increment ntasks)
  task_t *task = &tasks[ntasks++];
  // store task info in context for use by task_start()
  task->context.sp  = (uint32_t)&stack[stack_size]; // address of stack bottom
  task->context.lr  = (uint32_t)task_start;         // address of task_start()
  task->context.r11 = (uint32_t)func;               // task function pointer
  task->context.r12 = (uint32_t)arg;                // task function argument
  // fill task memory with this pattern for debug
  for (i=0; i<stack_size; i++)
    stack[i] = 0xCDCDCDCD;
  // return index of new task
  return( ntasks );
}

/*******************************************************************************
* task_switch() switches from "current" to "next" task
*******************************************************************************/
void task_switch( task_context_t *current, task_context_t *next ) {
// w/ gcc, we can assume arguments are in r0 and r1 (r0=current, r1=next)
#if defined(KINETISK)
    asm volatile (
        "MRS r3, MSP"           "\n\t" // save sp to r3
        "STMEA r0,{r3-r12,lr}"  "\n\t" // save r3-r12,lr to current context
        "LDMIA r1,{r3-r12,lr}"  "\n\t" // load r3-r12,lr from next context
        "MSR MSP, r3"           "\n"   // load sp from r3
    );
#elif defined(KINETISL)
    asm volatile (
        "MRS r3, MSP"           "\n\t" // move sp to r3 (JWP use MSP)
        "STMIA r0!,{r3-r7}"     "\n\t" // Save r3,r4-r7 to r0! (current->sp)
    	"MOV r2,r8"             "\n\t" // move r8       into r2
        "MOV r3,r9"             "\n\t" // move r9       into r3
        "MOV r4,sl"             "\n\t" // move sl (r10) into r4
        "MOV r5,fp"             "\n\t" // move fp (r11) into r5
        "MOV r6,ip"             "\n\t" // move ip (r12) into r6
        "MOV r7,lr"             "\n\t" // move lr       into r7
        "STMIA r0!,{r2-r7}"     "\n\t" // Save r8-r12,lr to r0! (current->r8)

    	"ADD r1,#20"            "\n\t" // add 20 to r1 so r1 = &(next->r8)
        "LDMIA r1!,{r2-r7}"     "\n\t" // load r2-r7 from r1!  (r1 += 6*4=24)
        "MOV r8,r2"             "\n\t" // move r2 into r8
        "MOV r9,r3"             "\n\t" // move r3 into r9
        "MOV sl,r4"             "\n\t" // move r4 into sl (r10)
        "MOV fp,r5"             "\n\t" // move r5 into fp (r11)
        "MOV ip,r6"             "\n\t" // move r6 into ip (r12)
        "MOV lr,r7"             "\n\t" // move r7 into lr
        "SUB r1,#44"            "\n\t" // sub 44 from r1 so r1 = &(next->sp)
        "LDMIA r1,{r3-r7}"      "\n\t" // load r3-r7 from r1
        "MSR MSP, r3"           "\n"   // move r3 into sp (JWP use MSP)
    );
#endif
}

/*******************************************************************************
* yield() CPU to next task (replaces default yield() in teensyduino)
*******************************************************************************/
void yield( void ) {
  if (ntasks <= 1) return;
  // get prev/next task id and increment curtask w/ wrap to zero
  uint8_t prev = curtask++;
  uint8_t next = curtask = (curtask >= ntasks) ? 0 : curtask;
  // switch from "prev" task to "next" task
  task_switch( &tasks[prev].context, &tasks[next].context );
}
