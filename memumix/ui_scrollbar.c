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
 *  ui_scrollbar.c - Implementació de 'ui_scrollbar.h'.
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
#include "ui_scrollbar.h"




/**********/
/* MACROS */
/**********/

#define THICKNESS 5

#define BG_COLOR SCREEN_PAL_GRAY_4
#define FG_COLOR SCREEN_PAL_RED




/*********************/
/* FUNCIONS PRIVADES */
/*********************/


static void
recalc_sel (
            ui_scrollbar_t *self
            )
{

  double esize;
  int tmp;
  
  
  esize= self->_length / (double) self->_N;
  self->_sel_pos= (int) (esize*self->_pos + 0.5);
  self->_sel_length= (int) (esize*self->_wsize + 0.5);
  tmp= self->_sel_pos + self->_sel_length;
  if ( tmp > self->_length )
    self->_sel_length-= (tmp-self->_length);
  
} // end recalc_sel


static void
move_sel (
          ui_scrollbar_t *self,
          const int       mouse_x,
          const int       mouse_y
          )
{

  int diff,new_pos,inc,old_pos;
  double esize;
  bool sub;
  

  // Calcula la nova posició
  esize= self->_length / (double) self->_N;
  diff= (self->_vertical ? mouse_y : mouse_x) - self->_old_pos_cord;
  if ( diff < 0 ) { diff= -diff; sub= true; }
  else sub= false;
  inc= (int) (diff/esize + 0.5);
  new_pos= sub ? (self->_old_pos - inc) : (self->_old_pos + inc);
  if ( new_pos < 0 ) new_pos= 0;
  else if ( (new_pos + self->_wsize) > self->_N )
    new_pos= self->_N-self->_wsize;

  // Actualitza el valor i crida acció si hi ha.
  old_pos= self->_pos;
  self->_pos= new_pos;
  if ( new_pos != old_pos )
    {
      recalc_sel ( self );
      if ( self->_action != NULL )
        self->_action ( UI_ELEMENT(self), self->_udata );
    }
  
} // end move_sel




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *self
       )
{
  g_free ( self );
} // end free


static void
draw (
      ui_element_t *s,
      int          *fb,
      const int     fb_width,
      const bool    has_focus
      )
{

  ui_scrollbar_t *self;
  int r,c,off;
  

  self= UI_SCROLLBAR(s);
  if ( self->visible )
    {

      // Vertical
      if ( self->_vertical )
        {
          
          // Fons
          for ( r= self->_y; r < self->_y+self->_length; ++r )
            {
              off= r*fb_width + self->_x;
              for ( c= 0; c < THICKNESS; ++c )
                fb[off+c]= BG_COLOR;
            }

          // Selecció
          if ( self->_wsize < self->_N )
            for ( r= self->_y+self->_sel_pos;
                  r < self->_y+self->_sel_pos+self->_sel_length;
                  ++r )
              {
                off= r*fb_width + self->_x;
                for ( c= 0; c < THICKNESS; ++c )
                fb[off+c]= FG_COLOR;
              }
            
        }

      // Horizontal
      else
        {

          // Fons
          for ( r= self->_y; r < self->_y+THICKNESS; ++r )
            {
              off= r*fb_width + self->_x;
              for ( c= 0; c < self->_length; ++c )
                fb[off+c]= BG_COLOR;
            }

          // Selecció
          if ( self->_wsize < self->_N )
            for ( r= self->_y; r < self->_y+THICKNESS; ++r )
              {
                off= r*fb_width + self->_x+self->_sel_pos;
                for ( c= 0; c < self->_sel_length; ++c )
                  fb[off+c]= FG_COLOR;
              }
          
        }
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_scrollbar_t *self;
  

  self= UI_SCROLLBAR(s);
  switch ( event->type )
    {
    case SDL_MOUSEMOTION:
      if ( self->_pressed )
        {
          if ( event->motion.state&SDL_BUTTON_LMASK )
            {
              move_sel ( self, event->motion.x, event->motion.y );
              return true;
            }
          else self->_pressed= false; // No consumisc l'event
        }
      break;
    case SDL_MOUSEBUTTONDOWN:
      if ( self->_N > self->_wsize )
        {
          // Cas vertical
          if ( self->_vertical )
            {
              if ( event->button.x >= self->_x &&
                   event->button.x < (self->_x+THICKNESS) &&
                   event->button.y >= (self->_y+self->_sel_pos) &&
                   event->button.y < (self->_y+self->_sel_pos+
                                      self->_sel_length) )
                {
                  self->_pressed= true;
                  self->_old_pos= self->_pos;
                  self->_old_pos_cord= event->button.y;
                  return true;
                }
            }
          // Cas horizontal
          else
            {
              if ( event->button.x >= (self->_x+self->_sel_pos) &&
                   event->button.x < (self->_x+self->_sel_pos+
                                      self->_sel_length) &&
                   event->button.y >= self->_y &&
                   event->button.y < (self->_y+THICKNESS) )
                {
                  self->_pressed= true;
                  self->_old_pos= self->_pos;
                  self->_old_pos_cord= event->button.x;
                  return true;
                }
            }
        }
      break;
    case SDL_MOUSEBUTTONUP:
      self->_pressed= false;
      return false; // No consumisc
      break;
    }
  
  return false;

} // end mouse_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_scrollbar_t *
ui_scrollbar_new (
                  const int    x,
                  const int    y,
                  const int    length,
                  const bool   is_vertical,
                  ui_action_t *action,
                  void        *user_data
                  )
{

  ui_scrollbar_t *new;
  
  
  // Reserva memòria.
  assert ( x >= 0 && y >= 0 && length > 0 );
  new= g_new ( ui_scrollbar_t, 1 );
  new->_x= x;
  new->_y= y;
  new->_length= length;
  new->_vertical= is_vertical;
  new->_action= action;
  new->_udata= user_data;
  new->_N= 1;
  new->_wsize= 1;
  new->_pos= 0;
  new->_pressed= false;
  new->_old_pos= -1;
  new->_old_pos_cord= -1;
  recalc_sel ( new );
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= NULL;
  new->visible= true;
  
  return new;
  
} // end ui_scrollbar_new


void
ui_scrollbar_set_state (
                        ui_scrollbar_t *self,
                        const int       num_element,
                        const int       window_size,
                        const int       pos
                        )
{

  if ( num_element <= 0 )
    error ( "ui_scrollbar_set_state - num_element <= 0" );
  if ( window_size <= 0 )
    error ( "ui_scrollbar_set_state - window_size <= 0" );
  if ( window_size > num_element )
    error ( "ui_scrollbar_set_state - window_size > num_element" );
  if ( pos < 0 || pos >= num_element )
    error ( "ui_scrollbar_set_state - pos < 0 || pos >= num_element" );
  if ( (pos + window_size) > num_element )
    error ( "ui_scrollbar_set_state - (pos + window_size) > num_element" );
  self->_N= num_element;
  self->_wsize= window_size;
  self->_pos= pos;
  recalc_sel ( self );

  // Si la nova estructura fa impossible la última selecció aleshores
  // la desactivem.
  if ( self->_old_pos >= self->_N )
    self->_pressed= false;
  
} // end ui_scrollbar_set_state
