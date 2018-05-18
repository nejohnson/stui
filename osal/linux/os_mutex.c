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
   Initialise a mutex.
**/
extern int (osal_mutex_init)(
      osal_mutex_t * mutex,
      const char   * nametag
)
{
   int status = 0;
   pthread_mutexattr_t attr;
   
   if ( NULL == mutex )
      return EINVAL;
      
#ifndef NDEBUG
   memset( mutex->nametag, 0, sizeof(mutex->nametag) );
   if ( nametag )
      strncpy( mutex->nametag, nametag, sizeof(mutex->nametag) );
#endif

   pthread_mutexattr_init( &attr );
   pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
   
   if ( pthread_mutex_init( &(mutex->mtx), &attr ) != 0 )
   	status = EFAULT;

   pthread_mutexattr_destroy( &attr );
   
   return status;
}

/*****************************************************************************/
/**
   Destroy a mutex.
**/
extern int (osal_mutex_destroy)( osal_mutex_t * mutex )
{
   if ( NULL == mutex )
      return EINVAL;
      
   if ( pthread_mutex_destroy( &(mutex->mtx) ) != 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Obtain a mutex, suspending (if required) if the mutex is not free.
**/
int (osal_mutex_obtain)( 
      osal_mutex_t  * mutex,
      OSAL_SUSPEND    suspend
)
{
   if ( NULL == mutex )
      return EINVAL;
      
   if ( OSAL_SUSPEND_FOREVER == suspend )
   {
      if ( pthread_mutex_lock( &( mutex->mtx ) ) == 0 )
          return 0;
      else
          return EFAULT; 
   }
   else if ( OSAL_SUSPEND_NEVER == suspend )
   {
      if ( pthread_mutex_trylock( &( mutex->mtx ) ) == 0 )
          return 0;
      else if ( EBUSY == errno )
          return ETIMEDOUT;
      else
          return EFAULT;
   }
   else
   {
      do
      {
          int err = pthread_mutex_trylock( &( mutex->mtx ) );
	  	  
          if ( 0 == err )
              return 0;
	  else if ( EBUSY == err )
          {
	      osal_task_sleep( MIN( 10, suspend ) );	
	      suspend -= MIN( 10, suspend );      
	  }
	  else
              return EFAULT;
      } while ( suspend );
      
      return ETIMEDOUT;
   }	
}

/*****************************************************************************/
/**
   Release a mutex.
**/
int (osal_mutex_release)( osal_mutex_t * mutex )
{
   if ( NULL == mutex )
      return EINVAL;
   
   if ( pthread_mutex_unlock( &(mutex->mtx) ) != 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
