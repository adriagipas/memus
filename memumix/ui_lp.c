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
 *  ui_lp.c - Implementació de 'ui_lp.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "screen.h"
#include "ui_lp.h"
#include "ui_lp_bg.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define WIDTH_CHARS 60
#define HEIGHT_CHARS 23

#define SEP_PIXELS 2




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
recalc_cline (
              ui_lp_t *self
              )
{

  if ( self->_cline > (self->_sim_state->lp.N-HEIGHT_CHARS) )
    self->_cline= self->_sim_state->lp.N-HEIGHT_CHARS;
  if ( self->_cline < 0 ) self->_cline= 0;
  
} // end recalc_cline


static void
vsb_action (
            ui_element_t *e,
            void         *udata
            )
{

  ui_scrollbar_t *sb;
  ui_lp_t *lp;


  sb= UI_SCROLLBAR(e);
  lp= UI_LP(udata);
  lp->_cline= ui_scrollbar_get_pos ( sb );
  recalc_cline ( lp );
  
} // end vsb_action


static void
hsb_action (
            ui_element_t *e,
            void         *udata
            )
{

  ui_scrollbar_t *sb;
  ui_lp_t *lp;


  sb= UI_SCROLLBAR(e);
  lp= UI_LP(udata);
  lp->_ccol= ui_scrollbar_get_pos ( sb );
  
} // end hsb_action




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_lp_t *self;
  
  
  self= UI_LP(s);
  ui_element_free ( self->_vsb );
  ui_element_free ( self->_hsb );
  
} // end free_


