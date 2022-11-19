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
 *  ui_error_dialog.c - Implementació de 'ui_error_dialog.h'.
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
#include "ui_button.h"
#include "ui_error_dialog.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define PADDING_TITLE_Y 2

#define MARGIN 10

#define ERROR_LEN 5
#define BUTTON_WIDTH (7*9 + 2*(2+2))
#define BUTTON_HEIGHT (16 + 2*(2+2))




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
cb_cancel (
           ui_element_t *e,
           void         *udata
           )
{

  ui_error_dialog_t *self;
  
  
  self= UI_ERROR_DIALOG(udata);
  ui_element_set_visible ( self, false );
  
} // end cb_cancel


static void
recalc_w (
          ui_error_dialog_t *self
          )
{

  int tmp;


  self->_w= 1*2 + MARGIN*2 + ERROR_LEN*9;
  tmp= 1*2 + MARGIN*2 + self->_msg_length*9;
  if ( tmp > self->_w ) self->_w= tmp;
  tmp= 1*2 + MARGIN*2 + BUTTON_WIDTH;
  if ( tmp > self->_w ) self->_w= tmp;

  self->_x= (SCREEN_WIDTH-self->_w)/2;
  
} // end recalc_w




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_error_dialog_t *self;
  
  
  self= UI_ERROR_DIALOG(s);
  ui_element_free ( self->_but );
  if ( self->_msg != NULL )
    g_free ( self->_msg );
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
  
  ui_error_dialog_t *self;
  int r,i,off,c,x_title;
  
  
  self= UI_ERROR_DIALOG(s);
  if ( self->visible )
    {

      // Dibuixa Capçalera
      r= self->_y;
      for ( i= 0; i < 16+PADDING_TITLE_Y*2; ++i,++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < self->_w; ++c )
            fb[off+c]= SCREEN_PAL_GRAY_5;
        }

      // Imprimeix títol
      x_title= self->_x + (self->_w-ERROR_LEN*9)/2;
      vgafont_draw_string ( fb, fb_width, "ERROR",
                            x_title, self->_y+PADDING_TITLE_Y,
                            SCREEN_PAL_RED, SCREEN_PAL_WHITE,
                            VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );

      // Dibuixa resta de caixa
      for ( ; i < self->_h-1; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          fb[off]= SCREEN_PAL_GRAY_5;
          for ( c= 1; c < self->_w-1; ++c )
            fb[off+c]= SCREEN_PAL_WHITE;
          fb[off+self->_w-1]= SCREEN_PAL_GRAY_5;
        }
      off= r*fb_width + self->_x;
      for ( c= 0; c < self->_w; ++c )
        fb[off+c]= SCREEN_PAL_GRAY_5;

      // Dibuixa missatge
      if ( self->_msg_length > 0 )
        vgafont_draw_string ( fb, fb_width, self->_msg,
                              self->_x+1+MARGIN,
                              self->_y+PADDING_TITLE_Y*2+16 + MARGIN,
                              SCREEN_PAL_BLACK, SCREEN_PAL_WHITE,
                              VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
      
      // Dibuixa botó
      ui_element_draw ( self->_but, fb, fb_width, has_focus );
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_error_dialog_t *self;
  
  
  self= UI_ERROR_DIALOG(s);
  if ( !self->visible ) return false;
  
  if ( ui_element_mouse_event ( self->_but, event ) )
    return true;
  
  return true;

} // end mouse_event


static bool
key_event (
           ui_element_t    *s,
           const SDL_Event *event
           )
{

  ui_error_dialog_t *self;
  bool ret;
  
  
  self= UI_ERROR_DIALOG(s);
  if ( ui_element_key_event ( self->_but, event ) )
    ret= true;
  else ret= true;
  
  return ret;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_error_dialog_t *
ui_error_dialog_new (void)
{

  ui_error_dialog_t *new;
  int tmp_x,tmp_y;
  
  
  new= g_new ( ui_error_dialog_t, 1 );
  new->_msg= NULL;
  new->_msg_length= 0;
  recalc_w ( new );
  new->_h= 16 + PADDING_TITLE_Y*2 + MARGIN*3 + 16 + BUTTON_HEIGHT;

  // Inicialitza coordenades.
  new->_y= (SCREEN_HEIGHT-new->_h)/2;

  // Crea botó
  tmp_x= (SCREEN_WIDTH-BUTTON_WIDTH)/2;
  tmp_y= new->_y+new->_h-MARGIN-8-16;
  new->_but= ui_button_new ( "D'acord", tmp_x, tmp_y, cb_cancel, new );
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;
  
  return new;
  
} // end ui_error_dialog_new


void
ui_error_dialog_set_msg (
                         ui_error_dialog_t *self,
                         const char        *msg
                         )
{

  assert ( msg != NULL );
  
  if ( self->_msg != NULL ) g_free ( self->_msg );
  self->_msg= g_strdup ( msg );
  self->_msg_length= strlen ( msg );
  recalc_w ( self );
  
} // end ui_error_dialog_set_msg
