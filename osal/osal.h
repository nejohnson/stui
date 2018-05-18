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

#ifndef OSAL_H
#define OSAL_H

/* Pull in standard C libraries required by the OSAL */
#include <stdarg.h>
#include <limits.h>

/*****************************************************************************/
/*                             GENERAL UTILITY MACROS                        */
/*****************************************************************************/
/**
   Return the number of elements in an array
**/
#define NELEMS(n)                      ( (size_t) ( sizeof(n) / sizeof(n[0]) ) )

/**
   Return the minimum of two scalar values @p a and @p b.
**/
#define MIN(a,b)                       ( (a) <= (b) ? (a) : (b) )

/**
   Return the maximum of two scalar values @p a and @p b.
**/
#define MAX(a,b)                       ( (a) >= (b) ? (a) : (b) )

/**
   Return the absolute value of @p n.
**/
#define ABS(n)                         ( (n) < 0 ? -(n) : (n) )

/**
   Length of nametag fields, only for debug.
**/
#define NAMETAG_LENGTH                 ( 16 )

/*****************************************************************************/
/**
   Status codes.  The normal return type is int.  A return value of 0 indicates
   success.  Other values are as listed below.  We use the standard names from
   errno.h, only providing values if they are not already defined.
**/
#if defined(__STDC_HOSTED__)
#  include <errno.h>
#else
#  define EFAULT        ( 1 )
#  define ENOMEM        ( 2 )
#  define ETIMEDOUT     ( 3 )
#  define EINVAL        ( 4 )
#endif

/* Not all hosted platforms support all these error codes.  So check and fill
 *  in any missing ones.
 */
#ifndef ETIMEDOUT
#define ETIMEDOUT       ( 1000 )
#endif

/*****************************************************************************/
/**                 PLATFORM-SPECIFIC DATA TYPES AND MACROS                 **/
/*****************************************************************************/

#ifdef OSAL_HOSTOS_LOCALDEFS
#include OSAL_HOSTOS_LOCALDEFS
#else
#include "os_localdefs.h"
#endif

/*****************************************************************************/
/**                  PLATFORM-NEUTRAL DATA TYPES AND MACROS                 **/
/*****************************************************************************/

/**
   Platform-dependent prefixes and suffixes for function prototypes
**/
#define OSALPRE_           extern
#define OSALPOST_

/**
   Wrap function names in a macro to allow build-time modification.
**/

#ifdef OSAL_TRACE
   #define OSAL(fn)        trace_##fn
#else
   #define OSAL(fn)        fn
#endif

/**
   Funcion prototype wrapper
**/
#define OSAL_FUNCT(type,fn) OSALPRE_ type OSALPOST_ fn

/**
   Function prototype wrapper for functions that can be traced
**/
#define OSAL_FUNCT_TRACEABLE(type,fn) OSAL_FUNCT(type,OSAL(fn))

/*****************************************************************************/
/**
   Suspend modes.
**/
typedef enum {
   OSAL_SUSPEND_FOREVER = -1,
   OSAL_SUSPEND_NEVER,
   OSAL_SUSPEND_MAX_TIMEOUT = INT_MAX 
} OSAL_SUSPEND;

/*****************************************************************************/
/**
   Matching modes for event flags.
**/
typedef enum {
   OSAL_EVENT_MODE_AND,       /**< All flags must match                **/
   OSAL_EVENT_MODE_OR,        /**< Any flag (at least one) may match   **/
   
   /* Alternate names */
   OSAL_EVENT_MODE_ALL = OSAL_EVENT_MODE_AND,
   OSAL_EVENT_MODE_ANY = OSAL_EVENT_MODE_OR
} OSAL_EVENT_MODE;

/*****************************************************************************/
/**
   Logging levels.
**/
#define OSAL_LOG_CRITICAL      ( 0 )
#define OSAL_LOG_IMPORTANT     ( 1 )
#define OSAL_LOG_INFORMATIVE   ( 2 )

/*****************************************************************************/
/**                               INITIALIZATION                            **/
/*****************************************************************************/

