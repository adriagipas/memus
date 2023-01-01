/*
 * Copyright 2014-2023 Adrià Giménez Pastor.
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
  
  conf->keys[0].up= SDLK_UP;
  conf->keys[0].left= SDLK_LEFT;
  conf->keys[0].down= SDLK_DOWN;
  conf->keys[0].right= SDLK_RIGHT;
  conf->keys[0].button_A= SDLK_z;
  conf->keys[0].button_B= SDLK_x;
  conf->keys[0].button_C= SDLK_c;
  conf->keys[0].button_X= SDLK_a;
  conf->keys[0].button_Y= SDLK_s;
  conf->keys[0].button_Z= SDLK_d;
  conf->keys[0].start= SDLK_SPACE;
  
  conf->keys[1].up= SDLK_KP_5;
  conf->keys[1].left= SDLK_KP_1;
  conf->keys[1].down= SDLK_KP_2;
  conf->keys[1].right= SDLK_KP_3;
  conf->keys[1].button_A= SDLK_j;
  conf->keys[1].button_B= SDLK_k;
  conf->keys[1].button_C= SDLK_l;
  conf->keys[1].button_X= SDLK_u;
  conf->keys[1].button_Y= SDLK_i;
  conf->keys[1].button_Z= SDLK_o;
  conf->keys[1].start= SDLK_RETURN;
  
  conf->screen_size= SCREEN_SIZE_WIN_X2;
  conf->scaler= SCALER_NONE;
  conf->vsync= TRUE;
  conf->devs.dev1= MD_IODEV_PAD6B;
  conf->devs.dev2= MD_IODEV_NONE;
  conf->devs.dev_exp= MD_IODEV_NONE;
  
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
get_dev_value (
               GKeyFile    *f,
               const gchar *name,
               MD_IODevice *dest
               )
{

  gchar *val;
  
  
  val= g_key_file_get_string ( f, GROUP, name, NULL );
  if ( val == NULL ) return;
  if ( !strcmp ( val, "none" ) ) *dest= MD_IODEV_NONE;
  else if ( !strcmp ( val, "pad" ) ) *dest= MD_IODEV_PAD;
  else if ( !strcmp ( val, "pad6b" ) ) *dest= MD_IODEV_PAD6B;
  else warning ( "'%s' no és un dispositiu conegut,"
        	 " '%s' tindrà el seu valor per defecte",
        	 val, name );
  g_free ( val );
  
} /* end get_dev_value */


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
  get_key_value ( f, "key-pad1-up", &(conf->keys[0].up) );
  get_key_value ( f, "key-pad1-down", &(conf->keys[0].down) );
  get_key_value ( f, "key-pad1-left", &(conf->keys[0].left) );
  get_key_value ( f, "key-pad1-right", &(conf->keys[0].right) );
  get_key_value ( f, "key-pad1-button-A", &(conf->keys[0].button_A) );
  get_key_value ( f, "key-pad1-button-B", &(conf->keys[0].button_B) );
  get_key_value ( f, "key-pad1-button-C", &(conf->keys[0].button_C) );
  get_key_value ( f, "key-pad1-button-X", &(conf->keys[0].button_X) );
  get_key_value ( f, "key-pad1-button-Y", &(conf->keys[0].button_Y) );
  get_key_value ( f, "key-pad1-button-Z", &(conf->keys[0].button_Z) );
  get_key_value ( f, "key-pad1-start", &(conf->keys[0].start) );
  
  get_key_value ( f, "key-pad2-up", &(conf->keys[1].up) );
  get_key_value ( f, "key-pad2-down", &(conf->keys[1].down) );
  get_key_value ( f, "key-pad2-left", &(conf->keys[1].left) );
  get_key_value ( f, "key-pad2-right", &(conf->keys[1].right) );
  get_key_value ( f, "key-pad2-button-A", &(conf->keys[1].button_A) );
  get_key_value ( f, "key-pad2-button-B", &(conf->keys[1].button_B) );
  get_key_value ( f, "key-pad2-button-C", &(conf->keys[1].button_C) );
  get_key_value ( f, "key-pad2-button-X", &(conf->keys[1].button_X) );
  get_key_value ( f, "key-pad2-button-Y", &(conf->keys[1].button_Y) );
  get_key_value ( f, "key-pad2-button-Z", &(conf->keys[1].button_Z) );
  get_key_value ( f, "key-pad2-start", &(conf->keys[1].start) );
  
  val= g_key_file_get_string ( f, GROUP, "screen-size", NULL );
  if ( val != NULL )
    {
      if ( !strcmp ( val, "win_x1" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X1;
      else if ( !strcmp ( val, "win_x2" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X2;
      else if ( !strcmp ( val, "win_x3" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X3;
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
  get_dev_value ( f, "dev1", &(conf->devs.dev1) );
  get_dev_value ( f, "dev2", &(conf->devs.dev2) );
  get_dev_value ( f, "dev-exp", &(conf->devs.dev_exp) );
  
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
set_dev_value (
               GKeyFile          *f,
               const gchar       *name,
               const MD_IODevice  val
               )
{
  
  const char *aux;
  

  aux= NULL;
  switch ( val )
    {
    case MD_IODEV_PAD: aux= "pad"; break;
    case MD_IODEV_PAD6B: aux= "pad6b"; break;
    case MD_IODEV_NONE: aux= "none"; break;
    default: break;
    }
  assert ( aux != NULL );
  g_key_file_set_string ( f, GROUP, name, aux );
  
} /* end set_dev_value */


void
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
  set_key_value ( f, "key-pad1-up", conf->keys[0].up );
  set_key_value ( f, "key-pad1-down", conf->keys[0].down );
  set_key_value ( f, "key-pad1-left", conf->keys[0].left );
  set_key_value ( f, "key-pad1-right", conf->keys[0].right );
  set_key_value ( f, "key-pad1-button-A", conf->keys[0].button_A );
  set_key_value ( f, "key-pad1-button-B", conf->keys[0].button_B );
  set_key_value ( f, "key-pad1-button-C", conf->keys[0].button_C );
  set_key_value ( f, "key-pad1-button-X", conf->keys[0].button_X );
  set_key_value ( f, "key-pad1-button-Y", conf->keys[0].button_Y );
  set_key_value ( f, "key-pad1-button-Z", conf->keys[0].button_Z );
  set_key_value ( f, "key-pad1-start", conf->keys[0].start );
  
  set_key_value ( f, "key-pad2-up", conf->keys[1].up );
  set_key_value ( f, "key-pad2-down", conf->keys[1].down );
  set_key_value ( f, "key-pad2-left", conf->keys[1].left );
  set_key_value ( f, "key-pad2-right", conf->keys[1].right );
  set_key_value ( f, "key-pad2-button-A", conf->keys[1].button_A );
  set_key_value ( f, "key-pad2-button-B", conf->keys[1].button_B );
  set_key_value ( f, "key-pad2-button-C", conf->keys[1].button_C );
  set_key_value ( f, "key-pad2-button-X", conf->keys[1].button_X );
  set_key_value ( f, "key-pad2-button-Y", conf->keys[1].button_Y );
  set_key_value ( f, "key-pad2-button-Z", conf->keys[1].button_Z );
  set_key_value ( f, "key-pad2-start", conf->keys[1].start );
  
  /* Grandària pantalla. */
  switch ( conf->screen_size )
    {
    case SCREEN_SIZE_WIN_X1: val= "win_x1"; break;
    case SCREEN_SIZE_WIN_X2: val= "win_x2"; break;
    case SCREEN_SIZE_WIN_X3: val= "win_x3"; break;
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

  /* Devs. */
  set_dev_value ( f, "dev1", conf->devs.dev1 );
  set_dev_value ( f, "dev2", conf->devs.dev2 );
  set_dev_value ( f, "dev-exp", conf->devs.dev_exp );
  
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
