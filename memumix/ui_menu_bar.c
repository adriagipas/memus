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
 *  ui_menu_bar.c - Implementació de 'ui_menu_bar.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "screen.h"
#include "ui_menu_bar.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define PADDING_Y 5
#define PADDING_X (PADDING_Y*2)

#define BG_COLOR SCREEN_PAL_GRAY_5
#define BG_SEL_COLOR SCREEN_PAL_RED
#define FG_COLOR SCREEN_PAL_WHITE
#define DISABLED_COLOR SCREEN_PAL_GRAY_4




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

// Selecciona el primer element del menú disponible
static void
select_first (
              ui_menu_bar_t *self
              )
{

  int p;

  
  if ( self->_selected != -1 || self->_N == 0 ) return;
  
  for ( p= 0; !self->_enabled[p] && p < self->_N; ++p );
  self->_selected= p == self->_N ? -1 : p;
  
} // end select_first


static int
get_new_entry (
               ui_menu_bar_t *self
               )
{

  int tmp,ret;
  
  
  if ( self->_N == self->_size )
    {
      tmp= self->_size*2;
      if ( tmp < self->_size )
        error ( "cannot allocate memory" );
      self->_v= g_renew ( ui_menu_bar_entry_t, self->_v, tmp );
      self->_enabled= g_renew ( bool, self->_enabled, tmp );
      self->_end_x= g_renew ( int, self->_end_x, tmp );
      self->_size= tmp;
    }
  ret= self->_N++;
  
  return ret;
  
} // end get_new_entry



static void
draw_entry (
            const ui_menu_bar_t *self,
            const int            entry,
            const int            x,
            const int            y,
            int                 *fb,
            const int            fb_width
            )
{
  
  int bg_color,fg_color;
  
  
  fg_color= self->_enabled[entry] ? FG_COLOR : DISABLED_COLOR;
  bg_color= entry==self->_selected ? BG_SEL_COLOR : BG_COLOR;
  
  // Escriu etiqueta
  vgafont_draw_string ( fb, fb_width, self->_v[entry].label,
                        x, y, fg_color, bg_color,
                        VGAFONT_XY_PIXELS );
  
  // Prepara per a dibuixar el menu.
  if ( self->_enabled[entry] && entry == self->_selected )
    {
      ui_menu_set_min_width ( self->_v[entry].menu,
                              self->_v[entry].width_body+2*PADDING_X );
      ui_menu_show ( self->_v[entry].menu, x-PADDING_X, y+16+PADDING_Y );
    }
  else
    {
      ui_element_set_visible ( self->_v[entry].menu, false );
      self->_v[entry].has_focus= false;
    }
  
} // end draw_entry


