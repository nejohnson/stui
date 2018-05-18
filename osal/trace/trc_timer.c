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

extern int (trace_osal_timer_init)( 
      osal_timer_t * timer,
      void      (* handler)(osal_timer_t *, void *),
      void *       arg,
      const char * nametag
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_timer_init );
   TRC_PARAM( timer,   "%p" );
   TRC_PARAM( handler, "%p" );
   TRC_PARAM( arg,     "%p" );
   TRC_PARAM( nametag, "%s" );
   TRC_CLOSE();
   
   int retval = (osal_timer_init)(timer, handler, arg, nametag);
   
   TRC_OPEN( osal_timer_init );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_timer_destroy)( osal_timer_t * timer )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_timer_destroy );
   TRC_PARAM( timer, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_timer_destroy)(timer);
   
   TRC_OPEN( osal_timer_destroy );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_timer_start)( 
      osal_timer_t  *     timer,
      unsigned int      delay_ms
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_timer_start );
   TRC_PARAM( timer,    "%p" );
   TRC_PARAM( delay_ms, "%u" );
   TRC_CLOSE();
   
   int retval = (osal_timer_start)(timer, delay_ms);
   
   TRC_OPEN( osal_timer_start );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
      
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_timer_stop)( osal_timer_t * timer )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_timer_stop );
   TRC_PARAM( timer,    "%p" );
   TRC_CLOSE();
      
   int retval = (osal_timer_stop)(timer);
   
   TRC_OPEN( osal_timer_stop );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern void (trace_osal_get_systime)( 
      unsigned int *    secs, 
      unsigned int *    microsecs
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_get_systime );
   TRC_PARAM( secs,      "%p" );
   TRC_PARAM( microsecs, "%p" );
   TRC_CLOSE();
      
   (osal_get_systime)(secs,microsecs);

   TRC_OPEN( osal_get_systime );
   if ( secs )
      TRC_RET_VAL( *secs, "%us" );
   if ( microsecs )
      TRC_RET_VAL( *microsecs, "%uus" );
   TRC_CLOSE();
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
