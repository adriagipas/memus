/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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
 *  ui_input.c - Implementació de 'ui_input.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "screen.h"
#include "ui.h"
#include "ui_input.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define BORDER 1
#define PADDING 2




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
callback_show_cursor (
                      void *user_data
                      )
{

  ui_input_t *self;

  
  self= UI_INPUT(user_data);
  self->_show_cursor= !self->_show_cursor;
  
  return true;
  
} // end callback_show_cursor




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{
  
  ui_input_t *self;
  
  
  self= UI_INPUT(s);
  if ( self->_cb_show_cursor != -1 )
    ui_remove_callback ( self->_cb_show_cursor );
  g_free ( self->_buf );
  g_free ( self );
  
} // end free_


static void
draw (
      ui_element_t *s,
      int          *fb,
      const int     fb_width,
      const bool    has_focus
      )
{

  ui_input_t *self;
  int off,r,i,c,j;
  

  self= UI_INPUT(s);
  if ( self->visible )
    {

      // Dibuixa caixa
      // --> Borera superior
      for ( r= self->_y, i= 0; i < BORDER; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < self->_w; ++c )
            fb[off+c]= SCREEN_PAL_BLACK;
        }
      // --> Cos central
      for ( i= 0; i < (16+PADDING*2); ++i, ++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < BORDER; ++c )
            fb[off+c]= SCREEN_PAL_BLACK;
          for ( j= 0; j < (self->_w-2*BORDER); ++j, ++c )
            fb[off+c]= SCREEN_PAL_WHITE;
          for ( j= 0; j < BORDER; ++j, ++c )
            fb[off+c]= SCREEN_PAL_BLACK;
        }
      // --> Borera inferior
      for ( i= 0; i < BORDER; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < self->_w; ++c )
            fb[off+c]= SCREEN_PAL_BLACK;
        }

      // Escriu text
      vgafont_draw_string ( fb, fb_width, self->_buf,
                            BORDER+PADDING+self->_x,
                            BORDER+PADDING+self->_y,
                            SCREEN_PAL_BLACK, SCREEN_PAL_WHITE,
                            VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );

      // Cursor
      if ( has_focus )
        {
          if ( self->_cb_show_cursor == -1 )
            self->_cb_show_cursor=
              ui_register_callback ( callback_show_cursor, self, 400000 );
          if ( self->_show_cursor )
            vgafont_draw_string ( fb, fb_width, "\xdb",
                                  BORDER+PADDING+self->_x+(self->_N)*9,
                                  BORDER+PADDING+self->_y,
                                  SCREEN_PAL_BLACK, SCREEN_PAL_WHITE,
                                  VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
        }
      else if ( self->_cb_show_cursor != -1 )
        {
          ui_remove_callback ( self->_cb_show_cursor );
          self->_cb_show_cursor= -1;
        }
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_input_t *self;
  

  self= UI_INPUT(s);
  if ( !self->visible ) return false;
  switch ( event->type )
    {
    case SDL_MOUSEBUTTONDOWN: // Agafa focus
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
        {
          return true;
        }
      break;
    }
  
  return false;
  
} // end mouse_event


static bool
key_event (
           ui_element_t    *s,
           const SDL_Event *event
           )
{

  bool ret;
  ui_input_t *self;
  char c;
  
  
  self= UI_INPUT(s);
  if ( event->type == SDL_KEYDOWN && event->key.keysym.mod == 0 )
    {
      switch ( event->key.keysym.sym )
        {
        case SDLK_a: c= 'A'; break;
        case SDLK_b: c= 'B'; break;
        case SDLK_c: c= 'C'; break;
        case SDLK_d: c= 'D'; break;
        case SDLK_e: c= 'E'; break;
        case SDLK_f: c= 'F'; break;
        case SDLK_g: c= 'G'; break;
        case SDLK_h: c= 'H'; break;
        case SDLK_i: c= 'I'; break;
        case SDLK_j: c= 'J'; break;
        case SDLK_k: c= 'K'; break;
        case SDLK_l: c= 'L'; break;
        case SDLK_m: c= 'M'; break;
        case SDLK_n: c= 'N'; break;
        case SDLK_o: c= 'O'; break;
        case SDLK_p: c= 'P'; break;
        case SDLK_q: c= 'Q'; break;
        case SDLK_r: c= 'R'; break;
        case SDLK_s: c= 'S'; break;
        case SDLK_t: c= 'T'; break;
        case SDLK_u: c= 'U'; break;
        case SDLK_v: c= 'V'; break;
        case SDLK_w: c= 'W'; break;
        case SDLK_x: c= 'X'; break;
        case SDLK_y: c= 'Y'; break;
        case SDLK_z: c= 'Z'; break;
        case SDLK_UNDERSCORE: c= '_'; break;
        case SDLK_MINUS: c= '-'; break;
        case SDLK_PERIOD: c= '.'; break;
        case SDLK_0: c= '0'; break;
        case SDLK_1: c= '1'; break;
        case SDLK_2: c= '2'; break;
        case SDLK_3: c= '3'; break;
        case SDLK_4: c= '4'; break;
        case SDLK_5: c= '5'; break;
        case SDLK_6: c= '6'; break;
        case SDLK_7: c= '7'; break;
        case SDLK_8: c= '8'; break;
        case SDLK_9: c= '9'; break;
        case SDLK_BACKSPACE: c= '\b'; break;
        case SDLK_RETURN: c= '\r'; break;
        default: c= '\0';
        }
      if ( c != '\0')
        {
          if ( c == '\r' )
            {
              if ( self->_action != NULL )
                self->_action ( UI_ELEMENT(self), self->_udata );
            }
          else if ( c == '\b' )
            {
              if ( self->_N > 0 )
                self->_buf[--self->_N]= '\0';
            }
          else if ( self->_N < self->_length )
            {
              self->_buf[self->_N++]= c;
              self->_buf[self->_N]= '\0';
            }
          ret= true;
        }
      else ret= false;
    }
  else ret= false;
  
  return ret;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_input_t *
ui_input_new (
              const int    x,
              const int    y,
              const int    length, // Longitut en caràcters
              ui_action_t *action,
              void        *udata
              )
{

  ui_input_t *new;
  
  
  // Reserva memòria.
  assert ( x >= 0 && y >= 0 );
  assert ( length > 0 );
  new= g_new ( ui_input_t, 1 );
  new->_x= x;
  new->_y= y;
  new->_w= 2*BORDER + 2*PADDING + (length+1)*9;
  new->_h= 2*BORDER + 2*PADDING + 16;
  new->_length= length;
  new->_N= 0;
  new->_buf= g_new ( char, length+1 );
  new->_buf[0]= '\0';
  new->_action= action;
  new->_udata= udata;
  new->_cb_show_cursor= -1;
  new->_show_cursor= false;
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;
  
  return new;
  
} // end ui_input_new


const char *
ui_input_get_text (
                   ui_input_t *self
                   )
{
  return self->_N==0 ? NULL : self->_buf;
} // end ui_input_get_text


void
ui_input_clear (
                ui_input_t *self
                )
{

  self->_N= 0;
  self->_buf[0]= '\0';
  
} // end ui_input_clear


void
ui_input_set_text (
                   ui_input_t *self,
                   const char *text
                   )
{

  int length,i;
  
  
  length= strlen ( text );
  length= MIN ( length, self->_length );
  for ( i= 0; i < length; ++i )
    self->_buf[i]= text[i];
  self->_buf[i]= '\0';
  self->_N= length;
  
} // end ui_input_set_text
