#include <stdio.h>

#include "stui.h"
#include "osal/osal.h"

void callback_topwin( STUI_WINDOW_T hWnd, unsigned int tl_row, unsigned int tl_col, unsigned int br_row, unsigned int br_col )
{
	unsigned int x, y;
   unsigned int w, h;
   
   stui_get_window_dims( hWnd, &w, &h);
   
   for ( x = 1; x < w-2; x++ )
   {
      stui_cb_putchar( hWnd, 0, x, '-' );
      stui_cb_putchar( hWnd, h-2, x, '-' );
   }
   
   for ( y = 1; y < h-2; y++ )
   {
      stui_cb_putchar( hWnd, y, 0, '|' );
      stui_cb_putchar( hWnd, y, w-2, '|' );
   }
   
   stui_cb_putchar( hWnd, 0, 0, '+' );
   stui_cb_putchar( hWnd, 0, w-2, '+' );
   stui_cb_putchar( hWnd, h-2, 0, '+' );
   stui_cb_putchar( hWnd, h-2, w-2, '+' );
	
	for ( y = 1; y < h-2; y++ )
		for ( x = 1; x < w-2; x++ )
			stui_cb_putchar( hWnd, y, x, 'X' );
}

void callback_rootwin( STUI_WINDOW_T hWnd, unsigned int tl_row, unsigned int tl_col, unsigned int br_row, unsigned int br_col )
{
	unsigned int x, y;
	
	for ( y = tl_row; y < br_row; y++ )
		for ( x = tl_col; x < br_col; x++ )
			stui_cb_putchar( hWnd, y, x, '.' );
}


void callback_hello( STUI_WINDOW_T hWnd, unsigned int tl_row, unsigned int tl_col, unsigned int br_row, unsigned int br_col )
{
	unsigned int x, y;
	
	for ( y = tl_row; y < br_row; y++ )
		for ( x = tl_col; x < br_col; x++ )
			stui_cb_putchar( hWnd, y, x, ' ' );
	
   stui_cb_printf( hWnd, 0, 0, 0, "%.20C#" );
   stui_cb_printf( hWnd, 9, 0, 0, "%.20C#" );
   for ( y = 1; y<9;y++)
	{
       stui_cb_putchar( hWnd, y, 0, '#' );
       stui_cb_putchar( hWnd, y, 19, '#');
	}
   
   for ( y = 1; y < 9; y++ )
   {
      stui_cb_printf( hWnd, y, y, STUI_ATTR_BOLD, "hello" );
   }
}

int main( void )
{
    STUI_WINDOW_T hWnd, rootwin, topwin;
    int err;
    int i,j;
    
    err = stui_server();
    if ( err )
    {
       printf(" oops " );
       return 1;
    }
    
    rootwin = stui_create_window( callback_rootwin );
    stui_resize_window( rootwin, 0, 0 );
    stui_show_window( rootwin );
    
    hWnd = stui_create_window( callback_hello );
    stui_move_window( hWnd, 3, 3 );
    stui_resize_window( hWnd, 20, 10 );
    stui_raise_window( hWnd );
    stui_show_window( hWnd );
    
    topwin = stui_create_window( callback_topwin );
    stui_move_window( topwin, 8, 8 );
    stui_resize_window( topwin, 8, 8 );
    stui_raise_window( topwin );
    stui_show_window( topwin );
    
#define DWELL_TIME   10

    //while(1)
    for(j=0;j<10;j++)
    {
        for ( i = 4; i < 20; i++ )
        {
           osal_task_sleep(DWELL_TIME);
	        stui_move_window( hWnd, i, i );
        }
        
        for ( i = 19; i > 4; i-- )
        {
           osal_task_sleep(DWELL_TIME);
           stui_move_window( hWnd, i, 38-i );
        }
        
        for ( i = 34; i > 4; i-- )
        {
           osal_task_sleep(DWELL_TIME);
           stui_move_window( hWnd, 4, i );
        }
    }
    
    
    getchar();
    
    stui_hide_window( hWnd );
    
    getchar();
    
    stui_destroy_window( hWnd );
    
    
    
}
