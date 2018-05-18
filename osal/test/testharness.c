/*****************************************************************************/
/**
OSAL - Operating System Abstraction Layer for Embedded Systems
Copyright (C) 2011, Neil Johnson
All rights reserved.

Redistribution and use in source and binary forms,
with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
/*****************************************************************************/

/*****************************************************************************
   System Includes
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
 
/*****************************************************************************
   Project Includes
 *****************************************************************************/

#include "osal.h"

/*****************************************************************************
   Private typedefs, macros and constants.
 *****************************************************************************/

#define LOG(msg)            osal_log_message( OSAL_LOG_INFORMATIVE, msg )
#define LOGP(msg,p)         osal_log_message( OSAL_LOG_INFORMATIVE, msg, p )

#define LOTS_OF_SEMAPHORES  ( 5000 )

#define NUM_EVENTS          ( 27 )
#define EV_TMR              ( 1 << 2 )

#define QUEUE_LENGTH        ( 10 )
#define QUEUE_MAX_DELAY     ( 100 )

/*****************************************************************************
   Private Function Declarations.  Declare as static.
 *****************************************************************************/

static void test_semaphores( void );
static void tmr_handler( osal_timer_t *, void * );
static void test_timers( void );
static void test_events( void );
static void test_queues( void );
static void test_tasks( void );
static void test_mutexes( void );

/*****************************************************************************
   Private Data.  Declare as static.
 *****************************************************************************/

static osal_sem_t tmr_sem;

/*****************************************************************************
   Private Functions.  Declare as static.
 *****************************************************************************/

/*****************************************************************************/
/**
   Run semaphore tests.
**/
static void test_semaphores( void )
{
    int status;
    int i;
    
    osal_sem_t sem;
    
    LOG( "Initialise a semaphore" );
    status = osal_sem_init( &sem, 0, "sem:test" );
    assert( status == 0 );
    
    LOG( "Try obtaining the semaphore, should timeout" );
    status = osal_sem_obtain( &sem, 5000 );
    assert( status == ETIMEDOUT );
    
    LOG( "Bump the semaphore" );
    status = osal_sem_release( &sem );
    assert( status == 0 );
    
    LOG( "Try obtaining the semaphore" );
    status = osal_sem_obtain( &sem, 1000 );
    assert( status == 0 );
    
    LOG( "Try obtaining the semaphore again, should timeout" );
    status = osal_sem_obtain( &sem, 1000 );
    assert( status == ETIMEDOUT );

    LOG( "Release lots of times..." );
    for ( i = 0; i < LOTS_OF_SEMAPHORES; i++ )
    {
        status = osal_sem_release( &sem );
        assert( status == 0 );
    }
    
    LOG( "Obtain lots of times..." );
    for ( i = 0; i < LOTS_OF_SEMAPHORES; i++ )
    {
        status = osal_sem_obtain( &sem, 1000 );
        assert( status == 0 );
    }
    
    LOG( "Obtain once more, should timeout" );
    status = osal_sem_obtain( &sem, 1000 );
    assert( status == ETIMEDOUT );
    
    LOG( "Check suspend_never does, should timeout" );
    status = osal_sem_obtain( &sem, OSAL_SUSPEND_NEVER );
    assert( status == ETIMEDOUT );

    LOG( "Obtain a silly semaphore (arg checking), should fail" );
    status = osal_sem_obtain( NULL, 1000 );
    assert( status == EINVAL );
    
    LOG( "Destroy semaphore" );
    status = osal_sem_destroy( &sem );
    assert( status == 0 );
}

/*****************************************************************************/
/**
   Timer handler.
   
   Prints out a message, then decrements a counter.  If the count has ended
   then raises the global timer semaphore.
   
   @param tmr       Handle to timer object that called the handler.
   @param arg       General argument defined when timer object created.
**/
static void tmr_handler( osal_timer_t * tmr, void * arg )
{
    LOGP( "Tick (%d)", *(int *)arg );
    *(int *)arg = *(int *)arg - 1;
    
    /* Decide if we restart the timer, or raise the semaphore */
    if ( *(int *)arg > 0 )
        osal_timer_start( tmr, 1000 );
    else
        osal_sem_release( &tmr_sem );        
}

