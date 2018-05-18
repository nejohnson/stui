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

extern int (trace_osal_event_init)(
      osal_event_t *   event, 
      unsigned int     num_events,
      const char *     nametag
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_event_create );
   TRC_PARAM( event, "%p" );
   TRC_PARAM( num_events, "%u" );
   TRC_PARAM( nametag, "%s" );
   TRC_CLOSE();
   
   int retval = (osal_event_init)(event,num_events,nametag);
   
   TRC_OPEN( osal_event_create );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_event_destroy)( osal_event_t * event )
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_event_destroy );
   TRC_PARAM( event, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_event_destroy)(event);
   
   TRC_OPEN( osal_event_destroy );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_event_set)( 
      osal_event_t    *    event, 
      unsigned int      event_mask
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_event_set );
   TRC_PARAM( event, "%p" );
   TRC_PARAM( event_mask, "%u" );
   TRC_CLOSE();
   
   int retval = (osal_event_set)(event,event_mask);
   
   TRC_OPEN( osal_event_set );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_event_clear)( 
      osal_event_t    *    event, 
      unsigned int      event_mask
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_event_clear );
   TRC_PARAM( event, "%p" );
   TRC_PARAM( event_mask, "%u" );
   TRC_CLOSE();
   
   int retval = (osal_event_clear)(event,event_mask);
   
   TRC_OPEN( osal_event_clear );
   TRC_RET_STATUS( retval );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_event_get)(
      osal_event_t   *     event, 
      unsigned int *    event_mask
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_event_get );
   TRC_PARAM( event, "%p" );
   TRC_PARAM( event_mask, "%p" );
   TRC_CLOSE();
   
   int retval = (osal_event_get)(event,event_mask);
   
   TRC_OPEN( osal_event_get );
   TRC_RET_STATUS( retval );
   if ( 0 == retval )
      TRC_RET_VAL( *event_mask, "%u" );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/

extern int (trace_osal_event_wait)( 
      osal_event_t    *        event, 
      unsigned int          event_mask, 
      unsigned int *        matched_events,
      OSAL_EVENT_MODE     mode,
      int                   suspend
)
{
   unsigned int idx = trace_getidx();
   
   TRC_OPEN( osal_event_wait );
   TRC_PARAM( event, "%p" );
   TRC_PARAM( event_mask, "%u" );
   TRC_PARAM( matched_events, "%p" );
   TRC_PARAM( mode, "%i" );
   TRC_PARAM_SUS( suspend );
   TRC_CLOSE();
   
   int retval = (osal_event_wait)( event, event_mask,
                                   matched_events, mode,
                                   suspend );
   
   TRC_OPEN( osal_event_wait );
   TRC_RET_STATUS( retval );
   if ( 0 == retval && matched_events )
      TRC_RET_VAL( *matched_events, "%u" );
   TRC_CLOSE();
   
   return retval;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
