/*
 * Copyright 2020-2024 Adrià Giménez Pastor.
 *
 * This file is part of adriagipas/memus.
 *
 * adriagipas/memus is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * adriagipas/memus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  screen.c - Implementació de 'screen.h'.
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "error.h"
#include "icon.h"
#include "screen.h"
#include "windowfb.h"




/*********/
/* ESTAT */
/*********/

static struct
{
  int  screen_size;
  int  width,height;
} _wsize;

static uint32_t _palette[SCREEN_PAL_SIZE];




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
decode_screen_size (
                    const int screen_size
                    )
{

  
  // Fixa grandaria base
  _wsize.width= SCREEN_WIDTH;
  _wsize.height= SCREEN_HEIGHT;

  // Decodifica screen_size.
  switch ( screen_size )
    {
    case SCREEN_SIZE_WIN_X1:
      break;
    case SCREEN_SIZE_WIN_X1_5:
      _wsize.width=  (int) (_wsize.width*1.5);
      _wsize.height= (int) (_wsize.height*1.5);
      break;
    case SCREEN_SIZE_WIN_X2:
      _wsize.width*= 2;
      _wsize.height*= 2;
      break;
    case SCREEN_SIZE_FULLSCREEN:
      _wsize.width= 0;
      _wsize.height= 0;
      break;
    default:
      error ( "WTF!! decode_screen_size - screen_size desconegut" );
    }

  // Desa valors.
  _wsize.screen_size= screen_size;
  
} // end decode_screen_size


static void
init_palette (void)
{
  
  _palette[SCREEN_PAL_WHITE]= windowfb_get_color ( 0xff, 0xff, 0xff );
  _palette[SCREEN_PAL_BLACK]= windowfb_get_color ( 0x00, 0x00, 0x00 );
  _palette[SCREEN_PAL_RED]= windowfb_get_color ( 0xff, 0x00, 0x00 );
  _palette[SCREEN_PAL_RED_2]= windowfb_get_color ( 0xbf, 0x00, 0x00 );
  _palette[SCREEN_PAL_RED_3]= windowfb_get_color ( 0x7f, 0x00, 0x00 );
  _palette[SCREEN_PAL_GRAY_1]= windowfb_get_color ( 0xee, 0xee, 0xee );
  _palette[SCREEN_PAL_GRAY_2]= windowfb_get_color ( 0xdd, 0xdd, 0xdd );
  _palette[SCREEN_PAL_GRAY_3]= windowfb_get_color ( 0xcc, 0xcc, 0xcc );
  _palette[SCREEN_PAL_GRAY_4]= windowfb_get_color ( 0xbb, 0xbb, 0xbb );
  _palette[SCREEN_PAL_GRAY_5]= windowfb_get_color ( 0x33, 0x33, 0x33 );
  //_palette[SCREEN_PAL_BLUE_LP1]= windowfb_get_color ( 0xc1, 0xe9, 0xf3 );
  _palette[SCREEN_PAL_BLUE_LP1]= windowfb_get_color ( 0xcb, 0xf3, 0xfd );
  _palette[SCREEN_PAL_BLUE_LP1B]= windowfb_get_color ( 0xb7, 0xdf, 0xe9 );
  //_palette[SCREEN_PAL_BLUE_LP2]= windowfb_get_color ( 0x67, 0xb1, 0xe5 );
  _palette[SCREEN_PAL_BLUE_LP2]= windowfb_get_color ( 0x71, 0xbb, 0xef );
  _palette[SCREEN_PAL_BLUE_LP2B]= windowfb_get_color ( 0x5d, 0xa7, 0xdb );
  _palette[SCREEN_PAL_GREEN]= windowfb_get_color ( 0x00, 0xff, 0x00 );
  
} // end init_palette




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_screen (void)
{
  close_windowfb ();
} // end close_screen


void
init_screen (
             const conf_t *conf
             )
{
  
  
  // Finestra.
  decode_screen_size ( conf->screen_size );
  init_windowfb ( _wsize.width, _wsize.height,
                  SCREEN_WIDTH, SCREEN_HEIGHT,
                  "memuMIX", ICON, conf->vsync );
  windowfb_enable_cursor ( true, true );

  // Paleta de colors.
  init_palette ();

  windowfb_show ();
  
} // end init_screen


bool
screen_next_event (
        	   SDL_Event *event
        	   )
{
  return windowfb_next_event ( event );
} // end screen_next_event


void
screen_change_size (
        	    const int screen_size
        	    )
{
  
  if ( screen_size == _wsize.screen_size ) return;
  
  // Obté noves dimensions i redimensiona.
  decode_screen_size ( screen_size );
  windowfb_set_wsize ( _wsize.width, _wsize.height );
  
} // end screen_change_size


void
screen_update (
               const int *fb
               )
{
  windowfb_update ( fb, _palette );  
} // end screen_update


void
screen_show_error (
                   const char *title,
                   const char *message
                   )
{
  windowfb_show_error ( title, message );
} // end screen_show_error