/*****************************************************************************/
/**
   Run timer tests.
**/
static void test_timers( void )
{
    int status;
    osal_timer_t tmr;    
    volatile int tmr_val;
    
    LOG( "Initialise test semaphore" );
    status = osal_sem_init( &tmr_sem, 0, "sem:tmr" );
    assert( status == 0 );
    
    LOG( "Initialise test timer" );
    status = osal_timer_init( &tmr, tmr_handler, (void *)&tmr_val, "tmr:test" );
    assert( status == 0 );
    
    /* Set the iteration counter */
    tmr_val = 5;
    
    LOG( "Start the test timer" );
    status = osal_timer_start( &tmr, 1000 );
    assert( 0 == status );
    
    LOG( "Wait for clock to tick..." );
    status = osal_sem_obtain( &tmr_sem, 10000 );
    assert( 0 == status );
    
    LOG( "Destroy test timer" );    
    status = osal_timer_destroy( &tmr );    
    assert( 0 == status );
    
    LOG( "Destroy test semaphore" );    
    status = osal_sem_destroy( &tmr_sem );    
    assert( 0 == status );
}

/*****************************************************************************/
/**
   Event timer handler.
   
   Timer handler for testing events.
   
   @param tmr       Handle to timer object that called the handler.
   @param arg       General argument defined when timer object created.
**/
static void event_tmr( osal_timer_t * tmr, void * arg )
{
    LOG( "Event timer ticked" );
    osal_event_set( (osal_event_t *)arg, EV_TMR );
}

/*****************************************************************************/
/**
   Run event tests.
**/
static void test_events( void )
{
    int status;
    osal_event_t event;
    osal_timer_t tmr;
    unsigned int evmask;
    
    LOG( "Initialise the event group" );
    status = osal_event_init( &event, NUM_EVENTS, "evt:test" );
    assert( status == 0 );
    
    LOG( "Setting a non-existent event should fail" );
    status = osal_event_set( &event, 1 << NUM_EVENTS );
    assert( EINVAL == status );
    
#define MASK1   ( 1 << 0 )

    LOG( "Setting an event should work" );
    status = osal_event_set( &event, MASK1 );
    assert( 0 == status );
    
    LOG("Retrieve the event mask and check with what was set" );
    status = osal_event_get( &event, &evmask );
    assert( 0 == status );
    assert( evmask == MASK1 );
    
    LOG( "Clear the event mask, then check it is clear" );
    status = osal_event_clear( &event, MASK1 );
    assert( 0 == status );    
    status = osal_event_get( &event, &evmask );
    assert( 0 == status );
    assert( evmask == 0 );
    
    LOG( "Initialise a timer to set an event some time in the future" );
    status = osal_timer_init( &tmr, event_tmr, (void *)&event, "tmr:event" );
    assert( status == 0 );
    
    LOG( "Start the timer to kick in 10s" );
    status = osal_timer_start( &tmr, 10000 );
    assert( 0 == status );
    
    LOG( "Then sit waiting for the timer to trigger a specific event" );
    status = osal_event_wait( &event, EV_TMR, NULL, OSAL_EVENT_MODE_ANY, 20000 );
    assert( 0 == status );
    
    LOG("Clear timer event" );
    status = osal_event_clear( &event, EV_TMR );
    assert( 0 == status );    
    status = osal_event_get( &event, &evmask );
    assert( 0 == status );
    assert( 0 == evmask );
    
    LOG( "Set one event" );
    status = osal_event_set( &event, MASK1 );
    assert( 0 == status );
    
    LOG( "Start the timer to kick in 10s" );
    status = osal_timer_start( &tmr, 10000 );
    assert( 0 == status );
    
    LOG( "Then sit waiting for the timer to trigger a specific event" );
    status = osal_event_wait( &event, ( MASK1 | EV_TMR ), NULL, OSAL_EVENT_MODE_ALL, 20000 );
    assert( 0 == status );
        
    LOG( "Confirm that the events set are the ones we waited on" );
    status = osal_event_get( &event, &evmask );
    assert( 0 == status );
    assert( ( MASK1 | EV_TMR ) == evmask );
    
    LOG(" Destroy the timer" );
    status = osal_timer_destroy( &tmr );    
    assert( 0 == status );
    
    LOG( "Destroy the event group" );
    status = osal_event_destroy( &event );    
    assert( 0 == status );
}

