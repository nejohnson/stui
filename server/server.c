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
#include <stdio.h>

/*****************************************************************************/
/* Project Includes                                                          */
/*****************************************************************************/

#include "stui.h"
#include "driver_api.h"
#include "osal/osal.h"

/*****************************************************************************/
/* Macros, constants                                                         */
/*****************************************************************************/


/*****************************************************************************/
/* Data types                                                                */
/*****************************************************************************/

/**
   A visual represents the physical visual interface.  All it needs are the
   visual buffer vbuf and the dimensions of the screen.
**/
struct visual {
    STUI_CHAR_T *vbuf;
    unsigned int width, height;
};

/**
   Internal window data type.
**/
struct window {
    /* Window dimensions */
    unsigned int width, height;
    unsigned int row, col;
    
    /* User-supplied repaint callback */
    STUI_CALLBACK_T callback;
    
    /* List pointers */
    struct window *up, *down;
    
    /* Window properties */
    struct {
        unsigned int visible:1;
        unsigned int dirty:1;
    } flag;
    
    /* Other */
    void * userdata;
};

/*****************************************************************************/
/* Private Data.  Declare as static.                                         */
/*****************************************************************************/

/** Assume a single visual **/
static struct visual vis = { NULL, 0, 0 };

/**
   Windows are stored in a linked list, with root pointing to the bottom of the
   stack of windows.  I.e.:
   
              /-----------------/   ---- GLASS ----
             /                 /
            /                 /
           /                 /--/
          /                 /  /
         /_________________/  /
           /                 /--/
          /                 /  /
         /_________________/  /
           /                 /--/
          /                 /  /
         /_________________/  /
           /                 /--/
          /                 /  /
         /_________________/  /
           /                 /
          /                 /
  root-> /_________________/
    
   
   Redrawing then naturally starts at the bottom and works its way up the 
   stack.     
**/   
static struct window *root = NULL;

/** Global lock on the internal data **/
static osal_mutex_t svr_lock;

/** The server task is started up at initialisation time.  Its main job is to
    kick off visual refreshes at timed intervals.
**/
static osal_task_t serverTCB;

/*****************************************************************************/
/* Private function prototypes.  Declare as static.                          */
/*****************************************************************************/


/*****************************************************************************/
/* Private functions.  Declare as static.                                    */
/*****************************************************************************/

/*****************************************************************************/
/**
    Server task
    
    Updates the screen at regular intervals.
**/
static void server_task( osal_task_t *tcb, void * param1, void *param2 )
{
    while(1)
    {
        osal_task_sleep( 100 );

        if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
   {
            struct window * hWnd;
       int need_refresh;
       
       need_refresh = 0;
       for ( hWnd = root; hWnd; hWnd = hWnd->up )
       {
          if ( hWnd->flag.visible && hWnd->flag.dirty ) 
      {
          /* Do the simple full repaint for the moment */
          hWnd->callback( hWnd, 0, 0, hWnd->height, hWnd->width );
          
          hWnd->flag.dirty = 0;
          need_refresh     = 1;
      }
       }
       
       if ( need_refresh )
           drv_put_screen( vis.vbuf );
      
       osal_mutex_release( &svr_lock );
   }
    }   
}

/*****************************************************************************/
/**
    Given source window src mark as dirty all windows that src overlaps.
**/
static void mark_dirty_underlapping( struct window * win, struct window * src )
{
    while ( win )
    {
        if ( win->flag.visible && !win->flag.dirty )
   {
       if (    ( src->col               <= win->col + win->width  ) 
            && ( src->col + src->width  >= win->col               )
       && ( src->row               <= win->row + win->height ) 
       && ( src->row + src->height >= win->row               ) )
            {
           win->flag.dirty = 1;       
       }   
   }
   win = win->down;
    }
}

/*****************************************************************************/
/**
    Given source window src mark as dirty all windows that overlaps src.
**/
static void mark_dirty_overlapping( struct window * win, struct window * src )
{
    while ( win )
    {
       if ( win->flag.visible && !win->flag.dirty )
       {
           if (    ( win->col               <= src->col + src->width  )
           && ( win->col + win->width  >= src->col               )
      && ( win->row               <= src->row + src->height )
      && ( win->row + win->height >= src->row               ) )
      {
         win->flag.dirty = 1;
         mark_dirty_overlapping( win->up, win );      
      }       
       }
       win = win->up;
    }
}

/*****************************************************************************/
/**
    Redimension a window (size and position) doing any dirty tagging if the 
    window is visible.
**/
static void redim_window( struct window *win, 
                          unsigned int row, unsigned int col, 
           unsigned int width, unsigned int height )
{
    if ( win->flag.visible )
    {
   mark_dirty_underlapping( win->down, win );
   mark_dirty_overlapping( root->up, root );
    }

    win->row = row;
    win->col = col;
    win->width = width;
    win->height = height;
    
    if ( win->flag.visible )   
   mark_dirty_overlapping( win->up, win );
}