static void
draw (
      ui_element_t *s,
      int          *fb,
      const int     fb_width,
      const bool    has_focus
      )
{

  ui_lp_t *self;
  int colors[2],colorsbg[2],i,j,r,off,c,line,beg_c,end_c,color,colorbg,
    bg_x,bg_y;
  char buf[12],tmp_char;
  

  
  self= UI_LP(s);
  if ( self->visible )
    {

      // Recalcula cline
      recalc_cline ( self );
      
      // Dibuixa fons línies
      if ( self->_cline%2 == 0 )
        {
          colors[0]= SCREEN_PAL_BLUE_LP1;
          colors[1]= SCREEN_PAL_BLUE_LP2;
          colorsbg[0]= SCREEN_PAL_BLUE_LP1B;
          colorsbg[1]= SCREEN_PAL_BLUE_LP2B;
        }
      else
        {
          colors[0]= SCREEN_PAL_BLUE_LP2;
          colors[1]= SCREEN_PAL_BLUE_LP1;
          colorsbg[0]= SCREEN_PAL_BLUE_LP2B;
          colorsbg[1]= SCREEN_PAL_BLUE_LP1B;
        }
      for ( r= self->_y, i= 0; i < HEIGHT_CHARS; ++i )
        {
          color= colors[i%2];
          colorbg= colorsbg[i%2];
          for ( j= 0; j < 16; ++j, ++r )
            {
              off= r*fb_width + self->_x + 3*9;
              bg_y= (r-self->_y+self->_cline*16)%UI_LP_BG_HEIGHT;
              for ( c= 0; c < WIDTH_CHARS*9; ++c )
                {
                  bg_x= (c + self->_ccol*9)%UI_LP_BG_WIDTH;
                  fb[off+c]= MEMUMIX_BG[bg_y][bg_x]==0 ? color : colorbg;
                }
            }
        }

      // Dibuixa contingut
      for ( i= 0; i < HEIGHT_CHARS; ++i )
        {
          line= self->_cline + i;
          if ( line >= self->_sim_state->lp.N ) continue;
          beg_c= self->_ccol;
          end_c= self->_ccol + WIDTH_CHARS ;
          if ( end_c > self->_sim_state->lp.lengths[line] )
            end_c= self->_sim_state->lp.lengths[line];
          if ( beg_c >= end_c ) continue;
          tmp_char= self->_sim_state->lp.lines[line][end_c];
          self->_sim_state->lp.lines[line][end_c]= '\0';
          vgafont_draw_string ( fb, fb_width,
                                &(self->_sim_state->lp.lines[line][beg_c]),
                                self->_x + 3*9, self->_y + i*16,
                                SCREEN_PAL_BLACK,
                                SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
          self->_sim_state->lp.lines[line][end_c]= tmp_char;
        }
      
      // Dibuixa número línies
      for ( i= 0; i < HEIGHT_CHARS; ++i )
        {
          sprintf ( buf, "%03d", self->_cline + 1 + i );
          vgafont_draw_string ( fb, fb_width, buf,
                                self->_x, self->_y + i*16,
                                SCREEN_PAL_BLACK,
                                SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
        }
      
      // Dibuixa scrollbars
      // --> Vertical
      if ( self->_sim_state->lp.N > HEIGHT_CHARS )
        ui_scrollbar_set_state ( self->_vsb, self->_sim_state->lp.N,
                                 HEIGHT_CHARS, self->_cline );
      else
        ui_scrollbar_set_state ( self->_vsb, HEIGHT_CHARS,
                                 HEIGHT_CHARS, 0 );
      ui_element_draw ( self->_vsb, fb, fb_width, has_focus );
      // --> Horizontal
      ui_scrollbar_set_state ( self->_hsb, 120, WIDTH_CHARS, self->_ccol );
      ui_element_draw ( self->_hsb, fb, fb_width, has_focus );
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_lp_t *self;
  
  
  self= UI_LP(s);
  if ( !self->visible ) return false;

  // Events scrollbar
  if ( ui_element_mouse_event ( self->_vsb, event  ) )
    return true;
  if ( ui_element_mouse_event ( self->_hsb, event  ) )
    return true;

  // Processa events de la roda
  switch ( event->type )
    {
    case SDL_MOUSEWHEEL:
      if ( event->wheel.y < 0 ) ++self->_cline;
      else if ( event->wheel.y > 0 ) --self->_cline;
      recalc_cline ( self );
      return true;
      break;
    case SDL_MOUSEBUTTONDOWN: // Per a agafar focus
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
        return true;
    }
  
  return false;
  
} // end mouse_event


static bool
key_event (
           ui_element_t    *s,
           const SDL_Event *event
           )
{

  ui_lp_t *self;
  bool ret;

  
  self= UI_LP(s);
  if ( event->type == SDL_KEYDOWN &&
       event->key.keysym.mod == 0 )
    {
      switch ( event->key.keysym.sym )
        {
        case SDLK_DOWN:
          self->_cline+= event->key.repeat ? 3 : 1;
          recalc_cline ( self );
          ret= true;
          break;
        case SDLK_UP:
          self->_cline-= event->key.repeat ? 3 : 1;
          recalc_cline ( self );
          ret= true;
          break;
        case SDLK_PAGEDOWN:
          self->_cline+= HEIGHT_CHARS;
          recalc_cline ( self );
          ret= true;
          break;
        case SDLK_PAGEUP:
          self->_cline-= HEIGHT_CHARS;
          recalc_cline ( self );
          ret= true;
          break;
        case SDLK_RIGHT:
          self->_ccol+= event->key.repeat ? 3 : 1;
          if ( self->_ccol >= (120-WIDTH_CHARS) )
            self->_ccol= 120-WIDTH_CHARS;
          ret= true;
          break;
        case SDLK_LEFT:
          self->_ccol-= event->key.repeat ? 3 : 1;
          if ( self->_ccol < 0 )
            self->_ccol= 0;
          ret= true;
          break;
        default: ret= false;
        }
    }
  else ret= false;
  
  return ret;

} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_lp_t *
ui_lp_new (
           ui_sim_state_t *sim_state,
           const int       x,
           const int       y
           )
{

  ui_lp_t *new;
  
  
  // Reserva memòria.
  assert ( x >= 0 && y >= 0 );
  assert ( sim_state != NULL );
  new= g_new ( ui_lp_t, 1 );
  new->_x= x;
  new->_y= y;
  new->_sim_state= sim_state;
  new->_cline= 0;
  new->_ccol= 0;
  
  // Calcula grandàries
  // --> Num lin (3) + cos + sep 
  new->_w= (3 + WIDTH_CHARS)*9 + SEP_PIXELS;
  // --> cos + sep
  new->_h= HEIGHT_CHARS*16 + SEP_PIXELS;
  // Crea scrollbars
  new->_vsb= ui_scrollbar_new ( x + new->_w, y, HEIGHT_CHARS*16, true,
                                vsb_action, new );
  new->_hsb= ui_scrollbar_new ( x + 3*9, y + new->_h, WIDTH_CHARS*9, false,
                                hsb_action, new );
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;

  return new;
  
} // end ui_lp_new


void
ui_lp_show_last_line (
                      ui_lp_t *self
                      )
{
  
  self->_cline= self->_sim_state->lp.N-HEIGHT_CHARS;
  if ( self->_cline < 0 ) self->_cline= 0;
  self->_ccol= 0;
  
} // end ui_lp_show_last_line