// S'enten que estem dins del menú
static void
mouse_selection (
                 ui_menu_bar_t *self,
                 const int      mouse_x
                 )
{

  int n,beg,end;

  
  beg= 0;
  // Sols canvia si moguen a una posició seleccionable.
  // --> Busca entrada
  for ( n= 0; n < self->_N; ++n )
    {
      end= self->_end_x[n];
      if ( mouse_x >= beg && mouse_x < end )
        break;
      beg= end;
    }
  if ( n < self->_N && self->_enabled[n] )
    self->_selected= n;
  
} // end mouse_selection




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_menu_bar_t *self;
  int n;
  

  self= UI_MENU_BAR(s);
  for ( n= 0; n < self->_N; ++n )
    {
      g_free ( self->_v[n].label );
      ui_element_free ( UI_ELEMENT(self->_v[n].menu) );
      }
  g_free ( self->_v );
  g_free ( self->_enabled );
  g_free ( self->_end_x );
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

  ui_menu_bar_t *self;
  int off,c,i,j,n,color,beg_c,end_c,r;
  

  self= UI_MENU_BAR(s);
  if ( self->visible )
    {

      // Actualitza selecció en funció del focus
      if ( has_focus )
        {
          if ( self->_selected == -1 && self->_N > 0 )
            select_first ( self );
        }
      else self->_selected= -1;
      
      // Pinta
      beg_c= end_c= 0;
      // --> Pinta entrades
      for ( n= 0; n < self->_N; ++n )
        {

          color= self->_selected==n ? BG_SEL_COLOR : BG_COLOR;
          end_c= self->_end_x[n];
          
          // --> Pinta padding superior
          for ( r= 0, i= 0; i < PADDING_Y; ++i, ++r )
            {
              off= r*fb_width;
              for ( c= beg_c; c < end_c; ++c )
                fb[off+c]= color;
            }

          // --> Pinta padding esquerra
          for ( r= PADDING_Y, i= 0; i < 16; ++i, ++r )
            {
              off= r*fb_width + beg_c;
              for ( j= 0; j < PADDING_X; ++j )
                fb[off+j]= color;
            }

          // --> Pinta contingut
          draw_entry ( self, n, beg_c+PADDING_X, PADDING_Y, fb, fb_width );

          // --> Pinta padding dreta
          for ( r= PADDING_Y, i= 0; i < 16; ++i, ++r )
            {
              off= r*fb_width + beg_c + self->_v[n].width_body + PADDING_X;
              for ( j= 0; j < PADDING_X; ++j )
                fb[off+j]= color;
            }
          
          // --> Pinta padding inferior
          for ( r= PADDING_Y+16, i= 0; i < PADDING_Y; ++i, ++r )
            {
              off= r*fb_width;
              for ( c= beg_c; c < end_c; ++c )
                fb[off+c]= color;
            }
          
          beg_c= end_c;
          
        }

      // Pinta resta
      for ( r= 0, i= 0; i < 16+2*PADDING_Y; ++i, ++r )
        {
          off= r*fb_width;
          for ( c= end_c; c < fb_width; ++c )
            fb[off+c]= BG_COLOR;
        }
      
      // Dibuixa submenu si és el cas.
      // NOTA!! Faig molta comprovació redundant
      if ( self->_selected != -1 &&
           self->_enabled[self->_selected] &&
           self->_v[self->_selected].menu->visible )
        {
          assert ( has_focus );
          ui_element_draw ( self->_v[self->_selected].menu,
                            fb, fb_width,
                            self->_v[self->_selected].has_focus );
        }
    }
  else self->_selected= -1; // Redundant!
  
} // end draw