/*****************************************************************************/
/**
   Initialize the OS layer and start the application.
   
   Under normal circumstances this call does not return.  If it does then this
   should be treated as a fatal error and the system must deal with it in a safe
   manner.
   
   The arguments to osal_start specify the application's main task, with 
   arguments compatible with hosted C environments.
   
   @param app_main      Name of application main task.
   @param argc          Number of runtime args to main task.
   @param argv          Pointer to array of args to main task.

   @retval 0            if successful,
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT(int, osal_start)( 
      int           (*app_main)(int, const char **), 
      int             argc, 
      const char **   argv );

/*****************************************************************************/
/**
   Shut down the OS layer, including releasing any resources.
   
   @retval 0            if successful,
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT(int, osal_shutdown)( void );

/*****************************************************************************/
/**                                  TIMERS                                 **/
/*****************************************************************************/

/* Simple one-shot timers are provided.  Repetitive timers can be easily
   implemented be resetting and starting the same timer, with the option
   of changing the timeout period.
   
   The delay time is guaranteed to be at least the required time.  It may be
   longer, depending on the operating environment and on the processor load.
   
   When a timer fires it calls a timer handler function.  This takes the timer
   object which triggered it and a user-supplied argument.  Handler functions
   must not block under any circumstance.  If blocking function is required
   then this should be isolated from the timer handler with, for example, a 
   non-blocking call to a message queue or semaphore.
*/

/*****************************************************************************/
/**
   Initialise a timer.
   
   @param tmr           Address of timer to initialise.
   @param handler       Address of a timer handler function.
   @param arg           Argument passed to the timer function when called.
   @param nametag       Address of a string constant, used in debug only.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_timer_init)( 
      osal_timer_t  * tmr,
      void         (* handler)(osal_timer_t *, void *),
      void *          arg,
      const char    * nametag
);

/*****************************************************************************/
/**
   Delete a timer, stopping it if is currently running.
   
   @param timer         Address of timer object to destroy.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_timer_destroy)( osal_timer_t * timer );

/*****************************************************************************/
/**
   Starts a timer to run for a specified delay period.
   
   This function may be used by a timer handler to restart itself, possibly
   with a different duration.
   
   @param timer         Address of timer to start.
   @param delay_ms      Timeout period, in milliseconds.   
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_timer_start)( 
      osal_timer_t  * timer,
      unsigned int    delay_ms
);

/*****************************************************************************/
/**
   Stops a timer.
   
   Note that the timer handler function is not triggered by this function, even
   though the timer is forced to expire.
   
   @param timer         Address of timer to stop.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_timer_stop)( osal_timer_t * timer );

/*****************************************************************************/
/**
   Return the current system time.
   
   That the accuracy of the returned value is OS-specific.
   The value in microseconds will wrap every 71 minutes or so, and so is only
   recommended for use in timing or measuring over shortish periods.
   
   @param secs          Address of a variable in which to store the current 
                         system time in seconds.  May be NULL if this value is 
                         not needed.
   @param microsecs     Address of a variable in which to store the current 
                         system time in microseconds.  May be NULL if this 
                         value is not needed.
**/
OSAL_FUNCT_TRACEABLE(void, osal_get_systime)( 
      unsigned int * secs, 
      unsigned int * microsecs
);

/*****************************************************************************/
/**                                SEMAPHORES                               **/
/*****************************************************************************/

/* OSAL supprots counting semaphores.  The initial count can be specified, or
   left to calls to release() to bump up the count.
   
   Tasks can suspend on a semaphore, either for a specified timeout, or 
   forever.  A call to osal_sem_obtain() returns ETIMEDOUT if the semaphore 
   count is 0 by the end of the timeout period.
   
   Multiple tasks can suspend on a semaphore.  The order in which the tasks are
   resumed when the semaphore becomes available is implementation-defined.
*/

