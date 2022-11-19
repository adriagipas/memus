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
 *  ui_filechooser.c - Implementació de 'ui_filechooser.h'.
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
#include "ui_file_chooser.h"
#include "vgafont.h"




/**********/
/* MACROS */
/**********/

#define FG_COLOR SCREEN_PAL_BLACK
#define SEL_COLOR SCREEN_PAL_RED

#define SEP 2
#define PADDING_X 2
#define PADDING_Y 1

#define WIDTH_CHARS 50




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static ui_fchooser_render_mng_t *
render_mng_new (void)
{

  ui_fchooser_render_mng_t *new;
  int i;
  

  new= g_new ( ui_fchooser_render_mng_t, 1 );
  new->free_nodes= NULL;
  for ( i= 0; i <= UI_FILE_CHOOSER_ROWS; ++i )
    {
      new->v[i].N= 0;
      new->v[i].capacity= 1;
      new->v[i].v= g_new ( int, 16*1 );
      new->v[i].next= new->free_nodes;
      new->free_nodes= &(new->v[i]);
    }

  return new;
  
} // end render_mng_new


static void
render_mng_free (
                 ui_fchooser_render_mng_t *mng
                 )
{

  int i;


  for ( i= 0; i <= UI_FILE_CHOOSER_ROWS; ++i )
    g_free ( mng->v[i].v );
  g_free ( mng );
  
} // end render_mng_free


// Torna un node i rendertiza en este el nom del fitxer (UTF8)
static ui_fchooser_render_node_t *
render_mng_get_node (
                     ui_fchooser_render_mng_t *mng,
                     const char               *string
                     )
{

  ui_fchooser_render_node_t *ret;
  int max_length,length;

  
  // Obté el node
  if ( mng->free_nodes == NULL )
    error ( "ui_file_chooser.c - render_mng_get: no hi han nodes disponibles" );
  ret= mng->free_nodes;
  mng->free_nodes= ret->next;

  // Reserva memòria
  max_length= strlen ( string ) * 9;
  if ( max_length > ret->capacity )
    {
      ret->capacity= max_length;
      ret->v= g_renew ( int, ret->v, max_length*16 );
    }

  // Renderitza.
  length= vgafont_draw_string_utf8 ( ret->v, ret->capacity, string,
                                     0, 0, FG_COLOR, SCREEN_PAL_WHITE,
                                     VGAFONT_XY_PIXELS );
  ret->N= length*9;
  
  return ret;
  
} // end render_mng_get_node


static void
render_mng_free_node (
                      ui_fchooser_render_mng_t  *mng,
                      ui_fchooser_render_node_t *node
                      )
{

  node->next= mng->free_nodes;
  mng->free_nodes= node;
  
} // end render_mng_free_node


static void
add_entry (
           ui_file_chooser_t *self,
           const gchar       *path,
           const bool         is_dir
           )
{

  int tmp;

  
  // Reserva memòria.
  if ( self->_N == self->_size_nodes )
    {
      tmp= self->_size_nodes*2;
      if ( tmp < self->_size_nodes )
        error ( "cannot allocate memory" );
      self->_size_nodes= tmp;
      self->_nodes=
        g_renew ( ui_fchooser_node_t, self->_nodes, self->_size_nodes );
    }

  // Afegeix.
  self->_nodes[self->_N].path= path;
  self->_nodes[self->_N].path_name=
    path==NULL ? NULL :
    ( path[0]=='\0' ? g_strdup ( path ) : g_path_get_basename ( path ) );
  self->_nodes[self->_N].is_dir= is_dir;
  self->_nodes[self->_N].p= NULL;
  ++self->_N;
  
} // end add_entry


static const gchar *
get_entry_name (
                ui_file_chooser_t *self,
                const int          entry
                )
{
  
  if ( self->_nodes[entry].path_name==NULL ) return "..";
  else if ( self->_nodes[entry].path_name[0]=='\0' ) return "--EMPTY--";
  else return self->_nodes[entry].path_name;
  
} // end get_entry_name


static void
recalc_max_width_chars (
                        ui_file_chooser_t *self
                        )
{

  guint n;
  int width,tmp;


  width= self->_cdir_render_node->N;
  for ( n= self->_first; n <= self->_last; ++n )
    {
      tmp= self->_nodes[n].p->N;
      if ( self->_nodes[n].is_dir )
        tmp+= 9;
      if ( tmp > width ) width= tmp;
    }
  self->_max_width_chars= width/9;
  
} // end recalc_max_width_chars