/*****************************************************************************/
/* Public functions.  Defined in header file.                                */
/*****************************************************************************/

/*****************************************************************************/
/**
    Start the STUI server system.
    
    @return 0 if successful, -1 if failure.
**/
extern int stui_server( void )
{
    unsigned int rows, cols;
    int status;
    unsigned int i;
    
    status = drv_open();
    if ( status == -1 )
        return -1;   
   
    status = osal_mutex_init( &svr_lock, "stui:svrlock" );
    if ( status )
    {
        drv_close();
   return -1;
    }
        
    drv_get_screen_size( &rows, &cols );
    vis.width  = cols;
    vis.height = rows;
    vis.vbuf   = calloc( rows * cols, sizeof(STUI_CHAR_T) );
    if ( !vis.vbuf )
    {
        osal_mutex_destroy( &svr_lock );
        drv_close();
        return -1;
    }
    
    /* Initialise the visual buffer */
    for ( i = 0; i < vis.width * vis.height; i++ )
        vis.vbuf[i] = ' '; /* | STUI_ATTR_REVERSE; */
   
    if ( osal_task_init( &serverTCB, 0, server_task, NULL, NULL, 10, "stui_server" ) )
    {
       free( vis.vbuf );
   osal_mutex_destroy( &svr_lock );
   drv_close();
   return -1;
    }
    
    if ( osal_task_start( &serverTCB ) )
    {
       osal_task_destroy( &serverTCB );
       free( vis.vbuf );
   osal_mutex_destroy( &svr_lock );
   drv_close();
   return -1;
    }
    
    return 0;
}

/*****************************************************************************/
/**
    Create a window.
    
    The initial window is placed at (0,0), has zero size and is not visible.
    
    @param cb      Pointer to callback function.
    
    @return Window handle if successful, NULL if failed.
**/
extern STUI_WINDOW_T stui_create_window( STUI_CALLBACK_T cb )
{
    struct window * hWnd = NULL;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
       hWnd = calloc( 1, sizeof(*hWnd) );
   
       if ( hWnd )
        {    
           hWnd->callback = cb;
    
           if ( root )
       {
           hWnd->up = root;
      hWnd->up->down = hWnd;
            }
      
            root = hWnd;
   }
    
        osal_mutex_release( &svr_lock );
    }
    
    return (STUI_WINDOW_T)hWnd;
}