/*****************************************************************************/
/**
   Initialise a semaphore.
   
   @param sem           Address of semaphore to initialise.
   @param count         Specifies the initial count value of the semaphore.
   @param nametag       Address of a string constant, used in debug only.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_sem_init)(
      osal_sem_t *     sem,
      unsigned int     count,
      const char *     nametag
);

/*****************************************************************************/
/**
   Destroy a semaphore.
   
   @param semaphore     Address of semaphore to destroy.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_sem_destroy)( 
      osal_sem_t *  sem
);

/*****************************************************************************/
/**
   Obtain a semaphore, suspending (if required) if the semaphore is not free.
   
   @param semaphore     Address of semaphore to obtain.
   @param suspend       Specifies suspend mode, either OSAL_SUSPEND_FOREVER,
                         OSAL_SUSPEND_NEVER, or a timeout period given in
                         milliseconds.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval ETIMEDOUT    if suspend timed out, or count not available.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_sem_obtain)( 
      osal_sem_t   * sem,
      OSAL_SUSPEND   suspend
);

/*****************************************************************************/
/**
   Release a semaphore.
   
   @param semaphore     Address of semaphore to release.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_sem_release)( 
      osal_sem_t   * sem
);

/*****************************************************************************/
/**
   Release a semaphore from within interrupt context.
   
   @param semaphore     Address of semaphore to release.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT(int, osal_sem_release_INT)( 
      osal_sem_t   * sem
);

/*****************************************************************************/
/**                                MUTEXES                                  **/
/*****************************************************************************/

/* Mutexes are notionally similar to semaphores, but there are some subtle
   differences.  Only binary mutexes are supported in OSAL: they are indended
   to protect a single resource or section of critical code, and follow the
   traditional P and V semantics.
   
   Mutexes may also implement priority changing mechanisms to try to avoid 
   deadlock, e.g., priority inheritance protocol or priority ceiling protocol.
   This is a host OS implementation detail and is not exposed in the OSAL API.
*/

/*****************************************************************************/
/**
   Initialise a mutex.
   
   @param mutex         Address of mutex to initialise.
   @param nametag       Address of a string constant, used in debug only.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_mutex_init)(
      osal_mutex_t * mutex,
      const char   * nametag
);

/*****************************************************************************/
/**
   Destroy a mutex.
   
   @param mutex         Address of mutex to delete.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_mutex_destroy)( 
      osal_mutex_t * mutex
);

/*****************************************************************************/
/**
   Obtain a mutex, suspending (if required) if the mutex is not free.
   
   @param mutex         Address of mutex to obtain.
   @param suspend       Specifies suspend mode, either OSAL_SUSPEND_FOREVER,
                         OSAL_SUSPEND_NEVER, or a timeout period given in
                         milliseconds.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval ETIMEDOUT    if suspend timed out, or count not available.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_mutex_obtain)( 
      osal_mutex_t  * mutex,
      OSAL_SUSPEND    suspend
);

/*****************************************************************************/
/**
   Release a mutex.
   
   @param mutex         Address of mutex to release.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_mutex_release)( 
      osal_mutex_t *  mutex
);

/*****************************************************************************/
/**                              EVENT GROUPS                               **/
/*****************************************************************************/

/* Event groups extend semaphores with the ability to specify logical 
   associations between events.  Events are grouped together into blocks of 
   event flags.  Initially all events within an event group are cleared.
   
   Multiple events can be set concurrently using bitmasks supplied to the set
   function.
   
   Waiting for events specifies both the event mask and also the matching mode.
   Two matching modes are supported: all matching (AND) and any matching (OR).
   
   Events are not automatically cleared when a suspended task resumes.  It is
   the responbility of the resumed task to clear events it deems handled.
   
   If required the caller may also be given a copy of the matching events that
   caused the successful resumption.
*/   