static void
fill_nodes (
            ui_file_chooser_t *self
            )
{

  const GSList *p;
  guint n;
  
  
  // Ompli directori actual.
  self->_cdir= filesel_get_current_dir ( self->_fsel );
  self->_cdir_name= g_path_get_basename ( self->_cdir );
  self->_cdir_render_node=
    render_mng_get_node ( self->_render, self->_cdir_name );

  // Ompli les entrades.
  self->_N= 0;
  if ( self->_empty_entry )
    add_entry ( self, "", false );
  add_entry ( self, NULL /*..*/, true );
  for ( p= filesel_get_dirs ( self->_fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    add_entry ( self, (const gchar *) p->data, true );
  if ( self->_show_files )
    for ( p= filesel_get_files ( self->_fsel );
          p != NULL;
          p= g_slist_next ( p ) )
      add_entry ( self, (const gchar *) p->data, false );

  // Inicialitza la posició.
  self->_first= 0;
  self->_current= self->_empty_entry ? 1 : 0;
  self->_last= MIN ( self->_N, UI_FILE_CHOOSER_ROWS ) - 1;
  
  // Assigna els render nodes
  for ( n= self->_first; n <= self->_last; ++n )
    self->_nodes[n].p= 
      render_mng_get_node ( self->_render, get_entry_name ( self, n ) );

  // Recalcula posició columna.
  recalc_max_width_chars ( self );
  self->_ccol= 0;
  
} // end fill_nodes


static void
clean_nodes (
             ui_file_chooser_t *self
             )
{

  guint n;

  
  if ( self->_cdir_name != NULL ) g_free ( self->_cdir_name );
  self->_cdir_name= NULL;
  render_mng_free_node ( self->_render, self->_cdir_render_node );
  self->_cdir_render_node= NULL;
  for ( n= 0; n < self->_N; ++n )
    {
      if ( self->_nodes[n].path_name != NULL )
        {
          g_free ( self->_nodes[n].path_name );
          self->_nodes[n].path_name= NULL;
        }
      if ( self->_nodes[n].p != NULL )
        {
          render_mng_free_node ( self->_render, self->_nodes[n].p );
          self->_nodes[n].p= NULL;
        }
    }
  
} // end clean_nodes


static void
move_down (
           ui_file_chooser_t *self,
           const int          desp
           )
{

  int inc;
  guint n;
  
  
  // No pot fer res.
  if ( self->_current == self->_N-1 ) return;
  
  // No es desplaça.
  if ( self->_current+desp <= self->_last )
    {
      self->_current+= desp;
      return;
    }

  // Estem en l'última per tant ens desplacem elminant el primer.
  inc= desp - (self->_last-self->_current);
  if ( (self->_last+inc) >= self->_N )
    inc= (self->_N-self->_last)-1;
  self->_last+= inc;
  self->_current+= desp;
  if ( self->_current > self->_last ) self->_current= self->_last;
  for ( n= 0; n < inc; ++n )
    {
      render_mng_free_node ( self->_render, self->_nodes[self->_first+n].p );
      self->_nodes[self->_first+n].p= NULL;
    }
  for ( n= 0; n < inc; ++n )
    self->_nodes[self->_last-n].p= 
      render_mng_get_node ( self->_render,
                            get_entry_name ( self, self->_last-n ) );
  self->_first+= inc;
  
} // end move_down


static void
move_up (
         ui_file_chooser_t *self,
         const int          desp
         )
{

  int dec;
  guint n;

  
  // No pot fer res.
  if ( self->_current == 0 ) return;

  // No es desplaça.
  if ( desp <= (self->_current-self->_first) )
    {
      self->_current-= desp;
      return;
    }

  // Estem en el primer per tant ens desplacem elminant l'últim.
  dec= desp - (self->_current-self->_first);
  if ( dec > self->_first ) dec= self->_first;
  self->_first-= dec;
  if ( desp >= self->_current ) self->_current= 0;
  else                          self->_current-= desp;
  for ( n= 0; n < dec; ++n )
    {
      render_mng_free_node ( self->_render, self->_nodes[self->_last-n].p );
      self->_nodes[self->_last-n].p= NULL;
    }
  for ( n= 0; n < dec; ++n )
    self->_nodes[self->_first+n].p= 
      render_mng_get_node ( self->_render,
                            get_entry_name ( self, self->_first+n ) );
  self->_last-= dec;
  
} // end move_up


static void
vsb_action (
            ui_element_t *e,
            void         *udata
            )
{

  ui_scrollbar_t *sb;
  ui_file_chooser_t *fc;
  guint n;
  

  sb= UI_SCROLLBAR(e);
  fc= UI_FILE_CHOOSER(udata);

  // Allibera
  for ( n= fc->_first; n <= fc->_last; ++n )
    {
      render_mng_free_node ( fc->_render, fc->_nodes[n].p );
      fc->_nodes[n].p= NULL;
    }

  // Nous valors
  fc->_first= ui_scrollbar_get_pos ( sb );
  fc->_last= MIN ( fc->_first + UI_FILE_CHOOSER_ROWS, fc->_N ) - 1;
  if ( fc->_current < fc->_first ) fc->_current= fc->_first;
  else if ( fc->_current > fc->_last ) fc->_current= fc->_last;

  // Renderitza nou
  for ( n= fc->_first; n <= fc->_last; ++n )
    fc->_nodes[n].p= 
      render_mng_get_node ( fc->_render, get_entry_name ( fc, n ) );
  
} // end vsb_action


static void
hsb_action (
            ui_element_t *e,
            void         *udata
            )
{

  ui_scrollbar_t *sb;
  ui_file_chooser_t *fc;


  sb= UI_SCROLLBAR(e);
  fc= UI_FILE_CHOOSER(udata);
  if ( fc->_max_width_chars > WIDTH_CHARS )
    fc->_ccol= ui_scrollbar_get_pos ( sb );
  
} // end hsb_action


static void
change_dir (
            ui_file_chooser_t *self,
            const gchar       *path
            )
{

  filesel_change_dir ( self->_fsel, path );
  clean_nodes ( self );
  fill_nodes ( self );
  
} // end change_dir


static void
change_parent_dir (
                   ui_file_chooser_t *self
                   )
{
  
  filesel_change_parent_dir ( self->_fsel );
  clean_nodes ( self );
  fill_nodes ( self );
  
} // end change_parent_dir


static void
select_current_file (
                     ui_file_chooser_t *self
                     )
{

  const ui_fchooser_node_t *node;


  node= &(self->_nodes[self->_current]);
  if ( node->is_dir )
    {
      if ( node->path == NULL ) change_parent_dir ( self );
      else                      change_dir ( self, node->path );
    }
  else if ( self->_action != NULL )
    self->_action ( UI_ELEMENT(self), self->_udata );
  
} // end select_current_file


static int
get_clicked_entry (
                   ui_file_chooser_t *self,
                   const int          mouse_y
                   )
{

  int r;
  guint n;
  

  r= self->_y + 16 + SEP*2 + PADDING_Y*2;
  for ( n= self->_first; n <= self->_last; ++n )
    {
      if ( mouse_y >= r && mouse_y < (r+16+PADDING_Y*2) )
        return (int) (n-self->_first);
      r+= 16 + SEP + PADDING_Y*2;
    }

  return -1;
  
} // end get_clicked_entry




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_file_chooser_t *self;
  guint n;
  
  
  self= UI_FILE_CHOOSER(s);
  ui_element_free ( self->_vsb );
  ui_element_free ( self->_hsb );
  if ( self->_cdir_name != NULL )
    g_free ( self->_cdir_name );
  for ( n= 0; n < self->_N; ++n )
    if ( self->_nodes[n].path_name != NULL )
      g_free ( self->_nodes[n].path_name );
  g_free ( self->_nodes );
  filesel_free ( self->_fsel );
  render_mng_free ( self->_render );
  g_free ( self );
  
} // end free_


static void
render_node_in_fb (
                   ui_file_chooser_t               *self,
                   int                             *fb,
                   const int                        fb_width,
                   const int                        y,
                   const ui_fchooser_render_node_t *node,
                   const bool                       draw_folder,
                   const int                        fg_color,
                   const int                        bg_color
                   )
{
  
  static const unsigned char FOLDER[16]=
    {
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,
      0xFC,0xFC,0xFC,0xFC,0x00,0x00,0x00,0x00
    };
 
  
  int r,off,c,i,off_node,j,pad_folder;
  unsigned char byte;
  

  // Dibuixar caixa
  for ( r= y, i= 0; i < 16 + PADDING_Y*2; ++i, ++r )
    {
      off= r*fb_width + self->_x;
      for ( c= 0; c < WIDTH_CHARS*9 + PADDING_X*2; ++c )
        fb[off+c]= bg_color;
    }
  
  // Dibuixa carpeta
  if ( self->_ccol == 0 && draw_folder )
    {
      for ( r= y+PADDING_Y, i= 0; i < 16; ++i, ++r )
        {
          off= r*fb_width + self->_x + PADDING_X + 1; // ha d'ocupar 9 bits
          byte= FOLDER[i];
          for ( j= 0; j < 8; ++j )
            {
              if ( byte&0x80 )
                fb[off+j]= FG_COLOR;
              byte<<= 1;
            }
        }
    }
  
  // Dibuixa text del node
  pad_folder= 9;
  if ( self->_ccol*9 < (node->N+pad_folder) )
    {
      for ( r= y+PADDING_Y, i= 0; i < 16; ++i, ++r )
        {
          off= r*fb_width + self->_x + PADDING_X;
          off_node= i*node->capacity;
          for ( c= 0, j= self->_ccol*9-pad_folder;
                c < WIDTH_CHARS*9 && j < node->N;
                ++c, ++j )
            if ( j >= 0 && node->v[off_node+j] == FG_COLOR )
              fb[off+c]= fg_color;
        }
    }
  
} // end render_node


static void
draw (
      ui_element_t *s,
      int          *fb,
      const int     fb_width,
      const bool    has_focus
      )
{

  ui_file_chooser_t *self;
  int r;
  guint n;
  
  
  self= UI_FILE_CHOOSER(s);
  if ( self->visible )
    {

      r= self->_y;
      
      // Dibuixa nom carpeta
      render_node_in_fb ( self, fb, fb_width, r,
                          self->_cdir_render_node,
                          false, SEL_COLOR, SCREEN_PAL_GRAY_5 );
      r+= 16 + SEP*2 + PADDING_Y*2;

      // Dibuixa contingut
      for ( n= self->_first; n <= self->_last; ++n )
        {
          render_node_in_fb ( self, fb, fb_width, r,
                              self->_nodes[n].p,
                              self->_nodes[n].is_dir,
                              n==self->_current?SEL_COLOR:FG_COLOR,
                              SCREEN_PAL_GRAY_2 );
          r+= 16 + SEP + PADDING_Y*2;
        }

      // Dibuixa scrollbars
      // --> Vertical
      if ( self->_first > 0 || self->_last < (self->_N-1) )
        ui_scrollbar_set_state ( self->_vsb, self->_N,
                                 UI_FILE_CHOOSER_ROWS, self->_first );
      else
        ui_scrollbar_set_state ( self->_vsb, UI_FILE_CHOOSER_ROWS,
                                 UI_FILE_CHOOSER_ROWS, 0 );
      ui_element_draw ( self->_vsb, fb, fb_width, has_focus );
      // --> Horizontal
      if ( self->_max_width_chars > WIDTH_CHARS )
        ui_scrollbar_set_state ( self->_hsb, self->_max_width_chars,
                                 WIDTH_CHARS, self->_ccol );
      else
        ui_scrollbar_set_state ( self->_hsb, WIDTH_CHARS, WIDTH_CHARS, 0 );
      ui_element_draw ( self->_hsb, fb, fb_width, has_focus );
      
    }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_file_chooser_t *self;
  int entry;
  
  
  self= UI_FILE_CHOOSER(s);
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
      if ( event->wheel.y < 0 ) move_down ( self, 1 );
      else if ( event->wheel.y > 0 ) move_up ( self, 1 );
      return true;
      break;
    case SDL_MOUSEBUTTONDOWN: // Per a agafar focus
      if ( event->button.x >= self->_x &&
           event->button.x < (self->_x+self->_w) &&
           event->button.y >= self->_y &&
           event->button.y < (self->_y+self->_h) )
        {
          if ( event->button.button == 1 &&
               event->button.state == SDL_PRESSED )
            {
              entry= get_clicked_entry ( self, event->button.y );
              if ( entry != -1 )
                {
                  self->_current= self->_first + entry;
                  if ( event->button.clicks > 1 )
                    select_current_file ( self );
                }
            }
          return true;
        }
    }
  
  return false;
  
} // end mouse_event


