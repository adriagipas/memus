/*
 * Copyright 2014-2024 Adrià Giménez Pastor.
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


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "hud.h"
#include "icon.h"
#include "scalers2d.h"
#include "screen.h"
#include "windowfb.h"




/**********/
/* MACROS */
/**********/

#define PALSIZE 32768




/*********/
/* ESTAT */
/*********/

static struct
{
  
  int width;
  int height;
  int screen_size;
  
} _wsize;

static uint32_t _palette[PALSIZE];

/* Escalat. */
static struct
{
  
  int *buffer;
  void (*scaler) (const int *,int *,const int,const int);
  int width,height;
  int factor;
  
} _scaler;

/* Últim frame buffer. */
static int _last_fb[WIDTH*HEIGHT];




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
decode_screen_size (
        	    const int screen_size
        	    )
{
  
  switch ( screen_size )
    {
    case SCREEN_SIZE_WIN_X1:
      _wsize.width= WIDTH;
      _wsize.height= HEIGHT;
      break;
    case SCREEN_SIZE_WIN_X2:
      _wsize.width= WIDTH*2;
      _wsize.height= HEIGHT*2;
      break;
    case SCREEN_SIZE_WIN_X3:
      _wsize.width= WIDTH*3;
      _wsize.height= HEIGHT*3;
      break;
    case SCREEN_SIZE_WIN_X4:
      _wsize.width= WIDTH*4;
      _wsize.height= HEIGHT*4;
      break;
    case SCREEN_SIZE_FULLSCREEN:
      _wsize.width= 0;
      _wsize.height= 0;
      break;
    default:
      error ( "WTF!! decode_screen_size - screen_size desconegut" );
    }
  _wsize.screen_size= screen_size;
  
} /* end decode_screen_size */


static void
init_palette (void)
{
  
  double frac;
  int i;
  uint8_t r,g,b;
  
  
  frac= 255.0/31.0;
  for ( i= 0; i < PALSIZE; ++i )
    {
      r= (uint8_t) ((i&0x1F)*frac + 0.5);
      g= (uint8_t) (((i>>5)&0x1F)*frac + 0.5);
      b= (uint8_t) (((i>>10)&0x1F)*frac + 0.5);
      _palette[i]= windowfb_get_color ( r, g, b );
    }
  
} // end init_palette


static void
set_scaler (
            const int scaler
            )
{
  
  switch ( scaler )
    {
      
    case SCALER_SCALE2X:
      _scaler.factor= 2;      
      _scaler.width= WIDTH*2;
      _scaler.height= HEIGHT*2;
      _scaler.scaler= s2d_scale2x;
      _scaler.buffer= g_new0 ( int, _scaler.width*_scaler.height );
      
      break;
      
    case SCALER_NONE:
    default:
      _scaler.factor= 1;
      _scaler.width= WIDTH;
      _scaler.height= HEIGHT;
      _scaler.scaler= NULL;
      _scaler.buffer= NULL;
      break;
      
    }
  
} /* end set_scaler */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_screen (void)
{
  
  if ( _scaler.buffer != NULL )
    g_free ( _scaler.buffer );
  close_windowfb ();
  
} /* end close_screen */


void
init_screen (
             const conf_t   *conf,
             const char     *title,
             const gboolean  big_screen
             )
{
  
  /* Escalat. */
  set_scaler ( conf->scaler );
  
  /* Finestra. */
  if ( big_screen ) { _wsize.width= _wsize.height= 0; }
  else decode_screen_size ( conf->screen_size );
  init_windowfb ( _wsize.width, _wsize.height,
                  _scaler.width, _scaler.height,
                  title, ICON, conf->vsync );
  
  /* Paleta de colors. */
  init_palette ();

  /* Last frame buffer. */
  memset ( _last_fb, 0, sizeof(_last_fb) );
  
} /* end init_screen */


gboolean
screen_next_event (
        	   SDL_Event *event
        	   )
{

  gboolean ret;
  
  
  ret= windowfb_next_event ( event );
  if ( ret )
    {
      switch ( event->type )
        {
        case SDL_MOUSEMOTION:
          event->motion.x/= _scaler.factor;
          event->motion.y/= _scaler.factor;
          break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
          event->button.x/= _scaler.factor;
          event->button.y/= _scaler.factor;
          break;
        }
    }
  
  return ret;
  
} // end screen_next_event


void
screen_change_scaler (
        	      const int scaler
        	      )
{
  
  /* Allibera memòria. */
  if ( _scaler.buffer != NULL ) free ( _scaler.buffer );
  
  /* Crea nous buffers. */
  set_scaler ( scaler );
  windowfb_set_fbsize ( _scaler.width, _scaler.height );
  
} /* end screen_change_scaler */


void
screen_change_size (
        	    const int screen_size
        	    )
{
  
  if ( screen_size == _wsize.screen_size ) return;
  
  /* Obté noves dimensions i redimensiona. */
  decode_screen_size ( screen_size );
  windowfb_set_wsize ( _wsize.width, _wsize.height );
  
} /* end screen_change_size */


void
screen_change_title (
                     const char *title
                     )
{
  windowfb_set_title ( title );
} /* end screen_change_title */


void
screen_change_vsync (
                     const bool vsync
                     )
{
  windowfb_set_vsync ( vsync );
} // end screen_change_vsync


void
screen_update (
               const int  fb[WIDTH*HEIGHT],
               void      *udata
               )
{
  
  const int *hudfb;
  const int *auxfb;
  

  memcpy ( _last_fb, fb, sizeof(_last_fb) );
  hudfb= hud_update_fb ( fb );
  if ( _scaler.scaler == NULL ) auxfb= hudfb;
  else
    {
      _scaler.scaler ( hudfb, _scaler.buffer, WIDTH, HEIGHT );
      auxfb= _scaler.buffer;
    }
  windowfb_update ( auxfb, _palette );
  
} /* end screen_update */


void
screen_show_error (
                   const char *title,
                   const char *message
                   )
{
  windowfb_show_error ( title, message );
} /* end screen_show_error */


const int *
screen_get_last_fb (void)
{
  return _last_fb;
} /* end screen_get_last_fb */


void
screen_enable_cursor (
                      const bool enable
                      )
{
  windowfb_enable_cursor ( enable, false );
} // end screen_enable_cursor
