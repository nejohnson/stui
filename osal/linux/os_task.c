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
 
#include <string.h>     /* for mem*() and str*() functions */
 
/*****************************************************************************
   Project Includes
 *****************************************************************************/

#undef OSAL_TRACE   /* Make sure we have the non-trace defs */
#include "osal.h"

/*****************************************************************************
   Private typedefs, macros and constants.
 *****************************************************************************/

/**
   Set the default stack size.
**/
#define DEFAULT_STACK_SIZE    ( 64 * 1024 )     /* Allow 64kB */

/*****************************************************************************
   Private Function Declarations.  Declare as static.
 *****************************************************************************/                  

static void * task_starter( void * );

/*****************************************************************************
   Private Functions.  Declare as static.
 *****************************************************************************/

/*****************************************************************************/
/**
   Wrapper function used to marshal arguments for OSAL tasks to get round
   limits of Linux.
   
   @param argv    Used to pass the address of a task descriptor
**/
static void * task_starter( void * argv )
{
   osal_task_t * task = (osal_task_t *)argv;
   
   if ( task && task->task_func )
   {
      osal_sem_obtain( &(task->start_sem), OSAL_SUSPEND_FOREVER );
      (task->task_func)( task, task->param1, task->param2 );
   }
   task->exited = TRUE;
   
   return NULL;
}

/*****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 *****************************************************************************/
 
/*****************************************************************************/
/**
   Initialise a task.
**/
extern int (osal_task_init)(
      osal_task_t * task, 
      size_t        stack_size,
      void       (* task_func)(osal_task_t *, void *, void *),
      void *        param1,
      void *        param2,
      unsigned int  priority,
      const char  * nametag
)
{
   /* Check arguments first */
   if ( NULL == task || NULL == task_func )
      return EINVAL;
      
   /* Use default stack size if required */
   if ( 0 == stack_size )
      stack_size = DEFAULT_STACK_SIZE;
      
   /* Make sure we meet the minimal stack size demanded of pthreads */
   stack_size = MAX( stack_size, PTHREAD_STACK_MIN );
         
#ifndef NDEBUG
   memset( task->nametag, 0, sizeof(task->nametag) );
   if ( nametag )
      strncpy( task->nametag, nametag, sizeof(task->nametag) );
#endif   
   
   task->task_func  = task_func;
   task->param1     = param1;
   task->param2     = param2;
   task->stack_size = stack_size;
   task->priority   = priority;
   task->started    = FALSE;
   task->stopped    = TRUE;
   task->exited     = FALSE;
   
   /* Initialise the start semaphore */
   if ( osal_sem_init( &(task->start_sem), 0, "task:start_sem" ) )
      return EFAULT;
   
   if ( osal_task_reset( task ) != 0 )
   {
      osal_sem_destroy( &(task->start_sem) );
      return EFAULT;
   }
   
   return 0;
}

/*****************************************************************************/
/**
   Reset a task. Stops the task first if it is currently running.
**/
extern int (osal_task_reset)( osal_task_t * task )
{
   pthread_attr_t      attr;
   struct sched_param  param;
   int                 retcode;
   int                 status;
   
   if ( NULL == task )
      return EINVAL;
   
   status = osal_task_stop( task );
   if ( status != 0 )
      return status; 
   
   /* Reset started+stopped flags */
   task->started = task->stopped = task->exited = FALSE;
   
   /* Set up thread attributes */
   pthread_attr_init( &attr );
   pthread_attr_setstacksize( &attr, task->stack_size );
   param.sched_priority = 255 - task->priority;
   pthread_attr_setschedparam( &attr, &param );
      
   /* Start the thread */
   retcode = pthread_create( &(task->tcb), &attr, task_starter, task );
   
   /* Tidy up */
   pthread_attr_destroy( &attr );
   
   return ( retcode ? EFAULT : 0 );
}


/*****************************************************************************/
/**
   Destroy a task.
**/
extern int (osal_task_destroy)( osal_task_t * task )
{
   int status;
   
   if ( NULL == task )
      return EINVAL;
   
   /* If we are deleting a still-running thread then we need to kill it
    *  before we delete the task object.
    */
   status = osal_task_stop( task );
   if ( status != 0 )
     return status;
      
   osal_sem_destroy( &(task->start_sem) );
   return 0;
}

/*****************************************************************************/
/**
   Starts or resumes a task.
   
   NOTE: Linux version does not support stopping tasks in a way that allows
    them to be restarted.  All we can do here is to start a task for the first
    time.
**/
extern int (osal_task_start)( osal_task_t * task )
{
   if ( NULL == task )
      return EINVAL;
   
   if ( !task->started )
   {
      if ( osal_sem_release( &(task->start_sem) ) != 0 )
         return EFAULT;
      
      task->started = TRUE;
   }
      
   return 0;
}

/*****************************************************************************/
/**
   Stops a task executing.  May not be later resumed.
**/
extern int (osal_task_stop)( osal_task_t * task )
{
   if ( NULL == task )
      return EINVAL;
      
   /* If we are deleting a still-running thread then we need to kill it
    *  before we delete the task object.
    */
   if ( !task->stopped )
   {
      /* Tell the thread to stop.  It may have already exited though, which 
       *  results in an error.
       */
      if ( pthread_cancel( task->tcb ) != 0 && !task->exited )
         return EFAULT;
         
      /* Wait for thread to stop and reclaim thread resources */
      if ( pthread_join( task->tcb, NULL ) != 0 )
         return EFAULT;
         
      task->stopped = TRUE;      
   }
   
   return 0;
}

/*****************************************************************************/
/**
   Sets a task's priority level.
**/
extern int (osal_task_set_priority)(
      osal_task_t  * task,
      unsigned int   priority,
      unsigned int * old_priority
)
{
   struct sched_param param, old_param;
   int                ignored;
   
   if ( NULL == task )
      return EINVAL;
      
   if ( old_priority )
   {
      pthread_getschedparam( task->tcb, &ignored, &old_param );
      *old_priority = old_param.sched_priority;
   }
   
   param.sched_priority = 255 - priority;
   pthread_setschedparam( task->tcb, 0, &param );
   
   return 0;
}

/*****************************************************************************/
/**
   Get a task's priority level.
**/
extern int (osal_task_get_priority)(
      osal_task_t   * task,
      unsigned int  * priority
)
{
   struct sched_param old_param;
   int                ignored;
   
   if ( NULL == task || NULL == priority )
      return EINVAL;

   pthread_getschedparam( task->tcb, &ignored, &old_param );
   *priority = old_param.sched_priority;
   
   return 0;
}

/*****************************************************************************/
/**
   Put the current task to sleep for a specified period.
**/
extern void (osal_task_sleep)( unsigned int delay_ms )
{
   struct timespec delay;
   
   pthread_testcancel();   // Allow this task to be deleted

   delay.tv_sec  = delay_ms / 1000;
   delay.tv_nsec = ( delay_ms % 1000 ) * 1000000;
   while ( delay.tv_nsec >= 1000000000L )
   {
      delay.tv_nsec -= 1000000000L;
      delay.tv_sec++;
   }
   nanosleep( &delay, NULL );

   pthread_testcancel();   // Allow this task to be deleted
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
