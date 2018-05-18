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

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/*****************************************************************************/
/**
   Initialise a queue for holding messages.
**/
extern int (osal_queue_init)(
      osal_queue_t   * queue,
      unsigned int     length,
      size_t           size,
      const char     * nametag
)
{
   int status;
   
   if ( NULL == queue || 0 == length || 0 == size )
      return EINVAL;

#ifndef NDEBUG
   memset( queue->nametag, 0, sizeof(queue->nametag) );
   if ( nametag )
      strncpy( queue->nametag, nametag, sizeof(queue->nametag) );
#endif

   /* Create the queue store */
   queue->msg_size = size;
   queue->len      = length;
   queue->qmem     = calloc( length, size );
   queue->head     = 0;
   queue->tail     = 0;
   
   status  = osal_sem_init( &(queue->sem_get), 0,      "q:getsem" );
   status |= osal_sem_init( &(queue->sem_put), length, "q:putsem" );
   status |= osal_mutex_init( &(queue->mtx), "qmemmtx" );
   
   if ( NULL == queue->qmem || status )
   {
      osal_queue_destroy( queue );
      
      return EFAULT;
   }
   
   return 0;
}

/*****************************************************************************/
/**
   Destroy a queue.
**/
extern int (osal_queue_destroy)( osal_queue_t * queue )
{
   if ( NULL == queue )
      return EINVAL;
      
   osal_mutex_destroy( &(queue->mtx) );
   osal_sem_destroy( &(queue->sem_put) );
   osal_sem_destroy( &(queue->sem_get) );
   free( queue->qmem );

   return 0;
}

/*****************************************************************************/
/**
   Send a message into a queue.
**/
extern int (osal_queue_send_to)( 
      osal_queue_t * queue,
      const void   * message,
      int            suspend
)
{
   int status;
   
   if ( NULL == queue || NULL == message )
      return EINVAL;
      
   status = osal_sem_obtain( &(queue->sem_put), suspend );
   if ( 0 == status )
   {
      /* There is space for this message, so grab the mutex and push */
      status = osal_mutex_obtain( &(queue->mtx), suspend );
      if ( 0 == status )
      {
	 /* Put the message body into the queue memory. */ 
	 memcpy( &(queue->qmem[queue->head * queue->msg_size]), message, queue->msg_size );
	 
	 /* Bump and wrap */
	 queue->head++;
	 if ( queue->head >= queue->len )
	    queue->head = 0;
	 
	 /* All done noodling with shared data */
	 osal_mutex_release( &(queue->mtx) );
	    
	 /* Signal to getters there is a message in the queue */
	 osal_sem_release( &(queue->sem_get) );
      }
      else
      {
         /* Failed to get the mutex, so put back the message count as we have
	  *  not actually put a message into the queue.
	  */
	 osal_sem_release( &(queue->sem_put) );
      }
   }
   
   return status;   
}

/*****************************************************************************/
/**
   Receive a message from a queue.
**/
extern int (osal_queue_recv_from)( 
      osal_queue_t * queue,
      void         * message,
      int            suspend
)
{
   int status;
   
   /* Check input arguments */
   if ( NULL == queue || NULL == message )
      return EINVAL;
      
   /* Wait here for a message to arrive in the queue */
   status = osal_sem_obtain( &(queue->sem_get), suspend );
   if ( 0 == status )
   {
      /* There is at least one message in the queue, so grab the queue mutex */
      status = osal_mutex_obtain( &(queue->mtx), suspend );
      if ( 0 == status )
      {
         memcpy( message, &(queue->qmem[queue->tail * queue->msg_size]), queue->msg_size );
	 
	 /* Bump and wrap */
         queue->tail++;
	 if ( queue->tail >= queue->len )
	    queue->tail = 0;
         
	 /* All done noodling with shared data */
	 osal_mutex_release( &(queue->mtx) );
	 
	 /* Signal to putters that there is space in the queue */
         osal_sem_release( &(queue->sem_put) );
      }
      else
      {
         /* Failed to get the qmem mutex, so put back the message count 
          *  because we have not actually removed a message from the queue.
          */
         osal_sem_release( &(queue->sem_get) );
      }
   }
   
   return status;
}

/*****************************************************************************/
/** For user-space Linux there is no distinction between normal and interrupt
    modes.  So the _INT functions just bounce to the normal versions ans they
    never suspend.
**/

extern int (osal_queue_send_to_INT)( 
      osal_queue_t * queue,
      const void   * message
)
{
   return osal_queue_send_to( queue, message, OSAL_SUSPEND_NEVER );
}

extern int (osal_queue_recv_from_INT)( 
      osal_queue_t * queue,
      void         * message
)
{
   return osal_queue_recv_from( queue, message, OSAL_SUSPEND_NEVER );
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

