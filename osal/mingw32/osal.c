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
   Project Includes
 *****************************************************************************/

#undef OSAL_TRACE   /* Make sure we have the non-trace defs */
#include "osal.h"

/*****************************************************************************
   Private Data.  Declare as static.
 *****************************************************************************/

/**
   Table of initialization functions to call, in given order.
**/
static int (* init_table[])(void) = {
     osal_init_timer,
     NULL
};

/**
   Table of shutdown functions to call, in given order.
**/
static int (* shdn_table[])(void) = {
     osal_shdn_timer,
     NULL
};

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/*****************************************************************************/
/**
   Initialize the OS layer and start the application.
   
   Under normal circumstances this call does not return.  If it does then this
   should be treated as a fatal error and the system must deal with it in a safe
   manner.
   
   @param app_main    Name of application main task.  Takes no arguments and 
                       returns nothing.

   @retval 0      if successful,
   @retval EFAULT otherwise.
**/
extern int osal_start( int (*app_main)(int, const char**), int argc, const char **argv )
{
   int status = 0;
   int i;
   
   for ( i = 0; init_table[i]; i++ )
      if ( (init_table[i])() != 0 )
         status = EFAULT;

   if ( 0 == status )
       status = (app_main)(argc, argv);
   
   return status;
}

/*****************************************************************************/
/**
   Terminate the OS layer, shutting down and releasing any resources.
   
   Executes all shutdown functions.  If any fail, for whatever reason,
   we return a failed status.
   
   @retval 0     if successful
   @retval EFAULT otherwise
**/
extern int osal_shutdown( void )
{
   int status = 0;
   int i;
   
   for ( i = 0; shdn_table[i]; i++ )
      if ( (shdn_table[i])() != 0 )
         status = EFAULT;
   
   return status;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
