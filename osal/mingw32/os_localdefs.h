/****************************************************************************/
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
/****************************************************************************/

#ifndef OS_LOCALDEFS_H
#define OS_LOCALDEFS_H

/*****************************************************************************
   System-wide Includes
 *****************************************************************************/

/* Windows-specific headers */

/* Note: timer functions only available from Win2000 onwards */
#define WINVER      0x0500

#include <windows.h>
#include <mmsystem.h>
#include <winbase.h>

/*****************************************************************************
   Public manifest constants and macros.
 *****************************************************************************/

/**
   Length of nametag fields, only for debug.
**/
#define NAMETAG_LENGTH              ( 16 )

/*****************************************************************************
   Public Data Types.
 *****************************************************************************/

/*****************************************************************************/
/**
   Timer data type.
**/
typedef struct osal_timer_s {
#ifndef NDEBUG
   char nametag[NAMETAG_LENGTH];
#endif

   HANDLE hTimer;
   int    is_active;
   void (* handler)(struct osal_timer_s *, void *);
   void * arg;
   
} osal_timer_t;

/*****************************************************************************/
/**
   Semaphore data type.
**/
typedef struct {
#ifndef NDEBUG
   char nametag[NAMETAG_LENGTH];
#endif
    HANDLE sem;
} osal_sem_t;

/*****************************************************************************/
/**
   Mutex data type.
**/
typedef struct {
#ifndef NDEBUG
   char nametag[NAMETAG_LENGTH];
#endif

   HANDLE mtx;
} osal_mutex_t;

/*****************************************************************************/
/**
   Queue data type.
**/
typedef struct {
#ifndef NDEBUG
   char nametag[NAMETAG_LENGTH];
#endif

   size_t            msg_size;        /**< Size in bytes of each message      **/
   unsigned int      len;             /**< Length of queue in messages        **/
   unsigned char *   qmem;            /**< Queue memory block                 **/
   unsigned int      head, tail;
   osal_sem_t        sem_put;         /**< Putting semaphore                  **/
   osal_sem_t        sem_get;         /**< Getting semaphore                  **/
   osal_mutex_t      mtx;             /**< Controls access to queue memory    **/
} osal_queue_t;

/*****************************************************************************/
/**
   Task data type.
**/
typedef struct osal_task_s {
#ifndef NDEBUG
   char nametag[NAMETAG_LENGTH];
#endif   

   HANDLE                  thread_handle;
   DWORD                   thread_id;
   void                 (* task_func)(struct osal_task_s *, void *, void *);
   void *                  param1;
   void *                  param2;
   size_t                  stack_size;
   unsigned char           priority;
   int                     running;
} osal_task_t;

/*****************************************************************************/
/**
   Event data type.
**/
typedef struct {
#ifndef NDEBUG
   char nametag[NAMETAG_LENGTH];
#endif
   osal_mutex_t      lock;
   void *            evlist;
   unsigned int      ev;
   unsigned int      num_events;
} osal_event_t;

/****************************************************************************
   Public Functions.
 ****************************************************************************/

extern int osal_init_timer(void);
extern int osal_shdn_timer(void);

/*****************************************************************************
                                  E N D
 *****************************************************************************/
#endif
