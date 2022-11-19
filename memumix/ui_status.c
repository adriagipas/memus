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
 *  ui_status.c - Implementació de 'ui_status.h'.
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
#include "ui_status.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define MARGIN 5
#define SEP 2

#define HEIGHT (MARGIN*2 + 16*2 + SEP)

#define FG_COLOR SCREEN_PAL_GRAY_1

#define DISABLED_COLOR SCREEN_PAL_GRAY_4

#define RUNNING_BAR 7




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
callback_run_bar (
                  void *user_data
                  )
{

  ui_status_t *self;

  
  self= UI_STATUS(user_data);
  if ( self->_running_bar_inc )
    {
      if ( self->_running_bar == RUNNING_BAR-1 )
        self->_running_bar_inc= false;
    }
  else
    {
      if ( self->_running_bar == 0 )
        self->_running_bar_inc= true;
    }
  if ( self->_running_bar_inc )
    ++(self->_running_bar);
  else
    --(self->_running_bar);
  
  return true;
  
} // end callback_run_bar


static void
draw_busy_label (
                 int        *fb,
                 const int   fb_width,
                 const int   x,
                 const int   y,
                 const char *label,
                 const bool  busy,
                 const bool  enabled
                 )
{

  vgafont_draw_string ( fb, fb_width, "\x09",
                        x, y,
                        SCREEN_PAL_BLACK, SCREEN_PAL_GRAY_1,
                        VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
  vgafont_draw_string ( fb, fb_width, "\x07",
                        x, y,
                        (enabled ?
                         (busy ? SCREEN_PAL_GREEN : SCREEN_PAL_RED) :
                         SCREEN_PAL_GRAY_4),
                        SCREEN_PAL_GRAY_1,
                        VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
  vgafont_draw_string ( fb, fb_width, label,
                        x+9, y,
                        (enabled ? FG_COLOR : SCREEN_PAL_GRAY_4),
                        SCREEN_PAL_GRAY_1,
                        VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
  
} // end draw_busy_label


static void
draw_tape (
           int        *fb,
           const int   fb_width,
           const int   x,
           const int   y,
           const int   id,
           const int   pos,
           const int   N,
           const bool  busy
           )
{

  char buf[5];
  const int WIDTH_TAPE= 5*9+4;
  int r,i,off,c,bar_len;
  
  
  sprintf ( buf, "TP%d", id );
  draw_busy_label ( fb, fb_width, x, y, buf, busy, N>0 );

  // Progrés
  bar_len= (int) ((pos / (double) N)*WIDTH_TAPE + 0.5);
  for ( r= y+5, i= 0; i < 3; ++i,++r )
    {
      off= r*fb_width + x + 4*9 + 2;
      for ( c= 0; c < bar_len; ++c )
        fb[off+c]= SCREEN_PAL_RED;
      for ( ; c < WIDTH_TAPE; ++c )
        fb[off+c]= SCREEN_PAL_GRAY_4;
    }
  
} // end draw_tape




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{
  
  ui_status_t *self;
  
  
  self= UI_STATUS(s);
  if ( self->_cb_run_bar != -1 )
    ui_remove_callback ( self->_cb_run_bar );
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

  ui_status_t *self;
  int off,r,c,c_off,i;
  char buf[20];
  

  self= UI_STATUS(s);
  if ( self->visible )
    {

      // Dibuixa caixa
      for ( r= SCREEN_HEIGHT-HEIGHT; r < SCREEN_HEIGHT; ++r )
        {
          off= r*fb_width;
          for ( c= 0; c < SCREEN_WIDTH; ++c )
            fb[off+c]= SCREEN_PAL_GRAY_5;
        }

      // Dibuixa si està en execució o no
      if ( self->_state->running )
        {
          if ( self->_cb_run_bar == -1 )
            self->_cb_run_bar=
              ui_register_callback ( callback_run_bar, self, 100000 );
          vgafont_draw_string ( fb, fb_width, "En execuci\xa2",
                                2*MARGIN,
                                SCREEN_HEIGHT-MARGIN-16-SEP-16,
                                FG_COLOR, SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
          vgafont_draw_string ( fb, fb_width,
                                "\xb0\xb1\xb2\xb1\xb0",
                                self->_running_bar*9+2*MARGIN,
                                SCREEN_HEIGHT-MARGIN-16,
                                SCREEN_PAL_RED_3, SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
          vgafont_draw_string ( fb, fb_width,
                                "\xb1\xb2\xb1",
                                (self->_running_bar + 1)*9+2*MARGIN,
                                SCREEN_HEIGHT-MARGIN-16,
                                SCREEN_PAL_RED_2, SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
          vgafont_draw_string ( fb, fb_width,
                                "\xb2",
                                (self->_running_bar + 2)*9+2*MARGIN,
                                SCREEN_HEIGHT-MARGIN-16,
                                SCREEN_PAL_RED, SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
        }
      else
        {
          if ( self->_cb_run_bar != -1 )
            {
              ui_remove_callback ( self->_cb_run_bar );
              self->_cb_run_bar= -1;
            }
          vgafont_draw_string ( fb, fb_width, "Aturat",
                                2*MARGIN,
                                SCREEN_HEIGHT-MARGIN-16-SEP-16,
                                FG_COLOR, SCREEN_PAL_WHITE,
                                VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
        }

      // Separador
      for ( r= SCREEN_HEIGHT-HEIGHT+MARGIN; r < SCREEN_HEIGHT-MARGIN; ++r )
        {
          off= r*fb_width;
          c= 2*MARGIN + 11*9 + MARGIN;
          fb[off+c]= SCREEN_PAL_GRAY_4;
        }

      // Informació targetes perforades
      c_off= 2*MARGIN + 11*9 + 2*MARGIN;
      // --> Lector
      sprintf ( buf, "CR:%02d/%02d",
                self->_state->cr.N_read, self->_state->cr.N );
      draw_busy_label ( fb, fb_width,
                        c_off, SCREEN_HEIGHT-MARGIN-16-SEP-16,
                        buf, self->_state->cr.busy, true );
      // --> Perforadora
      sprintf ( buf, "CP:%02d", self->_state->cp.N );
      draw_busy_label ( fb, fb_width,
                        c_off, SCREEN_HEIGHT-MARGIN-16,
                        buf, self->_state->cp.busy, true );
      c_off+= 11*9;

      // Cintes magnètiques
      for ( i= 0; i < 4; ++i )
        {
          draw_tape ( fb, fb_width,
                      c_off, SCREEN_HEIGHT-MARGIN-16-SEP-16,
                      2*i+1, self->_state->tapes[2*i].pos,
                      self->_state->tapes[2*i].N,
                      self->_state->tapes[2*i].busy );
          draw_tape ( fb, fb_width,
                      c_off, SCREEN_HEIGHT-MARGIN-16,
                      2*i+1+1, self->_state->tapes[2*i+1].pos,
                      self->_state->tapes[2*i+1].N,
                      self->_state->tapes[2*i+1].busy );
          c_off+= (4+5)*9 + 9;
        }
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{
  return false;
} // end mouse_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_status_t *
ui_status_new (
               const ui_sim_state_t *state
               )
{

  ui_status_t *new;

  
  // Reserva memòria.
  new= g_new ( ui_status_t, 1 );
  new->_state= state;
  new->_running_bar= 0;
  new->_running_bar_inc= true;
  new->_cb_run_bar= -1;

  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= NULL;
  new->visible= true;
  
  return new;
  
} // end ui_status_new