/*****************************************************************************/
/**
    Destroy a window.
    
    @param hWnd      Handle to window to destroy.
**/
extern void stui_destroy_window( STUI_WINDOW_T hWnd )
{
    struct window * win = (struct window *)hWnd;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        /* remove window from list */
   
        if ( win->down )
       win->down->up = win->up;
   else
       root = win->up;
   
   if ( win->up )
       win->up->down = win->down;

        if ( win->flag.visible )
   {
       mark_dirty_underlapping( win->down, win );
       mark_dirty_overlapping( root->up, root );
   }
   
   free( win );   
   
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Make a window visible.
    
    @param hWnd      Handle to window to make visible.
**/
extern void stui_show_window( STUI_WINDOW_T hWnd )
{
    struct window * win = (struct window *)hWnd;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        win->flag.visible = 1;
   
   mark_dirty_overlapping( win, win );
   
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Make a window hidden.
    
    @param hWnd      Handle to window to hide.
**/
extern void stui_hide_window( STUI_WINDOW_T hWnd )
{
    struct window * win = (struct window *)hWnd;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        if ( win->flag.visible )
   {
       win->flag.visible = 0;
       mark_dirty_underlapping( win->down, win );
       mark_dirty_overlapping( root->up, root );
   }
   
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Change a window's position.
    
    @param hWnd      Handle to window to move.
    @param row       New row
    @param col       New column
**/
extern void stui_move_window( STUI_WINDOW_T hWnd, 
                              unsigned int row, unsigned int col )
{
    struct window * win = (struct window *)hWnd;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        redim_window( win, row, col, win->width, win->height );
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Change a window's size.
    
    @param hWnd      Handle to window to move.
    @param width     New width.  Set to 0 for maximum width.
    @param height    New height.  Set to 0 for maximum height.
**/
extern void stui_resize_window( STUI_WINDOW_T hWnd, 
                                unsigned int width, unsigned int height )
{
    struct window * win = (struct window *)hWnd;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        /* If either of the new dimensions are 0 then we need to query the
    *  driver for the visual dimensions and set accordingly.
    */
   if ( 0 == width || 0 == height )
   {
       unsigned int rows, cols;
      
       drv_get_screen_size( &rows, &cols );
      
            if ( 0 == width  ) width  = cols - win->col;
            if ( 0 == height ) height = rows - win->row;
        }
   
   redim_window( win, win->row, win->col, width, height );
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Raise a window.
    
    @param hWnd      Handle to window to destroy.
**/
extern void stui_raise_window( STUI_WINDOW_T hWnd )
{
    struct window * win = (struct window *)hWnd;
    
    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        /* First hide the window */
        if ( win->flag.visible )
   {
       mark_dirty_underlapping( win->down, win );
       mark_dirty_overlapping( root->up, root );
   }
   
        /* remove window from list */
   if ( win->down )
       win->down->up = win->up;
   else
       root = win->up;
   
   if ( win->up )
       win->up->down = win->down;

        /* Put onto top of list */
   win->up   = NULL;
   win->down = root;
   while ( win->down->up )
       win->down = win->down->up;
       
   win->down->up = win;
   
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Flag a window as needing repainting.
    
    @param hWnd      Handle to window to move.
**/
extern void stui_repaint( STUI_WINDOW_T hWnd )
{
    struct window * win = (struct window *)hWnd;

    if ( !osal_mutex_obtain( &svr_lock, OSAL_SUSPEND_FOREVER ) )
    {
        mark_dirty_overlapping( win, win );
    
        osal_mutex_release( &svr_lock );
    }
}

/*****************************************************************************/
/**
    Attach/update a user-supplied pointer to a window.
    
    @param hWnd      Handle to window to modify.
    @param ud        General-purpose user pointer
**/
extern void stui_set_userdata( STUI_WINDOW_T hWnd, void * ud )
{
    struct window * win = (struct window *)hWnd;
    
    win->userdata = ud;
}

/*****************************************************************************/
/**
    Get the user-supplied pointer of a window.
    
    @param hWnd      Handle to window to query.
    
    @return User data, or NULL if none.
**/
extern void * stui_get_userdata( STUI_WINDOW_T hWnd )
{
    struct window * win = (struct window *)hWnd;
    
    return win->userdata;
}

/*****************************************************************************/
/**
    Get the dimensions of a window.
    
    @param hWnd      Handle to window to query.
    @param p_width   Pointer to store window width.  Can be NULL.
    @param p_height  Pointer to store widow height.  Can be NULL.
**/

extern void stui_get_window_dims( STUI_WINDOW_T hWnd, 
                                  unsigned int *p_width, 
                                  unsigned int *p_height )
{
   struct window * win = (struct window *)hWnd;
   
   if ( p_width )  *p_width  = win->width;
   if ( p_height ) *p_height = win->height;
}
                                  
/*****************************************************************************/
/*****************************************************************************/
/*                              Callback Functions                           */
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/**
    Put a single character into the visual buffer for a given window.
    
    CAUTION: this function must only be called from within a callback context.
    
    @param hWnd      Handle to window to move.
    @param row       Row, relative to top-left window, to put character.
    @param col       Column, relative to top-left window, to put character.
    @param c         Attributed character to put
**/
extern void stui_cb_putchar( STUI_WINDOW_T hWnd, 
                             unsigned int row, unsigned int col, 
                             STUI_CHAR_T c )
{
    struct window * win = (struct window *)hWnd;
    
    if ( row < win->height && col < win->width 
    && ( win->row + row ) < vis.height 
    && ( win->col + col ) < vis.width )
    {
        vis.vbuf[ ( ( win->row + row ) * vis.width ) + ( win->col + col ) ] = c;
    }
}

#if defined( STUI_USE_FORMAT )

#include "format.h"

/** data structure type for holding output data **/
struct cb_out {
    STUI_WINDOW_T hWnd;
    STUI_CHAR_T attr;
    unsigned int row;
    unsigned int col;
};

/** wrapper function for format that uses user data to then call stui_cb_putchar **/
static void * wrapper_putchar( void *ptr, const char *s, size_t n  )
{
   struct cb_out * p = (struct cb_out *)ptr;
   while ( n )
   {
      stui_cb_putchar( p->hWnd, p->row, p->col, *s++ | p->attr );
      p->col++;
      n--;
   }

   return ptr;
}

/*****************************************************************************/
 /**
     Put a single character into the visual buffer for a given window.
     
     CAUTION: this function must only be called from within a callback context.

     @param hWnd      Handle to window to move.
     @param row       Row, relative to top-left window, to start output.
     @param col       Column, relative to top-left window, to start output.
     @param attr      Character attributes to apply to output string
     @param fmt       Format string, followed by optional arguments
 **/
extern void stui_cb_printf( STUI_WINDOW_T hWnd, 
                            unsigned int row, unsigned int col, 
                            STUI_CHAR_T attr, const char *fmt, ... )
{
   struct cb_out udata;
   va_list ap;
   
   udata.hWnd = hWnd;
   udata.attr = attr;
   udata.row = row;
   udata.col = col;

   va_start( ap, fmt );

   format( wrapper_putchar, &udata, fmt, ap );

   va_end( ap );
}

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
