/*
 * Copyright 2015-2023 Adrià Giménez Pastor.
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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  mainmenu.c - Implementació de 'mainmenu.h'.
 *
 */


#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "background.h"
#include "conf.h"
#include "dirs.h"
#include "effects.h"
#include "error.h"
#include "filesel.h"
#include "frontend.h"
#include "hud.h"
#include "mainmenu.h"
#include "menu.h"
#include "mpad.h"
#include "rom.h"
#include "screen.h"
#include "t8biso.h"
#include "tiles16b.h"




/**********/
/* MACROS */
/**********/

/* Número de entrades visible. */
#define FCNVIS 16

/* Amplaria dels banners. */
#define FCBANNER_CDIR_WIDTH (BG_WIDTH-16-2)
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

  gboolean         is_dir;
  const gchar     *path;    /* Punter a filesel_t. NULL significa ".." */
  gchar           *path_name;
  t8biso_banner_t *banner;
  
} fchooser_node_t;




/*********/
/* ESTAT */
/*********/

/* Verbositat. */
static gboolean _verbose;

/* Configuració per defecte. */
conf_t *_conf;

/* Frame buffer. */
static int _fb[BG_WIDTH*BG_HEIGHT];

/* Background. */
static int _background[BG_WIDTH*BG_HEIGHT];


/* Selector de fitxers. */
static struct
{

  filesel_t       *fsel;
  
  const gchar     *cdir; /* Punter a fsel. */
  gchar           *cdir_name;

  fchooser_node_t *v;
  guint            size;
  guint            N;
  t8biso_banner_t  banners[FCNVIS+1]; /* 0 és per a cdir. */

  guint            first;
  guint            last;
  guint            current;

  int              fgcolor_cdir;
  int              fgcolor_entry;
  int              fgcolor_selected_entry;
  int              fgcolor_sc;
  
} _fchooser;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

/*** COMMON *******************************************************************/
static void
init_background (void)
{

  int i;


  for ( i= 0; i < BG_WIDTH*BG_HEIGHT; ++i )
    _background[i]= (int) BACKGROUND[i];
  
} /* end init_background */


static void
draw_background (void)
{
  memcpy ( _fb, _background, sizeof(_fb) );
} /* end draw_background */


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
  

  off= y*BG_WIDTH + x;
  for ( row= 0; row < height; ++row, off+= BG_WIDTH )
    {
      p= &(_fb[off]);
      for ( col= 0; col < width; ++col )
        p[col]= color;
    }
  
} /* end draw_rectangle */


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
  

  off= y*BG_WIDTH + x;
  for ( row= 0; row < 8; ++row, off+= BG_WIDTH )
    {
      p= &(_fb[off]);
      b= FOLDER[row];
      for ( col= 0; col < 8; ++col )
        {
          if ( b&0x80 ) p[col]= color;
          b<<= 1;
        }
    }
  
} /* end draw_folder */


static void
draw_scrollbar (
        	const int x,
        	const int y,
        	const int width,
        	const int height,
        	const int pos,
        	const int nitems,
        	const int N,  /* total items. */
        	const int color
        	)
{

  int y_sc, height_sc;
  double seg_size;

  
  /* Fons. */
  effect_fade ( _fb, BG_WIDTH, x, y, width, height, 0.50 );

  /* Barra. */
  seg_size= height / (double) N;
  y_sc= y + (int) (seg_size*pos + 0.5);
  height_sc= (int) (seg_size*nitems + 0.5);
  draw_rectangle ( x, y_sc, width, height_sc, color );
  
} /* end draw_scrollbar */


static void
error_dialog_draw (void)
{

  /* NOTA: "Error!! :(" -> 10 caracters. Quadrat de 12x4, centrat queda com:
   *   - x: 14 (112)
   *   - y: 13 (104)
   *   - width: 12 (96)
   *   - height: 4 (32)
   */
  static const int x= 14;
  static const int y= 13;
  static const int width= 12;
  static const int height= 4;
  static const int fgcolor= 0x01F;
  static const int fgcolor2= 0x03F;
  static const int bgcolor= 0x007;


  draw_rectangle ( x*8-1, y*8-1, width*8+2, height*8+2, 0x000 );
  draw_rectangle ( x*8, y*8, width*8, height*8, bgcolor );
  tiles16b_draw_string ( _fb, BG_WIDTH, "ERROR!!", x+1, (y+1)/2,
        		 fgcolor, fgcolor2, bgcolor, 0 );
  t8biso_draw_string ( _fb, BG_WIDTH, ":(", x+1+8, y+1,
        	       fgcolor2, bgcolor, 0 );
  screen_update ( _fb, NULL );
  
} /* end error_dialog_draw */


