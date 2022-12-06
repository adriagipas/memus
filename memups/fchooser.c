/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dirs.h"
#include "error.h"
#include "fchooser.h"
#include "filesel.h"
/*
#include "hud.h"
*/
#include "mpad.h"
#include "screen.h"
#include "t8biso.h"
#include "tiles16b.h"
#include "tiles8b.h"
#include "windowtex.h"




/**********/
/* MACROS */
/**********/

// El 90% de 320x240
#define WIDTH 288
#define HEIGHT 216

// Número de entrades visible.
#define FCNVIS 16

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
/* TIPUS */
/*********/

enum {
  CONTINUE,
  ERROR,
  QUIT,
};

typedef struct
{

  bool             is_dir;
  const gchar     *path;    // Punter a filesel_t. NULL significa ".."
  gchar           *path_name;
  t8biso_banner_t *banner;
  
} fchooser_node_t;

enum
  {
   BGCOLOR= 0,
   BGCOLOR_CDIR,
   BGCOLOR_ENTRY,
   FGCOLOR_CDIR,
   FGCOLOR_ENTRY,
   FGCOLOR_SELECTED_ENTRY,
   FGCOLOR_SC,
   BGCOLOR_ERROR,
   FGCOLOR_ERROR_1,
   FGCOLOR_ERROR_2,
   FGCOLOR_TITLE,
   
   NUM_COLORS
  };




/*********/
/* ESTAT */
/*********/

// Títol.
static const gchar *_title;
  
// Verbositat.
static bool _verbose;

// Frame buffer.
static int _fb[WIDTH*HEIGHT];

// Textura utilitzada per a dibuixar.
static tex_t *_fb_tex;


// Selector de fitxers.
static struct
{

  filesel_t       *fsel;
  
  const gchar     *cdir; // Punter a fsel.
  gchar           *cdir_name;

  fchooser_node_t *v;
  guint            size;
  guint            N;
  t8biso_banner_t  banners[FCNVIS+1]; // 0 és per a cdir.

  guint            first;
  guint            last;
  guint            current;

  int              fgcolor_cdir;
  int              fgcolor_entry;
  int              fgcolor_selected_entry;
  int              fgcolor_sc;

  bool             empty_entry;
  
} _fchooser;

// Paleta de colors.
static uint32_t _pal[NUM_COLORS];




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static uint32_t
get_color (
           const uint8_t r,
           const uint8_t g,
           const uint8_t b,
           const uint8_t a
           )
{

  uint32_t ret;


  ((uint8_t *) &ret)[0]= r;
  ((uint8_t *) &ret)[1]= g;
  ((uint8_t *) &ret)[2]= b;
  ((uint8_t *) &ret)[3]= a;
  
  return ret;
  
} // end get_color


static void
init_pal (void)
{

  _pal[BGCOLOR]= get_color ( 0x00, 0x00, 0x00, 0x7f );
  _pal[BGCOLOR_CDIR]= get_color ( 0x7F, 0x7F, 0x7F, 0x7f );
  _pal[BGCOLOR_ENTRY]= get_color ( 0xFF, 0xFF, 0xFF, 0xbf );
  _pal[FGCOLOR_CDIR]= get_color ( 0xe0, 0x00, 0x00, 0xff );
  _pal[FGCOLOR_ENTRY]= get_color ( 0x00, 0x00, 0x00, 0xff );
  _pal[FGCOLOR_SELECTED_ENTRY]= get_color ( 0xe0, 0x00, 0x00, 0xff );
  _pal[FGCOLOR_SC]= get_color ( 0xe0, 0x00, 0x00, 0xff );
  _pal[BGCOLOR_ERROR]= get_color ( 0xff, 0x00, 0x00, 0xff );
  _pal[FGCOLOR_ERROR_2]= get_color ( 0xff, 0xff, 0x00, 0xff );
  _pal[FGCOLOR_ERROR_1]= get_color ( 0x7f, 0x7f, 0x00, 0xff );
  _pal[FGCOLOR_TITLE]= get_color ( 250, 60, 60, 0xff );
  
} // end init_pal


