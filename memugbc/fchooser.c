/*
 * Copyright 2015-2024 Adrià Giménez Pastor.
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
 *  fchooser.c - Implementació de 'fchooser.h'.
 *
 */

#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "dirs.h"
#include "effects.h"
#include "error.h"
#include "fchooser.h"
#include "filesel.h"
#include "mpad.h"
#include "screen.h"
#include "t8biso.h"
#include "tiles8b.h"




/**********/
/* MACROS */
/**********/

// Amplaria dels banners.
#define FCBANNER_CDIR_WIDTH (WIDTH-16-2)
#define FCBANNER_WIDTH (FCBANNER_CDIR_WIDTH-8)




/*************/
/* CONSTANTS */
/*************/

static const unsigned char FOLDER[8]=
  {
    0x00,0x00,0x00,0x70,0x7E,0x7E,0x7E,0x7E
  };




/*********/
/* ESTAT */
/*********/






/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
draw_background (
                 fchooser_t *fc
                 )
{
  memcpy ( fc->fb, fc->background, sizeof(fc->fb) );
} // end draw_background


static void
draw_rectangle (
                fchooser_t  *fc,
        	const int    x,
        	const int    y,
        	const int    width,
        	const int    height,
        	const int    color
        	)
{

  int off, row, col;
  int *p;
  
  
  off= y*WIDTH + x;
  for ( row= 0; row < height; ++row, off+= WIDTH )
    {
      p= &(fc->fb[off]);
      for ( col= 0; col < width; ++col )
        p[col]= color;
    }
  
} // end draw_rectangle


static void
draw_folder (
             fchooser_t  *fc,
             const int    x,
             const int    y,
             const int    color
             )
{

  int off, row, col;
  int *p;
  unsigned char b;
  

  off= y*WIDTH + x;
  for ( row= 0; row < 8; ++row, off+= WIDTH )
    {
      p= &(fc->fb[off]);
      b= FOLDER[row];
      for ( col= 0; col < 8; ++col )
        {
          if ( b&0x80 ) p[col]= color;
          b<<= 1;
        }
    }
  
} // end draw_folder


static void
draw_scrollbar (
                fchooser_t  *fc,
        	const int    x,
        	const int    y,
        	const int    width,
        	const int    height,
        	const int    pos,
        	const int    nitems,
        	const int    N,  // total items.
        	const int    color
        	)
{
  
  int y_sc, height_sc;
  double seg_size;

  
  // Fons.
  effect_fade ( fc->fb, x, y, width, height, 0.50 );

  // Barra.
  seg_size= height / (double) N;
  y_sc= y + (int) (seg_size*pos + 0.5);
  height_sc= (int) (seg_size*nitems + 0.5);
  draw_rectangle ( fc, x, y_sc, width, height_sc, color );
  
} // end draw_scrollbar


static const gchar *
get_entry_name (
                fchooser_t *fc,
                const int   entry
                )
{

  if ( fc->v[entry].path_name==NULL ) return "..";
  else if ( fc->v[entry].path_name[0]=='\0' ) return "--EMPTY--";
  else return fc->v[entry].path_name;
  
} // end get_entry_name


// Pot tornar NULL.
static gchar *
fchooser_read_cdir (
                    fchooser_t *fc
                    )
{

  gchar *fn, *cdir;
  gsize length;
  gboolean ret;
  
  
  fn= g_build_filename ( get_sharedir (), "cdir", NULL );
  cdir= NULL;
  ret= g_file_get_contents ( fn, &cdir, &length, NULL );
  if ( ret )
    {
      if ( fc->verbose )
        fprintf ( stderr, "S'ha llegit el directori actual de '%s'\n", fn );
      cdir[length-1]= '\0';
    }
  else cdir= NULL;
  g_free ( fn );

  return cdir;
  
} // end fchooser_read_cdir


