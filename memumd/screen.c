/*
 * Copyright 2014-2025 Adrià Giménez Pastor.
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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  screen.c - Implementació de 'screen.h'.
 *
 */


#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "MD.h"
#include "error.h"
#include "hud.h"
#include "icon.h"
#include "lock.h"
#include "scalers2d.h"
#include "screen.h"
#include "windowfb.h"




/**********/
/* MACROS */
/**********/

#define PALSIZE (0x600)

#define WIDTH 320
#define HEIGHT 240
#define HEIGHT_PAL 240
#define HEIGHT_NTSC 224
#define MAXWIDTH (WIDTH*2)
#define MAXHEIGHT (HEIGHT*2)




/*********/
/* ESTAT */
/*********/

static struct
{
  int screen_size;
  int width,height;
} _wsize;

static uint32_t _palette[PALSIZE];

/* Resolució actual. */
static struct
{
  int width,height;
} _res;

/* Escalat. */
static struct
{
  
  int id;
  int *buffer;
  void (*scaler) (const int *,int *,const int,const int);
  int width,height;
  
} _scaler;

/* Últim framebuffer. */
static int _last_fb[MAXWIDTH*MAXHEIGHT];




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
  
  MD_RGB ret;
  int i;
  
  
  for ( i= 0; i < PALSIZE; ++i )
    {
      ret= MD_color2rgb ( i );
      _palette[i]= windowfb_get_color ( ret.r, ret.g, ret.b );
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
      _scaler.width= _res.width*2;
      _scaler.height= _res.height*2;
      _scaler.scaler= s2d_scale2x;
      _scaler.buffer= g_new0 ( int, _scaler.width*_scaler.height );
      break;
      
    case SCALER_NONE:
    default:
      _scaler.width= _res.width;
      _scaler.height= _res.height;
      _scaler.scaler= NULL;
      _scaler.buffer= NULL;
      break;
      
    }
  _scaler.id= scaler;
  
} /* end set_scaler */


static void
update_scaler_and_buffer (
                          const int scaler
                          )
{
  
  /* Allibera memòria. */
  if ( _scaler.buffer != NULL ) free ( _scaler.buffer );
  
  /* Crea nous buffers. */
  set_scaler ( scaler );
  windowfb_set_fbsize ( _scaler.width, _scaler.height );
  
} /* end update_scaler_and_buffer */




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
  _res.width= WIDTH;
  _res.height= HEIGHT_PAL;
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

  windowfb_show ();
  
} /* end init_screen */


gboolean
screen_next_event (
        	   SDL_Event *event
        	   )
{
  
  gboolean ret;


  if ( lock_check_signals () )
    windowfb_raise ();
  
  ret= windowfb_next_event ( event );
  
  return ret;
  
} // end screen_next_event


void
screen_change_scaler (
        	      const int scaler
        	      )
{
  
  if ( scaler == _scaler.id ) return;
  update_scaler_and_buffer ( scaler );
  
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


const int *
screen_get_last_fb (void)
{
  return _last_fb;
} /* end screen_get_last_fb */


void
screen_get_res (
                int *width,
                int *height
                )
{
  
  *width= _res.width;
  *height= _res.height;
  
} /* end screen_get_res */


void
screen_sres_changed (
                     const int  width,
                     const int  height,
                     void      *udata
                     )
{
  
  if ( _res.width == width && _res.height == height ) return;
  _res.width= width;
  _res.height= height;
  update_scaler_and_buffer ( _scaler.id );
  
} /* end screen_sres_changed */


void
screen_update (
               const int  fb[],
               void      *udata
               )
{

  const int *auxfb;
  const int *hudfb;
  
  
  memcpy ( _last_fb, fb, _res.width*_res.height*sizeof(int) );
  hudfb= hud_update_fb ( fb, _res.width, _res.height );
  if ( _scaler.scaler == NULL ) auxfb= hudfb;
  else
    {
      _scaler.scaler ( hudfb, _scaler.buffer, _res.width, _res.height );
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