/* Un -1 indica que s'ha forçat l'eixida. */
static int
error_dialog (void)
{

  int buttons;

  
  error_dialog_draw (); /* No canvia l'estat. */
  for (;;)
    {
      mpad_clear ();
      g_usleep ( 20000 );
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) return -1;
      else if ( buttons&(K_ESCAPE|K_BUTTON) ) return 0;
    }
  
  return 0;
  
} /* end error_dialog */


/* Torna CONTINUE, ERROR o QUIT. */
static int
run_romfn (
           const gchar *fn
           )
{

  MD_Rom rom;
  MD_RomHeader header;
  const char *rom_id;
  menu_response_t ret;
  

  /* Inicialitza. */
  if ( load_rom ( fn, &rom, _verbose ) != 0 )
    return ERROR;
  MD_rom_get_header ( &rom, &header );
  rom_id= get_rom_id ( &rom, &header, _verbose );
  
  /* Executa. */
  ret= frontend_run ( &rom, &header, rom_id, NULL, NULL, NULL,
        	      MENU_MODE_INGAME_MAINMENU, _verbose );
  
  /* Tanca. */
  MD_rom_free ( &rom );
  screen_sres_changed ( BG_WIDTH, BG_HEIGHT, NULL );
  
  return ret==MENU_QUIT ? QUIT : CONTINUE;
  
} /* end run_romfn */


/*** FCHOOSER *****************************************************************/
/* Pot tornar NULL. */
static gchar *
fchooser_read_cdir (void)
{

  gchar *fn, *cdir;
  gsize length;
  gboolean ret;
  
  
  fn= g_build_filename ( get_sharedir (), "cdir", NULL );
  cdir= NULL;
  ret= g_file_get_contents ( fn, &cdir, &length, NULL );
  if ( ret )
    {
      if ( _verbose )
        fprintf ( stderr, "S'ha llegit el directori actual de '%s'\n", fn );
      cdir[length-1]= '\0';
    }
  else cdir= NULL;
  g_free ( fn );

  return cdir;
  
} /* end fchooser_read_cdir */


static void
fchooser_write_cdir (
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
  else if ( _verbose )
    fprintf ( stderr, "S'ha escrit el directori actual en '%s'\n", fn );
  g_free ( fn );
  
} /* end fchooser_write_cdir */


static void
fchooser_clean (void)
{

  guint n;

  
  if ( _fchooser.cdir_name != NULL ) g_free ( _fchooser.cdir_name );
  _fchooser.cdir_name= NULL;
  for ( n= 0; n < _fchooser.N; ++n )
    if ( _fchooser.v[n].path_name != NULL )
      g_free ( _fchooser.v[n].path_name );
  
} /* end fchooser_clean_entries */


static void
fchooser_add_entry (
        	    const gchar    *path,
        	    const gboolean  is_dir
        	    )
{

  /* Reserva memòria. */
  if ( _fchooser.N == _fchooser.size )
    {
      if ( _fchooser.size == 0 ) _fchooser.size= 1;
      else                       _fchooser.size*= 2;
      _fchooser.v= g_renew ( fchooser_node_t, _fchooser.v, _fchooser.size );
    }

  /* Afegeix. */
  _fchooser.v[_fchooser.N].path= path;
  _fchooser.v[_fchooser.N].path_name=
    path==NULL ? NULL : g_path_get_basename ( path );
  _fchooser.v[_fchooser.N].is_dir= is_dir;
  _fchooser.v[_fchooser.N].banner= NULL;
  ++_fchooser.N;
  
} /* end fchooser_add_entry */