static void
fchooser_write_cdir (
                     fchooser_t  *fc,
        	     const gchar *cdir
        	     )
{

  gchar *fn;
  GError *err;
  

  fn= g_build_filename ( get_sharedir (), "cdir", NULL );
  err= NULL;
  if ( !g_file_set_contents ( fn, cdir, strlen ( cdir ) + 1, &err ) )
    {
      warning ( "no s'ha pogut desar el directori actual en '%s': %s",
        	fn, err->message );
      g_error_free ( err );
    }
  else if ( fc->verbose )
    fprintf ( stderr, "S'ha escrit el directori actual en '%s'\n", fn );
  g_free ( fn );
  
} // end fchooser_write_cdir


static void
fchooser_clean (
                fchooser_t *fc
                )
{

  guint n;

  
  if ( fc->cdir_name != NULL ) g_free ( fc->cdir_name );
  fc->cdir_name= NULL;
  for ( n= 0; n < fc->N; ++n )
    if ( fc->v[n].path_name != NULL )
      g_free ( fc->v[n].path_name );
  
} // end fchooser_clean


static void
fchooser_add_entry (
                    fchooser_t     *fc,
        	    const gchar    *path,
        	    const gboolean  is_dir
        	    )
{

  // Reserva memòria.
  if ( fc->N == fc->size )
    {
      if ( fc->size == 0 ) fc->size= 1;
      else                 fc->size*= 2;
      fc->v= g_renew ( fchooser_node_t, fc->v, fc->size );
    }

  // Afegeix.
  fc->v[fc->N].path= path;
  fc->v[fc->N].path_name=
    path==NULL ? NULL :
    ( path[0]=='\0' ? g_strdup ( path ) : g_path_get_basename ( path ) );
  fc->v[fc->N].is_dir= is_dir;
  fc->v[fc->N].banner= NULL;
  ++fc->N;
  
} // end fchooser_add_entry


