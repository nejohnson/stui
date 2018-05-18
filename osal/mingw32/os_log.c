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
 
#include <stdio.h>
#include <time.h>
 
/*****************************************************************************
   Project Includes
 *****************************************************************************/

#include "osal.h"

/*****************************************************************************
   Private typedefs, macros and constants.
 *****************************************************************************/

/**
   The maximum length of the timestamp string.
**/
#define MAX_LOGNAME_LENGTH     ( 32 )

/*****************************************************************************
   Private Data.  Declare as static.
 *****************************************************************************/
 
static FILE *   logfile;
static unsigned int loglevel;

#ifdef OSAL_LOG_SIZE_LIMIT
static unsigned int logfile_size = 0;
#endif

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/*****************************************************************************/
/**
   Open system debug log.
   
   @param logname          Name of log to open.  On systems that write the log
                            to a file this will be the name of that file.  In
                            other systems this will be ignored.
                           If NULL, then the logging system will synthesize a
                            unique name if one is required.
**/
extern void osal_log_open( const char * logname )
{
   if ( NULL == logname )  /* Synthesize a log filename */
   {
#ifdef STDERR_LOGGING
      logfile = stderr;
#else
      time_t time_now;
      char   mylogname[MAX_LOGNAME_LENGTH];
      
      time( &time_now );
      strftime( mylogname, sizeof(mylogname), 
                "osal_log_%Y%m%d%H%M%S.txt",  /* format: YYYYMMDDhhmmss */
                localtime( &time_now ) );
                
      logfile = fopen( mylogname, "w" );
#endif
   }
   else
   {
      logfile = fopen( logname, "w" );
   }
   
#ifdef OSAL_LOG_SIZE_LIMIT   
   logfile_size = 0;
#endif   
}

/*****************************************************************************/
/**
   Close system debug log.
**/
extern void osal_log_close( void )
{
   if ( logfile )
   {
      fflush( logfile );
      if ( logfile != stderr )
      {
         fclose( logfile );
         logfile = NULL;
      }
   }
}

/*****************************************************************************/
/**
   Set the logging level.
   
   @param level         The new logging level.
**/
extern void osal_log_set_level( unsigned int level )
{
   loglevel = level;
}

/*****************************************************************************/
/**
   Get the current logging level.

   @return The current logging level.
**/
extern unsigned int osal_log_get_level( void )
{
   return loglevel;
}

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
extern void osal_log_message( 
      unsigned int     level,
      const char * format, 
      ...
)
{
   va_list ap;
   
   /* For simplicity, simply wrap around the next function */
   va_start( ap, format );
   osal_log_message_var( level, format, ap );
   va_end( ap );
}

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
extern void osal_log_message_var( 
      unsigned int     level,
      const char * format, 
      va_list      arg
)
{
   int nch;
   
#ifdef OSAL_LOG_SIZE_LIMIT
   if ( logfile_size >= OSAL_LOG_SIZE_LIMIT )
      return;
#endif

   if ( logfile && level <= loglevel )
   {
      time_t time_now;
      char   timestamp[MAX_LOGNAME_LENGTH];
      
      time( &time_now );
      strftime( timestamp, sizeof(timestamp), 
                "%H:%M:%S",  /* format: hhmmss */
                localtime( &time_now ) );
      nch = fprintf( logfile, "[%s] ", timestamp );
#ifdef OSAL_LOG_SIZE_LIMIT      
      if(nch > 0)
         logfile_size += nch;
#endif
      
      nch = vfprintf( logfile, format, arg );
#ifdef OSAL_LOG_SIZE_LIMIT      
      if(nch > 0)
         logfile_size += nch;
#endif
      
      fputc( '\n', logfile );
      fflush( logfile );
   }
}

/*****************************************************************************
                                  E N D
 *****************************************************************************/
