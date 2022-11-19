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
 *  ui_menu.c - Implementació de 'ui_menu.h'.
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
#include "ui_menu.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define PADDING_Y 2
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
              ui_menu_t *self
              )
{

  int p;

  
  if ( self->_selected != -1 || self->_N == 0 ) return;

  
  for ( p= 0; p < self->_N && !self->_enabled[p]; ++p );
  self->_selected= p == self->_N ? -1 : p;
  
} // end select_first


static int
get_new_entry (
               ui_menu_t *self
               )
{

  int tmp,ret;

  
  if ( self->_N == self->_size )
    {
      tmp= self->_size*2;
      if ( tmp < self->_size )
        error ( "cannot allocate memory" );
      self->_v= g_renew ( ui_menu_entry_t, self->_v, tmp );
      self->_enabled= g_renew ( bool, self->_enabled, tmp );
      self->_end_y= g_renew ( int, self->_end_y, tmp );
      self->_size= tmp;
    }
  ret= self->_N++;

  return ret;
  
} // end get_new_entry


// Torna la nova y
static int
draw_action (
             const ui_menu_t  *self,
             const int         entry,
             const int         x,
             const int         y,
             int              *fb,
             const int         fb_width
             )
{

  int i,r,c,j,bg_color,fg_color,off;


  r= y;

  fg_color= self->_enabled[entry] ? FG_COLOR : DISABLED_COLOR;
  bg_color= entry==self->_selected ? BG_SEL_COLOR : BG_COLOR;

  // Dibuixa fons
  for ( i= 0; i < 16; ++i, ++r )
    {
      off= r*fb_width + x;
      for ( c= 0, j= 0; j < self->_w; ++j,++c )
        fb[off+c]= bg_color;
    }

  // Escriu
  vgafont_draw_string ( fb, fb_width, self->_v[entry].action.label,
                        PADDING_X+9+x, y,
                        fg_color, bg_color,
                        VGAFONT_XY_PIXELS );
  
  return r;
  
} // end draw_action


// Torna la nova y
static int
draw_separator (
                const ui_menu_t  *self,
                const int         entry,
                const int         x,
                const int         y,
                int              *fb,
                const int         fb_width
                )
{

  int off,c,j;


  off= y*fb_width + x;
  
  // Padding esquerra
  for ( c= 0, j= 0; j < PADDING_X; ++j, ++c )
    fb[off+c]= BG_COLOR;

  // Separador
  for ( j= 0; j < self->_w-PADDING_X*2; ++j, ++c )
    fb[off+c]= DISABLED_COLOR;
  
  // Padding dreta
  for ( j= 0; j < PADDING_X; ++j, ++c )
    fb[off+c]= BG_COLOR;
  
  return y+1;
  
} // end draw_separator


// Torna la nova y
static int
draw_submenu (
              const ui_menu_t  *self,
              const int         entry,
              const int         x,
              const int         y,
              int              *fb,
              const int         fb_width
              )
{
  
  int i,r,c,j,bg_color,fg_color,off;
  
  
  r= y;
  
  fg_color= self->_enabled[entry] ? FG_COLOR : DISABLED_COLOR;
  bg_color= entry==self->_selected ? BG_SEL_COLOR : BG_COLOR;
  
  // Dibuixa fons
  for ( i= 0; i < 16; ++i, ++r )
    {
      off= r*fb_width + x;
      for ( c= 0, j= 0; j < self->_w; ++j,++c )
        fb[off+c]= bg_color;
    }

  // Escriu etiqueta
  vgafont_draw_string ( fb, fb_width, self->_v[entry].submenu.label,
                        PADDING_X+x+9, y,
                        fg_color, bg_color,
                        VGAFONT_XY_PIXELS );

  // Escriu fletxa
  vgafont_draw_string ( fb, fb_width, "\x10",
                        (x+self->_w)-(9+PADDING_X), y,
                        fg_color, bg_color,
                        VGAFONT_XY_PIXELS );

  // Prepara per a dibuixar el submenu.
  if ( self->_enabled[entry] && entry == self->_selected )
    ui_menu_show ( self->_v[entry].submenu.menu,
                   x+self->_w, y-PADDING_Y );
  else
    {
      ui_element_set_visible ( self->_v[entry].submenu.menu, false );
      self->_v[entry].submenu.has_focus= false;
    }
  
  return r;
  
} // end draw_submenu


