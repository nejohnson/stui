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
   Private data.  Declare as static.
 *****************************************************************************/

static HANDLE hTimerQueue = NULL;

/****************************************************************************
   Private Functions.  Declare as static.
 ****************************************************************************/
 
static VOID CALLBACK timerCB( PVOID lpParam, BOOLEAN TimerOrWaitFired )
{
    osal_timer_t * tmr;

    if ( lpParam )
    {
        tmr = (osal_timer_t *)lpParam;
    
        if ( tmr && tmr->handler && tmr->is_active )
            (tmr->handler)(tmr, tmr->arg);
    }
}
 
/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/
 
/*****************************************************************************/
/**
   Initialise timer module.
   
   @retval 0       if successful.
   @retval EFAULT   otherwise.
**/
extern int osal_init_timer(void)
{
    /* Create a timer queue if it has not already been created. */    
    if ( NULL == hTimerQueue )
    {
        hTimerQueue = CreateTimerQueue();
        if ( NULL == hTimerQueue )
            return EFAULT;
    }  

    return 0;
} 

/*****************************************************************************/
/**
   Shutdown timer module.
   
   @retval 0       if successful.
   @retval EFAULT   otherwise.
**/
extern int osal_shdn_timer(void)
{
    if ( NULL == hTimerQueue || !DeleteTimerQueueEx( hTimerQueue, INVALID_HANDLE_VALUE ) )
        return EFAULT;

    return 0;
} 

/*****************************************************************************/
/**
   Initialise a timer.
   
   @param timer      Address of a timer to initialise
   @param handler    Address of a timer handler function
   @param arg        Argument passed to the timer handler function when called.
   @param nametag    Address of a string constant, used in debug only.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_timer_init)( 
      osal_timer_t * timer,
      void        (* handler)(osal_timer_t *, void *),
      void         * arg,
      const char   * nametag
)
{
    if ( NULL == timer || NULL == handler )
        return EINVAL;
        
#ifndef NDEBUG
   memset( timer->nametag, 0, sizeof(timer->nametag) );
   if ( nametag )
      strncpy( timer->nametag, nametag, sizeof(timer->nametag) );
#endif        

    timer->handler   = handler;
    timer->arg       = arg;
    timer->is_active = 0;

    return 0;
}

/*****************************************************************************/
/**
   Destroy a timer, stopping it if is currently running.
   
   @param timer      Address of timer to delete.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_timer_destroy)( osal_timer_t * timer )
{
    if ( NULL == timer )
        return EINVAL;
        
    timer->is_active = 0;
    if ( !DeleteTimerQueueTimer( hTimerQueue, timer->hTimer, INVALID_HANDLE_VALUE ) )
       return EFAULT;
       
    return 0;
}

/*****************************************************************************/
/**
   Starts a timer to run for a specified delay period.
   
   This function may be used by a timer handler to restart itself, possibly
   with a different duration than previously set.
   
   @param timer      Address of timer to start.
   @param delay_ms   Timeout period, in milliseconds.   
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_timer_start)( 
      osal_timer_t  * timer,
      unsigned int    delay_ms
)
{
    if ( NULL == timer )
        return EINVAL;

    timer->is_active = 1;
    if ( !CreateTimerQueueTimer( &(timer->hTimer), hTimerQueue, 
                                (WAITORTIMERCALLBACK)timerCB, (PVOID *)timer, 
                                delay_ms, 0, 0 ) )
        return EFAULT;

    return 0;
}

/*****************************************************************************/
/**
   Stops a timer.
   
   Note that the timer handler function is not triggered by this function, even
   though the timer is forced to expire.
   
   @param timer      Address of timer to stop.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
extern int (osal_timer_stop)( osal_timer_t * timer )
{
    return osal_timer_destroy( timer );
}

/*****************************************************************************/
/**
   Return the current system time.
   
   Note that the accuracy of the returned value is OS-specific.
   The value in microseconds will wrap every 71 minutes or so, and so is only
   recommended for use in timing or measuring over shortish periods.
   
   @param secs          Address of a variable in which to store the current 
                         system time in seconds.  May be NULL if this value is 
                         not needed.
   @param microsecs     Address of a variable in which to store the current 
                         system time in microseconds.  May be NULL if this 
                         value is not needed.
**/
extern void (osal_get_systime)( unsigned int *secs, unsigned int *microsecs )
{
   LARGE_INTEGER counter;    /* Elapsed time counter       */
   LARGE_INTEGER frequency;  /* Frequency of counter in Hz */
   
   if(   QueryPerformanceFrequency(&frequency)
      && QueryPerformanceCounter(&counter)
      && (frequency.QuadPart > 0LL))
   {
      if ( microsecs )
         *microsecs = (unsigned int)( (1000000 * counter.QuadPart) 
                                                    / 
                                           frequency.QuadPart );
         
      if ( secs )
         *secs = (unsigned int)(counter.QuadPart / frequency.QuadPart);
   }
   else
   {
      if ( microsecs ) *microsecs = 0;
      if (      secs ) *secs      = 0;
   }
}

/*****************************************************************************
                                  E N D
 *****************************************************************************/
