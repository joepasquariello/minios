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
#if ( !defined( __MINICOOP_H_ ) )
#define __MINICOOP_H_

#include "Arduino.h"

#if( !defined( TEENSYDUINO ) )
#error "Teensy 3.x & Teensy lc Only!!!"
#endif

// define max number of tasks (fewer for Teensy LC)
#if defined(KINETISK)
#define MAX_TASKS 32
#elif defined(KINETISL)
#define MAX_TASKS 8
#endif

// make these visible externally for use by MINIOS
extern uint32_t ntasks;  // number of tasks (always at least 1 for main task)
extern uint32_t curtask; // index of active task (0..N-1)

// cooperative tasks are defined as void function with void* argument
typedef void ( *task_func_t )( void *arg );

// task_create() adds a cooperative task (w/ arbitrary data via void *arg)
// returns the index of the new task index (1..N-1)
uint16_t task_create( task_func_t func, uint32_t *stack, size_t stack_size, void *arg );

// yield() surrenders the CPU and causes a switch to the next cooperative task
void yield( void );

#endif