/*****************************************************************************/
/**
   Initialise an event group.
   
   Clears all event flags prior to use.
   
   @param evgroup       Address of event group to initialise.
   @param num_events    Number of events to create within an event group.
   @param nametag       Address of a string constant, used in debug only.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_event_init)(
      osal_event_t   * evgroup,
      unsigned int     num_events,
      const char *     nametag );

/*****************************************************************************/
/**
   Destroy an event group.
   
   When an event group is destroyed all events are first set, allowing any
   pending tasks to resume.  Then the event group is deleted.
   
   @param event         Address of an event group to delete.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_event_destroy)( osal_event_t * evgroup );

/*****************************************************************************/
/**
   Set one or more events within a given event group.
   
   @param event         Address of an event group to modify.
   @param event_mask    Bitmask of events to set.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_event_set)( 
      osal_event_t   *   event, 
      unsigned int       event_mask
);

/*****************************************************************************/
/**
   Set one or more events within a given event group in interrupt context.
   
   @param event         Address of an event group to modify.
   @param event_mask    Bitmask of events to set.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT(int, osal_event_set_INT)( 
      osal_event_t   *   event, 
      unsigned int       event_mask
);

/*****************************************************************************/
/**
   Clear one or more events within a given event group.
   
   @param event         Address of an event group to modify.
   @param event_mask    Bitmask of events to clear.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_event_clear)( 
      osal_event_t  *    event, 
      unsigned int       event_mask
);

/*****************************************************************************/
/**
   Clear one or more events within a given event group in interrupt context.
   
   @param event         Address of an event group to modify.
   @param event_mask    Bitmask of events to clear.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT(int, osal_event_clear_INT)( 
      osal_event_t  *    event, 
      unsigned int       event_mask
);

/*****************************************************************************/
/**
   Get the current set of events within a given event group.
   
   @param event         Address of an event group to query.
   @param event_mask    Address of a variable into which is written the state
                         of the events.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_event_get)(
      osal_event_t  *    event, 
      unsigned int  *    event_mask
);

/*****************************************************************************/
/**
   Get the current set of events within a given event group in interrupt context.
   
   @param event         Address of an event group to query.
   @param event_mask    Address of a variable into which is written the state
                         of the events.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT(int, osal_event_get_INT)(
      osal_event_t   *     event, 
      unsigned int   *     event_mask
);

/*****************************************************************************/
/**
   Wait for a specific combination of events within a given event group.
   
   @param event         Address of an event group to wait on.
   @param event_mask    Bitmask of events to wait on.
   @param match_events  Address of a variable into which is written the set
                         of matching events on a successful call.  May be
                         NULL if caller not interested in this information.
   @param mode          Specify how events may match:
                         OSAL_EVENT_MODE_AND requires all event flags 
                         in @p event_mask to match;
                         OSAL_EVENT_MODE_OR requires at least one event
                         flag in @p event_mask to match.
   @param suspend       Specifies suspend mode, either 
                         OSAL_SUSPEND_FOREVER, OSAL_SUSPEND_NEVER, 
                         or a timeout period given in milliseconds.
   
   @retval 0            if successful.
   @retval EINVAL       if one or more parameters are incorrect.
   @retval ETIMEDOUT    if suspend timed out.
   @retval EFAULT       otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_event_wait)( 
      osal_event_t     *      event, 
      unsigned int            event_mask, 
      unsigned int *          match_events,
      OSAL_EVENT_MODE         mode,
      int                     suspend
);

/*****************************************************************************/
/**                              MESSAGE QUEUES                             **/
/*****************************************************************************/

/* A queue is modelled as a message-wide FIFO.  The queue store is provided by 
   the OSAL library.
   
   Putting messages into the queue is a matter of pointing to the message to 
   push and specifying the suspend behaviour.  The call may suspend if there 
   is not enough memory in the queue to hold the entire message, ensuring that
   messages in the queue are not fragmented.
   
   If multiple tasks are pending on a queue at the same time the order in which
   messages are retrieved and marshalled to the tasks is unspecified.
*/   

