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

#undef OSAL_TRACE
#include "osal.h"
#include "trc_defs.h"

/*****************************************************************************/

extern int (trace_osal_sem_init)(
      osal_sem_t   *   sem,
      unsigned int     initial_count,
      const char *     nametag
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_sem_init );
   TRC_PARAM( sem, "%p" );
   TRC_PARAM( initial_count, "%u" );
   TRC_PARAM( nametag, "%s" );
   TRC_CLOSE();
   
   int retval = (osal_sem_init)(sem, initial_count, nametag);
   
   TRC_OPEN( osal_sem_create );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_sem_destroy)( osal_sem_t * semaphore )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_sem_destroy );
   TRC_PARAM( semaphore, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_sem_destroy)(semaphore);
   
   TRC_OPEN( osal_sem_destroy );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_sem_obtain)( 
      osal_sem_t *  semaphore,
      OSAL_SUSPEND     suspend
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_sem_obtain );
   TRC_PARAM( semaphore, "%p" );
   TRC_PARAM_SUS( suspend );
   TRC_CLOSE();
   
   int retval = (osal_sem_obtain)(semaphore, suspend);
   
   TRC_OPEN( osal_sem_obtain );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_sem_release)( osal_sem_t * semaphore )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_sem_release );
   TRC_PARAM( semaphore, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_sem_release)(semaphore);
   
   TRC_OPEN( osal_sem_release );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
