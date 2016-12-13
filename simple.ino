/*********************************************************************************
* Example program for MINICOOP and MINIOS
*********************************************************************************/
#include <timerone.h>
#include <minicoop.h>  // task_create()
#include <minios.h>    // mos_init(), mos_tick(), etc.

#define STACK_SIZE 128
static uint32_t stack1[STACK_SIZE] __attribute__ ((aligned (4)));
static uint32_t stack2[STACK_SIZE] __attribute__ ((aligned (4)));

// periodic timer interrupt calls this function
static void timerFunc( void ) {
  // call minios timer function
  mos_tick();

  // toggle LED at about 8 Hz (use power of 2 to avoid ticks roll-over issue)
  if ((mos_ticks % 128) == 0)
    digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );

  int err;
  // signal task 1 at about 8 Hz
  if ((mos_ticks % 128) == 0)
    mos_post( &mos_mbox[1], MOS_SIGNAL, &err );

  // signal task 2 at about 4 Hz
  if ((mos_ticks % 256) == 0)
    mos_post( &mos_mbox[2], MOS_SIGNAL, &err );
}

// pend on mailbox and write task id each time mail is received
static void print_task( void *arg ) {
  int err;
  while ( 1 ) {
    mos_pend( &mos_mbox[curtask], 0, &err );
    Serial.print( curtask );
  }
}

// setup() is called once at bootup, before loop()
void setup( void ) {
  // configure LED pin as digital output
  pinMode(LED_BUILTIN , OUTPUT);

  // init minios and periodic timer (1 kHz)
  mos_init( yield );
  Timer1.initialize( 1000 );
  Timer1.attachInterrupt( timerFunc );

  // create 2 instances of print_task(), each with its own stack
  task_create( print_task, stack1, STACK_SIZE, 0 );
  task_create( print_task, stack2, STACK_SIZE, 0 );
}

// loop() is called repeatedly
void loop( void ) {
  while (!Serial) {}
  while (1) {
    mos_delay( 1024*2 );
    Serial.println();
  }
}