/*****************************************************************************/
/**
   Initialise a queue for holding messages of fixed size.
   
   @param queue      Address of queue to initialise.
   @param length     Length of queue specified as number of messages.
   @param size       Size in bytes of a single message.
   @param nametag    Address of a string constant, used in debug only.
   
   @retval 0         if successful.
   @retval EINVAL    if one or more parameters are incorrect.
   @retval EFAULT    otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_queue_init)(
      osal_queue_t   * queue,
      unsigned int     length,
      size_t           size,
      const char *     nametag
);

/*****************************************************************************/
/**
   Destroy a queue.
   
   @param queue      Address of queue to destroy.
   
   @retval 0        if successful.
   @retval EINVAL   if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_queue_destroy)( osal_queue_t * queue );

/*****************************************************************************/
/**
   Send a message into a queue.
   
   @param queue      Address of queue to send the message to.
   @param message    Address of message to put into queue.
   @param suspend    Specifies suspend mode, either OSAL_SUSPEND_FOREVER,
                      OSAL_SUSPEND_NEVER, or a timeout period given in
                      milliseconds.
   
   @retval 0          if successful.
   @retval EINVAL     if one or more parameters are incorrect.
   @retval ETIMEDOUT  if suspend timed out.
   @retval EFAULT     otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_queue_send_to)( 
      osal_queue_t * queue,
      const void   * message,
      int            suspend
);

/*****************************************************************************/
/**
   Send a message into a queue from interrupt context.
   
   @param queue       Address of queue to send the message to.
   @param message     Address of message to put into queue.
   
   @retval 0          if successful.
   @retval EINVAL     if one or more parameters are incorrect.
   @retval ETIMEDOUT  if no room in the queue for the message.
   @retval EFAULT     otherwise.
**/
OSAL_FUNCT(int, osal_queue_send_to_INT)( 
      osal_queue_t  * queue,
      const void    * message
);

/*****************************************************************************/
/**
   Receive a message from a queue.
   
   @param queue      Address of queue to receive a message from.
   @param buffer     Address of a buffer to store the received message.
   @param suspend    Specifies suspend mode, either OSAL_SUSPEND_FOREVER,
                      OSAL_SUSPEND_NEVER, or a timeout period given in
                      milliseconds.
   
   @retval 0          if successful.
   @retval EINVAL     if one or more parameters are incorrect.
   @retval ETIMEDOUT  if no messages available.
   @retval EFAULT     otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_queue_recv_from)( 
      osal_queue_t * queue,
      void         * message,
      int            suspend
);

/*****************************************************************************/
/**
   Receive a message from a queue in interrupt context.
   
   @param queue      Address of queue to receive a message from.
   @param buffer     Address of a buffer to store the received message.
   
   @retval 0          if successful.
   @retval EINVAL     if one or more parameters are incorrect.
   @retval ETIMEDOUT  if no messages available.
   @retval EFAULT     otherwise.
**/
OSAL_FUNCT(int, osal_queue_recv_from_INT)( 
      osal_queue_t * queue,
      void         * message
);

/*****************************************************************************/
/**                                  TASKS                                  **/
/*****************************************************************************/

/* Tasks are created at runtime with a simple set of task functions.  Creating
   a task requires the specification of a stack block for the task.  Either a
   specific size can be specified, or a default can be requested, which will
   be OS-specific.
   
   Tasks exist in one of three states -- reset, running, and stopped.
   
   {init]-> RESET ->[start]->  RUN  ->[stop]-> STOP  ->[destroy]
            STATE             STATE            STATE
              ^                                  |
              +------------<-[reset]<------------+
   
   When a task is first created it is in the reset state.  This allows 
   multiple tasks to be created without concern for interdependencies between 
   them.  Once they are all created they can then be started in the required 
   order.
   
   A task is then started, and moves to the running state.
   
   A task may either finish itself (i.e., it reaches the end of the task's 
   main function), or be stopped externally.  In both cases the task moves
   to the stopped state.   A stopped task cannot be resumed, but it may be
   reset and restarted.
   
   Once a task is stopped it can be deleted.  Deleting a running task 
   automatically forces it into the stopped state prior to deleting it.
  
   A task function takes two user-supplied arbitrary arguments, useful for 
   passing specific arguments to instances of a task.
   
   Tasks may be assigned a priority, such that during pre-emption waiting tasks
   with higher priorities will run before lower-priority tasks.  Highest
   priority is 0, lowest priority is 255.
   
   When a task is no longer needed it can be deleted and its stack memory
   reused for something else.
*/

