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
#include <stdlib.h>     /* for malloc/free */
#include <signal.h>
#include <sys/time.h>
 
/*****************************************************************************
   Project Includes
 *****************************************************************************/

#undef OSAL_TRACE   /* Make sure we have the non-trace defs */
#include "osal.h"

/*****************************************************************************
   Private Functions.  Declare as static.
 *****************************************************************************/

/*****************************************************************************/
/**
   Timer notification proxy.
   
   @param value      Parameter containing a pointer to a timer control block.
**/
static void notify_proxy( union sigval value )
{
   osal_timer_t * timer = value.sival_ptr;
   
   if ( timer && timer->handler )
      (timer->handler)( timer, timer->arg );   
}

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/*****************************************************************************/
/**
   Initialise a timer.
*/
extern int (osal_timer_init)( 
      osal_timer_t * timer,
      void        (* handler)(osal_timer_t *, void *),
      void         * arg,
      const char   * nametag
)
{
   struct sigevent ev;
   
   if ( NULL == timer )
      return EINVAL;
      
#ifndef NDEBUG
   memset( timer->nametag, 0, sizeof(timer->nametag) );
   if ( nametag )
      strncpy( timer->nametag, nametag, sizeof(timer->nametag) );
#endif

   timer->handler = handler;
   timer->arg     = arg;
   
   /* Configure the notification handler for this timer */
   ev.sigev_notify            = SIGEV_THREAD;
   ev.sigev_value.sival_ptr   = timer;
   ev.sigev_notify_function   = notify_proxy;
   ev.sigev_notify_attributes = NULL;
   
   if ( timer_create( CLOCK_REALTIME, &ev , &(timer->timerid) ) != 0 )
   {
      return EFAULT;
   }
   
   return 0;
}

/*****************************************************************************/
/**
   Destroy a timer, stopping it if is currently running.
**/
extern int (osal_timer_destroy)( osal_timer_t * timer )
{
   if ( NULL == timer )
      return EINVAL;
   
   if ( timer_delete( timer->timerid ) == 0 )
      return 0;
   else
      return EFAULT;
}

/*****************************************************************************/
/**
   Starts a timer to run for a specified delay period.
**/
extern int (osal_timer_start)( 
      osal_timer_t   * timer,
      unsigned int     delay_ms
)
{
   struct itimerspec ivl;
   
   if ( NULL == timer || 0 == delay_ms )
      return EINVAL;
      
   ivl.it_interval.tv_sec  = 0;
   ivl.it_interval.tv_nsec = 0;
   ivl.it_value.tv_sec     = delay_ms / 1000;
   ivl.it_value.tv_nsec    = ( delay_ms % 1000 ) * 1000000;
      
   if ( timer_settime( timer->timerid, 0, &ivl, NULL ) == 0 )
      return 0;
   else
      return EFAULT;
}

/*****************************************************************************/
/**
   Stops a timer.
**/
extern int (osal_timer_stop)( osal_timer_t * timer )
{
   struct itimerspec ivl;
   
   if ( NULL == timer )
      return EINVAL;
   
   /* Setting an interval time of 0 stops the timer */
   ivl.it_value.tv_sec  = 0;
   ivl.it_value.tv_nsec = 0;
   
   if ( timer_settime( timer->timerid, 0, &ivl, NULL ) == 0 )
      return 0;
   else
      return EFAULT;
}

/*****************************************************************************/
/**
   Return the current system time.
**/
extern void (osal_get_systime)( unsigned int *secs, unsigned int *microsecs )
{
   struct timeval tp;
   
   if ( gettimeofday( &tp, NULL ) == -1 )
      return;
   
   if ( secs )
      *secs = tp.tv_sec;
      
   if ( microsecs )
      *microsecs = tp.tv_sec * 1000000 + tp.tv_usec;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
