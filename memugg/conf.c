/*
 * Copyright 2014-2022 Adrià Giménez Pastor.
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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  conf.c - Implementació de 'conf.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "dirs.h"
#include "error.h"




/**********/
/* MACROS */
/**********/

#define GROUP "main"




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_conf_file_name (
        	    const char *rom_id
        	    )
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_confdir (), rom_id, NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, ".cfg" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_conf_file_name */


static gchar *
get_default_conf_file_name (void)
{
  return g_build_filename ( get_confdir (), "default.cfg", NULL );
} /* end get_default_conf_file_name */


static void
set_default_values (
        	    conf_t *conf
        	    )
{
  
  conf->keys.up= SDLK_UP;
  conf->keys.left= SDLK_LEFT;
  conf->keys.down= SDLK_DOWN;
  conf->keys.right= SDLK_RIGHT;
  conf->keys.tl= SDLK_z;
  conf->keys.tr= SDLK_x;
  conf->keys.start= SDLK_SPACE;
  conf->screen_size= SCREEN_SIZE_WIN_X3;
  conf->scaler= SCALER_NONE;
  conf->vsync= TRUE;
  
} /* end set_default_values */


/* Si existeix el camp el guarda en dest. */
static void
get_key_value (
               GKeyFile    *f,
               const gchar *name,
               SDL_Keycode *dest
               )
{
  
  gchar *val;
  SDL_Keycode aux;
  
  
  val= g_key_file_get_string ( f, GROUP, name, NULL );
  if ( val == NULL ) return;
  aux= SDL_GetKeyFromName ( val );
  if ( aux == SDLK_UNKNOWN )
    warning ( "'%s' no és un nom de tecla vàlid,"
              " '%s' tindrà el seu valor per defecte",
              val, name );
  else *dest= aux;
  g_free ( val );
  
} /* end get_key_value */


