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
   
   @param mutex        Address of mutex to initialise
   @param nametag      Address of a string constant, used in debug only.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_mutex_init)(
      osal_mutex_t  * mutex,
      const char    * nametag
)
{
   if ( NULL == mutex )
      return EINVAL;
      
#ifndef NDEBUG
   memset( mutex->nametag, 0, sizeof(mutex->nametag) );
   if ( nametag )
      strncpy( mutex->nametag, nametag, sizeof(mutex->nametag) );
#endif

   mutex->mtx = CreateMutex( NULL, FALSE, NULL );

   if ( NULL == mutex->mtx )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Destroy a sempahore.
   
   @param sempahore    Address of mutex to delete.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_mutex_destroy)( osal_mutex_t * mutex )
{
   if ( NULL == mutex )
      return EINVAL;

   if ( CloseHandle( mutex->mtx ) == 0 )
      return EFAULT;
      
   return 0;
}

/*****************************************************************************/
/**
   Obtain a mutex, suspending (if required) if the mutex is not free.
   
   @param sempahore    Address of mutex to obtain.
   @param suspend      Specifies suspend mode, either OSAL_SUSPEND_FOREVER,
                        OSAL_SUSPEND_NEVER, or a timeout period given in
                        milliseconds.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval ETIMEDOUT   if suspend timed out, or count not available.
   @retval EFAULT      otherwise.
**/
extern int (osal_mutex_obtain)( 
      osal_mutex_t     * mutex,
      OSAL_SUSPEND     suspend
)
{
   DWORD wait_time;

   if ( NULL == mutex )
      return EINVAL;

   switch ( suspend )
   {
      case OSAL_SUSPEND_FOREVER: wait_time = INFINITE; break;
      case OSAL_SUSPEND_NEVER:   wait_time = 0;        break;
      default :                  wait_time = suspend;  break;
   }

   switch( WaitForSingleObject( mutex->mtx, wait_time ) )
   {
      case WAIT_OBJECT_0:  return 0;         // Obtained mutex
      case WAIT_TIMEOUT:   return ETIMEDOUT; // Timed out
      default :
      case WAIT_ABANDONED: return EFAULT;     // Semaphore destroyed
   }
}

/*****************************************************************************/
/**
   Release a mutex.
   
   @param sempahore    Address of mutex to release.
   
   @retval 0           if successful.
   @retval EINVAL      if one or more parameters are incorrect.
   @retval EFAULT      otherwise.
**/
extern int (osal_mutex_release)( osal_mutex_t * mutex )
{
   if ( NULL == mutex )
      return EINVAL;

   if ( ReleaseMutex( mutex->mtx ) == 0 )
      return EFAULT;

   return 0;
}

/*****************************************************************************
                                  E N D
 *****************************************************************************/