/*****************************************************************************/
/**
   Queue test task.
   
   Randomly pushes a message of random length into a queue.   
   
   @param task      Handle to task object that called the handler.
   @param param1    General argument defined when task object created.
   @param param2    General argument defined when task object created.
**/
static void queue_task( osal_task_t * task, void * param1, void * param2 )
{
    int status;
    osal_queue_t * q2 = (osal_queue_t *)param1;
    unsigned int count = 0;
    int i;
        
    LOG( "Entering queue test task" );
    
    for ( i = 0; i < 100; i++ )
    {
        struct msg {
            int a, b, c;
        } txmsg;
            unsigned int delay;
            
            txmsg.a = 'a' + ( count++ % 26 );
        txmsg.b = 'a' + ( count++ % 26 );
        txmsg.c = 'a' + ( count++ % 26 );
            
        LOGP( "T: [%d]", i );
            LOGP( "T: Message is: %c", txmsg.a );
        LOGP( "               %c", txmsg.b );
        LOGP( "               %c", txmsg.c );
        
        LOG( "Sending message" );
        status = osal_queue_send_to( q2, &txmsg, OSAL_SUSPEND_FOREVER );
        assert( 0 == status );
        
        delay = rand() % QUEUE_MAX_DELAY;
        LOGP( "Then sleep for %ims", delay );
        osal_task_sleep( delay );
    }
    
    LOG( "Leaving queue test task" );
}


/*****************************************************************************/
/**
   Run queue tests.
**/
static void test_queues( void )
{
    int status;
    osal_queue_t q;
    osal_queue_t q2;
    osal_task_t task;
    struct msg {
        int a, b, c;
    } txmsg, rxmsg;
    int i;
    
    LOG( "Initialise test message queue" );
    status = osal_queue_init( &q, QUEUE_LENGTH, sizeof(struct msg), "q:test" );
    assert( status == 0 );
    
    LOG( "Put a test message into the queue" );
    txmsg.a = 'a';
    txmsg.b = 'b';
    txmsg.c = 'c';    
    status = osal_queue_send_to( &q, &txmsg, 10000 );
    assert( 0 == status );
    
    LOG( "Get a test message from the queue" );
    memset( &rxmsg, 0, sizeof(rxmsg) );
    status = osal_queue_recv_from( &q, &rxmsg, 10000 );
    assert( 0 == status );
    assert( txmsg.a == rxmsg.a && txmsg.b == rxmsg.b && txmsg.c == rxmsg.c );
    
    LOG( "Fill the queue with messages" );
    for ( i = 0; i < QUEUE_LENGTH; i++ )
    {
        txmsg.a = i;
        txmsg.b = i + 1;
        txmsg.c = i + 2;
        LOGP( "Sending message %i", i+1 );
        status = osal_queue_send_to( &q, &txmsg, 10000 );
        assert( 0 == status );
    }
    
    LOG( "Try pushing one last message in, should timeout" );
    status = osal_queue_send_to( &q, &txmsg, 100 );
    assert( ETIMEDOUT == status );
    
    LOG( "Now extract each message, and check the sequence" );
    for ( i = 0; i < QUEUE_LENGTH; i++ )
    {
        memset( &rxmsg, 0, sizeof(rxmsg) );
        LOGP( "Receiving message %i", i );
        status = osal_queue_recv_from( &q, &rxmsg, 10000 );
        assert( 0 == status );
        assert( rxmsg.a == i && rxmsg.b == i + 1 && rxmsg.c == i + 2 );
    }    
    
    LOG( "Try pulling out one last message, should timeout" );
    status = osal_queue_recv_from( &q, &rxmsg, 100 );
    assert( ETIMEDOUT == status );
    
    LOG( "Initialise another queue" );
    status = osal_queue_init( &q2, QUEUE_LENGTH, sizeof(struct msg), "q:test2" );
    assert( status == 0 );
    
    LOG( "Initialise test task" );
    status = osal_task_init( &task, 0, queue_task, (void *)&q2, NULL, 0, "task:queue" );
    assert( status == 0 );
    
    LOG( "Start the test task" );
    status = osal_task_start( &task );
    assert( 0 == status );
    
    osal_task_sleep( 1000 );
    
    for ( i = 0; i < 100; i++ )
    {
        struct msg {
            int a, b, c;
        } rxmsg;
        
        LOGP( "Receive message %d", i );
        status = osal_queue_recv_from( &q2, &rxmsg, 3 * QUEUE_MAX_DELAY );
        assert( 0 == status || ETIMEDOUT == status );  
        LOGP( "R: Message is: %c", rxmsg.a );
        LOGP( "               %c", rxmsg.b );
        LOGP( "               %c", rxmsg.c );   

        {
           int delay = rand() % QUEUE_MAX_DELAY;
               LOGP( "Then sleep for %ims", delay );
               osal_task_sleep( delay ); 
        }
    }    

    LOG( "Destroy the test task" );
    status = osal_task_destroy( &task );
    assert( 0 == status );
    
    LOG( "Destroy the test queue" );
    status = osal_queue_destroy( &q2 );
    assert( 0 == status );
        
    LOG( "Destroy the test queue" );
    status = osal_queue_destroy( &q );
    assert( 0 == status );
}

