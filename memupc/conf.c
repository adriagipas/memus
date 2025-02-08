/*
 * Copyright 2014-2025 Adrià Giménez Pastor.
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
  
  conf->screen_size= SCREEN_SIZE_WIN_640_480;
  conf->vsync= FALSE;
  conf->bios_fn= NULL;
  conf->vgabios_fn= NULL;
  conf->hdd_fn= NULL;
  
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
      if ( !strcmp ( val, "win_640x480" ) )
        conf->screen_size= SCREEN_SIZE_WIN_640_480;
      else if ( !strcmp ( val, "win_800x600" ) )
        conf->screen_size= SCREEN_SIZE_WIN_800_600;
      else if ( !strcmp ( val, "win_960x720" ) )
        conf->screen_size= SCREEN_SIZE_WIN_960_720;
      else if ( !strcmp ( val, "fullscreen" ) )
        conf->screen_size= SCREEN_SIZE_FULLSCREEN;
      else warning ( "grandària de finestra no suportada: %s", val );
      g_free ( val );
    }
  conf->vsync= g_key_file_get_boolean ( f, GROUP, "vsync", &err );
  if ( conf->bios_fn != NULL ) g_free ( conf->bios_fn );
  conf->bios_fn= g_key_file_get_string ( f, GROUP, "bios", NULL );
  if ( conf->vgabios_fn != NULL ) g_free ( conf->vgabios_fn );
  conf->vgabios_fn= g_key_file_get_string ( f, GROUP, "vgabios", NULL );
  if ( conf->hdd_fn != NULL ) g_free ( conf->hdd_fn );
  conf->hdd_fn= g_key_file_get_string ( f, GROUP, "hdd", NULL );
  
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
    case SCREEN_SIZE_WIN_640_480: val= "win_640x480"; break;
    case SCREEN_SIZE_WIN_800_600: val= "win_800x600"; break;
    case SCREEN_SIZE_WIN_960_720: val= "win_960x720"; break;
    case SCREEN_SIZE_FULLSCREEN: val= "fullscreen"; break;
    default: val= NULL;
    }
  assert ( val != NULL );
  if ( val != NULL ) g_key_file_set_string ( f, GROUP, "screen-size", val );
  
  // Vsync.
  g_key_file_set_boolean ( f, GROUP, "vsync", conf->vsync );

  // Bios.
  if ( conf->bios_fn != NULL )
    g_key_file_set_string ( f, GROUP, "bios", conf->bios_fn );

  // VGA Bios.
  if ( conf->vgabios_fn != NULL )
    g_key_file_set_string ( f, GROUP, "vgabios", conf->vgabios_fn );

  // HDD.
  if ( conf->hdd_fn != NULL )
    g_key_file_set_string ( f, GROUP, "hdd", conf->hdd_fn );
  
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
  
  if ( conf->bios_fn != NULL ) g_free ( conf->bios_fn );
  if ( conf->vgabios_fn != NULL ) g_free ( conf->vgabios_fn );
  if ( conf->hdd_fn != NULL ) g_free ( conf->hdd_fn );
  
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


void
conf_set_bios_fn (
                  conf_t      *conf,
                  const gchar *bios_fn
                  )
{
  
  if ( conf->bios_fn != NULL ) g_free ( conf->bios_fn );
  conf->bios_fn= bios_fn==NULL ? NULL : g_strdup ( bios_fn );
  
} // end conf_set_bios_fn


void
conf_set_vgabios_fn (
                     conf_t      *conf,
                     const gchar *vgabios_fn
                     )
{
  
  if ( conf->vgabios_fn != NULL ) g_free ( conf->vgabios_fn );
  conf->vgabios_fn= vgabios_fn==NULL ? NULL : g_strdup ( vgabios_fn );
  
} // end conf_set_vgabios_fn


void
conf_set_hdd_fn (
                 conf_t      *conf,
                 const gchar *hdd_fn
                 )
{
  
  if ( conf->hdd_fn != NULL ) g_free ( conf->hdd_fn );
  conf->hdd_fn= hdd_fn==NULL ? NULL : g_strdup ( hdd_fn );
  
} // end conf_set_hdd_fn
