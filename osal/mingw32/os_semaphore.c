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
   
   @param semaphore    Address of semaphore to initialise
   @param count        Specifies the initial count value of the semaphore.
   @param nametag      Address of a string constant, used in debug only.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_sem_init)(
      osal_sem_t    * semaphore,
      unsigned int    count,
      const char    * nametag
)
{
   if ( NULL == semaphore )
      return EINVAL;
      
#ifndef NDEBUG
   memset( semaphore->nametag, 0, sizeof(semaphore->nametag) );
   if ( nametag )
      strncpy( semaphore->nametag, nametag, sizeof(semaphore->nametag) );
#endif

   semaphore->sem = CreateSemaphore( NULL, count, LONG_MAX, NULL );

   if ( NULL == semaphore->sem )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Destroy a sempahore.
   
   @param sempahore    Address of semaphore to delete.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_sem_destroy)( osal_sem_t * semaphore )
{
   if ( NULL == semaphore )
      return EINVAL;

   if ( CloseHandle( semaphore->sem ) == 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Obtain a semaphore, suspending (if required) if the semaphore is not free.
   
   @param sempahore    Address of semaphore to obtain.
   @param suspend      Specifies suspend mode, either OSAL_SUSPEND_FOREVER,
                        OSAL_SUSPEND_NEVER, or a timeout period given in
                        milliseconds.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval ETIMEDOUT   if suspend timed out, or count not available.
   @retval EFAULT      otherwise.
**/
extern int (osal_sem_obtain)( 
      osal_sem_t     * semaphore,
      OSAL_SUSPEND   suspend
)
{
   DWORD wait_time;

   if ( NULL == semaphore )
      return EINVAL;

   switch ( suspend )
   {
      case OSAL_SUSPEND_FOREVER: wait_time = INFINITE; break;
      case OSAL_SUSPEND_NEVER:   wait_time = 0;        break;
      default :                  wait_time = suspend;  break;
   }

   switch( WaitForSingleObject( semaphore->sem, wait_time ) )
   {
      case WAIT_OBJECT_0:  return 0;         // Obtained semaphore
      case WAIT_TIMEOUT:   return ETIMEDOUT; // Timed out
      default :
      case WAIT_ABANDONED: return EFAULT;     // Semaphore destroyed
   }
}

/*****************************************************************************/
/**
   Release a semaphore.
   
   @param sempahore    Address of semaphore to release.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_sem_release)( osal_sem_t * semaphore )
{
   if ( NULL == semaphore )
      return EINVAL;

   if ( ReleaseSemaphore( semaphore->sem, 1, NULL ) == 0 )
      return EFAULT;

   return 0;
}

/*****************************************************************************
                                  E N D
 *****************************************************************************/
