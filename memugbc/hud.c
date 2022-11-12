/*
 * Copyright 2015-2022 Adrià Giménez Pastor.
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
 *  hud.c - Implementació de 'hud.h'.
 *
 */


#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "effects.h"
#include "hud.h"
#include "tiles8b.h"




/**********/
/* MACROS */
/**********/

#define FB_WIDTH 160
#define FB_HEIGHT 144

#define MSG_HEIGHT 11




/*********/
/* ESTAT */
/*********/

/* Frame buffer. */
static int _fb[FB_WIDTH*FB_HEIGHT];

/* Estil. */
static struct
{
  
  int fgcolor;
  int linecolor;
  
} _style;


/* Estat missatge. */
static struct
{

  int fb[FB_WIDTH*MSG_HEIGHT];
  enum {
    MSG_HIDDEN,
    MSG_FADEIN,
    MSG_VISIBLE,
    MSG_FADEOUT
  } state;
  int counter;  /* Contador de frames en l'actual estat. */
  int visible;  /* Número de línies visibles. */
  
} _msg;


/* Estat flash. */
static struct
{

  enum {
    FLS_STOP,
    FLS_FADEIN,
    FLS_WAIT,
    FLS_FADEOUT
  } state;
  int counter; /* Contador de frames en l'actual estat. */
  
} _flash;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
clear_msg_fb (void)
{

  int i;


  for ( i= 0; i < FB_WIDTH*10; ++i )
    _msg.fb[i]= -1;
  for ( ; i < FB_WIDTH*MSG_HEIGHT; ++i )
    _msg.fb[i]= _style.linecolor;
  
} /* end clear_msg_fb */


static void
render_msg (void)
{

  const int step_size= 2;
  const int step_fadein= 2;
  const int time_visible= 60;
  const int step_fadeout= 2;

  int i, aux;
  const int *src;
  

  /* Actualitza l'estat. */
  switch ( _msg.state )
    {
    case MSG_FADEIN:
      if ( ++_msg.counter == step_fadein )
        {
          _msg.counter= 0;
          _msg.visible+= step_size;
          if ( _msg.visible >= MSG_HEIGHT )
            {
              _msg.visible= MSG_HEIGHT;
              _msg.state= MSG_VISIBLE;
            }
        }
      break;
    case MSG_VISIBLE:
      if ( ++_msg.counter == time_visible )
        {
          _msg.counter= 0;
          _msg.state= MSG_FADEOUT;
        }
      break;
    case MSG_FADEOUT:
      if ( ++_msg.counter == step_fadeout )
        {
          _msg.counter= 0;
          _msg.visible-= step_size;
          if ( _msg.visible <= 0 )
            {
              _msg.visible= 0;
              _msg.state= MSG_HIDDEN;
            }
        }
      break;
    default: break;
    }
  
  /* Dibuixa. */
  effect_fade ( _fb, 0, 0, FB_WIDTH, _msg.visible, 0.8 );
  aux= (MSG_HEIGHT-_msg.visible)*FB_WIDTH;
  src= &(_msg.fb[aux]);
  aux= _msg.visible*FB_WIDTH;
  for ( i= 0; i < aux; ++i )
    if ( src[i] != -1 )
      _fb[i]= src[i];
  
} /* end render_msg */


static void
render_flash (void)
{

  const int fadein_frames= 4;
  const int time_wait= 3;
  const int fadeout_frames= 4;

  double factor;
  
  
  /* Actualitza l'estat. */
  factor= 0.0;
  switch ( _flash.state )
    {
    case FLS_FADEIN:
      factor= _flash.counter*(1.0/fadein_frames);
      if ( ++_flash.counter == fadein_frames )
        {
          _flash.state= FLS_WAIT;
          _flash.counter= 0;
        }
      break;
    case FLS_WAIT:
      factor= 1.0;
      if ( ++_flash.counter == time_wait )
        {
          _flash.state= FLS_FADEOUT;
          _flash.counter= 0;
        }
      break;
    case FLS_FADEOUT:
      factor= 1.0 - _flash.counter*(1.0/fadeout_frames);
      if ( ++_flash.counter == fadeout_frames )
        {
          _flash.state= FLS_STOP;
          _flash.counter= 0;
        }
      break;
    default: break;
    }
  
  /* Dibuixa. */
  effect_interpolate_color ( _fb, 0, 0, FB_WIDTH, FB_HEIGHT, 0x7FFF, factor );
  
} /* end render_flash */ 




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
init_hud (void)
{

  /* Estil. */
  _style.fgcolor= 0x7FFF; /* Blanc. */
  _style.linecolor= 0x001F; /* Roig. */

  /* Missatge. */
  _msg.state= MSG_HIDDEN;

  /* Flash. */
  _flash.state= FLS_STOP;
  
} /* end init_hud */


const int *
hud_update_fb (
               const int *fb
               )
{

  /* Comprova que cal fer algo. */
  if ( _msg.state == MSG_HIDDEN &&
       _flash.state == FLS_STOP ) return fb;
  
  /* Renderitza. */
  memcpy ( _fb, fb, FB_WIDTH*FB_HEIGHT*sizeof(int) );
  if ( _msg.state != MSG_HIDDEN ) render_msg ();
  if ( _flash.state != FLS_STOP ) render_flash ();
  
  return _fb;
  
} /* end hud_update_fb */


void
hud_show_msg (
              const char *msg
              )
{

  /* Renderitza el text. */
  clear_msg_fb ();
  tiles8b_draw_string ( &(_msg.fb[FB_WIDTH*1]), FB_WIDTH, msg, 1, 0,
        		_style.fgcolor, 0, BG_TRANS );

  /* Prepara. */
  _msg.visible= 0;
  _msg.counter= 0;
  _msg.state= MSG_FADEIN;
  
} /* end hud_show_msg */


void
hud_flash (void)
{

  _flash.state= FLS_FADEIN;
  _flash.counter= 0;
  
} /* end hud_flash */


void
hud_hide (void)
{
  
  _msg.state= MSG_HIDDEN;
  _flash.state= FLS_STOP;
  
} /* end hud_hide */
