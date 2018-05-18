/* ****************************************************************************
 * STUI - Simple Text User Interface
 * Copyright (C) 2011, Neil Johnson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ************************************************************************* */

/*****************************************************************************/
/* System Includes                                                           */
/*****************************************************************************/

#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


/*****************************************************************************/
/* Project Includes                                                          */
/*****************************************************************************/

#include "driver_api.h"

/*****************************************************************************/
/* Macros, constants                                                         */
/*****************************************************************************/


/*****************************************************************************/
/* Data types                                                                */
/*****************************************************************************/



/*****************************************************************************/
/* Private Data.  Declare as static.                                         */
/*****************************************************************************/



static int fd = 0;
static unsigned int rows, cols;


/*****************************************************************************/
/* Private function prototypes.  Declare as static.                          */
/*****************************************************************************/


static void update_size( void );
static void resize_tty( int );
static void xterm_out( STUI_CHAR_T );


/*****************************************************************************/
/* Private functions.  Declare as static.                                    */
/*****************************************************************************/

static void update_size( void )
{
    struct winsize ws;

    if ( !ioctl( fd, TIOCGWINSZ, &ws ) )
    {
        cols = ws.ws_col;
        rows = ws.ws_row;
    }
}


static void resize_tty( int sig )
{
    signal( SIGWINCH, resize_tty );
    
    /* Do something about this ? */ 
}

static void xterm_out( STUI_CHAR_T sc )
{
    printf( "\x1B[0" );
    
    if ( sc & STUI_ATTR_BOLD )
        printf( ";1" );
    if ( sc & STUI_ATTR_BLINK )
        printf( ";5" );
    if ( sc & STUI_ATTR_REVERSE )
        printf( ";7" );
    if ( sc & STUI_ATTR_UNDLINE )
        printf( ";4" );
        
    putchar( 'm' );
    putchar( sc & STUI_CHAR_MASK );
}

static void goto_rowcol( unsigned int row, unsigned int col )
{
    printf( "\x1B[%d;%dH", row+1, col+1 );
}

/*****************************************************************************/
/* Public functions.  Defined in header file.                                */
/*****************************************************************************/


int drv_open( void )
{
    fd = open( "/dev/tty", O_RDWR );
	if ( !fd )
		return -1;
        
    signal( SIGWINCH, resize_tty );
        
    update_size();
    
    return 0;
}

void drv_get_screen_size( unsigned int *prows, unsigned int *pcols )
{
    update_size();
    if ( prows ) *prows = rows;
    if ( pcols ) *pcols = cols;
}


extern void drv_put_screen( STUI_CHAR_T *vbuf )
{
    int r, c;
    
    goto_rowcol( 0, 0 );
    for ( r = 0; r < rows; r++ )
        for ( c = 0; c < cols; c++ )
            xterm_out( *vbuf++ );
    fflush(stdout);
}

extern void drv_close( void )
{
    close( fd );
    fd = 0;
    printf( "\x1B[0m\x1B[1;1H\x1B[2J" );
    fflush( stdout );
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