// Té que estar net.
static void
fchooser_fill_entries (
                       fchooser_t *fc
                       )
{
  
  const GSList *p;
  guint n;
  t8biso_banner_t *b;
  
  
  // Ompli directori actual.
  fc->cdir= filesel_get_current_dir ( fc->fsel );
  fc->cdir_name= g_path_get_basename ( fc->cdir );
  t8biso_banner_set_msg ( &(fc->banners[0]),
        		  fc->cdir_name,
        		  FCBANNER_CDIR_WIDTH );
  
  // Ompli les entrades.
  fc->N= 0;
  if ( fc->empty_entry )
    fchooser_add_entry ( fc, "", FALSE );
  fchooser_add_entry ( fc, NULL /*..*/, TRUE );
  for ( p= filesel_get_dirs ( fc->fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    fchooser_add_entry ( fc, (const gchar *) p->data, TRUE );
  for ( p= filesel_get_files ( fc->fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    fchooser_add_entry ( fc, (const gchar *) p->data, FALSE );

  // Inicialitza la posició.
  fc->first= 0;
  fc->current= fc->empty_entry ? 1 : 0;
  fc->last= MIN ( fc->N, FCNVIS ) - 1;
  
  // Assigna els banners.
  for ( n= fc->first, b= &(fc->banners[1]);
        n <= fc->last;
        ++n, ++b )
    {
      fc->v[n].banner= b;
      t8biso_banner_set_msg ( b,
                              get_entry_name ( fc, n ),
        		      FCBANNER_WIDTH );
      if ( n != fc->current ) t8biso_banner_pause ( b );
    }
  
} // end fchooser_fill_entries


static void
fchooser_change_dir (
                     fchooser_t  *fc,
        	     const gchar *path
        	     )
{

  filesel_change_dir ( fc->fsel, path );
  fchooser_clean ( fc );
  fchooser_fill_entries ( fc );
  
} // end fchooser_change_dir


static void
fchooser_change_parent_dir (
                            fchooser_t *fc
                            )
{

  filesel_change_parent_dir ( fc->fsel );
  fchooser_clean ( fc );
  fchooser_fill_entries ( fc );
  
} // end fchooser_change_parent_dir


static void
fchooser_move_up (
                  fchooser_t *fc
                  )
{

  t8biso_banner_t *b;
  
  
  // No pot fer res.
  if ( fc->current == 0 ) return;
  
  // Para el banner actual.
  t8biso_banner_pause ( fc->v[fc->current].banner );
  
  // No es desplaça.
  if ( fc->current > fc->first )
    {
      --fc->current;
      t8biso_banner_resume ( fc->v[fc->current].banner );
      return;
    }
  
  // Estem en el primer per tant ens desplacem elminant l'últim.
  --fc->first;
  --fc->current;
  b= fc->v[fc->last].banner;
  t8biso_banner_set_msg ( b,
                          get_entry_name ( fc, fc->current ),
        		  FCBANNER_WIDTH );
  fc->v[fc->current].banner= b;
  t8biso_banner_resume ( b );
  --fc->last;
  
} // end fchooser_move_up


static void
fchooser_move_down (
                    fchooser_t *fc
                    )
{
  
  t8biso_banner_t *b;
  
  
  // No pot fer res.
  if ( fc->current == fc->N-1 ) return;

  // Para el banner actual.
  t8biso_banner_pause ( fc->v[fc->current].banner );
  
  // No es desplaça.
  if ( fc->current < fc->last )
    {
      ++fc->current;
      t8biso_banner_resume ( fc->v[fc->current].banner );
      return;
    }

  // Estem en l'última per tant ens desplacem elminant el primer.
  ++fc->last;
  ++fc->current;
  b= fc->v[fc->first].banner;
  t8biso_banner_set_msg ( b,
                          get_entry_name ( fc, fc->current ),
        		  FCBANNER_WIDTH );
  fc->v[fc->current].banner= b;
  t8biso_banner_resume ( b );
  ++fc->first;
  
} // end fchooser_move_down


// Torna el fitxer actual o NULL si no hi ha cap fitxer (per exemple
// era un directori)
static const char *
fchooser_select_current_file (
                              fchooser_t *fc
                              )
{

  const fchooser_node_t *node;
  const gchar *ret;
  

  node= &(fc->v[fc->current]);
  if ( node->is_dir )
    {
      if ( node->path == NULL ) fchooser_change_parent_dir ( fc );
      else                      fchooser_change_dir ( fc, node->path );
      ret= NULL;
    }
  else ret= node->path;
  
  return (const char *) ret;
  
} // end fchooser_select_current_file


// Dibuixa background, fchooser i updateja la pantalla.
static void
fchooser_draw (
               fchooser_t *fc
               )
{
  
  static const int OFFY= 8;
  
  guint n;
  int offy;
  const fchooser_node_t *node;
  
  
  // Background
  draw_background ( fc );

  // Directori actual.
  effect_fade ( fc->fb, 8, OFFY, WIDTH-16, 10, 0.70 );
  t8biso_banner_draw ( &(fc->banners[0]), fc->fb, WIDTH, 9, OFFY+1,
        	       fc->fgcolor_cdir, 0, T8BISO_BG_TRANS );

  // Entrades.
  offy= OFFY+18;
  for ( n= fc->first; n <= fc->last; ++n )
    {
      node= &(fc->v[n]);
      effect_interpolate_color ( fc->fb, 8, offy,
                                 WIDTH-16, 10, 0x7FFF, 0.60 );
      if ( node->is_dir ) draw_folder ( fc, 8, offy, fc->fgcolor_entry );
      t8biso_banner_draw ( node->banner, fc->fb, WIDTH, 8+9, offy+1,
        		   (n==fc->current) ?
        		   fc->fgcolor_selected_entry :
        		   fc->fgcolor_entry,
        		   0, T8BISO_BG_TRANS );
      offy+= 11; // 1 píxel blanc.
    }

  // Barra lliscant
  if ( fc->N > FCNVIS )
    draw_scrollbar ( fc, WIDTH-5, OFFY+18, 2, 11*FCNVIS-1, fc->first,
        	     FCNVIS, fc->N, fc->fgcolor_sc );
  
  // Updateja.
  screen_update ( fc->fb, NULL );
  
} // end fchooser_draw


static void
error_dialog_draw (
                   fchooser_t *fc
                   )
{

  /* NOTA: "Error!! :(" -> 10 caracters. Quadrat de 12x3, centrat queda com:
   *   - x: 4 (32)
   *   - y: 7 (56)
   *   - width: 12 (96)
   *   - height: 3 (24)
   */
  static const int x= 4;
  static const int y= 7;
  static const int width= 12;
  static const int height= 3;
  static const int fgcolor= 0x3FF;
  static const int bgcolor= 0x01F;


  draw_rectangle ( fc, x*8-1, y*8-1, width*8+2, height*8+2, 0x000 );
  draw_rectangle ( fc, x*8, y*8, width*8, height*8, bgcolor );
  tiles8b_draw_string ( fc->fb, WIDTH, "ERROR!!", x+1, y+1,
        		fgcolor, bgcolor, 0 );
  t8biso_draw_string ( fc->fb, WIDTH, ":(", x+1+8, y+1,
        	       fgcolor, bgcolor, 0 );
  screen_update ( fc->fb, NULL );
  
} // end error_dialog_draw




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
fchooser_free (
               fchooser_t *fc
               )
{

  int i;


  // Escriu cdir.
  fchooser_write_cdir ( fc, fc->cdir );
  
  // Allibera memòria.
  fchooser_clean ( fc );
  for ( i= 0; i <= FCNVIS; ++i )
    t8biso_banner_free ( &(fc->banners[i]) );
  if ( fc->v != NULL ) g_free ( fc->v );
  if ( fc->cdir_name != NULL ) g_free ( fc->cdir_name );
  filesel_free ( fc->fsel );
  g_free ( fc );
  
} // end fchooser_free


fchooser_t *
fchooser_new (
              const gchar    *selector,
              const bool      empty_entry,
              const int      *background,
              const gboolean  verbose
              )
{

  int i;
  gchar *cdir;
  fchooser_t *new;

  new= g_new ( fchooser_t, 1 );
  
  new->verbose= verbose;
  new->background= background;
  
  cdir= fchooser_read_cdir ( new );
  new->empty_entry= empty_entry;
  new->fsel= filesel_new ( cdir, selector );
  if ( cdir != NULL ) g_free ( cdir );
  
  new->cdir= NULL;
  new->cdir_name= NULL;

  new->v= NULL;
  new->size= 0;
  new->N= 0;
  for ( i= 0; i <= FCNVIS; ++i )
    t8biso_banner_init ( &(new->banners[i]) );

  new->fgcolor_cdir= 0x01F; // Roig.
  new->fgcolor_entry= 0x000; // Negre.
  new->fgcolor_selected_entry= 0x01F; // Roig.
  new->fgcolor_sc= 0x01F; // Roig.
  
  fchooser_fill_entries ( new );

  return new;
  
} // end fchooser_new


// Torna -1 per a indicar que cal eixir.
const char *
fchooser_run (
              fchooser_t *fc,
              bool       *quit
              )
{
  
  int buttons;
  const char *ret;
  

  ret= NULL;
  *quit= false;
  for (;;)
    {

      mpad_clear ();
      fchooser_draw ( fc );
      g_usleep ( 20000 ); // 20ms
      
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) { *quit= true; return NULL; }
      else if ( buttons&K_ESCAPE ) return NULL;
      else if ( buttons&K_BUTTON )
        {
          ret= fchooser_select_current_file ( fc );
          if ( ret != NULL ) break;
        }
      else if ( buttons&K_UP )
        fchooser_move_up ( fc );
      else if ( buttons&K_DOWN )
        fchooser_move_down ( fc );
      
    }
  
  return ret;
  
} // end fchooser_run


int
fchooser_error_dialog (
                       fchooser_t *fc
                       )
{
  
  int buttons;
  
  
  error_dialog_draw ( fc ); // No canvia l'estat.
  for (;;)
    {
      mpad_clear ();
      g_usleep ( 20000 );
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) return -1;
      else if ( buttons&(K_ESCAPE|K_BUTTON) ) return 0;
    }
  
  return 0;
  
} // end fchooser_error_dialog
