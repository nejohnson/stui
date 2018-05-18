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



/*****************************************************************************
   Project Includes
 *****************************************************************************/

#undef OSAL_TRACE   /* Make sure we have the non-trace defs */
#include "osal.h"

/*****************************************************************************
   Private typedefs, macros and constants.
 *****************************************************************************/

/*****************************************************************************
   Private Functions.  Declare as static.
 *****************************************************************************/

/*****************************************************************************/
/**
   Wrapper function used marshal arguments for OSAL tasks.
   Also apply any host-specific task setup and shutdown here.

   @param argc    UNUSED
   @param argv    Used to pass a task handle
**/
static DWORD WINAPI task_starter( LPVOID argv )
{
   osal_task_t * task = (osal_task_t *)argv;

   if ( task && task->task_func )
      (task->task_func)( task, task->param1, task->param2 );

   return 0;
}

/*****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 *****************************************************************************/

/*****************************************************************************/
/**
   Create a task.

   @param stack_size   Size in bytes of the required task stack, or 0 for
                        an OS-specific default size.
   @param task_func    Address of the task's entry function.
   @param param1       First task argument.
   @param param2       Second task argument.
   @param priority     Task's initial priority level.
   @param nametag      Address of a string constant, used in debug only.

   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_task_init)(
      osal_task_t * task,
      size_t        stack_size,
      void       (* task_func)(osal_task_t *, void *, void *),
      void        * param1,
      void        * param2,
      unsigned int       priority,
      const char  * nametag
)
{
   if ( NULL == task || NULL == task_func )
      return EINVAL;

#ifndef NDEBUG
   memset( task->nametag, 0, sizeof(task->nametag) );
   if ( nametag )
      strncpy( task->nametag, nametag, sizeof(task->nametag) );
#endif

   task->task_func     = task_func;
   task->param1        = param1;
   task->param2        = param2;
   task->stack_size    = stack_size;
   task->priority      = priority;
   task->running       = FALSE;

   // Might be better to call _beginthreadex

   task->thread_handle = CreateThread(
                              NULL,              // security
                              stack_size,        // stack size (0 gives default)
                              task_starter,      // Entry point
                              task,            // Parameter for entry point
                              CREATE_SUSPENDED,  // Don't start immediately
                              &(task->thread_id)); // Thread identifier for debug

   if ( task->thread_handle == NULL )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Reset a task. Stops the task first if it is currently running.

   Returns the task to its initial state (as after task_create).

   @param task          Address of task to reset.

   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_task_reset)( osal_task_t * task )
{
   return EFAULT;  // Not implemented
}

/*****************************************************************************/
/**
   Destroy a task.

   Stops the task if it is currently running.

   @param task          Address of task to delete.

   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_task_destroy)( osal_task_t * task )
{
   int status = 0;
   
   if ( NULL == task )
      return EINVAL;

   /* If we are deleting a still-running thread then we need to kill it
    *  before we delete the task object.
    */
   if ( task->running )
      status = osal_task_stop( task );

   /* Should probably call GetExitCodeThread to check the termination status
    * of the thread, and maybe wait for it to stop
    */

   /* Close the thread handle to reclaim thread resources */
   if ( 0 == status)
      if( CloseHandle( task->thread_handle ) != 0 )
         return EFAULT;

   return 0;
}

/*****************************************************************************/
/**
   Starts or resumes a task.

   @param task          Address of task to start or resume.

   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_task_start)( osal_task_t * task )
{
   if ( NULL == task )
      return EINVAL;

   if ( ResumeThread( task->thread_handle ) == 0 )
      return EFAULT;
      
   task->running = TRUE;
   return 0;
}

/*****************************************************************************/
/**
   Stops a task executing.  May not be later resumed.

   Ideally you should signal a thread to close itself down, and the thread that
   is to be closed should then call this function.

   @param task          Address of task to stop.

   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_task_stop)( osal_task_t * task )
{
   if ( NULL == task )
      return EINVAL;

   /* If we are deleting a still-running thread then we need to kill it
    *  before we delete the task object.
    */
   if ( task->running )
   {
      DWORD exit_code;

      /* Get the termination status of the thread
       */
      if( GetExitCodeThread( task->thread_handle, &exit_code) == 0 )
         return EFAULT;

      /* Tell the thread to stop
       */
      if ( TerminateThread( task->thread_handle, exit_code ) == 0 )
         return EFAULT;

      task->thread_handle = NULL;
      task->running       = FALSE;
   }

   return 0;
}

/*****************************************************************************/
/**
   Sets a task's priority level.

   @param task          Address of task to modify.
   @param priority      New priority level for the task.
   @param old_priority  Address of a variable in which is stored the task's
                         previous priority.  Can be NULL, in which case the
                         previous priority is not returned.

   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_task_set_priority)(
      osal_task_t   * task,
      unsigned int   priority,
      unsigned int * old_priority
)
{
   unsigned int wpri;
   
   if ( NULL == task )
      return EINVAL;

   if ( old_priority )
      *old_priority = task->priority;

   /*
    * Convert an OSAL priority (0=highest, 255=lowest) into a Windows priority
    * (one of seven values).
    */
   switch ( priority / 32 )  // INTERIM
   {
      case 0  : wpri = THREAD_PRIORITY_TIME_CRITICAL; break;  // 00..1f
      case 1  : wpri = THREAD_PRIORITY_ABOVE_NORMAL;  break;  // 10..3f
      case 2  : wpri = THREAD_PRIORITY_HIGHEST;       break;  // 40..5f
      case 3  : wpri = THREAD_PRIORITY_NORMAL;        break;  // 60..7f
      case 4  : wpri = THREAD_PRIORITY_BELOW_NORMAL;  break;  // 80..9f
      case 5  : wpri = THREAD_PRIORITY_LOWEST;        break;  // a0..bf
      default :
      case 6  : wpri = THREAD_PRIORITY_IDLE;          break;  // co..ff
   }

   if ( SetThreadPriority( task->thread_handle, wpri ) == 0 )
      return EFAULT;

   task->priority = priority;
   return 0;
}

/*****************************************************************************/
/**
   Get a task's priority level.

   @param task          Address of task to query.
   @param priority      Address of a variable in which is stored the task's
                         priority.

   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_task_get_priority)(
      osal_task_t    * task,
      unsigned int  * priority
)
{
   if ( NULL == task || NULL == priority )
      return EINVAL;

   /* We could interrogate the thread with GetThreadPriority and convert the
    * result to a 0..255 range, but instead simply return the last value we set.
    */
   *priority = task->priority;
   return 0;
}

/*****************************************************************************/
/**
   Put the current task to sleep for a specified period.
   
   In Windows we have to specify the accuracy of timing, which we do to within
   1ms, while we go to sleep.

   @param delay_ms   Time to sleep, in milliseconds.
**/
extern void (osal_task_sleep)( unsigned int delay_ms )
{
   timeBeginPeriod( 1 );
   Sleep( delay_ms );
   timeEndPeriod( 1 );
}

/*****************************************************************************
                                  E N D
 *****************************************************************************/