/*****************************************************************************/
/**
   Test task.
   
   Bounces between two semaphores with the test harness until we timeout.   
   
   @param task      Handle to task object that called the handler.
   @param param1    General argument defined when task object created.
   @param param2    General argument defined when task object created.
**/
static void task_func( osal_task_t * task, void * param1, void * param2 )
{
    int status;
    osal_sem_t * sem1 = param1;
    osal_sem_t * sem2 = param2;
    
    LOG( "Entering test task" );
    
    while ( 1 )
    {
        LOG( "Wait for the first semaphore" );
        status = osal_sem_obtain( sem1, 5000 );
        if ( ETIMEDOUT == status )
            break;
        else
            assert( 0 == status );
        
        LOG( "Raise the second semaphore" );
        status = osal_sem_release( sem2 );
        assert( 0 == status );
    }
    
    LOG( "Leaving test task" );
}

/*****************************************************************************/
/**
   Run task tests.
**/
static void test_tasks( void )
{
    int status;
    osal_task_t task;
    osal_sem_t sem1, sem2;
    int i;
    
    LOG( "Initialise two test semaphores" );
    status = osal_sem_init( &sem1, 0, "sem:tasktest1" );
    assert( status == 0 );
    status = osal_sem_init( &sem2, 0, "sem:tasktest2" );
    assert( status == 0 );
    
    LOG( "Initialise the test task" );
    status = osal_task_init( &task, 0, task_func, (void *)&sem1, (void *)&sem2, 0, "task:test" );
    assert( status == 0 );
    
    LOG( "Start the test task" );
    status = osal_task_start( &task );
    assert( 0 == status );
    
    for ( i = 0; i < 10; i++ )
    {
        LOG(" Raise the first semaphore" );
        status = osal_sem_release( &sem1 );
        assert( 0 == status );
        
        LOG( "Wait for the second semaphore" );
        status = osal_sem_obtain( &sem2, OSAL_SUSPEND_FOREVER );
        assert( 0 == status );
    }    
    
    LOG( "Finished test, wait for task to quit" );
    osal_task_sleep( 10000 );
    
    LOG( "Stop the test task" );
    status = osal_task_stop( &task );
    assert( 0 == status );
    
    LOG( "Destroy the test task" );
    status = osal_task_destroy( &task );
    assert( 0 == status );
}

