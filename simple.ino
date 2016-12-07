/*********************************************************************************
* Example program for ZILCH (2.1) and MINIOS
*********************************************************************************/
#include <zilch.h>
#include <timerone.h>
#include "minios.h"

// perform a set of actions every 100 ms
static void timed_task(void *arg) {
  uint32_t count=0;
  while ( 1 ) {
	// delay 100 ms and update counter (10 Hz)
	mos_delay( 100 );
	count++;

	// toggle LED every 200 ms (5 Hz)
    if ((count % 2) == 0) {
      digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
    }

    // signal task[1] every 500 ms (2 Hz)
    int err;
    if ((count % 5) == 0)
  	  mos_post( &mos_mbox[1], MOS_SIGNAL, &err );

    // signal task[2] every 1000 ms (1 Hz)
    if ((count % 10) == 0)
  	  mos_post( &mos_mbox[2], MOS_SIGNAL, &err );
  }
}

// pend on mailbox and write task id each time mail is received
static void print_task(void *arg) {
  int err;
  while ( 1 ) {
    mos_pend( &mos_mbox[curtask], 0, &err );
    Serial.print( curtask );
    // write a newline for nice output
    if (curtask==ntasks-1)
      Serial.println();
  }
}

Zilch task;		// Use default memory fill of 0xCDCDCDCD
#define STACK_SIZE 128
static uint32_t stack0[STACK_SIZE] __attribute__ ((aligned (4)));
static uint32_t stack1[STACK_SIZE] __attribute__ ((aligned (4)));
static uint32_t stack2[STACK_SIZE] __attribute__ ((aligned (4)));

void setup() {
  // configure LED pin and wait for Serial init
  pinMode(LED_BUILTIN , OUTPUT);
  while (!Serial);
  delay(100);

  // timer and mailbox init
  mos_init(yield);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(mos_tick);

  // zilch task create and begin()
  task.create(timed_task, stack0, STACK_SIZE, 0);
  task.create(print_task, stack1, STACK_SIZE, 0);
  task.create(print_task, stack2, STACK_SIZE, 0);
  task.begin();
}

// loop() not used with Zilch 2.1
void loop() {
}