static void
read_conf (
           conf_t     *conf,
           const char *name,
           const int   verbose
           )
{
  
  GKeyFile *f;
  gboolean ret;
  GError *err;
  gchar *val;
  
  
  if ( verbose )
    fprintf ( stderr, "Llegint fitxer de configuració: %s\n", name );
  
  /* Obri. */
  f= g_key_file_new ();
  err= NULL;
  ret= g_key_file_load_from_file ( f, name, G_KEY_FILE_NONE, &err );
  if ( !ret )
    error ( "no s'ha pogut obrir el fitxer de configuració '%s': %s",
            name, err->message );
  
  /* Fixa valors. */
  get_key_value ( f, "key-up", &(conf->keys.up) );
  get_key_value ( f, "key-down", &(conf->keys.down) );
  get_key_value ( f, "key-left", &(conf->keys.left) );
  get_key_value ( f, "key-right", &(conf->keys.right) );
  get_key_value ( f, "key-tl", &(conf->keys.tl) );
  get_key_value ( f, "key-tr", &(conf->keys.tr) );
  get_key_value ( f, "key-start", &(conf->keys.start) );
  val= g_key_file_get_string ( f, GROUP, "screen-size", NULL );
  if ( val != NULL )
    {
      if ( !strcmp ( val, "win_x1" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X1;
      else if ( !strcmp ( val, "win_x2" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X2;
      else if ( !strcmp ( val, "win_x3" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X3;
      else if ( !strcmp ( val, "win_x4" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X4;
      else if ( !strcmp ( val, "fullscreen" ) )
        conf->screen_size= SCREEN_SIZE_FULLSCREEN;
      else warning ( "grandària de finestra no suportada: %s", val );
      g_free ( val );
    }
  val= g_key_file_get_string ( f, GROUP, "scaler", NULL );
  if ( val != NULL )
    {
      if ( !strcmp ( val, "none" ) ) conf->scaler= SCALER_NONE;
      else if ( !strcmp ( val, "scale2x" ) ) conf->scaler= SCALER_SCALE2X;
      else warning ( "tipus d'escalat desconegut: %s", val );
      g_free ( val );
    }
  conf->vsync= g_key_file_get_boolean ( f, GROUP, "vsync", &err );
  
  /* Allibera memòria. */
  g_key_file_free ( f );
  
} /* end read_conf */


static void
set_key_value (
               GKeyFile    *f,
               const gchar *name,
               SDL_Keycode  val
               )
{
  
  const char *aux;
  
  
  aux= SDL_GetKeyName ( val );
  assert ( aux != NULL );
  g_key_file_set_string ( f, GROUP, name, aux );
  
} /* end set_key_value */


static void
write_conf_common (
        	   const conf_t *conf,
        	   const gchar  *name,
        	   const int     verbose
        	   )
{
  
  const gchar *val;
  GKeyFile *f;
  gboolean ret;
  GError *err;
  
  
  /* Inicialitza. */
  if ( verbose )
    fprintf ( stderr, "Escrivint configuració en '%s'\n", name );
  f= g_key_file_new ();
  
  /* Tecles. */
  set_key_value ( f, "key-up", conf->keys.up );
  set_key_value ( f, "key-down", conf->keys.down );
  set_key_value ( f, "key-left", conf->keys.left );
  set_key_value ( f, "key-right", conf->keys.right );
  set_key_value ( f, "key-tl", conf->keys.tl );
  set_key_value ( f, "key-tr", conf->keys.tr );
  set_key_value ( f, "key-start", conf->keys.start );
  
  /* Grandària pantalla. */
  switch ( conf->screen_size )
    {
    case SCREEN_SIZE_WIN_X1: val= "win_x1"; break;
    case SCREEN_SIZE_WIN_X2: val= "win_x2"; break;
    case SCREEN_SIZE_WIN_X3: val= "win_x3"; break;
    case SCREEN_SIZE_WIN_X4: val= "win_x4"; break;
    case SCREEN_SIZE_FULLSCREEN: val= "fullscreen"; break;
    default: val= NULL;
    }
  assert ( val != NULL );
  if ( val != NULL ) g_key_file_set_string ( f, GROUP, "screen-size", val );
  
  /* Scaler. */
  switch ( conf->scaler )
    {
    case SCALER_SCALE2X: val= "scale2x"; break;
    case SCALER_NONE: val= "none"; break;
    default: val= NULL;
    }
  assert ( val != NULL );
  if ( val != NULL ) g_key_file_set_string ( f, GROUP, "scaler", val );

  /* Vsync. */
  g_key_file_set_boolean ( f, GROUP, "vsync", conf->vsync );
  
  /* Escriu. */
  err= NULL;
  ret= g_key_file_save_to_file ( f, name, &err );
  if ( !ret )
    error ( "no s'ha pogut escriure la configuració en '%s': %s",
            name, err->message );
  
  /* Allibera. */
  g_key_file_free ( f );
  
} /* end write_conf_common */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
get_conf (
          conf_t       *conf,
          const char   *rom_id,
          const char   *conf_fn, /* Pot ser NULL. */
          const conf_t *default_conf,   /* Pot ser NULL. */
          const int     verbose
          )
{
  
  const gchar *name;
  
  
  if ( default_conf == NULL ) set_default_values ( conf );
  else memcpy ( conf, default_conf, sizeof(conf_t) );
  name= conf_fn==NULL ? get_conf_file_name ( rom_id ) : conf_fn;
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) ) /* Existeix */
    read_conf ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} /* end get_conf */


void
get_default_conf (
        	  conf_t     *conf,
        	  const char *conf_fn, /* Pot ser NULL. */
        	  const int   verbose
        	  )
{
  
  const gchar *name;
  
  
  set_default_values ( conf );
  name= conf_fn==NULL ? get_default_conf_file_name () : conf_fn;
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) ) /* Existeix */
    read_conf ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} /* end get_conf */


void
write_conf (
            const conf_t *conf,
            const char   *rom_id,
            const char   *conf_fn,
            const int     verbose
            )
{
  
  const gchar *name;
  
  
  name= conf_fn==NULL ? get_conf_file_name ( rom_id ) : conf_fn;
  write_conf_common ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} /* end write_conf */


void
write_default_conf (
        	    const conf_t *conf,
        	    const char   *conf_fn,
        	    const int     verbose
        	    )
{
  
  const gchar *name;
  
  
  name= conf_fn==NULL ? get_default_conf_file_name () : conf_fn;
  write_conf_common ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} /* end write_default_conf */