/*****************************************************************************/
/**
   Initialise a task.
   
   @param task          Address of task to initialise.
   @param stack_size    Size in bytes of the required task stack, or 0 for
                         an OS-specific default size.
   @param task_func     Address of the task's entry function.
   @param param1        First task argument.
   @param param2        Second task argument.
   @param priority      Task's initial priority level.
   @param nametag       Address of a string constant, used in debug only.
   
   @retval 0         if successful.
   @retval EINVAL    if one or more parameters are incorrect.
   @retval EFAULT    otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_init)( 
      osal_task_t  * task,
      size_t         stack_size,
      void        (* task_func)(osal_task_t *, void *, void *),
      void *         param1,
      void *         param2,
      unsigned int   priority,
      const char   * nametag
);

/*****************************************************************************/
/**
   Destroy a task.  Stops the task if it is currently running.
   
   @param task          Address of task to delete.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_destroy)( osal_task_t * task );

/*****************************************************************************/
/**
   Starts a task.  Has no effect unless the task is in reset state.
   
   @param task          Address of task to start.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_start)( osal_task_t * task );

/*****************************************************************************/
/**
   Stops a task executing.  Has no effect if the task is already stopped.
   
   @param task          Address of task to stop.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_stop)( osal_task_t * task );

/*****************************************************************************/
/**
   Reset a task.  Stops the task first if it is currently running.
   
   Returns the task to its initial state (as after task_create).
   
   @param task          Address of task to reset.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_reset)( osal_task_t * task );

/*****************************************************************************/
/**
   Sets a task's priority level.
   
   @param task          Address of task to modify.
   @param priority      New priority level for the task.
   @param old_priority  Address of a variable in which is stored the task's
                         previous priority.  Can be NULL, in which case the
                         previous priority is not returned.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_set_priority)(
      osal_task_t  * task,
      unsigned int            priority,
      unsigned int *          old_priority
);

/*****************************************************************************/
/**
   Get a task's priority level.
   
   @param task          Address of task to query.
   @param priority      Address of a variable in which is stored the task's
                         priority.
   
   @retval 0       if successful.
   @retval EINVAL if one or more parameters are incorrect.
   @retval EFAULT   otherwise.
**/
OSAL_FUNCT_TRACEABLE(int, osal_task_get_priority)(
      osal_task_t *  task,
      unsigned int *          priority
);

/*****************************************************************************/
/**
   Put the current task to sleep for a specified period.
   
   @param delay_ms   Time to sleep, in milliseconds.
**/
OSAL_FUNCT_TRACEABLE(void, osal_task_sleep)( unsigned int delay_ms );

/*****************************************************************************/
/**                                  LOGGING                                **/
/*****************************************************************************/

/* OSAL provides a simple logging system.  The logging calls are fairly 
   lightweight, with no return codes -- if a logging function fails the system
   simply ignores it.
   
   The logging system supports logging levels, allowing control over the
   degree of logging provided for a particular execution.
   The level hierarchy follows the same model as task priority.  Level 0 is the
   highest level, and only very critical log messages should be at this level.
   Increasing the log level corresponds to an increasingly verbose log.
   
   The default logging levels are:
      OSAL_LOG_CRITICAL      for the most important log messages,
      OSAL_LOG_IMPORTANT     for logging important but not critical messages,
      OSAL_LOG_INFORMATIVE   for all other messages.
*/

/*****************************************************************************/
/**
   Open system debug log.
   
   @param logname          Name of log to open.  On systems that write the log
                            to a file this will be the name of that file.  In
                            other systems this will be ignored.
                           If NULL, then the logging system will synthesize a
                            unique name if one is required.
**/
OSAL_FUNCT(void, osal_log_open)( const char * logname );

/*****************************************************************************/
/**
   Close system debug log.
**/
OSAL_FUNCT(void, osal_log_close)( void );

/*****************************************************************************/
/**
   Set the logging level.
   
   @param level         The new logging level.
**/
OSAL_FUNCT(void, osal_log_set_level)( unsigned int level );