// Torna la nova y
static int
draw_choice (
             const ui_menu_t  *self,
             const int         entry,
             const int         x,
             const int         y,
             int              *fb,
             const int         fb_width
             )
{
  
  int i,r,c,j,bg_color,fg_color,off;
  const char *sel_sym;
  

  r= y;
  
  fg_color= self->_enabled[entry] ? FG_COLOR : DISABLED_COLOR;
  bg_color= entry==self->_selected ? BG_SEL_COLOR : BG_COLOR;
  
  // Dibuixa fons
  for ( i= 0; i < 16; ++i, ++r )
    {
      off= r*fb_width + x;
      for ( c= 0, j= 0; j < self->_w; ++j,++c )
        fb[off+c]= bg_color;
    }

  // Escriu
  vgafont_draw_string ( fb, fb_width, self->_v[entry].choice.label,
                        PADDING_X+9+x, y,
                        fg_color, bg_color,
                        VGAFONT_XY_PIXELS );

  // Dibuixa cercle selecció
  sel_sym=
    *(self->_v[entry].choice.var)==self->_v[entry].choice.val ?
    "\x1a" : "\x0";
  vgafont_draw_string ( fb, fb_width,
                        sel_sym,
                        PADDING_X+x, y,
                        fg_color, bg_color,
                        VGAFONT_XY_PIXELS );
  
  return r;
  
} // end draw_choice


