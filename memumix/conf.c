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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
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
get_default_conf_file_name (void)
{
  return g_build_filename ( get_confdir (), "default.cfg", NULL );
} // end get_default_conf_file_name


static void
set_default_values (
        	    conf_t *conf
        	    )
{
    
  conf->screen_size= SCREEN_SIZE_WIN_X1;
  conf->vsync= TRUE;
  
} // end set_default_values


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
  
  // Obri.
  f= g_key_file_new ();
  err= NULL;
  ret= g_key_file_load_from_file ( f, name, G_KEY_FILE_NONE, &err );
  if ( !ret )
    error ( "no s'ha pogut obrir el fitxer de configuració '%s': %s",
            name, err->message );
  
  // Fixa valors.  
  val= g_key_file_get_string ( f, GROUP, "screen-size", NULL );
  if ( val != NULL )
    {
      if ( !strcmp ( val, "win_x1" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X1;
      else if ( !strcmp ( val, "win_x1.5" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X1_5;
      else if ( !strcmp ( val, "win_x2" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X2;
      else if ( !strcmp ( val, "fullscreen" ) )
        conf->screen_size= SCREEN_SIZE_FULLSCREEN;
      else warning ( "grandària de finestra no suportada: %s", val );
      g_free ( val );
    }
  conf->vsync= g_key_file_get_boolean ( f, GROUP, "vsync", &err );
  
  // Allibera memòria.
  g_key_file_free ( f );
  
} // end read_conf


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
  
  
  // Inicialitza.
  if ( verbose )
    fprintf ( stderr, "Escrivint configuració en '%s'\n", name );
  f= g_key_file_new ();
  
  // Grandària pantalla.
  switch ( conf->screen_size )
    {
    case SCREEN_SIZE_WIN_X1: val= "win_x1"; break;
    case SCREEN_SIZE_WIN_X1_5: val= "win_x1.5"; break;
    case SCREEN_SIZE_WIN_X2: val= "win_x2"; break;
    case SCREEN_SIZE_FULLSCREEN: val= "fullscreen"; break;
    default: val= NULL;
    }
  assert ( val != NULL );
  if ( val != NULL ) g_key_file_set_string ( f, GROUP, "screen-size", val );
  
  // Vsync.
  g_key_file_set_boolean ( f, GROUP, "vsync", conf->vsync );
  
  // Escriu.
  err= NULL;
  ret= g_key_file_save_to_file ( f, name, &err );
  if ( !ret )
    error ( "no s'ha pogut escriure la configuració en '%s': %s",
            name, err->message );
  
  // Allibera.
  g_key_file_free ( f );
  
} // end write_conf_common




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
free_conf (
           conf_t *conf
           )
{
} // end free_conf


void
get_conf (
          conf_t     *conf,
          const char *conf_fn, // Pot ser NULL.
          const int   verbose
          )
{
  
  const gchar *name;
  
  
  set_default_values ( conf );
  name= conf_fn==NULL ? get_default_conf_file_name () : conf_fn;
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) ) // Existeix
    read_conf ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} // end get_conf


void
write_conf (
            const conf_t *conf,
            const char   *conf_fn,
            const int     verbose
            )
{
  
  const gchar *name;
  
  
  name= conf_fn==NULL ? get_default_conf_file_name () : conf_fn;
  write_conf_common ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} // end write_conf
