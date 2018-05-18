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

#ifndef TRC_DEFS_H
#define TRC_DEFS_H

/*****************************************************************************
   Public manifest constants and macros.
 *****************************************************************************/

/**
   Open a trace entry.
   
   @param func       Name of function to be traced.
**/
#define TRC_OPEN(func)        osal_log_message( OSAL_LOG_CRITICAL,  \
                                      "[TRC-%9.9u] " #func "()", idx )

/**
   Enter a function parameter into the trace.
   
   @param name       Name of parameter.
   @param fmt        Format specifier for parameter (see printf()).
**/
#define TRC_PARAM(name,fmt)   osal_log_message( OSAL_LOG_CRITICAL,  \
                       "[TRC-%9.9u]    " #name " = " fmt , idx, name )

/**
   Interpret and enter a suspend parameter into the trace.
   
   @param sus        Suspend parameter name.
**/
#define TRC_PARAM_SUS(sus)    {if(OSAL_SUSPEND_NEVER==sus)               \
      osal_log_message(OSAL_LOG_CRITICAL, "[TRC-%9.9u]    " #sus " = NEVER", idx ); \
    else if(OSAL_SUSPEND_FOREVER==sus)                                   \
      osal_log_message(OSAL_LOG_CRITICAL, "[TRC-%9.9u]    " #sus " = FOREVER", idx );\
    else TRC_PARAM(sus, "%ums");}
                                                                              
/**
   Enterint a function return value into the trace.
   
   @param name       Name of return value.
   @param fmt        Format specifier for return value (see printf()).
**/
#define TRC_RET_VAL(name,fmt)  osal_log_message( OSAL_LOG_CRITICAL, \
                          "[TRC-%9.9u]    returned " fmt , idx, name )

/**
   Enter a function return status value into the trace.
   
   @param name       Name of return status value.
**/
#define TRC_RET_STATUS(name)   osal_log_message( OSAL_LOG_CRITICAL, \
               "[TRC-%9.9u]    returned %s", idx, trace_strerror(name) )

/**
   Close a trace entry.
**/
#define TRC_CLOSE()

/****************************************************************************
   Public Functions.  Declare as extern.
 ****************************************************************************/

extern const char * trace_strerror( int );
extern unsigned int trace_getidx( void );

/* Tasks */

extern int (trace_osal_task_init)(
      osal_task_t *, size_t, void (*)(osal_task_t *, void *, void *),
      void *, void *, unsigned int, const char * );

extern int (trace_osal_task_reset)( osal_task_t * );
extern int (trace_osal_task_destroy)( osal_task_t * );
extern int (trace_osal_task_start)( osal_task_t * );
extern int (trace_osal_task_stop)( osal_task_t * );
extern int (trace_osal_task_set_priority)(
      osal_task_t *, unsigned int, unsigned int * );

extern int (trace_osal_task_get_priority)( osal_task_t *, unsigned int * );
extern void (trace_osal_task_sleep)( unsigned int );

/* Semaphores */

extern int (trace_osal_sem_init)( osal_sem_t *, unsigned int, const char * );
extern int (trace_osal_sem_destroy)( osal_sem_t * );
extern int (trace_osal_sem_obtain)( osal_sem_t *, OSAL_SUSPEND );
extern int (trace_osal_sem_release)( osal_sem_t * );

/* Timers */

extern int (trace_osal_timer_init)( 
      osal_timer_t *, void (*)(osal_timer_t *, void *),
      void *, const char * );

extern int (trace_osal_timer_destroy)( osal_timer_t * );
extern int (trace_osal_timer_start)( osal_timer_t *, unsigned int );
extern int (trace_osal_timer_stop)( osal_timer_t * );
extern void (trace_osal_get_systime)( unsigned int *, unsigned int * );

/* Events */

extern int (trace_osal_event_init)(
      osal_event_t *, unsigned int, const char * );

extern int (trace_osal_event_destroy)( osal_event_t * );
extern int (trace_osal_event_set)( osal_event_t *, unsigned int );
extern int (trace_osal_event_clear)( osal_event_t *, unsigned int );
extern int (trace_osal_event_get)( osal_event_t *, unsigned int * );
extern int (trace_osal_event_wait)( 
      osal_event_t *, unsigned int, unsigned int *, OSAL_EVENT_MODE, int );

/* Queues */
extern int (trace_osal_queue_init)(
      osal_queue_t *, unsigned int, size_t, const char * );

extern int (trace_osal_queue_destroy)( osal_queue_t * );
extern int (trace_osal_queue_send_to)( osal_queue_t *, const void *, int );
extern int (trace_osal_queue_recv_from)( osal_queue_t *, void *, int );

/* Mutexes */

extern int (trace_osal_mutex_init)( osal_mutex_t *, const char * );
extern int (trace_osal_mutex_destroy)( osal_mutex_t * );
extern int (trace_osal_mutex_obtain)( osal_mutex_t *, OSAL_SUSPEND );
extern int (trace_osal_mutex_release)( osal_mutex_t * );

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
 
#endif /* TRC_DEFS_H */