/* Té que estar net. */
static void
fchooser_fill_entries (void)
{

  const GSList *p;
  guint n;
  t8biso_banner_t *b;
  
  
  /* Ompli directori actual. */
  _fchooser.cdir= filesel_get_current_dir ( _fchooser.fsel );
  _fchooser.cdir_name= g_path_get_basename ( _fchooser.cdir );
  t8biso_banner_set_msg ( &(_fchooser.banners[0]),
        		  _fchooser.cdir_name,
        		  FCBANNER_CDIR_WIDTH );
  
  /* Ompli les entrades. */
  _fchooser.N= 0;
  fchooser_add_entry ( NULL /*..*/, TRUE );
  for ( p= filesel_get_dirs ( _fchooser.fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    fchooser_add_entry ( (const gchar *) p->data, TRUE );
  for ( p= filesel_get_files ( _fchooser.fsel );
        p != NULL;
        p= g_slist_next ( p ) )
    fchooser_add_entry ( (const gchar *) p->data, FALSE );

  /* Inicialitza la posició. */
  _fchooser.first= _fchooser.current= 0;
  _fchooser.last= MIN ( _fchooser.N, FCNVIS ) - 1;

  /* Assigna els banners. */
  for ( n= _fchooser.first, b= &(_fchooser.banners[1]);
        n <= _fchooser.last;
        ++n, ++b )
    {
      _fchooser.v[n].banner= b;
      t8biso_banner_set_msg ( b,
        		      _fchooser.v[n].path_name==NULL ? ".." :
        		      _fchooser.v[n].path_name,
        		      FCBANNER_WIDTH );
      if ( n != _fchooser.current ) t8biso_banner_pause ( b );
    }
  
} /* end fchooser_fill_entries */


static void
close_fchooser (void)
{

  int i;


  /* Escriu cdir. */
  fchooser_write_cdir ( _fchooser.cdir );
  
  /* Allibera memòria. */
  fchooser_clean ();
  for ( i= 0; i <= FCNVIS; ++i )
    t8biso_banner_free ( &(_fchooser.banners[i]) );
  if ( _fchooser.v != NULL ) g_free ( _fchooser.v );
  if ( _fchooser.cdir_name != NULL ) g_free ( _fchooser.cdir_name );
  filesel_free ( _fchooser.fsel );
  
} /* end close_fchooser */


static void
init_fchooser (void)
{

  int i;
  gchar *cdir;

  
  cdir= fchooser_read_cdir ();
  _fchooser.fsel= filesel_new ( cdir, "[.](gen|bin|md)$" );
  if ( cdir != NULL ) g_free ( cdir );
  
  _fchooser.cdir= NULL;
  _fchooser.cdir_name= NULL;
  
  _fchooser.v= NULL;
  _fchooser.size= 0;
  _fchooser.N= 0;
  for ( i= 0; i <= FCNVIS; ++i )
    t8biso_banner_init ( &(_fchooser.banners[i]) );
  
  _fchooser.fgcolor_cdir= 0x007; /* Roig. */
  _fchooser.fgcolor_entry= 0x000; /* Negre. */
  _fchooser.fgcolor_selected_entry= 0x007; /* Roig. */
  _fchooser.fgcolor_sc= 0x007; /* Roig. */
  
  fchooser_fill_entries ();
  
} /* end init_fchooser */


static void
fchooser_change_dir (
        	     const gchar *path
        	     )
{

  filesel_change_dir ( _fchooser.fsel, path );
  fchooser_clean ();
  fchooser_fill_entries ();
  
} /* end fchooser_change_dir */


static void
fchooser_change_parent_dir (void)
{

  filesel_change_parent_dir ( _fchooser.fsel );
  fchooser_clean ();
  fchooser_fill_entries ();
  
} /* end fchooser_change_parent_dir */


static void
fchooser_move_up (void)
{

  t8biso_banner_t *b;

  
  /* No pot fer res. */
  if ( _fchooser.current == 0 ) return;

  /* Para el banner actual. */
  t8biso_banner_pause ( _fchooser.v[_fchooser.current].banner );
  
  /* No es desplaça. */
  if ( _fchooser.current > _fchooser.first )
    {
      --_fchooser.current;
      t8biso_banner_resume ( _fchooser.v[_fchooser.current].banner );
      return;
    }

  /* Estem en el primer per tant ens desplacem elminant l'últim. */
  --_fchooser.first;
  --_fchooser.current;
  b= _fchooser.v[_fchooser.last].banner;
  t8biso_banner_set_msg ( b,
        		  _fchooser.v[_fchooser.current].path_name==NULL ?
        		  ".." : _fchooser.v[_fchooser.current].path_name,
        		  FCBANNER_WIDTH );
  _fchooser.v[_fchooser.current].banner= b;
  t8biso_banner_resume ( b );
  --_fchooser.last;
  
} /* end fchooser_move_up */


static void
fchooser_move_down (void)
{

  t8biso_banner_t *b;
  
  
  /* No pot fer res. */
  if ( _fchooser.current == _fchooser.N-1 ) return;
  
  /* Para el banner actual. */
  t8biso_banner_pause ( _fchooser.v[_fchooser.current].banner );
  
  /* No es desplaça. */
  if ( _fchooser.current < _fchooser.last )
    {
      ++_fchooser.current;
      t8biso_banner_resume ( _fchooser.v[_fchooser.current].banner );
      return;
    }

  /* Estem en l'última per tant ens desplacem elminant el primer. */
  ++_fchooser.last;
  ++_fchooser.current;
  b= _fchooser.v[_fchooser.first].banner;
  t8biso_banner_set_msg ( b,
        		  _fchooser.v[_fchooser.current].path_name==NULL ?
        		  ".." : _fchooser.v[_fchooser.current].path_name,
        		  FCBANNER_WIDTH );
  _fchooser.v[_fchooser.current].banner= b;
  t8biso_banner_resume ( b );
  ++_fchooser.first;
  
} /* end fchooser_move_down */


/* Un -1 vol dir que s'ha forçat l'eixida. */
static int
fchooser_select_current_file (void)
{

  const fchooser_node_t *node;
  int ret;
  
  
  node= &(_fchooser.v[_fchooser.current]);
  if ( node->is_dir )
    {
      if ( node->path == NULL ) fchooser_change_parent_dir ();
      else                      fchooser_change_dir ( node->path );
    }
  else
    {
      ret= run_romfn ( node->path );
      switch ( ret )
        {
        case ERROR: return error_dialog ();
        case QUIT: return -1;
        default: break;
        }

    }
  
  return 0;
  
} /* end fchooser_select_current_file */


/* Dibuixa background, fchooser i updateja la pantalla. */
static void
fchooser_draw (void)
{

  static const int OFFY= 10;

  guint n;
  int offy;
  const fchooser_node_t *node;
  
  
  /* Background */
  draw_background ();

  /* Directori actual. */
  effect_fade ( _fb, BG_WIDTH, 8, OFFY, BG_WIDTH-16, 10, 0.70 );
  t8biso_banner_draw ( &(_fchooser.banners[0]), _fb, BG_WIDTH, 9, OFFY+1,
        	       _fchooser.fgcolor_cdir, 0, T8BISO_BG_TRANS );

  /* Entrades. */
  offy= OFFY+18;
  for ( n= _fchooser.first; n <= _fchooser.last; ++n )
    {
      node= &(_fchooser.v[n]);
      effect_interpolate_color ( _fb, BG_WIDTH, 8, offy,
        			 BG_WIDTH-16, 10, 0x1FF, 0.60 );
      if ( node->is_dir ) draw_folder ( 8, offy, _fchooser.fgcolor_entry );
      t8biso_banner_draw ( node->banner, _fb, BG_WIDTH, 8+9, offy+1,
        		   (n==_fchooser.current) ?
        		   _fchooser.fgcolor_selected_entry :
        		   _fchooser.fgcolor_entry,
        		   0, T8BISO_BG_TRANS );
      offy+= 11; /* 1 píxel blanc. */
    }

  /* Barra deslliçant. */
  if ( _fchooser.N > FCNVIS )
    draw_scrollbar ( BG_WIDTH-5, OFFY+18, 2, 11*FCNVIS-1, _fchooser.first,
        	     FCNVIS, _fchooser.N, _fchooser.fgcolor_sc );
  
  /* Updateja. */
  screen_update ( _fb, NULL );
  
} /* end fchooser_draw */


/* Torna -1 per a indicar que cal eixir. */
static int
fchooser_run (void)
{
  
  int buttons, ret;
  

  screen_sres_changed ( BG_WIDTH, BG_HEIGHT, NULL );
  for (;;)
    {

      mpad_clear ();
      fchooser_draw ();
      g_usleep ( 20000 ); /* 20ms */
      
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) return -1;
      else if ( buttons&K_ESCAPE )
        {
          if ( menu_run ( MENU_MODE_MAINMENU ) == MENU_QUIT )
            return -1;
        }
      else if ( buttons&K_BUTTON )
        {
          ret= fchooser_select_current_file ();
          if ( ret == -1 ) return ret;
        }
      else if ( buttons&K_UP )
        fchooser_move_up ();
      else if ( buttons&K_DOWN )
        fchooser_move_down ();
      
    }
  
  return 0;
  
} /* end fchooser_run */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
main_menu (
           conf_t         *conf,
           const gboolean  verbose
           )
{

  /* Prepara. */
  _verbose= verbose;
  _conf= conf;
  hud_hide ();
  init_background ();
  init_fchooser ();

  /* Executa. */
  fchooser_run ();

  /* Allibera. */
  close_fchooser ();
  
} /* end main_menu */
