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
 *  ui_button.c - Implementació de 'ui_button.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"
#include "ui.h"
#include "ui_button.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define BORDER 2
#define PADDING 2




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
callback_release_key (
                      void *user_data
                      )
{

  ui_button_t *self;

  
  self= UI_BUTTON(user_data);
  self->_cb_release_key= -1;
  self->_state= UI_BUTTON_RELEASED;

  return false;
  
} // end callback_release_key


static void
reset_state (
             ui_button_t *self
             )
{

  if ( self->_cb_release_key != -1 )
    {
      ui_remove_callback ( self->_cb_release_key );
      self->_cb_release_key= -1;
    }
  self->_state= UI_BUTTON_RELEASED;
  
} // end reset_state




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_button_t *self;


  self= UI_BUTTON(s);
  if ( self->_cb_release_key != -1 )
    ui_remove_callback ( self->_cb_release_key );
  g_free ( self->_label );
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

  ui_button_t *self;
  int border_color,bg_color,fg_color,off,r,c,i,j;
  

  self= UI_BUTTON(s);
  if ( self->visible )
    {

      // Colors
      switch ( self->_state )
        {
        default:
        case UI_BUTTON_RELEASED:
          if ( has_focus )
            {
              border_color= SCREEN_PAL_GRAY_5;
              fg_color= SCREEN_PAL_GRAY_5;
              bg_color= SCREEN_PAL_WHITE;
            }
          else
            {
              border_color= SCREEN_PAL_GRAY_3;
              fg_color= SCREEN_PAL_GRAY_3;
              bg_color= SCREEN_PAL_WHITE;
            }
          break;
        case UI_BUTTON_PRESSED_MOUSE:
        case UI_BUTTON_PRESSED_KEY:
          border_color= SCREEN_PAL_GRAY_5;
          fg_color= SCREEN_PAL_RED;
          bg_color= SCREEN_PAL_GRAY_5;
          break;
        }

      // Dibuixa border superior
      for ( r= self->_y, i= 0; i < BORDER; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < self->_w; ++c )
            fb[off+c]= border_color;
        }

      // Dibuixa part central
      for ( i= 0; i < 16 + 2*PADDING; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          // Border esquerra
          for ( c= 0, j= 0; j < BORDER; ++j,++c )
            fb[off+c]= border_color;
          // Padding
          for ( j= 0; j < self->_label_w + 2*PADDING; ++j,++c )
            fb[off+c]= bg_color;
          // Border dreta
          for ( j= 0; j < BORDER; ++j,++c )
            fb[off+c]= border_color;
        }
      
      // Dibuixa border inferior
      for ( i= 0; i < BORDER; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < self->_w; ++c )
            fb[off+c]= border_color;
        }

      // Dibuixa el text
      vgafont_draw_string ( fb, fb_width, self->_label,
                            BORDER+PADDING+self->_x,
                            BORDER+PADDING+self->_y,
                            fg_color, bg_color,
                            VGAFONT_XY_PIXELS );
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_button_t *self;
  

  self= UI_BUTTON(s);
  if ( !self->visible ) return false;
  switch ( event->type )
    {
      // Este event encara que afecta al botó, no l'elimine perquè a
      // altres elements li podria afectar.
    case SDL_MOUSEMOTION:
      if ( self->_state == UI_BUTTON_PRESSED_MOUSE &&
           (event->motion.x < self->_x ||
            event->motion.x >= (self->_x+self->_w) ||
            event->motion.y < self->_y ||
            event->motion.y >= (self->_y+self->_h)) )
        reset_state ( self );
      break;
    case SDL_MOUSEBUTTONDOWN:
      reset_state ( self );
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
        {
          if ( event->button.button == 1 )
            self->_state= UI_BUTTON_PRESSED_MOUSE;
          return true;
        }
      break;
    case SDL_MOUSEBUTTONUP:
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
        {
          if ( self->_state == UI_BUTTON_PRESSED_MOUSE &&
               event->button.button == 1 )
            {
              self->_state= UI_BUTTON_RELEASED;
              if ( self->_action != NULL )
                self->_action ( UI_ELEMENT(self), self->_udata );
            }
          return true;
        }
      else reset_state ( self );
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
  ui_button_t *self;

  
  self= UI_BUTTON(s);
  if ( event->type == SDL_KEYDOWN &&
       (event->key.keysym.sym == SDLK_SPACE ||
        event->key.keysym.sym == SDLK_RETURN) )
    {
      reset_state ( self );
      if ( self->_action != NULL )
        self->_action ( UI_ELEMENT(self), self->_udata );
      self->_state= UI_BUTTON_PRESSED_KEY;
      self->_cb_release_key=
        ui_register_callback ( callback_release_key, self, 100000 );
      ret= true;
    }
  else ret= false;
  
  return ret;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_button_t *
ui_button_new (
               const char  *label,
               const int    x,
               const int    y,
               ui_action_t *action,
               void        *udata
               )
{
  
  ui_button_t *new;
  
  
  // Reserva memòria.
  assert ( x >= 0 && y >= 0 );
  new= g_new ( ui_button_t, 1 );
  new->_label= g_strdup ( label );
  new->_x= x;
  new->_y= y;
  new->_label_w= 9*strlen ( label );
  new->_w= new->_label_w + 2*(BORDER+PADDING);
  new->_h= 16 + 2*(BORDER+PADDING);
  new->_state= UI_BUTTON_RELEASED;
  new->_action= action;
  new->_udata= udata;
  new->_cb_release_key= -1;
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;
  
  return new;
  
} // end ui_button_new
