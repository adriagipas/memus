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
 *  ui_save_dialog.c - Implementació de 'ui_save_dialog.h'.
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
#include "ui_input.h"
#include "ui_save_dialog.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define WIDTH 480
#define HEIGHT 390

#define PADDING_TITLE_Y 2

#define MARGIN 10




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
cb_cancel (
           ui_element_t *e,
           void         *udata
           )
{

  ui_save_dialog_t *self;


  self= UI_SAVE_DIALOG(udata);
  ui_element_set_visible ( self, false );
  
} // end cb_cancel


static void
cb_sel_file (
             ui_element_t *e,
             void         *udata             
             )
{
  
  ui_save_dialog_t *self;
  

  self= UI_SAVE_DIALOG(udata);
  if ( self->_action != NULL )
    self->_action ( UI_ELEMENT(self), self->_udata );
  
} // end cb_sel_file




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_save_dialog_t *self;
  
  
  self= UI_SAVE_DIALOG(s);
  ui_element_free ( self->_root );
  g_free ( self->_title );
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

  ui_save_dialog_t *self;
  int r,i,off,c,x_title;
  
  
  self= UI_SAVE_DIALOG(s);
  if ( self->visible )
    {

      // Dibuixa Capçalera
      r= self->_y;
      for ( i= 0; i < 16+PADDING_TITLE_Y*2; ++i,++r )
        {
          off= r*fb_width + self->_x;
          for ( c= 0; c < WIDTH; ++c )
            fb[off+c]= SCREEN_PAL_GRAY_5;
        }
      
      // Imprimeix títol
      x_title= self->_x + (WIDTH-self->_title_length)/2;
      vgafont_draw_string ( fb, fb_width, self->_title,
                            x_title, self->_y+PADDING_TITLE_Y,
                            SCREEN_PAL_WHITE, SCREEN_PAL_WHITE,
                            VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );

      // Dibuixa resta de caixa
      for ( ; i < HEIGHT-1; ++i, ++r )
        {
          off= r*fb_width + self->_x;
          fb[off]= SCREEN_PAL_GRAY_5;
          for ( c= 1; c < WIDTH-1; ++c )
            fb[off+c]= SCREEN_PAL_WHITE;
          fb[off+WIDTH-1]= SCREEN_PAL_GRAY_5;
        }
      off= r*fb_width + self->_x;
      for ( c= 0; c < WIDTH; ++c )
        fb[off+c]= SCREEN_PAL_GRAY_5;

      // Dibuixa text per a introduir text
      vgafont_draw_string ( fb, fb_width, "Nom del fitxer:",
                            self->_x+MARGIN,
                            self->_y+HEIGHT-MARGIN-16*3-5,
                            SCREEN_PAL_BLACK, SCREEN_PAL_WHITE,
                            VGAFONT_XY_PIXELS|VGAFONT_BG_TRANS );
      
      // Dibuixa elements
      ui_element_draw ( self->_root, fb, fb_width, has_focus );
      
    }
  
} // end draw


static void
set_visible (
             ui_element_t *s,
             const bool    val
             )
{

  ui_save_dialog_t *self;


  self= UI_SAVE_DIALOG(s);
  self->visible= val;
  if ( val )
    ui_list_set_focus_begin ( self->_root );
  
} // end set_visible


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_save_dialog_t *self;
  
  
  self= UI_SAVE_DIALOG(s);
  if ( !self->visible ) return false;
  
  if ( ui_element_mouse_event ( self->_root, event ) )
    return true;
  
  return true;

} // end mouse_event


static bool
key_event (
           ui_element_t    *s,
           const SDL_Event *event
           )
{

  ui_save_dialog_t *self;
  bool ret;
  
  
  self= UI_SAVE_DIALOG(s);
  if ( ui_element_key_event ( self->_root, event ) )
    ret= true;
  else if ( event->type == SDL_KEYDOWN && event->key.keysym.mod == 0 )
    {
      if ( event->key.keysym.sym == SDLK_ESCAPE )
        ui_element_set_visible ( self, false );
      ret= true;
    }
  else ret= true;
  
  return ret;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_save_dialog_t *
ui_save_dialog_new (
                    const char  *title, // cp437
                    const gchar *cdir,
                    ui_action_t *action,
                    void        *user_data
                    )
{

  ui_save_dialog_t *new;
  ui_button_t *b1;
  int tmp_x,tmp_y;
  
  
  new= g_new ( ui_save_dialog_t, 1 );
  new->_root= ui_list_new ( NULL, NULL, NULL );
  new->_title= g_strdup ( title );
  new->_action= action;
  new->_udata= user_data;
  new->_title_length= strlen ( title )*9;

  // Inicialitza coordenades.
  new->_x= (SCREEN_WIDTH-WIDTH)/2;
  new->_y= (SCREEN_HEIGHT-HEIGHT)/2;

  // Crea filechooser
  new->_fchooser=
    ui_file_chooser_new ( new->_x + MARGIN,
                          new->_y+16+PADDING_TITLE_Y*2 + MARGIN,
                          cdir, false, false,
                          "[.]", NULL, NULL );
  ui_list_add ( new->_root, UI_ELEMENT(new->_fchooser) );
  
  // Crea botons
  tmp_x= new->_x+WIDTH-MARGIN-8-9*5;
  tmp_y= new->_y+HEIGHT-MARGIN-8-16;
  b1= ui_button_new ( "Tanca", tmp_x, tmp_y, cb_cancel, new );
  ui_list_add ( new->_root, UI_ELEMENT(b1) );
  tmp_x= new->_x+MARGIN+9*15;
  tmp_y= new->_y+HEIGHT-MARGIN-10-16*3;
  new->_input= ui_input_new ( tmp_x, tmp_y, 34, cb_sel_file, new );
  ui_list_add ( new->_root, UI_ELEMENT(new->_input) );
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= set_visible;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;
  
  return new;
  
} // end ui_save_dialog_new


gchar *
ui_save_dialog_get_file (
                         ui_save_dialog_t *self
                         )
{

  const gchar *file_name;
  gchar *ret;
  

  file_name= ui_input_get_text ( self->_input );
  if ( file_name == NULL ) ret= NULL;
  else
    ret= g_build_path ( G_DIR_SEPARATOR_S,
                        ui_file_chooser_get_current_dir ( self->_fchooser ),
                        ui_input_get_text ( self->_input ),
                        NULL );
  
  return ret;
  
} // end ui_open_dialog_get_current_file
