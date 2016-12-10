/***********************************************************************************
 * Minimal subset of Zilch cooperative scheduler for Teensy LC/3.x.
 * MiniCoop Copyright (c) 2016, Joe Pasquariello https://github.com/joepasquariello
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

typedef struct {
    uint32_t  sp;                             // stack pointer
    uint32_t  r4,r5,r6,r7,r8,r9,r10,r11,r12;  // registers
    uint32_t  lr;                             // return address (pc)
} task_context_t;

typedef struct {
    task_func_t    func;    // task function pointer
    void           *arg;    // task function argument
	task_context_t context; // task context
} task_t;

static task_t tasks[MAX_TASKS];
uint32_t ntasks=1, curtask=0;

//////////////////////////////////////////////////////////////////////
// task launch pad
//////////////////////////////////////////////////////////////////////
static void task_start( task_t *p ) {
    // argument "p" is r0, and is defined as a pointer to task_t
	// task_create() saved pointer to task_t in r12
    // load r0 (p) with r12 and call task function via p->func
	asm volatile( "mov r0,r12 \n" );
    p->func( p->arg );
    // if task returns, it will loop here forever
    for (;;) yield( );
}
//////////////////////////////////////////////////////////////////////
// Set up a task to execute, will launch when yield switches in
//////////////////////////////////////////////////////////////////////
uint16_t task_create( task_func_t func, uint32_t *stack, size_t stack_size, void *arg ) {
	// return error if MAX_TASKS already exist
    if (ntasks >= MAX_TASKS)
    	return( -1 );
    // get pointer to new task struct (and post-increment ntasks)
    task_t *task = &tasks[ntasks++];
    // save task function pointer and void* arg for use by task_start()
    task->arg = arg;
    task->func = func;
    // save stack frame values for first context switch
    //   sp  = END of stack array
    //   r12 = pointer to task_t struct (w/ function pointer and argument)
    //   lr  = link reg = task_start ptr (where all tasks "start")
    task->context.sp  = (uint32_t)&stack[stack_size];
    task->context.r12 = (uint32_t)task;
    task->context.lr  = (uint32_t)task_start;
    // fill task memory with this pattern for debug
    for ( uint16_t i=0; i<stack_size; i++ )
    	stack[i] = 0xCDCDCDCD;
    // return index of new task
    return( ntasks );
}
//////////////////////////////////////////////////////////////////////
// switch from "current" context to "next" context
//////////////////////////////////////////////////////////////////////
void context_switch( task_context_t *current, task_context_t *next ) {
// r0=current, r1=next
#if defined(KINETISK)
    asm volatile (
        "MRS r3, MSP"           "\n\t" // save sp to r3
        "STMEA r0,{r3-r12,lr}"  "\n\t" // save r3-r12,lr to current context
        "LDMIA r1,{r3-r12,lr}"  "\n\t" // load r3-r12,lr from next context
        "MSR MSP, r3"           "\n"   // load sp from r3
    );
#elif defined(KINETISL)
    asm volatile (
        "MRS r3, MSP"           "\n\t" // move sp to r3		// 12-03-16 JWP use MSP
        "STMIA r0!,{r3-r7}"     "\n\t" // Save r3,r4-r7 to r0! (currentframe->sp)
    	"MOV r2,r8"             "\n\t" // move r8       into r2
        "MOV r3,r9"             "\n\t" // move r9       into r3
        "MOV r4,sl"             "\n\t" // move sl (r10) into r4
        "MOV r5,fp"             "\n\t" // move fp (r11) into r5
        "MOV r6,ip"             "\n\t" // move ip (r12) into r6
        "MOV r7,lr"             "\n\t" // move lr       into r7
        "STMIA r0!,{r2-r7}"     "\n\t" // Save r8-r12,lr to r0! (currentframe->r8)

    	"ADD r1,#20"            "\n\t" // add 20 to r1 so r1 = &(nextframe->r8)
        "LDMIA r1!,{r2-r7}"     "\n\t" // load r2-r7 from r1! (inc r1 by 6*4=24)
        "MOV r8,r2"             "\n\t" // move r2 into r8
        "MOV r9,r3"             "\n\t" // move r3 into r9
        "MOV sl,r4"             "\n\t" // move r4 into sl (r10)
        "MOV fp,r5"             "\n\t" // move r5 into fp (r11)
        "MOV ip,r6"             "\n\t" // move r6 into ip (r12)
        "MOV lr,r7"             "\n\t" // move r7 into lr
        "SUB r1,#44"            "\n\t" // sub 44 from r1 so r1 = &(nextframe->sp)
        "LDMIA r1,{r3-r7}"      "\n\t" // load r3-r7 from r1
        "MSR MSP, r3"           "\n"   // move r3 into sp	// 12-03-16 JWP use MSP
    );
#endif
}
//////////////////////////////////////////////////////////////////////
// yield to next task (replaces default yield() in teensyduino)
//////////////////////////////////////////////////////////////////////
void yield( void ) {
	if (ntasks <= 1) return;
    // get prev/next task id and increment curtask w/ wrap to zero
    uint8_t prev = curtask++;
    uint8_t next = curtask = curtask >= ntasks ? 0 : curtask;
    // switch from "prev" task to "next" task
    context_switch( &tasks[prev].context, &tasks[next].context );
}