static void
set_visible (
             ui_element_t *s,
             const bool    val
             )
{

  ui_menu_bar_t *self;

  
  self= UI_MENU_BAR(s);
  self->visible= val;
  if ( !val ) self->_selected= -1;
  
} // end set_visible


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_menu_bar_t *self;
  
  
  self= UI_MENU_BAR(s);
  if ( !self->visible ) return false;

  // Si hi ha algun submenú obert processa
  // NOTA!! Faig molta comprovació redundant
  if ( self->_selected != -1 &&
       self->_enabled[self->_selected] &&
       self->_v[self->_selected].menu->visible )
    {
      if ( ui_element_mouse_event ( self->_v[self->_selected].menu,
                                    event ) )
        {
          self->_v[self->_selected].has_focus= true;
          return true;
        }
    }
  
  // Processa event
  switch ( event->type )
    {
      // Este event sols l'elimine si la posició és dins del menú
      // altres elements li podria afectar.
    case SDL_MOUSEMOTION:
      // Dins del menú
      if ( self->_N > 0 &&
           event->motion.y >= 0 && event->motion.y < self->_h &&
           event->motion.x >= 0 && event->motion.x < self->_end_x[self->_N-1] )
        {
          mouse_selection ( self, event->motion.x );
          return true;
        }
      // NOTA!! No lleve el focus perque igual no l'agafa ningú
      else return false;
      break;
    case SDL_MOUSEBUTTONDOWN:
      if ( self->_N > 0 &&
           event->button.y >= 0 && event->button.y < self->_h &&
           event->motion.x >= 0 && event->motion.x < self->_end_x[self->_N-1] )
        {
          mouse_selection ( self, event->button.x );
          return true;
        }
      break;
    case SDL_MOUSEBUTTONUP:
      if ( self->_N > 0 &&
           event->button.y >= 0 && event->button.y < self->_h &&
           event->motion.x >= 0 && event->motion.x < self->_end_x[self->_N-1] )
        return true;
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

  ui_menu_bar_t *self;
  bool ret;
  int p;

  
  self= UI_MENU_BAR(s);

  // Si hi ha algun submenú obert amb el FOCUS processa
  // NOTA!! Faig molta comprovació redundant
  if ( self->_selected != -1 &&
       self->_enabled[self->_selected] &&
       self->_v[self->_selected].menu->visible &&
       self->_v[self->_selected].has_focus &&
       ui_element_key_event ( self->_v[self->_selected].menu,
                              event ) )
    ret= true;
  
  // Comportament normal
  else if ( event->type == SDL_KEYDOWN )
    {
      switch ( event->key.keysym.sym )
        {
        case SDLK_RIGHT: // Mou següent
          if ( self->_selected == -1 )
            select_first ( self );
          else
            {
              for ( p= self->_selected+1;
                    p < self->_N && !self->_enabled[p];
                    ++p );
              if ( p < self->_N ) self->_selected= p;
            }
          ret= true;
          break;
        case SDLK_LEFT: // Mou anterior
          if ( self->_selected != -1 )
            {
              for ( p= self->_selected-1;
                    p >= 0 && !self->_enabled[p];
                    --p );
              if ( p >= 0 ) self->_selected= p;
            }
          ret= true;
          break;
        case SDLK_SPACE:
        case SDLK_RETURN: // Acció
          if ( self->_selected != -1 )
            self->_v[self->_selected].has_focus= true;
          ret= true;
          break;
        case SDLK_UP: // Si estic en un menu lleva el focus.
          if ( self->_selected != -1 &&
               self->_enabled[self->_selected] &&
               self->_v[self->_selected].menu->visible )
            {
              self->_v[self->_selected].has_focus= false;
              ret= true;
            }
          else ret= false;
          break;
        case SDLK_DOWN: // Si estic en un menu dona el focus.
          if ( self->_selected != -1 &&
               self->_enabled[self->_selected] &&
               self->_v[self->_selected].menu->visible )
            {
              self->_v[self->_selected].has_focus= true;
              ret= true;
            }
          else ret= false;
          break;
        default: ret= false;
          
        }
    }
  else ret= false;
  
  return ret;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/***********************/

ui_menu_bar_t *
ui_menu_bar_new (void)
{

  ui_menu_bar_t *new;
  

  // Reserva memòria.
  new= g_new ( ui_menu_bar_t, 1 );
  new->_h= 16 + PADDING_Y*2;
  new->_selected= -1;
  new->_v= g_new ( ui_menu_bar_entry_t, 1 );
  new->_enabled= g_new ( bool, 1 );
  new->_end_x= g_new ( int, 1 );
  new->_N= 0;
  new->_size= 1;
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= set_visible;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;

  return new;
  
} // end ui_menu_bar_new


void
ui_menu_bar_enable_entry (
                          ui_menu_bar_t *self,
                          const int      entry,
                          const bool     val
                          )
{

  assert ( entry >= 0 && entry < self->_N );
  self->_enabled[entry]= val;
  if ( !val && self->_selected == entry )
    {
      self->_selected= -1;
      select_first ( self );
    }
  
} // end ui_menu_bar_enabled_entry


int
ui_menu_bar_add_entry (
                       ui_menu_bar_t  *self,
                       const char     *label, // CP437
                       ui_menu_t      *menu
                       )
{

  int ret;
  

  // Crea.
  ret= get_new_entry ( self );
  self->_enabled[ret]= true;
  self->_v[ret].label= g_strdup ( label );
  self->_v[ret].menu= menu;
  self->_v[ret].has_focus= false;
  assert ( menu != NULL );

  // Actualitza dimensions
  self->_v[ret].width_body= strlen ( label )*9;
  self->_end_x[ret]=
    (ret==0 ? 0 : self->_end_x[ret-1]) +
    self->_v[ret].width_body + 2*PADDING_X;
  
  return ret;
  
} // end ui_menu_bar_add_entry