/*****************************************************************************/
/**
   Run mutex tests.
**/
static void test_mutexes( void )
{
    int status;
    int i;
    
    osal_mutex_t mtx;
    
    LOG( "Initialise a mutex" );
    status = osal_mutex_init( &mtx, "mtx:test" );
    assert( status == 0 );
    
    LOG( "Try locking the mutex, should be OK" );
    status = osal_mutex_obtain( &mtx, 5000 );
    assert( status == 0 );
    
    LOG( "Try locking the mutex again, should timeout" );
    status = osal_mutex_obtain( &mtx, 5000 );
    assert( status == ETIMEDOUT );
    
    LOG( "Release the mutex" );
    status = osal_mutex_release( &mtx );
    assert( status == 0 );
    
    LOG( "Release the mutex again, should fail" );
    status = osal_mutex_release( &mtx );
    assert( status == EFAULT );
    
    LOG( "Obtain a silly mutex (arg checking), should fail" );
    status = osal_mutex_obtain( NULL, 1000 );
    assert( status == EINVAL );
    
    LOG( "Destroy mutex" );
    status = osal_mutex_destroy( &mtx );
    assert( status == 0 );
}

/*****************************************************************************/
/**
   Test application task main function.
**/
static int test_app_main( int argc, const char **argv )
{
    enum {
        TEST_SEMAPHORE = 1 << 0,
        TEST_TIMER     = 1 << 1,
        TEST_EVENT     = 1 << 2,
        TEST_QUEUE     = 1 << 3,
        TEST_TASK      = 1 << 4,
	TEST_MUTEX     = 1 << 5
    } test = 0;
    
    if ( argc == 1 )
        test = ~0;  /* run all tests */
    else if ( argc == 2 )
    {
        if ( strcmp( argv[1], "all" ) == 0 )
            test = ~0;
        else if ( strcmp( argv[1], "semaphore" ) == 0 )
            test = TEST_SEMAPHORE;
        else if ( strcmp( argv[1], "timer" ) == 0 )
            test = TEST_TIMER;
        else if ( strcmp( argv[1], "event" ) == 0 )
            test = TEST_EVENT;
        else if ( strcmp( argv[1], "queue" ) == 0 )
            test = TEST_QUEUE;
        else if ( strcmp( argv[1], "task" ) == 0 )
            test = TEST_TASK;
	else if ( strcmp( argv[1], "mutex" ) == 0 )
            test = TEST_MUTEX;
        else
        {
            fprintf( stderr, "Error: unknown test \"%s\".\n"
                             "       Must be one of: all, semaphore, timer, \n"
                             "                       event, task, queue, mutex\n", argv[1] );
            return -1;
        }    
    }
    else
    {
        fprintf( stderr, "Usage: osaltest <testname>\n\n"
                         "testname: all - run all tests, otherwise one of:\n"
                         "          semaphore, timer, event, task, queue\n" );
        return -1;
    }

    osal_log_open( "osal_test.txt" );    
    osal_log_set_level( OSAL_LOG_INFORMATIVE );    
    osal_log_message( OSAL_LOG_INFORMATIVE, "Starting OSAL log" );
    
    if ( test & TEST_SEMAPHORE )
    {
        LOG( "## SEMAPHORE TESTS ###########################################" );
        test_semaphores();
        LOG( "##############################################################" );
    }
    
    if ( test & TEST_TIMER )
    {
        LOG( "## TIMER TESTS ###############################################" );
        test_timers();
        LOG( "##############################################################" );
    }
        
    if ( test & TEST_EVENT )
    {
        LOG( "## EVENT TESTS ###############################################" );
        test_events();
        LOG( "##############################################################" );
    }
        
    if ( test & TEST_QUEUE )
    {
        LOG( "## QUEUE TESTS ###############################################" );
        test_queues();
        LOG( "##############################################################" );
    }

    if ( test & TEST_TASK )
    {
        LOG( "## TASK TESTS ################################################" );
        test_tasks();
        LOG( "##############################################################" );
    }
    
    if ( test & TEST_MUTEX )
    {
        LOG( "## MUTEX TESTS ###############################################" );
        test_mutexes();
        LOG( "##############################################################" );
    }
        
    osal_log_message( OSAL_LOG_INFORMATIVE, "Closing OSAL log" );
    osal_log_close();
    
    return 0;
}


/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/ 

/*****************************************************************************/
/**
   Main application.
   
   Runs through the various module tests.
   
   @param argc          UNUSED
   @param argv          UNUSED
   
   @return EXIT_SUCCESS if successful, else EXIT_FAILURE.
**/
int main( int argc, char **argv )
{
    osal_start( test_app_main, argc, (const char **)argv );
    osal_shutdown();
    
    return EXIT_SUCCESS;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
