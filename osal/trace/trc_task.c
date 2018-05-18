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

extern int (trace_osal_task_init)(
      osal_task_t     * task, 
      size_t            stack_size,
      void           (* task_func)(osal_task_t *, void *, void *),
      void *            param1,
      void *            param2,
      unsigned int      priority,
      const char *      nametag
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_create );
   TRC_PARAM( task,       "%p" );
   TRC_PARAM( stack_size, "%i" );
   TRC_PARAM( task_func,  "%p" );
   TRC_PARAM( param1,     "%p" );
   TRC_PARAM( param2,     "%p" );
   TRC_PARAM( priority,   "%u" );
   TRC_PARAM( nametag,    "%s" );
   TRC_CLOSE();

   int retval = (osal_task_init)(task, stack_size, task_func, 
                                                     param1, param2, priority, 
                                                     nametag);
   
   TRC_OPEN( osal_task_init );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_task_reset)( osal_task_t * task )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_reset );
   TRC_PARAM( task, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_task_reset)(task);
   
   TRC_OPEN( osal_task_reset );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_task_destroy)( osal_task_t * task )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_destroy );
   TRC_PARAM( task, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_task_destroy)(task);
   
   TRC_OPEN( osal_task_destroy );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_task_start)( osal_task_t * task )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_start );
   TRC_PARAM( task, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_task_start)(task);
   
   TRC_OPEN( osal_task_start );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_task_stop)( osal_task_t * task )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_stop );
   TRC_PARAM( task, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_task_stop)(task);
   
   TRC_OPEN( osal_task_stop );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_task_set_priority)(
      osal_task_t       *  task,
      unsigned int      priority,
      unsigned int *    old_priority
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_set_priority );
   TRC_PARAM( task, "%p" );
   TRC_PARAM( priority, "%u" );
   TRC_PARAM( old_priority, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_task_set_priority)(task,priority,old_priority);
   
   TRC_OPEN( osal_task_set_priority );
   TRC_RET_STATUS( retval );
   if ( 0 == retval && old_priority )
      TRC_RET_VAL( *old_priority, "%u" );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_task_get_priority)(
      osal_task_t    *     task,
      unsigned int *    priority
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_get_priority );
   TRC_PARAM( task, "%p" );
   TRC_PARAM( priority, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_task_get_priority)(task,priority);
   
   TRC_OPEN( osal_task_get_priority );
   TRC_RET_STATUS( retval );
   if ( 0 == retval )
      TRC_RET_VAL( *priority, "%u" );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern void (trace_osal_task_sleep)( unsigned int delay_ms )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_task_sleep );
   TRC_PARAM( delay_ms, "%u" );
   TRC_CLOSE();
   
   (osal_task_sleep)(delay_ms);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
