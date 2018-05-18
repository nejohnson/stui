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

#ifndef STUI_H
#define STUI_H

/* Pull in the config file first */
#include "stui_config.h"

/*****************************************************************************/
/*  Public type definitions, macros, manifest constants                      */
/*****************************************************************************/

/**
   Characters are handled in a special encoding, including both the ASCII
   character code as well as various display attributes.
**/
typedef unsigned short STUI_CHAR_T;
#define STUI_CHAR_MASK      ( 0xFF )

#define STUI_ATTR_BOLD      ( 1 <<  8 )
#define STUI_ATTR_BLINK     ( 1 <<  9 )
#define STUI_ATTR_REVERSE   ( 1 << 10 )
#define STUI_ATTR_UNDLINE   ( 1 << 11 )

enum {
    STUI_MSG_TERM_RESIZE = 0x100,
    STUI_MSG_REPAINT,
    STUI_MSG_PAINT_REQ,
    STUI_MSG_PAINT
    
};

/**
   Windows are managed by opaque handles.
**/
typedef void * STUI_WINDOW_T;

/**
   Applications must associate a callback with each window created.  This is
   callbed by the server when the window needs to be repainted.  As well as the
   windows handle the server also provides the bounding box of the portion of
   the window that needs repainting.  The callback function can choose to either
   repaint its entire window or just the specified region.
   For example:
                111111111122222
      0123456789012345678901234  
   0  .........................
   1  .........................
   2  .........################
   3  .........################
   4  .........################
   5  .........################
   6  .........################
   7  .........................
   8  .........................
   9  .........................
      
   here the "dirty" region is hashed (for example from an overlapping window
   that has been removed).  In this case the coordinates of the dirty region
   are:
       topleft_(row,col) = 2,9
       btmright_(row,col) = 6,24
   
**/
typedef void (*STUI_CALLBACK_T)( STUI_WINDOW_T /* hWnd    */ , 
                                 unsigned int  /* topleft_row */ ,
                                 unsigned int  /* topleft_col */ ,
                                 unsigned int  /* btmright_row */ ,
                                 unsigned int  /* btmright_col */ );

/*****************************************************************************/
/* Public functions.  Declare as extern.                                     */
/*****************************************************************************/

extern int stui_server( void );

extern STUI_WINDOW_T stui_create_window( STUI_CALLBACK_T );
extern void stui_destroy_window( STUI_WINDOW_T );

extern void stui_move_window( STUI_WINDOW_T, unsigned int, unsigned int );
extern void stui_resize_window( STUI_WINDOW_T, unsigned int, unsigned int );

extern void stui_show_window( STUI_WINDOW_T );
extern void stui_hide_window( STUI_WINDOW_T );

extern void stui_raise_window( STUI_WINDOW_T );

extern void stui_set_userdata( STUI_WINDOW_T, void * );
extern void * stui_get_userdata( STUI_WINDOW_T );

extern void stui_get_window_dims( STUI_WINDOW_T, unsigned int *, unsigned int * );

extern void stui_repaint( STUI_WINDOW_T );

extern void stui_cb_putchar( STUI_WINDOW_T, unsigned int, unsigned int, STUI_CHAR_T );

#if defined( STUI_USE_FORMAT )
extern void stui_cb_printf( STUI_WINDOW_T, unsigned int, unsigned int, STUI_CHAR_T, const char *, ... );
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#endif
