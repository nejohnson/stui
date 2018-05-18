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

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/*****************************************************************************/
/**
   Initialise a semaphore.
**/
extern int (osal_sem_init)(
      osal_sem_t  * semaphore,
      unsigned int  count,
      const char  * nametag
)
{
   if ( NULL == semaphore )
      return EINVAL;
      
#ifndef NDEBUG
   memset( semaphore->nametag, 0, sizeof(semaphore->nametag) );
   if ( nametag )
      strncpy( semaphore->nametag, nametag, sizeof(semaphore->nametag) );
#endif
                                    
   if ( sem_init( &(semaphore->sem), 0, count ) != 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Destroy a sempahore.
**/
extern int (osal_sem_destroy)( osal_sem_t * semaphore )
{
   if ( NULL == semaphore )
      return EINVAL;
      
   if ( sem_destroy( &(semaphore->sem) ) != 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Obtain a semaphore, suspending (if required) if the semaphore is not free.
**/
extern int (osal_sem_obtain)( 
      osal_sem_t   * semaphore,
      OSAL_SUSPEND   suspend
)
{
   int status;

   if ( NULL == semaphore )
      return EINVAL;
      
   if ( OSAL_SUSPEND_FOREVER == suspend )
   {
      /* We need to watch out for semaphore blocking obtains being kicked
       *  by a signal handler.
       */
      do {
         status = sem_wait( &(semaphore->sem) );
	 pthread_testcancel();
      } while ( -1 == status && EINTR == errno );
      
      if ( 0 == status )
         return 0;
      else
         return EFAULT;
   }
   else if ( OSAL_SUSPEND_NEVER == suspend )
   {
      if ( sem_trywait( &(semaphore->sem) ) == 0 )
         return 0;
      else if ( EAGAIN == errno )
         return ETIMEDOUT;
      else
         return EFAULT;
   }
   else
   {
      struct timespec delay;
      
      if ( clock_gettime( CLOCK_REALTIME, &delay ) == -1 )
         return EFAULT;
      
      delay.tv_sec  += suspend / 1000;
      delay.tv_nsec += ( suspend % 1000 ) * 1000000;
      while ( delay.tv_nsec >= 1000000000L )
      {
         delay.tv_nsec -= 1000000000L;
         delay.tv_sec++;
      }
      
      /* We need to watch out for semaphore blocking obtains being kicked
       *  by a signal handler.
       */
      pthread_testcancel();

      do {
         status = sem_timedwait( &(semaphore->sem), &delay );
         pthread_testcancel(); 
      } while ( -1 == status && EINTR == errno );
      
      if ( 0 == status )
         return 0;
      else if ( ETIMEDOUT == errno )
         return ETIMEDOUT;
      else
         return EFAULT;
   }
}

/*****************************************************************************/
/**
   Release a semaphore.
**/
extern int (osal_sem_release)( osal_sem_t * semaphore )
{
   if ( NULL == semaphore )
      return EINVAL;
   
   if ( sem_post( &(semaphore->sem) ) == 0 )
      return 0;
   else
      return EFAULT;
}

/*****************************************************************************/
/** For user-space Linux there is no distinction between normal and interrupt
    modes.  So the _INT functions just bounce to the normal versions.
**/

extern int (osal_sem_release_INT)( osal_sem_t * semaphore )
{
   return osal_sem_release( semaphore );
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