/*****************************************************************************/
/**
   Get the current logging level.

   @return The current logging level.
**/
OSAL_FUNCT(unsigned int, osal_log_get_level)( void );

/*****************************************************************************/
/**
   Print debug message to system log.

   Uses same format specifiers as printf().  This function takes a variable 
   number of arguments.  See @p osal_log_message_var() for the va_arg 
   variant.
   
   @param level       Logging level of this message.
   @param format      Character string, including format specifiers.
   @param ...         Variable length argument list containing optional values,
                       consumed by format specifiers in @p format.
**/
OSAL_FUNCT(void, osal_log_message)(
      unsigned int     level,
      const char * format, 
      ...
);

/*****************************************************************************/
/**
   Print debug message to system log.

   Uses same format specifiers as printf().  This is the va_arg version of
   the @p osal_log_message() function.
   
   @param level       Logging level of this message.
   @param format      Character string, including format specifiers.
   @param arg         Variable argument list formatted using the standard
                       variable argument list macros.
**/
OSAL_FUNCT(void, osal_log_message_var)( 
      unsigned int     level,
      const char * format, 
      va_list      arg
);

/*****************************************************************************/
/**                           OSAL TRACE MACROS                             **/
/*****************************************************************************/

/* These macros enable trace wrappers around almost all of the OSAL functions.
 * They are enabled by the OSAL_TRACE build flag.
 */
 
#ifdef OSAL_TRACE

#define osal_timer_init(t,h,a,nt)          trace_osal_timer_init(t,h,a,nt)
#define osal_timer_destroy(t)              trace_osal_timer_destroy(t)
#define osal_timer_start(t,d)              trace_osal_timer_start(t,d)
#define osal_timer_stop(t)                 trace_osal_timer_stop(t)
#define osal_get_systime(s,u)              trace_osal_get_systime(s,u)

#define osal_sem_init(s,i,nt)              trace_osal_sem_init(s,i,nt)
#define osal_sem_destroy(s)                trace_osal_sem_destroy(s)
#define osal_sem_obtain(s,p)               trace_osal_sem_obtain(s,p)
#define osal_sem_release(s)                trace_osal_sem_release(s)

#define osal_mutex_init(m,nt)              trace_osal_mutex_init(m,nt)
#define osal_mutex_destroy(m)              trace_osal_mutex_destroy(m)
#define osal_mutex_obtain(m,p)             trace_osal_mutex_obtain(m,p)
#define osal_mutex_release(m)              trace_osal_mutex_release(m)

#define osal_event_init(e,n,nt)            trace_osal_event_init(e,n,nt)
#define osal_event_destroy(e)              trace_osal_event_destroy(e)
#define osal_event_set(e,m)                trace_osal_event_set(e,m)
#define osal_event_clear(e,m)              trace_osal_event_clear(e,m)
#define osal_event_get(e,m)                trace_osal_event_get(e,m)
#define osal_event_wait(e,m,x,o,s)         trace_osal_event_wait(e,m,x,o,s)

#define osal_queue_init(q,l,s,nt)          trace_osal_queue_init(q,l,s,nt)
#define osal_queue_destroy(q)              trace_osal_queue_destroy(q)
#define osal_queue_send_to(q,m,s)          trace_osal_queue_send_to(q,m,s)
#define osal_queue_recv_from(q,m,s)        trace_osal_queue_recv_from(q,m,s)

#define osal_task_init(t,z,f,a,b,p,nt)     trace_osal_task_init(t,z,f,a,b,p,nt)
#define osal_task_reset(t)                 trace_osal_task_reset(t)
#define osal_task_destroy(t)               trace_osal_task_destroy(t)
#define osal_task_start(t)                 trace_osal_task_start(t)
#define osal_task_stop(t)                  trace_osal_task_stop(t)
#define osal_task_set_priority(t,p,o)      trace_osal_task_set_priority(t,p,o)
#define osal_task_get_priority(t,p)        trace_osal_task_get_priority(t,p)
#define osal_task_sleep(d)                 trace_osal_task_sleep(d)

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#endif /* OSAL_H */