static void
redraw_fb (void)
{

  const draw_area_t DRAW_AREA= { .x0= 0.05, .x1= 0.95, .y0= 0.05, .y1= 0.95 };

  
  // Copia en textura.
  tex_copy_fb_pal ( _fb_tex, _fb, _pal, WIDTH, HEIGHT );

  // Redibuixa.
  windowtex_draw_begin ();
  screen_draw_body ();
  windowtex_draw_tex ( _fb_tex, NULL, &DRAW_AREA );
  windowtex_draw_end ();
  
} // end redraw_fb


static void
draw_rectangle (
        	const int x,
        	const int y,
        	const int width,
        	const int height,
        	const int color
        	)
{

  int off, row, col;
  int *p;
  

  off= y*WIDTH + x;
  for ( row= 0; row < height; ++row, off+= WIDTH )
    {
      p= &(_fb[off]);
      for ( col= 0; col < width; ++col )
        p[col]= color;
    }
  
} // end draw_rectangle


static void
draw_folder (
             const int x,
             const int y,
             const int color
             )
{

  int off, row, col;
  int *p;
  unsigned char b;
  

  off= y*WIDTH + x;
  for ( row= 0; row < 8; ++row, off+= WIDTH )
    {
      p= &(_fb[off]);
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
        	const int x,
        	const int y,
        	const int width,
        	const int height,
        	const int pos,
        	const int nitems,
        	const int N,  // total items.
        	const int color
        	)
{

  int y_sc, height_sc;
  double seg_size;

  
  // Fons.
  draw_rectangle ( x, y, width, height, BGCOLOR_CDIR );
  
  // Barra.
  seg_size= height / (double) N;
  y_sc= y + (int) (seg_size*pos + 0.5);
  height_sc= (int) (seg_size*nitems + 0.5);
  draw_rectangle ( x, y_sc, width, height_sc, color );
  
} // end draw_scrollbar


static void
error_dialog_draw (void)
{

  static const int x= 11;
  static const int y= 11;
  static const int width= 12;
  static const int height= 4;
  
  
  draw_rectangle ( x*8-1, y*8-1, width*8+2, height*8+2, 0x000 );
  draw_rectangle ( x*8, y*8, width*8, height*8, BGCOLOR_ERROR );
  tiles16b_draw_string ( _fb, WIDTH, "ERROR!!", x+1, (y+1)/2,
        		 FGCOLOR_ERROR_1, FGCOLOR_ERROR_2, BGCOLOR_ERROR, 0 );
  t8biso_draw_string ( _fb, WIDTH, ":(", x+1+8, y+1,
        	       FGCOLOR_ERROR_2, BGCOLOR_ERROR, 0 );
  redraw_fb ();
  
} // end error_dialog_draw


static const gchar *
get_entry_name (
                const int entry
                )
{
  
  if ( _fchooser.v[entry].path_name==NULL ) return "..";
  else if ( _fchooser.v[entry].path_name[0]=='\0' ) return "--EMPTY--";
  else return _fchooser.v[entry].path_name;
  
} // end get_entry_name


// Pot tornar NULL.
static gchar *
fchooser_read_cdir (
                    const gchar *id
                    )
{

  gchar *fn, *cdir;
  gsize length;
  gboolean ret;
  GString *buffer;
  
  
  buffer= g_string_new ( NULL );
  g_string_printf ( buffer, "cdir_%s", id );
  fn= g_build_filename ( get_sharedir (), buffer->str, NULL );
  g_string_free ( buffer, TRUE );
  cdir= NULL;
  ret= g_file_get_contents ( fn, &cdir, &length, NULL );
  if ( ret )
    {
      if ( _verbose )
        fprintf ( stderr, "S'ha llegit el directori actual per"
                  " a '%s' de '%s'\n", id, fn );
      cdir[length-1]= '\0';
    }
  else cdir= NULL;
  g_free ( fn );

  return cdir;
  
} // end fchooser_read_cdir


static void
fchooser_write_cdir (
        	     const gchar *cdir,
                     const gchar *id
        	     )
{

  gchar *fn;
  GError *err;
  GString *buffer;
  
  
  buffer= g_string_new ( NULL );
  g_string_printf ( buffer, "cdir_%s", id );
  fn= g_build_filename ( get_sharedir (), buffer->str, NULL );
  g_string_free ( buffer, TRUE );
  err= NULL;
  if ( !g_file_set_contents ( fn, cdir, strlen ( cdir ) + 1, &err ) )
    {
      warning ( "no s'ha pogut desar el directori actual"
                " per a '%s' en '%s': %s",
        	id, fn, err->message );
      g_error_free ( err );
    }
  else if ( _verbose )
    fprintf ( stderr, "S'ha escrit el directori actual en '%s'\n", fn );
  g_free ( fn );
  
} // end fchooser_write_cdir


static void
fchooser_clean (void)
{

  guint n;

  
  if ( _fchooser.cdir_name != NULL ) g_free ( _fchooser.cdir_name );
  _fchooser.cdir_name= NULL;
  for ( n= 0; n < _fchooser.N; ++n )
    if ( _fchooser.v[n].path_name != NULL )
      g_free ( _fchooser.v[n].path_name );
  
} // end fchooser_clean


static void
fchooser_add_entry (
        	    const gchar *path,
        	    const bool   is_dir
        	    )
{

  // Reserva memòria.
  if ( _fchooser.N == _fchooser.size )
    {
      if ( _fchooser.size == 0 ) _fchooser.size= 1;
      else                       _fchooser.size*= 2;
      _fchooser.v= g_renew ( fchooser_node_t, _fchooser.v, _fchooser.size );
    }

  // Afegeix.
  _fchooser.v[_fchooser.N].path= path;
  _fchooser.v[_fchooser.N].path_name=
    path==NULL ? NULL :
    ( path[0]=='\0' ? g_strdup ( path ) : g_path_get_basename ( path ) );
  _fchooser.v[_fchooser.N].is_dir= is_dir;
  _fchooser.v[_fchooser.N].banner= NULL;
  ++_fchooser.N;
  
} // end fchooser_add_entry


// Té que estar net.
static void
fchooser_fill_entries (void)
{

  const GSList *p;
  guint n;
  t8biso_banner_t *b;
  
  
  // Ompli directori actual.
  _fchooser.cdir= filesel_get_current_dir ( _fchooser.fsel );
  _fchooser.cdir_name= g_path_get_basename ( _fchooser.cdir );
  t8biso_banner_set_msg ( &(_fchooser.banners[0]),
        		  _fchooser.cdir_name,
        		  FCBANNER_CDIR_WIDTH );
  
  // Ompli les entrades.
  _fchooser.N= 0;
  if ( _fchooser.empty_entry )
    fchooser_add_entry ( "", false );
  fchooser_add_entry ( NULL /*..*/, true );
  for ( p= filesel_get_dirs ( _fchooser.fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    fchooser_add_entry ( (const gchar *) p->data, true );
  for ( p= filesel_get_files ( _fchooser.fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    fchooser_add_entry ( (const gchar *) p->data, false );

  // Inicialitza la posició.
  _fchooser.first= 0;
  _fchooser.current= _fchooser.empty_entry ? 1 : 0;
  _fchooser.last= MIN ( _fchooser.N, FCNVIS ) - 1;

  // Assigna els banners.
  for ( n= _fchooser.first, b= &(_fchooser.banners[1]);
        n <= _fchooser.last;
        ++n, ++b )
    {
      _fchooser.v[n].banner= b;
      t8biso_banner_set_msg ( b,
                              get_entry_name ( n ),
        		      FCBANNER_WIDTH );
      if ( n != _fchooser.current ) t8biso_banner_pause ( b );
    }
  
} // end fchooser_fill_entries


static void
fchooser_change_dir (
        	     const gchar *path
        	     )
{

  filesel_change_dir ( _fchooser.fsel, path );
  fchooser_clean ();
  fchooser_fill_entries ();
  
} // end fchooser_change_dir


static void
fchooser_change_parent_dir (void)
{

  filesel_change_parent_dir ( _fchooser.fsel );
  fchooser_clean ();
  fchooser_fill_entries ();
  
} // end fchooser_change_parent_dir


static void
fchooser_move_up (void)
{

  t8biso_banner_t *b;

  
  // No pot fer res.
  if ( _fchooser.current == 0 ) return;

  // Para el banner actual.
  t8biso_banner_pause ( _fchooser.v[_fchooser.current].banner );
  
  // No es desplaça.
  if ( _fchooser.current > _fchooser.first )
    {
      --_fchooser.current;
      t8biso_banner_resume ( _fchooser.v[_fchooser.current].banner );
      return;
    }

  // Estem en el primer per tant ens desplacem elminant l'últim.
  --_fchooser.first;
  --_fchooser.current;
  b= _fchooser.v[_fchooser.last].banner;
  t8biso_banner_set_msg ( b,
                          get_entry_name ( _fchooser.current ),
        		  FCBANNER_WIDTH );
  _fchooser.v[_fchooser.current].banner= b;
  t8biso_banner_resume ( b );
  --_fchooser.last;
  
} // end fchooser_move_up


static void
fchooser_move_down (void)
{

  t8biso_banner_t *b;
  
  
  // No pot fer res.
  if ( _fchooser.current == _fchooser.N-1 ) return;
  
  // Para el banner actual.
  t8biso_banner_pause ( _fchooser.v[_fchooser.current].banner );
  
  // No es desplaça.
  if ( _fchooser.current < _fchooser.last )
    {
      ++_fchooser.current;
      t8biso_banner_resume ( _fchooser.v[_fchooser.current].banner );
      return;
    }

  // Estem en l'última per tant ens desplacem elminant el primer.
  ++_fchooser.last;
  ++_fchooser.current;
  b= _fchooser.v[_fchooser.first].banner;
  t8biso_banner_set_msg ( b,
                          get_entry_name ( _fchooser.current ),
        		  FCBANNER_WIDTH );
  _fchooser.v[_fchooser.current].banner= b;
  t8biso_banner_resume ( b );
  ++_fchooser.first;
  
} // end fchooser_move_down


// Torna el fitxer actual o NULL si no hi ha cap fitxer (per exemple
// era un directori)
static const char *
fchooser_select_current_file (void)
{

  const fchooser_node_t *node;
  const gchar *ret;
  
  
  node= &(_fchooser.v[_fchooser.current]);
  if ( node->is_dir )
    {
      if ( node->path == NULL ) fchooser_change_parent_dir ();
      else                      fchooser_change_dir ( node->path );
      ret= NULL;
    }
  else ret= node->path;
  
  return (const char *) ret;
  
} // end fchooser_select_current_file


// Dibuixa background, fchooser i updateja la pantalla.
static void
fchooser_draw (void)
{

  static const int OFFY= 5;

  guint n;
  int offy;
  const fchooser_node_t *node;
  
  
  // Background
  for ( n= 0; n < WIDTH*HEIGHT; ++n )
    _fb[n]= BGCOLOR;
  
  // Directori actual.
  draw_rectangle ( 8, OFFY, WIDTH-16, 10, BGCOLOR_CDIR );
  t8biso_banner_draw ( &(_fchooser.banners[0]), _fb, WIDTH, 9, OFFY+1,
        	       FGCOLOR_CDIR, 0, T8BISO_BG_TRANS );

  // Entrades.
  offy= OFFY+18;
  for ( n= _fchooser.first; n <= _fchooser.last; ++n )
    {
      node= &(_fchooser.v[n]);
      draw_rectangle ( 8, offy, WIDTH-16, 10, BGCOLOR_ENTRY );
      if ( node->is_dir ) draw_folder ( 8, offy, FGCOLOR_ENTRY );
      t8biso_banner_draw ( node->banner, _fb, WIDTH, 8+9, offy+1,
        		   (n==_fchooser.current) ?
                           FGCOLOR_SELECTED_ENTRY :
                           FGCOLOR_ENTRY,
        		   0, T8BISO_BG_TRANS );
      offy+= 11; // 1 píxel blanc.
    }

  // Títol.
  // NOTA!! Les coordenades són en tiles.
  draw_rectangle ( 0, 26*8-4, WIDTH-3, 1, FGCOLOR_TITLE );
  tiles8b_draw_string ( _fb, WIDTH, _title, 0, 26,
                        FGCOLOR_TITLE, 0, BG_TRANS );
    
  // Barra deslliçant.
  if ( _fchooser.N > FCNVIS )
    draw_scrollbar ( WIDTH-5, OFFY+18, 2, 11*FCNVIS-1, _fchooser.first,
        	     FCNVIS, _fchooser.N, FGCOLOR_SC );
  
  // Updateja.
  redraw_fb ();
  
} // end fchooser_draw




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

const char *
fchooser_run (
              bool *quit
              )
{
  
  int buttons;
  const char *ret;
  

  ret= NULL;
  *quit= false;
  /*
  hud_hide ();
  */
  for (;;)
    {

      mpad_clear ();
      fchooser_draw ();
      g_usleep ( 20000 ); // 20ms
      
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) { *quit= true; return NULL; }
      else if ( buttons&K_ESCAPE ) return NULL;
      else if ( buttons&K_BUTTON )
        {
          ret= fchooser_select_current_file ();
          if ( ret != NULL ) break;
        }
      else if ( buttons&K_UP )
        fchooser_move_up ();
      else if ( buttons&K_DOWN )
        fchooser_move_down ();
      
    }
  
  return ret;
  
} // end fchooser_run


void
close_fchooser (
                const gchar *id
                )
{

  int i;


  // Escriu cdir.
  fchooser_write_cdir ( _fchooser.cdir, id );
  
  // Allibera memòria.
  fchooser_clean ();
  for ( i= 0; i <= FCNVIS; ++i )
    t8biso_banner_free ( &(_fchooser.banners[i]) );
  if ( _fchooser.v != NULL ) g_free ( _fchooser.v );
  if ( _fchooser.cdir_name != NULL ) g_free ( _fchooser.cdir_name );
  filesel_free ( _fchooser.fsel );

  // Allibera textura.
  tex_free ( _fb_tex );
  
} // end close_fchooser


void
init_fchooser (
               const gchar *id,
               const gchar *title,
               const gchar *selector,
               const bool   empty_entry,
               const bool   verbose
               )
{

  int i;
  gchar *cdir;

  
  // Prepara.
  _verbose= verbose;
  _title= title;
  
  // Textura.
  _fb_tex= windowtex_create_tex_fmt ( WIDTH, HEIGHT,
                                      SDL_PIXELFORMAT_RGBA32 );
  if ( SDL_SetTextureBlendMode ( _fb_tex->tex, SDL_BLENDMODE_BLEND ) != 0 )
    error ( "init_fchooser - no s'ha pogut crear la textura per a _fb_tex: %s",
            SDL_GetError () );

  // Paleta.
  init_pal ();
  
  // Selecciona directori.
  cdir= fchooser_read_cdir ( id );
  _fchooser.empty_entry= empty_entry;
  _fchooser.fsel= filesel_new ( cdir, selector );
  if ( cdir != NULL ) g_free ( cdir );
  
  _fchooser.cdir= NULL;
  _fchooser.cdir_name= NULL;
  
  _fchooser.v= NULL;
  _fchooser.size= 0;
  _fchooser.N= 0;
  for ( i= 0; i <= FCNVIS; ++i )
    t8biso_banner_init ( &(_fchooser.banners[i]) );
  
  fchooser_fill_entries ();
  
} // end init_fchooser


int
fchooser_error_dialog (void)
{

  int buttons;

  
  error_dialog_draw (); // No canvia l'estat.
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