// S'enten que estem dins del menú
static void
mouse_selection (
                 ui_menu_t *self,
                 const int  mouse_y
                 )
{

  int n,beg,end;

  
  beg= self->_y;
  // Sols canvia si moguen a una posició seleccionable.
  // --> Busca entrada
  for ( n= 0; n < self->_N; ++n )
    {
      end= self->_end_y[n] + self->_y;
      if ( mouse_y >= beg && mouse_y < end )
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

  ui_menu_t *self;
  int n;
  

  self= UI_MENU(s);
  for ( n= 0; n < self->_N; ++n )
    switch ( self->_v[n].type )
      {
      case UI_MENU_ACTION:
        g_free ( self->_v[n].action.label );
        break;
      case UI_MENU_SUBMENU:
        g_free ( self->_v[n].submenu.label );
        ui_element_free ( UI_ELEMENT(self->_v[n].submenu.menu) );
        break;
      case UI_MENU_CHOICE:
        g_free ( self->_v[n].choice.label );
        break;
      default: break;
      }
  g_free ( self->_v );
  g_free ( self->_enabled );
  g_free ( self->_end_y );
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

  ui_menu_t *self;
  int off,r,c,i,n,color;
  

  self= UI_MENU(s);
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
      r= self->_y;
      // --> Pinta entrades
      for ( n= 0; n < self->_N; ++n )
        {

          color= self->_selected==n ? BG_SEL_COLOR : BG_COLOR;
          
          // --> Pinta padding superior
          for ( i= 0; i < PADDING_Y; ++i, ++r )
            {
              off= r*fb_width + self->_x;
              for ( c= 0; c < self->_w; ++c )
                fb[off+c]= color;
            }
          
          // Pinta contingut
          switch ( self->_v[n].type )
            {
            case UI_MENU_ACTION:
              r= draw_action ( self, n, self->_x, r, fb, fb_width );
              break;
            case UI_MENU_SEPARATOR:
              r= draw_separator ( self, n, self->_x, r, fb, fb_width );
              break;
            case UI_MENU_SUBMENU:
              r= draw_submenu ( self, n, self->_x, r, fb, fb_width );
              break;
            case UI_MENU_CHOICE:
              r= draw_choice ( self, n, self->_x, r, fb, fb_width );
              break;
            default:
              printf("ui_menu - draw : WTF !!!\n");
              exit ( EXIT_FAILURE );
            }
          
          // Pinda padding inferior
          for ( i= 0; i < PADDING_Y; ++i, ++r )
            {
              off= r*fb_width + self->_x;
              for ( c= 0; c < self->_w; ++c )
                fb[off+c]= color;
            }
          
        }

      // Dibuixa submenu si és el cas.
      // NOTA!! Faig molta comprovació redundant
      if ( self->_selected != -1 &&
           self->_v[self->_selected].type==UI_MENU_SUBMENU &&
           self->_enabled[self->_selected] &&
           self->_v[self->_selected].submenu.menu->visible )
        {
          assert ( has_focus );
          ui_element_draw ( self->_v[self->_selected].submenu.menu,
                            fb, fb_width,
                            self->_v[self->_selected].submenu.has_focus );
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

  ui_menu_t *self;

  
  self= UI_MENU(s);
  self->visible= val;
  if ( !val ) self->_selected= -1;
  
} // end set_visible


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_menu_t *self;
  
  
  self= UI_MENU(s);
  if ( !self->visible ) return false;

  // Si hi ha algun submenú obert processa
  // NOTA!! Faig molta comprovació redundant
  if ( self->_selected != -1 &&
       self->_v[self->_selected].type==UI_MENU_SUBMENU &&
       self->_enabled[self->_selected] &&
       self->_v[self->_selected].submenu.menu->visible )
    {
      if ( ui_element_mouse_event ( self->_v[self->_selected].submenu.menu,
                                    event ) )
        {
          self->_v[self->_selected].submenu.has_focus= true;
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
      if ( event->motion.x >= self->_x &&
           event->motion.x < (self->_x+self->_w) &&
           event->motion.y >= self->_y &&
           event->motion.y < (self->_y+self->_h) )
        {
          mouse_selection ( self, event->motion.y );
          return true;
        }
      // NOTA!! No lleve el focus perque igual no l'agafa ningú
      else return false;
      break;
    case SDL_MOUSEBUTTONDOWN:
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
        {
          mouse_selection ( self, event->button.y );
          if ( self->_selected != -1 && event->button.button == 1 )
            switch ( self->_v[self->_selected].type )
              {
              case UI_MENU_ACTION:
                if ( self->_v[self->_selected].action.action != NULL )
                  self->_v[self->_selected].action.action
                    ( UI_ELEMENT(self),
                      self->_v[self->_selected].action.udata );
                break;
              case UI_MENU_SUBMENU:
                break;
              case UI_MENU_CHOICE:
                *(self->_v[self->_selected].choice.var)=
                  self->_v[self->_selected].choice.val;
                if ( self->_v[self->_selected].choice.action != NULL )
                  self->_v[self->_selected].choice.action
                    ( UI_ELEMENT(self),
                      self->_v[self->_selected].choice.udata );
                break;
              default: printf("ui_menu - mouse_event : WTF !!!\n");
              }
          return true;
        }
      break;
    case SDL_MOUSEBUTTONUP:
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
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

  ui_menu_t *self;
  bool ret;
  int p;

  
  self= UI_MENU(s);

  // Si hi ha algun submenú obert amb el FOCUS processa
  // NOTA!! Faig molta comprovació redundant
  if ( self->_selected != -1 &&
       self->_v[self->_selected].type==UI_MENU_SUBMENU &&
       self->_enabled[self->_selected] &&
       self->_v[self->_selected].submenu.menu->visible &&
       self->_v[self->_selected].submenu.has_focus &&
       ui_element_key_event ( self->_v[self->_selected].submenu.menu,
                              event ) )
    ret= true;
  
  // Comportament normal
  else if ( event->type == SDL_KEYDOWN )
    {
      switch ( event->key.keysym.sym )
        {
        case SDLK_DOWN: // Mou següent
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
        case SDLK_UP: // Mou anterior
          if ( self->_selected != -1 )
            {
              // Si és l'entrada de més dalt de tot permitim que el
              // menú pare la seleccione.
              if ( self->_selected == 0 ) ret= false;
              else
                {
                  for ( p= self->_selected-1;
                        p >= 0 && !self->_enabled[p];
                        --p );
                  if ( p >= 0 ) self->_selected= p;
                  ret= true;
                }
            }
          else ret= true;
          break;
        case SDLK_SPACE:
        case SDLK_RETURN: // Acció
          if ( self->_selected != -1 )
            {
              switch ( self->_v[self->_selected].type )
                {
                case UI_MENU_ACTION:
                  if ( self->_v[self->_selected].action.action != NULL )
                    self->_v[self->_selected].action.action
                      ( UI_ELEMENT(self),
                        self->_v[self->_selected].action.udata );
                  break;
                case UI_MENU_SUBMENU:
                  self->_v[self->_selected].submenu.has_focus= true;
                  break;
                case UI_MENU_CHOICE:
                  *(self->_v[self->_selected].choice.var)=
                    self->_v[self->_selected].choice.val;
                  if ( self->_v[self->_selected].choice.action != NULL )
                    self->_v[self->_selected].choice.action
                      ( UI_ELEMENT(self),
                        self->_v[self->_selected].choice.udata );
                  break;
                default: printf("ui_menu - key_event : WTF !!!\n");
                }
            }
          ret= true;
          break;
        case SDLK_LEFT: // Si estic en un submenu lleva el focus.
          if ( self->_selected != -1 &&
               self->_v[self->_selected].type==UI_MENU_SUBMENU &&
               self->_enabled[self->_selected] &&
               self->_v[self->_selected].submenu.menu->visible )
            {
              self->_v[self->_selected].submenu.has_focus= false;
              ret= true;
            }
          else ret= false;
          break;
        case SDLK_RIGHT: // Si estic en un submenu dona el focus.
          if ( self->_selected != -1 &&
               self->_v[self->_selected].type==UI_MENU_SUBMENU &&
               self->_enabled[self->_selected] &&
               self->_v[self->_selected].submenu.menu->visible )
            {
              self->_v[self->_selected].submenu.has_focus= true;
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

ui_menu_t *
ui_menu_new (
             const int x,
             const int y
             )
{

  ui_menu_t *new;


  // Reserva memòria.
  assert ( x >= 0 && y >= 0 );
  new= g_new ( ui_menu_t, 1 );
  new->_x= x;
  new->_y= y;
  new->_w= PADDING_X*2 + 9; // Deixa espai per a un caràcter a l'esquerra
  new->_h= PADDING_Y*2;
  new->_selected= -1;
  new->_v= g_new ( ui_menu_entry_t, 1 );
  new->_enabled= g_new ( bool, 1 );
  new->_end_y= g_new ( int, 1 );
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
  
} // end ui_menu_new


void
ui_menu_enable_entry (
                      ui_menu_t  *self,
                      const int   entry,
                      const bool  val
                      )
{

  assert ( entry >= 0 && entry < self->_N );
  if ( self->_v[entry].type == UI_MENU_SEPARATOR )
    error ( "no es pot habilitar/deshabilitar un separador" );
  self->_enabled[entry]= val;
  if ( !val && self->_selected == entry )
    {
      self->_selected= -1;
      select_first ( self );
    }
  
} // end ui_menu_enabled_entry


void
ui_menu_show (
              ui_menu_t *self,
              const int  x,
              const int  y
              )
{

  assert ( x>=0 && y>= 0 );
  self->_x= x;
  self->_y= y;
  self->visible= true;
  
} // end ui_menu_show


int
ui_menu_add_entry_action (
                          ui_menu_t   *self,
                          const char  *label,
                          ui_action_t *action,
                          void        *user_data
                          )
{

  int ret,length,new_h;
  

  // Crea.
  ret= get_new_entry ( self );
  self->_enabled[ret]= true;
  self->_v[ret].action.type= UI_MENU_ACTION;
  self->_v[ret].action.label= g_strdup ( label );
  self->_v[ret].action.action= action;
  self->_v[ret].action.udata= user_data;

  // Actualitza dimensions
  length= (strlen ( label ) + 1)*9 + PADDING_X*2;
  if ( length > self->_w ) self->_w= length;
  new_h= 16+PADDING_Y*2;
  if ( ret == 0 ) self->_h= new_h;
  else            self->_h+= new_h;
  self->_end_y[ret]= (ret==0 ? 0 : self->_end_y[ret-1]) + new_h;
  
  return ret;
  
} // end ui_menu_add_entry_action


int
ui_menu_add_entry_separator (
                             ui_menu_t *self
                             )
{

  int ret,new_h;
  

  // Crea.
  ret= get_new_entry ( self );
  self->_enabled[ret]= false;
  self->_v[ret].type= UI_MENU_SEPARATOR;
  
  // Actualitza dimensions
  new_h= 1+PADDING_Y*2;
  if ( ret == 0 ) self->_h= new_h;
  else            self->_h+= new_h;
  self->_end_y[ret]= (ret==0 ? 0 : self->_end_y[ret-1]) + new_h;
  
  return ret;
  
} // end ui_menu_add_entry_separator


int
ui_menu_add_entry_submenu (
                          ui_menu_t  *self,
                          const char *label, // CP437
                          ui_menu_t  *menu
                          )
{

  int ret,length,new_h;
  

  // Crea.
  ret= get_new_entry ( self );
  self->_enabled[ret]= true;
  self->_v[ret].submenu.type= UI_MENU_SUBMENU;
  self->_v[ret].submenu.label= g_strdup ( label );
  self->_v[ret].submenu.menu= menu;
  self->_v[ret].submenu.has_focus= false;
  assert ( menu != NULL );

  // Actualitza dimensions
  // --> NOTA!! Mostrem la label (un espai mínim) i una fletxa
  length= (strlen ( label ) + 2 + 1)*9 + PADDING_X*2;
  if ( length > self->_w ) self->_w= length;
  new_h= 16+PADDING_Y*2;
  if ( ret == 0 ) self->_h= new_h;
  else            self->_h+= new_h;
  self->_end_y[ret]= (ret==0 ? 0 : self->_end_y[ret-1]) + new_h;
  
  return ret;
  
} // end ui_menu_add_entry_submenu


int
ui_menu_add_entry_choice (
                          ui_menu_t   *self,
                          const char  *label,
                          int         *var,
                          const int    val,
                          ui_action_t *action,
                          void        *user_data
                          )
{

  int ret,length,new_h;
  
  
  // Crea.
  ret= get_new_entry ( self );
  self->_enabled[ret]= true;
  self->_v[ret].choice.type= UI_MENU_CHOICE;
  self->_v[ret].choice.label= g_strdup ( label );
  self->_v[ret].choice.action= action;
  self->_v[ret].choice.udata= user_data;
  self->_v[ret].choice.var= var; assert ( var != NULL );
  self->_v[ret].choice.val= val;

  // Actualitza dimensions
  length= (strlen ( label ) + 1)*9 + PADDING_X*2;
  if ( length > self->_w ) self->_w= length;
  new_h= 16+PADDING_Y*2;
  if ( ret == 0 ) self->_h= new_h;
  else            self->_h+= new_h;
  self->_end_y[ret]= (ret==0 ? 0 : self->_end_y[ret-1]) + new_h;
  
  return ret;
  
} // end ui_menu_add_entry_choice


void
ui_menu_set_min_width (
                       ui_menu_t *self,
                       const int  width
                       )
{

  if ( width > self->_w )
    self->_w= width;
  
} // end ui_menu_set_min_width
