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
 
/*****************************************************************************
   Project Includes
 *****************************************************************************/

#undef OSAL_TRACE   /* Make sure we have the non-trace defs */
#include "osal.h"

/*****************************************************************************
   Private typedefs, macros and constants.
 *****************************************************************************/

/**
   General timeout when waiting on the lock, long enough to be FOREVER in
    normal operation.
**/
#define LOCK_TIMEOUT              OSAL_SUSPEND_FOREVER

/**
   Time to allow pending tasks to finish up while deleting an event group.
**/
#define EV_DELETE_SLEEP_TIME_MS   ( 10 )

/**
   Data type for the task waiting list for events.
**/
typedef struct ew {
   struct ew *            next;
   unsigned int           event_mask;
   OSAL_EVENT_MODE        mode;
   osal_sem_t             sem;
} EVENT_WAIT;

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/*****************************************************************************/
/**
   Initialise an event group.
**/
extern int (osal_event_init)(
      osal_event_t   * event, 
      unsigned int     num_events,
      const char *     nametag )
{
   if ( 0 == num_events )
      return EINVAL;
      
   if ( NULL == event )
      return EINVAL;
      
#ifndef NDEBUG
   memset( event->nametag, 0, sizeof(event->nametag) );
   if ( nametag )
      strncpy( event->nametag, nametag, sizeof(event->nametag) );
#endif

   event->num_events = num_events;
   event->ev         = 0;
   event->evlist     = NULL;
   
   if ( osal_mutex_init( &(event->lock), "evlock" ) != 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Destroy an event group.
*/
extern int (osal_event_destroy)( osal_event_t * event )
{
   if ( NULL == event )
      return EINVAL;
      
   /* Set all events */
   osal_event_set( event, ( 1 << event->num_events ) - 1 );
   
   /* Wait for event list to empty. */
   do
   {
      void *v;
      
      osal_mutex_obtain( &(event->lock), LOCK_TIMEOUT );
      v = event->evlist;
      osal_mutex_release( &(event->lock) );
      
      if ( !v )
         break;
      
      /* Sleep to allow any tasks waiting on the event group to finish */
      osal_task_sleep( EV_DELETE_SLEEP_TIME_MS );
   } while( 1 );
   
   osal_mutex_destroy( &(event->lock) );
   
   return 0;
}

/*****************************************************************************/
/**
   Set one or more events within a given event group.
**/
extern int (osal_event_set)( 
      osal_event_t  * event, 
      unsigned int    event_mask
)
{
   if ( NULL == event )
      return EINVAL;
      
   if ( ( event_mask & ~( ( 1 << event->num_events ) - 1 ) ) != 0 )
      return EINVAL;
      
   if ( osal_mutex_obtain( &(event->lock), LOCK_TIMEOUT ) == 0 )
   {
      EVENT_WAIT *ew;
      event->ev |= event_mask;
      
      for ( ew = (EVENT_WAIT *)event->evlist; ew; ew = ew->next )
      {
         if ( ( ew->mode == OSAL_EVENT_MODE_AND
                && ( ( event->ev & ew->event_mask ) == ew->event_mask ) )
              ||
              ( ew->mode == OSAL_EVENT_MODE_OR
                && ( ( event->ev & ew->event_mask ) != 0 ) ) )
         {
            osal_sem_release( &(ew->sem) );
         }      
      }
   
      return osal_mutex_release( &(event->lock) );
   }
   else
      return EFAULT;
}

/*****************************************************************************/
/**
   Clear one or more events within a given event group.
**/
extern int (osal_event_clear)( 
      osal_event_t * event, 
      unsigned int   event_mask
)
{
   if ( NULL == event )
      return EINVAL;
      
   if ( ( event_mask & ~( ( 1 << event->num_events ) - 1 ) ) != 0 )
      return EINVAL;
      
   if ( osal_mutex_obtain( &(event->lock), LOCK_TIMEOUT ) == 0 )
   {
      event->ev &= ~event_mask;
      return osal_mutex_release( &(event->lock) );
   }
   else
      return EFAULT;
}

/*****************************************************************************/
/**
   Get the current set of events within a given event group.
**/
extern int (osal_event_get)(
      osal_event_t * event, 
      unsigned int * event_mask
)
{
   if ( NULL == event || NULL == event_mask )
      return EINVAL;
      
   /* We assume that this is an atomic read, so does not need to be within
    *  a lock.
    */
   *event_mask = event->ev;
   
   return 0;
}

/*****************************************************************************/
/**
   Wait for a specific combination of events within a given event group.
**/
extern int (osal_event_wait)( 
      osal_event_t     * event, 
      unsigned int       event_mask, 
      unsigned int     * match_events,
      OSAL_EVENT_MODE    mode,
      int                suspend
)
{
   int status;
   unsigned int retrieved_events = 0;
   
   if ( NULL == event )
      return EINVAL;
      
   if ( ( event_mask & ~( ( 1 << event->num_events ) - 1 ) ) != 0 )
      return EINVAL;
      
   /* Access event */
   if ( osal_mutex_obtain( &(event->lock), LOCK_TIMEOUT ) != 0 )
      return EFAULT;
   
   retrieved_events = event->ev & event_mask;
   
   osal_mutex_release( &(event->lock) );
   
   if ( ( mode == OSAL_EVENT_MODE_AND
          && ( retrieved_events == event_mask ) )
        ||
        ( mode == OSAL_EVENT_MODE_OR
          && ( retrieved_events != 0 ) ) )
   {
      /* Immediate success*/     
      status = 0;
   } 
   else if ( OSAL_SUSPEND_NEVER == suspend )
   {
      /* Immediate failure */
      status = ETIMEDOUT;
   }
   else
   {   
      /* Add new event wait to list */
      EVENT_WAIT * ew = malloc( sizeof *ew );
      if ( NULL == ew )
         return EFAULT;
      
      ew->event_mask = event_mask;
      ew->mode       = mode;
   
      if ( osal_sem_init( &(ew->sem), 0, "ewsem" ) )
      {
         free( ew );
         return EFAULT;
      }
      
      /* Add event wait object to the event object */
      if ( osal_mutex_obtain( &(event->lock), LOCK_TIMEOUT ) != 0 )
      {
         osal_sem_destroy( &(ew->sem) );
	 free( ew );
	 return EFAULT;
      }
      
      ew->next      = event->evlist;
      event->evlist = ew;
      
      osal_mutex_release( &(event->lock) );
      
      /* Now wait for our semaphore to be raised */
      status = osal_sem_obtain( &(ew->sem), suspend );
      
      /*
       *
       *
       *   Time passes.....
       *
       *
       */
       
      if ( osal_mutex_obtain( &(event->lock), LOCK_TIMEOUT ) == 0 )
      {         
	 EVENT_WAIT **p;

         /* Remove our ew from the event wait list */
         for ( p = (EVENT_WAIT **)&event->evlist; *p; p = &(*p)->next )
         {
            if ( *p == ew )
            {
               *p = (*p)->next;
               break;
            }
         }   
         retrieved_events = event->ev & ew->event_mask;
         osal_mutex_release( &(event->lock) );
         osal_sem_destroy( &(ew->sem) );
         free( ew );            
      }
      else
         return EFAULT;
   }

   if ( 0 == status && match_events )
      *match_events = retrieved_events;
   
   return status;
}

/*****************************************************************************
                                  E N D
 *****************************************************************************/