static bool
key_event (
           ui_element_t    *s,
           const SDL_Event *event
           )
{

  ui_file_chooser_t *self;
  bool ret;

  
  self= UI_FILE_CHOOSER(s);
  if ( event->type == SDL_KEYDOWN &&
       event->key.keysym.mod == 0 )
    {
      switch ( event->key.keysym.sym )
        {
        case SDLK_DOWN:
          move_down ( self, event->key.repeat ? 3 : 1 );
          ret= true;
          break;
        case SDLK_UP:
          move_up ( self, event->key.repeat ? 3 : 1 );
          ret= true;
          break;
        case SDLK_PAGEDOWN:
          move_down ( self, UI_FILE_CHOOSER_ROWS );
          ret= true;
          break;
        case SDLK_PAGEUP:
          move_up ( self, UI_FILE_CHOOSER_ROWS );
          ret= true;
          break;
        case SDLK_RETURN:
        case SDLK_SPACE:
          select_current_file ( self );
          ret= true;
          break;
        case SDLK_RIGHT:
          if ( self->_max_width_chars > WIDTH_CHARS )
            {
              self->_ccol+= event->key.repeat ? 3 : 1;
              if ( self->_ccol >= (self->_max_width_chars-WIDTH_CHARS) )
                self->_ccol= self->_max_width_chars-WIDTH_CHARS;
            }
          ret= true;
          break;
        case SDLK_LEFT:
          self->_ccol-= event->key.repeat ? 3 : 1;
          if ( self->_ccol < 0 )
            self->_ccol= 0;
          ret= true;
          break;
        default:
          ret= false;
        }
    }
  else ret= false;
  
  return ret;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_file_chooser_t *
ui_file_chooser_new (
                     const int    x,
                     const int    y,
                     const gchar *cdir,
                     const bool   show_files,
                     const bool   empty_entry,
                     const gchar *selector,
                     ui_action_t *action,
                     void        *user_data
                     )
{

  ui_file_chooser_t *new;
  

  // Reserva memòria
  assert ( x>=0 && y>= 0 );
  new= g_new ( ui_file_chooser_t, 1 );
  new->_x= x;
  new->_y= y;
  new->_w= WIDTH_CHARS*9 + PADDING_X*2 + SEP;
  new->_h= UI_FILE_CHOOSER_ROWS*(16 + SEP + PADDING_Y*2)
    + SEP + 16 + PADDING_Y*2 + SEP;
  new->_show_files= show_files;
  new->_empty_entry= empty_entry;
  new->_fsel= filesel_new ( cdir, selector );
  new->_render= render_mng_new ();
  new->_nodes= g_new ( ui_fchooser_node_t, 1 );
  new->_nodes[0].path_name= NULL;
  new->_N= 0;
  new->_size_nodes= 1;
  new->_cdir= NULL;
  new->_cdir_name= NULL;
  new->_action= action;
  new->_udata= user_data;
  fill_nodes ( new );

  // Crea scrollbars
  new->_vsb= ui_scrollbar_new ( x + new->_w,
                                y + 16 + 2*SEP + PADDING_Y*2,
                                UI_FILE_CHOOSER_ROWS*(16+SEP+PADDING_Y*2)-SEP,
                                true, vsb_action, new );
  new->_hsb= ui_scrollbar_new ( x, y + new->_h,
                                WIDTH_CHARS*9+PADDING_X*2, false,
                                hsb_action, new );
  
  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;
  
  return new;
  
} // end ui_file_chooser_new


const gchar *
ui_file_chooser_get_current_dir (
                                 ui_file_chooser_t *self
                                 )
{
  return self->_cdir;
} // end ui_file_chooser_get_current_dir


const gchar *
ui_file_chooser_get_current_file (
                                  ui_file_chooser_t *self
                                  )
{
  return self->_nodes[self->_current].path;
} // end ui_file_chooser_get_current_file
